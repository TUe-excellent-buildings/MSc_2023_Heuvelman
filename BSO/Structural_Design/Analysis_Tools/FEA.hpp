#ifndef FEA_HPP
#define FEA_HPP

#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>
#include <BSO/Structural_Design/Elements/Node_Ele.hpp>
#include <BSO/Structural_Design/Elements/Truss_Ele.hpp>
#include <BSO/Structural_Design/Elements/Beam_Ele.hpp>
#include <BSO/Structural_Design/Elements/Flat_Shell_Ele.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/Eigenvalues>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>
#include <algorithm>

namespace BSO { namespace Structural_Design {

    // typedefs:
    typedef std::map<unsigned long, Elements::Node*>::iterator node_iterator;
    typedef Eigen::Triplet<double> Triplet;

    class FEA
    {
    private:
        friend class SD_Analysis;
        friend void topopt_SIMP(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol);
        friend void topopt_SIMP_diff_elements(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol);
        friend void topopt_SIMP_old(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol);
        friend void topopt_SIMP_old2(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol);
        friend void topopt_robust(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol);

        std::map<unsigned long, Elements::Node*> m_node_map;

        std::vector<Elements::Element*> m_elements;

        unsigned long m_dof_count;
        std::vector<unsigned int> m_load_cases;
        std::map<unsigned int, Eigen::VectorXd> m_all_loads;
        std::map<unsigned int, Eigen::VectorXd> m_all_displacements;

        Eigen::SparseMatrix<double> m_sp_GSM; // this is the sparse global stiffness matrix

        // private functions that are used to initialise this class
        void add_node(unsigned long ID, double x, double y, double z); // adds a node to the node map, but checks for duplicate ID's first
		void add_node(Components::Point* point); // adds a point and the loads and constraints acting on it
        void add_node(Components::Point* point, bool activate_dofs); // adds a point and the loads and constraints acting on it
        void add_elements(Components::Component* component); // adds all elements in a component
        Elements::Node* get_node(unsigned long ID);
        void generate_system(); // generates freedom tables etc.
        void generate_GSM();
		std::map<Elements::Node*, std::vector<unsigned int> > get_nodes_with_free_dofs(double x);
        void clear_system(); // clears the system, GSM, loads, nodes, elements constraints etc

        FEA();
    public:
        FEA(std::string file_name);
        ~FEA();

        void solve();
        unsigned int get_element_count();
        Elements::Element* get_element_ptr(unsigned int);

        void write_ansys_input(std::string file_name, unsigned int lc);
        void write_old_FEM_input(std::string file_name);
        void write_results(std::string file_name);

        std::map<std::pair<Elements::Node*, unsigned int>, double> get_nodes_singular_values();
		Eigen::Vector6d get_node_displacements(Components::Point*);

    }; // FEA

    FEA::FEA()
    { // private ctor, can only be used by friend classes/functions

    } // ctor

    FEA::FEA(std::string file_name)
    {
        std::ifstream input(file_name.c_str()); // initialize input stream from file: file_name

        if (!input.is_open())
        {
            std::cerr << "Error, could not find file: " << file_name << ". Exiting now..." << std::endl;
            exit(1);
        }

        std::string line;
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
        std::string type_ID; // holds information about what type of information is described by the line currently read

        while (!input.eof())
        {
            getline(input,line); // get next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }

            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            type_ID = *token; // interpret first token as type ID
            boost::algorithm::trim(type_ID);

            if (type_ID == "N")
            {   // this reads a node
                token++; // next token is ID
                unsigned long ID = BSO::trim_and_cast_ulong(*token);
                token++; // next token is x-coordinate
                int x = BSO::trim_and_cast_double(*token);
                token++; // next token is y-coordinate
                int y = BSO::trim_and_cast_double(*token);
                token++; // next token is z-coordinate
                int z = BSO::trim_and_cast_double(*token);

                add_node(ID,x,y,z);
            }

            else if (type_ID == "T")
            {   // this reads a truss element
                token++; // next token is n1, first node_ID
                unsigned long n1 = BSO::trim_and_cast_ulong(*token);
                token++; // next token is n2, first node_ID
                unsigned long n2 = BSO::trim_and_cast_ulong(*token);

                double A = 1000;
                double E = 200000;

                Elements::Node* n_ptr_1 = get_node(n1);
                Elements::Node* n_ptr_2 = get_node(n2);

                m_elements.push_back(new Elements::Truss(A, E, n_ptr_1, n_ptr_2));
            }

            else if (type_ID == "B")
            {   // this reads a beam element
                token++; // next token is n1, first node_ID
                unsigned long n1 = BSO::trim_and_cast_int(*token);
                token++; // next token is n2, first node_ID
                unsigned long n2 = BSO::trim_and_cast_int(*token);

                double b = 300;
                double h = 300;
                double E = 30000;
                double v = 0.3;

                Elements::Node* n_ptr_1 = get_node(n1);
                Elements::Node* n_ptr_2 = get_node(n2);

                m_elements.push_back(new Elements::Beam(b, h, E, v, n_ptr_1, n_ptr_2));
            }

            else if (type_ID == "A")
            {   // this reads a shell element
                token++; // next token is the first node of the element
                unsigned long n1 = BSO::trim_and_cast_int(*token);
                token++; // second node
                unsigned long n2 = BSO::trim_and_cast_int(*token);
                token++; // third node
                unsigned long n3 = BSO::trim_and_cast_int(*token);
                token++; // fourth node
                unsigned long n4 = BSO::trim_and_cast_int(*token);

                double t = 150;
                double E = 30000;
                double v = 0.3;

                Elements::Node* n_ptr_1 = get_node(n1);
                Elements::Node* n_ptr_2 = get_node(n2);
                Elements::Node* n_ptr_3 = get_node(n3);
                Elements::Node* n_ptr_4 = get_node(n4);

                m_elements.push_back(new Elements::Flat_Shell(t, E, v, n_ptr_1, n_ptr_2, n_ptr_3, n_ptr_4));
            }

            else if (type_ID == "F")
            {   // this reads a load
                token++; // next token is the node_ID on which the load acts
                unsigned long fnID = BSO::trim_and_cast_ulong(*token);
                token++; // next token is the direction in which the load acts
                std::string direction = *token;
                boost::algorithm::trim(direction);
                token++; // next token is the magnitude of the load
                double load = BSO::trim_and_cast_double(*token);
                token++; // next token is the ID of the load combination to which the load belongs
                unsigned int lc = BSO::trim_and_cast_uint(*token);

                if (std::find(m_load_cases.begin(), m_load_cases.end(), lc) == m_load_cases.end())
                { // the load case does not yet exist
                    m_load_cases.push_back(lc);
                    for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
                    {
                        ite->second->add_load_case(lc);
                    }
                }

                Elements::Node* temp_ptr = get_node(fnID);
                temp_ptr->add_load(lc, direction, load); // add the load to the node
            }

            else if (type_ID == "D")
            {   // this reads constraint
                token++; // next token is the node_ID on which the constraint acts
                int cnID = BSO::trim_and_cast_int(*token);
                token++; // next token is the degree of freedom in which the constraint prevents displacement
                std::string dof = *token;
                boost::algorithm::trim(dof);

                Elements::Node* temp_ptr = get_node(cnID);

                temp_ptr->add_constraint(dof);
            }




            else {continue;}
        } // end if

        input.close(); // close the input file
        generate_system(); // generate the global stiffness matrix and global load vector
    } // ctor

    FEA::~FEA()
    {
        clear_system();
    } // dtor

    void FEA::clear_system()
    {
        // delete the nodes in the system
        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            delete ite->second;
        }
        m_node_map.clear();

        // delete the elements in the system
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            delete m_elements[i];
        }
        m_elements.clear();

        m_dof_count = 0;
        m_load_cases.clear();
        m_all_loads.clear();
        m_all_displacements.clear();

        m_sp_GSM.resize(0, 0);
    }

    void FEA::add_node(unsigned long ID, double x, double y, double z)
    {
        if (m_node_map.find(ID) == m_node_map.end())
        {
            m_node_map[ID] = new Elements::Node(ID, x, y, z);
        }
        else
        {
            std::cerr << "Error found duplicate node ID (FEA.hpp), exiting now..." << std::endl;
            exit(1);
        }
    } // add_node(ID, x, y, z)

	void FEA::add_node(Components::Point* point)
    {
		add_node(point,false);
	}

    void FEA::add_node(Components::Point* point, bool activate_dofs)
    {
        // add the node
        unsigned long ID = point->get_ID();

        Eigen::Vector3d coords = point->get_coords();

        add_node(ID,coords(0),coords(1),coords(2));

        // retrieve the pointer to the newly created node
        Elements::Node* node_ptr = get_node(ID);
		if (activate_dofs)
		{
			Eigen::Vector6i init_NFS;
			init_NFS << 1,1,1,0,0,0;
			node_ptr->update_NFS(init_NFS);
		}

        // add all known load cases to the node
        for (unsigned int i = 0; i < m_load_cases.size(); i++)
        {
            node_ptr->add_load_case(m_load_cases[i]);
        }

        // add the loads that act on the point to the node
        std::map<unsigned int, Eigen::Vector6d> loads = point->get_loads();
        typedef std::map<unsigned int, Eigen::Vector6d>::iterator lc_iterator;

        unsigned int lc;

        for (lc_iterator ite = loads.begin(); ite != loads.end(); ite++)
        { // for each load case acting on the point
            lc = ite->first;

            if (std::find(m_load_cases.begin(), m_load_cases.end(), lc) == m_load_cases.end())
            { // the load case does not yet exist
                m_load_cases.push_back(lc);
                for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
                { // for each node that is already part of this FEA instance
                    ite->second->add_load_case(lc);
                }
            }

            for (unsigned int j = 0 ; j < 6; j++)
            { // and for each direction

                if (ite->second(j) != 0)
                {
                    node_ptr->add_load(lc, j, ite->second(j));
                }
            }

        }

        // add the constraints that act on the point to the node
        std::vector<bool> constraints = point->get_constraints();

        for (unsigned int i = 0;  i < 6; i++)
        { // for each dof
            if (constraints[i])
            {
                node_ptr->add_constraint(i);
            }
        }

    } // add_node(Point*)

    void FEA::add_elements(Components::Component* component)
    {
        std::vector<unsigned long> node_IDs;
        bool is_ghost = component->is_ghost_component();
        bool is_transparant = component->get_vis_transparancy();
        if (component->is_truss())
        { // if the component is a truss
            double A;
            double E;

            for (unsigned int i = 0; i < component->get_element_count(); i++)
            {
                A = component->get_property(0);
                E = component->get_property(1);

                node_IDs = component->get_node_IDs(i);

                Elements::Node* n_ptr_1 = get_node(node_IDs[0]);
                Elements::Node* n_ptr_2 = get_node(node_IDs[1]);

                m_elements.push_back(new Elements::Truss(A, E, n_ptr_1, n_ptr_2));
                component->set_element_ptr(m_elements.back());
                m_elements.back()->set_activity_in_compliance(!is_ghost);
                m_elements.back()->set_vis_transparancy(is_transparant);
            }

        }
        else if (component->is_beam())
        { // if the component is a beam
            double b;
            double h;
            double E;
            double v;

            for (unsigned int i = 0; i < component->get_element_count(); i++)
            {
                b = component->get_property(0);
                h = component->get_property(1);
                E = component->get_property(2);
                v = component->get_property(3);

                node_IDs = component->get_node_IDs(i);

                Elements::Node* n_ptr_1 = get_node(node_IDs[0]);
                Elements::Node* n_ptr_2 = get_node(node_IDs[1]);

                m_elements.push_back(new Elements::Beam(b, h, E, v, n_ptr_1, n_ptr_2));
                component->set_element_ptr(m_elements.back());
                m_elements.back()->set_activity_in_compliance(!is_ghost);
                m_elements.back()->set_vis_transparancy(is_transparant);
            }
        }
        else if (component->is_flat_shell())
        { //  if the component is a flat shell
            double t;
            double E;
            double v;

            for (unsigned int i = 0; i < component->get_element_count(); i++)
            {
                t = component->get_property(0);
                E = component->get_property(1);
                v = component->get_property(2);

                node_IDs = component->get_node_IDs(i);

                Elements::Node* n_ptr_1 = get_node(node_IDs[0]);
                Elements::Node* n_ptr_2 = get_node(node_IDs[1]);
                Elements::Node* n_ptr_3 = get_node(node_IDs[2]);
                Elements::Node* n_ptr_4 = get_node(node_IDs[3]);

                m_elements.push_back(new Elements::Flat_Shell(t, E, v, n_ptr_1, n_ptr_2, n_ptr_3, n_ptr_4));
                component->set_element_ptr(m_elements.back());
                m_elements.back()->set_activity_in_compliance(!is_ghost);
                m_elements.back()->set_vis_transparancy(is_transparant);
            }
        }
        node_IDs.clear();
    } // add_elements()

    Elements::Node* FEA::get_node(unsigned long ID)
    {
        if (m_node_map.find(ID) == m_node_map.end())
        {
            std::cerr << "Could not find node with ID: " << ID << ", exiting now..." << std::endl;
            exit(1);
        }
        else
        {
            return m_node_map[ID];
        }
    } // get_node()

    void FEA::generate_system()
    {
        // generate freedom tables in each node instance
        unsigned long dof_count = 0;
        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            ite->second->set_NFT(dof_count); // set the freedom count to dof_count and generate the node freedom table
            dof_count += ite->second->get_freedom_count(); // get the number of dofs in the freedom signature of this node
        }
        m_dof_count = dof_count;

        // map the loads to m_all_loads
        for (unsigned int i = 0; i < m_load_cases.size(); i++)
        { // for all load cases
            double load = 0; // assigned to zero to avoid compiler warning about use of uninitialized variable
            m_all_loads[m_load_cases[i] ] = Eigen::VectorXd::Zero(m_dof_count);

            for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
            { // and all nodes
                for (int j = 0; j < 6; j++)
                { // in each possible node dof
                    if (ite->second->check_load(m_load_cases[i], j, load) && ite->second->get_constraints()[j] == 0)
                    { // if there is a load acting in the direction of that dof and it is not constrained, pass it through to "load"
                        m_all_loads[m_load_cases[i] ](ite->second->get_dof(j)) += load;
                    }
                }
            }
        }

        // initialise the displacement vectors to m_all_displacements
        for (unsigned int i = 0; i < m_load_cases.size(); i++)
        { // for all load cases
            Eigen::VectorXd temp_disp_vector(m_dof_count);
            temp_disp_vector.setZero();
            m_all_displacements[m_load_cases[i] ] = temp_disp_vector;
        }

        // generate freedom table in each element instance
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            m_elements[i]->generate_EFT();
        }

        generate_GSM();

    } // generate_system()

    void FEA::generate_GSM()
    {
        // initialise the sparse global stiffness matrix:
        m_sp_GSM.resize(0,0); // clears any contents that may have been in the sparse matrix
        m_sp_GSM.resize(m_dof_count, m_dof_count); // sets the size of the stiffness matrix to the number of dof's

        // initialise triplet list, i.e. one triplet is one entry into the sparse matrix (triplets with identical indices are summed)
        std::vector<Triplet> triplet_list, temp_element_list;

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            temp_element_list = m_elements[i]->get_SM_triplets();
            triplet_list.insert(triplet_list.end(), temp_element_list.begin(), temp_element_list.end());
            temp_element_list.clear();
        }

        // assemble the sparse global stiffness matrix from the triplets
        m_sp_GSM.setFromTriplets(triplet_list.begin(), triplet_list.end());
        triplet_list.clear();
    } // generate_GSM()

	std::map<Elements::Node*, std::vector<unsigned int> > FEA::get_nodes_with_free_dofs(double x)
	{
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(m_sp_GSM,Eigen::ComputeFullV);

		auto S = svd.singularValues();
		auto V = svd.matrixV();

		std::vector<int> free_dofs;

		for (int n = 0; n < V.cols(); n++)
		{
			if ((n < S.rows() && S(n) < x) || n > S.rows())
			{
				for (int m = 0; m < V.rows(); m++)
				{
					if (V(m,n) > 0.0001)
					{
						free_dofs.push_back(m);
					}
				}
			}
		}

		std::map<Elements::Node*, std::vector<unsigned int> > nodes_with_free_dofs;

		for (auto i : m_node_map)
		{
			for (int j = 0; j < 6; j++)
			{
				try
				{
					if (std::find(free_dofs.begin(), free_dofs.end(), i.second->get_dof(j)) != free_dofs.end())
					{
						if (nodes_with_free_dofs.count(i.second) > 0)
						{
							nodes_with_free_dofs[i.second].push_back(j);
						}
						else
						{
						    std::vector<unsigned int> temp_vec;
							nodes_with_free_dofs[i.second] = temp_vec;
							nodes_with_free_dofs[i.second].push_back(j);
						}

					}
				}
				catch (std::invalid_argument)
				{ // the jth dof does not exist in this node
					// do nothing...
				}
			}
		}

		return nodes_with_free_dofs;
	}

    void FEA::solve()
    {
        // SparseLLT decomposition
        Eigen::SimplicialLLT<Eigen::SparseMatrix<double> > solver;
        solver.compute(m_sp_GSM);

        if(solver.info() != Eigen::Success)
        {
            std::cerr << "Solver failed GSM decomposition, exiting now..." << std::endl;
            exit(1);
        }

        for (unsigned int i = 0; i < m_load_cases.size(); i++)
        { // for each load case
            m_all_displacements[m_load_cases[i] ] = solver.solve(m_all_loads[m_load_cases[i] ]);

            if(solver.info() != Eigen::Success)
            {
                std::cerr << "Solver failed GSM solving, exiting now..." << std::endl;
                exit(1);
            }
        }

        // BiCGSTAB with scaling
        //Eigen::BiCGSTAB<Eigen::SparseMatrix<double>/*, Eigen::DiagonalPreconditioner<double> */> solver;
        /*for (unsigned int i = 0; i < m_load_cases.size(); i++)
        { // for each load case
            // pre-compute some solver values
            solver.compute(m_sp_GSM);

            if(solver.info() != Eigen::Success)
            {
                std::cerr << "Solver failed GSM decomposition, exiting now..." << std::endl;
                exit(1);
            }

            // get a rough solution so the GSm can be scaled as such that it contains values with equal magnitudes
            solver.setMaxIterations(3);
            m_all_displacements[m_load_cases[i] ] = solver.solve(m_all_loads[m_load_cases[i] ]);

            // scaling of the GSM to a temporary GSM matrix 'C'
            Eigen::VectorXd w_inv = (m_all_displacements[m_load_cases[i] ].array().abs()+1e-6).inverse();
            Eigen::SparseMatrix<double> C;
            C.resize(m_dof_count, m_dof_count);
            C = m_sp_GSM * w_inv.asDiagonal();

            // solving the scaled system to a temporary displacement vector 'y'
            solver.setMaxIterations(m_dof_count*10);
            solver.compute(C);

            if(solver.info() != Eigen::Success)
            {
                std::cerr << "Solver failed scaled GSM decomposition, exiting now..." << std::endl;
                exit(1);
            }

            Eigen::VectorXd y(m_dof_count);
            y = solver.solve(m_all_loads[m_load_cases[i] ]);

            if(solver.info() != Eigen::Success)
            {
                std::cerr << std::endl << std::endl << "WARNING: BiCGSTAB probably did NOT converge successfully!" << std::endl << std::endl;
            }

            // scaling back to the multi scale displacement vector
            m_all_displacements[m_load_cases[i] ] = w_inv.asDiagonal() * y;

        }*/

        // BiCGSTAB without scaling
        //Eigen::BiCGSTAB<Eigen::SparseMatrix<double>/*, Eigen::DiagonalPreconditioner<double> */> solver;
        /*for (unsigned int i = 0; i < m_load_cases.size(); i++)
        { // for each load case
            // pre-compute some solver values
            solver.setTolerance(0.001);
            solver.compute(m_sp_GSM);

            if(solver.info() != Eigen::Success)
            {
                std::cerr << "Solver failed GSM decomposition, exiting now..." << std::endl;
                exit(1);
            }

            // solve the GSM
            m_all_displacements[m_load_cases[i] ] = solver.solve(m_all_loads[m_load_cases[i] ]);

        }*/

        // add displacements to the instances of the node class
        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            ite->second->add_displacements(m_all_displacements);
        }

        // calculate the strain energy in each element
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            m_elements[i]->calc_energies(m_load_cases);
        }

    } // solve

    unsigned int FEA::get_element_count()
    {
        return m_elements.size();
    } // get_element_count()

    Elements::Element* FEA::get_element_ptr(unsigned int n)
    {
        return m_elements[n];
    } // get_element_ptr()

    void FEA::write_ansys_input(std::string file_name, unsigned int lc)
    {
        std::ofstream output(file_name.c_str());
        output.flags (std::ios::scientific);
        output.precision (std::numeric_limits<double>::digits10 + 1);

        if (!output.is_open())
        {
            std::cerr << "Could not open output stream to Ansys input file: " << file_name << ", exiting now..." << std::endl;
            exit(1);
        }

        std::cerr << "WARNING in writing Ansys input file, all element types get the properties of the first mapped element of that type" << std::endl;

        output << "FINISH" << std::endl;
        output << "/CLEAR" << std::endl;
        output << "/PREP7" << std::endl << std::endl << std::endl;
        output << "!!!!NODES!!!!" << std::endl << std::endl;

        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            output << "N," << ite->second->get_ID()+1
                   << "," << ite->second->get_coord()(0)
                   << "," << ite->second->get_coord()(1)
                   << "," << ite->second->get_coord()(2) << std::endl;
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_truss())
            {
                output << std::endl << "!!!!TRUSS ELEMENTS!!!!" << std::endl << std::endl;
                output << "ET,1,LINK8" << std::endl;
                output << "R,1," << m_elements[i]->get_property(1) << std::endl; // set truss area
                output << "MP,EX,1," << m_elements[i]->get_property(2) << std::endl; // set youngs modulus
                output << "TYPE,1" << std::endl;
                output << "REAL,1" << std::endl;
                output << "MAT,1" << std::endl << std::endl;
                break;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_truss())
            {
                output << "E";
                for (int j = 0; j < 2; j++)
                {
                    output << "," << m_elements[i]->get_node_ID(j)+1;
                }
                output << std::endl;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_beam())
            {
                output << std::endl << "!!!!BEAM ELEMENTS!!!!" << std::endl << std::endl;
                output << "ET,2,BEAM4" << std::endl;
                output << "R,2," << m_elements[i]->get_property(3) << ","
                       << m_elements[i]->get_property(6) << ","
                       << m_elements[i]->get_property(7) << ","
                       << m_elements[i]->get_property(1) << ","
                       << m_elements[i]->get_property(2) << std::endl; // set cross_area, Iy, IZ, b, h
                output << "MP,EX,2," << m_elements[i]->get_property(4) << std::endl; // set youngs modulus
                output << "MP,PRXY,2," << m_elements[i]->get_property(5) << std::endl; // set poisson ratio
                output << "TYPE,2" << std::endl;
                output << "REAL,2" << std::endl;
                output << "MAT,2" << std::endl << std::endl;
                break;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_beam())
            {
                output << "E";
                for (int j = 0; j < 2; j++)
                {
                    output << "," << m_elements[i]->get_node_ID(j)+1;
                }
                output << std::endl;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_flat_shell())
            {
                output << std::endl << "!!!!FLAT SHELL ELEMENTS!!!!" << std::endl << std::endl;
                output << "ET,3,SHELL63" << std::endl;
                output << "KEYOPT,3,3,1" << std::endl;
                output << "R,3," << m_elements[i]->get_property(1) <<std::endl; // set thickness
                output << "MP,EX,3," << m_elements[i]->get_property(2) << std::endl; // set youngs modulus
                output << "MP,PRXY,3," << m_elements[i]->get_property(3) << std::endl; // set poisson ratio
                output << "TYPE,3" << std::endl;
                output << "REAL,3" << std::endl;
                output << "MAT,3" << std::endl;
                break;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_flat_shell())
            {
                output << "E";
                for (int j = 0; j < 4; j++)
                {
                    output << "," << m_elements[i]->get_node_ID(j)+1;
                }
                output << std::endl;
            }
        }

        output << std::endl << "!!!!CONSTRAINTS!!!!" << std::endl << std::endl;

        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            if (ite->second->get_constraints()(0) == 1)
                output << "D," << ite->second->get_ID()+1 << ",UX,0" << std::endl;
            if (ite->second->get_constraints()(1) == 1)
                output << "D," << ite->second->get_ID()+1 << ",UY,0" << std::endl;
            if (ite->second->get_constraints()(2) == 1)
                output << "D," << ite->second->get_ID()+1 << ",UZ,0" << std::endl;
            if (ite->second->get_constraints()(3) == 1)
                output << "D," << ite->second->get_ID()+1 << ",ROTX,0" << std::endl;
            if (ite->second->get_constraints()(4) == 1)
                output << "D," << ite->second->get_ID()+1 << ",ROTY,0" << std::endl;
            if (ite->second->get_constraints()(5) == 1)
                output << "D," << ite->second->get_ID()+1 << ",ROTZ,0" << std::endl;
        }

        output << std::endl << "!!!!Loads!!!!" << std::endl;

        output << std::endl << "!!!!Load case " << m_load_cases[lc] << "!!!!" << std::endl << std::endl;
        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            if (ite->second->get_loads(m_load_cases[lc])(0) != 0.0)
                output << "F," << ite->second->get_ID()+1 << ",FX," << ite->second->get_loads(m_load_cases[lc])(0) << std::endl;
            if (ite->second->get_loads(m_load_cases[lc])(1) != 0.0)
                output << "F," << ite->second->get_ID()+1 << ",FY," << ite->second->get_loads(m_load_cases[lc])(1) << std::endl;
            if (ite->second->get_loads(m_load_cases[lc])(2) != 0.0)
                output << "F," << ite->second->get_ID()+1 << ",FZ," << ite->second->get_loads(m_load_cases[lc])(2) << std::endl;
            if (ite->second->get_loads(m_load_cases[lc])(3) != 0.0)
                output << "F," << ite->second->get_ID()+1 << ",MX," << ite->second->get_loads(m_load_cases[lc])(3) << std::endl;
            if (ite->second->get_loads(m_load_cases[lc])(4) != 0.0)
                output << "F," << ite->second->get_ID()+1 << ",MY," << ite->second->get_loads(m_load_cases[lc])(4) << std::endl;
            if (ite->second->get_loads(m_load_cases[lc])(5) != 0.0)
                output << "F," << ite->second->get_ID()+1 << ",MZ," << ite->second->get_loads(m_load_cases[lc])(5) << std::endl;
        }

        output << std::endl << std::endl << "!!!!SOLUTION!!!!" << std::endl << std::endl;
        output << "/SOLU" << std::endl << "SOLVE" << std::endl << std::endl;
        output << "/POST1" << std::endl << "PLDISP,1" << std::endl
               << "PRNSOL,U,COMP" << std::endl << "PRNSOL,ROT,COMP"
               << std::endl << "PRESOL,SENE" << std::endl;
    }

    void FEA::write_old_FEM_input(std::string file_name)
    {
        std::ofstream output(file_name.c_str());
        //output.flags (std::ios::scientific);
        //output.precision (std::numeric_limits<double>::digits10 + 1);

        if (!output.is_open())
        {
            std::cerr << "Could not open output stream to old FEM input file: " << file_name << ", exiting now..." << std::endl;
            exit(1);
        }

        std::cerr << "WARNING in writing old FEM input file, all element types get the properties of the first mapped element of that type" << std::endl;

        output << "!!!!NODES!!!!" << std::endl << std::endl;

        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            output << "N," << ite->second->get_ID()
                   << "," << ite->second->get_coord()(0)
                   << "," << ite->second->get_coord()(1)
                   << "," << ite->second->get_coord()(2) << std::endl;
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_truss())
            {
                output << std::endl << "!!!!TRUSS ELEMENTS!!!!" << std::endl << std::endl;
                output << "AREA," << m_elements[i]->get_property(1) << std::endl; // set truss area
                output << "YMODULUS," << m_elements[i]->get_property(2) << std::endl << std::endl; // set youngs modulus
                break;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_truss())
            {
                output << "T";
                for (int j = 0; j < 2; j++)
                {
                    output << "," << m_elements[i]->get_node_ID(j);
                }
                output << std::endl;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_beam())
            {
                output << std::endl << "!!!!BEAM ELEMENTS!!!!" << std::endl << std::endl;
                output << "AREA," << m_elements[i]->get_property(3) << std::endl;
                output << "YMODULUS," << m_elements[i]->get_property(4) << std::endl; // set youngs modulus
                output << "POISSONR," << m_elements[i]->get_property(5) << std::endl << std::endl; // set poisson ratio
                break;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_beam())
            {
                output << "B";
                for (int j = 0; j < 2; j++)
                {
                    output << "," << m_elements[i]->get_node_ID(j);
                }
                output << std::endl;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_flat_shell())
            {
                output << std::endl << "!!!!FLAT SHELL ELEMENTS!!!!" << std::endl << std::endl;
                output << "THICKNESS," << m_elements[i]->get_property(1) <<std::endl; // set thickness
                output << "YMODULUS," << m_elements[i]->get_property(2) << std::endl; // set youngs modulus
                output << "POISSONR," << m_elements[i]->get_property(3) << std::endl << std::endl; // set poisson ratio

                break;
            }
        }

        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            if (m_elements[i]->is_flat_shell())
            {
                output << "A";
                for (int j = 0; j < 4; j++)
                {
                    output << "," << m_elements[i]->get_node_ID(j);
                }
                output << std::endl;
            }
        }

        output << std::endl << "!!!!CONSTRAINTS!!!!" << std::endl << std::endl;

        for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
        {
            if (ite->second->get_constraints()(0) == 1)
                output << "D," << ite->second->get_ID() << ",UX,0" << std::endl;
            if (ite->second->get_constraints()(1) == 1)
                output << "D," << ite->second->get_ID() << ",UY,0" << std::endl;
            if (ite->second->get_constraints()(2) == 1)
                output << "D," << ite->second->get_ID() << ",UZ,0" << std::endl;
            if (ite->second->get_constraints()(3) == 1)
                output << "D," << ite->second->get_ID() << ",ROTX,0" << std::endl;
            if (ite->second->get_constraints()(4) == 1)
                output << "D," << ite->second->get_ID() << ",ROTY,0" << std::endl;
            if (ite->second->get_constraints()(5) == 1)
                output << "D," << ite->second->get_ID() << ",ROTZ,0" << std::endl;
        }

        output << std::endl << "!!!!Loads!!!!" << std::endl;
        for (unsigned int i = 0; i < m_load_cases.size(); i++)
        {
            //output << std::endl << "!!!!Load case " << m_load_cases[i] << "!!!!" << std::endl << std::endl;
            for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
            {
                if (ite->second->get_loads(m_load_cases[i])(0) != 0.0)
                    output << "F," << ite->second->get_ID() << ",FX," << ite->second->get_loads(m_load_cases[i])(0) << "," << m_load_cases[i] << std::endl;
                if (ite->second->get_loads(m_load_cases[i])(1) != 0.0)
                    output << "F," << ite->second->get_ID() << ",FY," << ite->second->get_loads(m_load_cases[i])(1) << "," << m_load_cases[i] << std::endl;
                if (ite->second->get_loads(m_load_cases[i])(2) != 0.0)
                    output << "F," << ite->second->get_ID() << ",FZ," << ite->second->get_loads(m_load_cases[i])(2) << "," << m_load_cases[i] << std::endl;
                if (ite->second->get_loads(m_load_cases[i])(3) != 0.0)
                    output << "F," << ite->second->get_ID() << ",MX," << ite->second->get_loads(m_load_cases[i])(3) << "," << m_load_cases[i] << std::endl;
                if (ite->second->get_loads(m_load_cases[i])(4) != 0.0)
                    output << "F," << ite->second->get_ID() << ",MY," << ite->second->get_loads(m_load_cases[i])(4) << "," << m_load_cases[i] << std::endl;
                if (ite->second->get_loads(m_load_cases[i])(5) != 0.0)
                    output << "F," << ite->second->get_ID() << ",MZ," << ite->second->get_loads(m_load_cases[i])(5) << "," << m_load_cases[i] << std::endl;
            }
        }
    }

    void FEA::write_results(std::string file_name)
    {
        std::ofstream output(file_name.c_str());
        //output.flags (std::ios::scientific);
        //output.precision (std::numeric_limits<double>::digits10 + 1);

        if (!output.is_open())
        {
            std::cerr << "Could not open output stream to result file: " << file_name << ", exiting now..." << std::endl;
            exit(1);
        }

        output << "!!!Nodal Information!!!" << std::endl;

        for (unsigned int i = 0; i < m_load_cases.size(); i++)
        {
            output << std::endl << std::endl << "Loadcase: " << m_load_cases[i] << std::endl;
            output << "N,ID,x,y,z,ux,uy,uz,rx,ry,rz" << std::endl;
            for (node_iterator ite = m_node_map.begin(); ite != m_node_map.end(); ite++)
            { // for all nodes
                output << "N"
                       << "," << ite->second->get_ID()
                       << "," << ite->second->get_displacements(m_load_cases[i])(0)
                       << "," << ite->second->get_displacements(m_load_cases[i])(1)
                       << "," << ite->second->get_displacements(m_load_cases[i])(2)
                       << "," << ite->second->get_displacements(m_load_cases[i])(3)
                       << "," << ite->second->get_displacements(m_load_cases[i])(4)
                       << "," << ite->second->get_displacements(m_load_cases[i])(5) << std::endl;
            }
        }



        output << std::endl << std::endl << "!!!Element Information!!!" << std::endl << std::endl;
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
           if (m_elements[i]->is_flat_shell())
           {
                output << "A"
                       << "," << m_elements[i]->get_node_ID(0)
                       << "," << m_elements[i]->get_node_ID(1)
                       << "," << m_elements[i]->get_node_ID(2)
                       << "," << m_elements[i]->get_node_ID(3);
                for (unsigned int j = 0; j < m_load_cases.size(); j++)
                {
                    output << "," << m_elements[i]->get_energies()[m_load_cases[j] ];
                }
                output << std::endl;
           }
        }
    }
	/*
    std::map<std::pair<Elements::Node*, unsigned int>, double> FEA::get_nodes_singular_values()
	{
		std::map<std::pair<Elements::Node*, unsigned int>, double> nodes_singular_values;

		for (auto i : m_node_map)
		{
			for (int j = 0; j < 6; j++)
			{
                Eigen::JacobiSVD<Eigen::MatrixXd> svd(m_sp_GSM,Eigen::ComputeFullV);
                auto S = svd.singularValues();
                auto V = svd.matrixV();
                for (int n = 0; n < V.cols(); n++)
                {
                    for (unsigned int m = 0; m < V.rows(); m++)
                    {
                        // m = singular value ID, S(n) is singular value
						if (i.second->get_constraints()[j] != 1)
						{
	                        if (V(m,n) > 0.0001 && m == i.second->get_dof(j))
	                        {
	                            std::pair<Elements::Node*, int> temp_pair;
	                            temp_pair = std::make_pair(i.second, j);
	                            nodes_singular_values[temp_pair] = S(n);
	                        }
						}
                    }
                }
			}
		}
		return nodes_singular_values;
	}
	*/
	std::map<std::pair<Elements::Node*, unsigned int>, double> FEA::get_nodes_singular_values()
	{
		Eigen::JacobiSVD<Eigen::MatrixXd> svd(m_sp_GSM,Eigen::ComputeFullV);

		auto S = svd.singularValues();
		auto V = svd.matrixV();

		std::map<unsigned int, double> dof_singular;

	// m = singular value ID, S(n) is singular value
		for (int n = 0; n < V.cols(); n++)
		{
			for (unsigned int m = 0; m < V.rows(); m++)
			{
				if (V(m,n) > 0.0001)
				{
					dof_singular[m] = S(n);
				}
			}
		}

		std::map<std::pair<Elements::Node*, unsigned int>, double> nodes_singular_values;
		std::map<unsigned int, double>::iterator it; // dof_singular

		for (auto i : m_node_map)
		{
			for (int j = 0; j < 6; j++)
			{
				try
				{
					if (i.second->get_constraints()[j] != 1)
					{
						it = dof_singular.find(i.second->get_dof(j));
						if (it != dof_singular.end())
						{
							std::pair<Elements::Node*, unsigned int> temp_pair;
							temp_pair = std::make_pair(i.second, j);
							nodes_singular_values[temp_pair] = it->second;
						}
					}
				}
				catch (std::invalid_argument)
				{ // the jth dof does not exist in this node
					// do nothing...
				}
			}
		}

		return nodes_singular_values;
	}

	Eigen::Vector6d FEA::get_node_displacements(Components::Point* point)
	{
		Eigen::Vector6d point_displacements;
		for (auto i : m_node_map)
		{
			if (point->get_coords() == i.second->get_coord())
			{
				point_displacements = i.second->get_displacements(1);
				break;
			}
		}
		return point_displacements;
	}

} // namespace Structural_Design
} // namespace BSO



#endif // FEA_HPP
