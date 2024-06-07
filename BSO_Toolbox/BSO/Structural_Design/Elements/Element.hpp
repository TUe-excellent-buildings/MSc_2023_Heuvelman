#ifndef ELEMENT_HPP
#define ELEMENT_HPP

#include <BSO/Structural_Design/Elements/Node_Ele.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <iostream>

namespace Eigen {typedef Matrix<int, 6, 1> Vector6i;}

namespace BSO { namespace Structural_Design {  namespace Elements {

    typedef Eigen::Triplet<double> Triplet;

    struct Element_Node
    {
        Node* m_node_ptr; // pointer to the node
        Eigen::Vector6i m_EFS; // element freedom signature, arrangement:  [ux, uy, uz, rx, ry, rz]
    }; // Element_Node


    class Element
    {
    private:

    protected:
        static unsigned long m_count; // counter of number of instances of this class

        double m_vol; // element volume [m³]
        double m_x; // element density [-]
        double m_Emin; // minimum youngs modulus [N/mm²]
        double m_E0; // initial youngs modulus [N/mm²]
        double m_E; // current youngs modulus [N/mm²]
        double m_total_energy; // energy over all load cases

        Eigen::Vector3d m_center; // center coordinate of the element;

        std::vector<Element_Node> m_nodes; // pointers to all the nodes this element uses
        std::vector<unsigned long> m_EFT; // element freedom table
		std::vector<unsigned int> m_constraints; // the constraints for each element dof

        std::map<unsigned int, Eigen::VectorXd> m_displacements;
        std::map<unsigned int, double> m_energies; // stores the amount of energy in the element for each load case

        Eigen::MatrixXd m_original_SM; // element stiffness matrix at density = 1.0
        Eigen::MatrixXd m_SM; // element stiffness matrix at density = m_x
        bool m_is_truss;
        bool m_is_beam;
        bool m_is_flat_shell;
        bool m_is_hexahedron;
        bool m_visualise; // switch to turn visualisation for this element on or off

        bool m_active_in_compliance; // NEW switch to be turned off for total compliance (if Ghost_Flat_Shell_Comp)
        bool m_visualisation_transparancy; // NEW SB for ghost elements

    public:
        Element();
        virtual ~Element();

        virtual void generate_EFT();
        virtual std::vector<Triplet> get_SM_triplets();
        virtual void calc_energies(const std::vector<unsigned int>& load_cases);
        virtual void get_displacements(const std::vector<unsigned int>& load_cases);
        static unsigned long get_count();

        virtual Eigen::VectorXd get_nodal_element_coord(); // NEW
        virtual Eigen::VectorXd get_nodal_element_forces(unsigned int lc); //NEWSJONNIE

        virtual bool is_truss();
        virtual bool is_beam();
        virtual bool is_flat_shell();
        virtual bool visualise();
        virtual void set_vis_transparancy(bool); // new Sb
        virtual bool get_vis_transparancy();
        virtual bool is_active_in_compliance(); //NEW
        virtual void set_activity_in_compliance(bool); //NEW

        virtual unsigned long get_node_ID(int n);
        virtual double get_property(unsigned int n)=0;
        virtual void update_density_old(double& x, const double& p);
        virtual void update_density(double& x, const double& p);
        virtual void switch_visualisation();
        virtual double get_volume();
        virtual double get_diagonal()
        {
            return BSO::Vectors::length(m_nodes[2].m_node_ptr->get_coord() - m_nodes[0].m_node_ptr->get_coord());
        }

        virtual std::vector<Eigen::Vector3d> get_vis_coords();
        virtual double get_energy();
        virtual double get_normal_energy(); // only for flat shells!
        virtual double get_bending_energy(); // only for flat shells!
        virtual double get_shear_energy(); // only for flat shells!
        virtual double get_axial_energy(); // only for flat shells!
        virtual std::map<unsigned int, double> get_energies();
        virtual double get_energy_sensitivity(const double& p);
        virtual double get_volume_sensitivity();
        virtual double get_density();
        virtual Eigen::Vector3d get_center_coord();

    }; // Element

    // initialisation of static variables
    unsigned long Element::m_count = 0;

    Element::Element()
    {
        m_x = 1.0;
        m_is_truss = false;
        m_is_beam = false;
        m_is_flat_shell = false;
        m_is_hexahedron = false;
        m_visualise = true;
        m_visualisation_transparancy = false;
        m_active_in_compliance = true;
        ++m_count;

    } // ctor

    Element::~Element()
    {
        --m_count;

    } // dtor

    void Element::generate_EFT()
    { // this is generates the EFT in the right order under the assumption that the nodes are stored in the right order (relevant for shell and cuboid elements)
        m_EFT.clear();
		m_constraints.clear();
        for (unsigned int i = 0; i < m_nodes.size(); i++)
        { // do for all nodes
            for (int j = 0; j < 6; j++)
            { // and each possible dof
                if (m_nodes[i].m_EFS(j) == 1)
                { // if the dof is active, get the dof ID from the node
					m_constraints.push_back(m_nodes[i].m_node_ptr->get_constraints()[j]);
					try 
					{
						m_EFT.push_back(m_nodes[i].m_node_ptr->get_dof(j));
					}
                    catch (std::invalid_argument)
					{
						m_EFT.push_back(0);
					}
                }
            }
        }
    } // generate_EFT()

    std::vector<Triplet> Element::get_SM_triplets()
    {

        if (m_EFT.size() != (unsigned int)m_SM.cols())
        {
            std::cerr << "Error in element assembly, EFT size is not equal to SM size, exiting..." << std::endl;
            exit(1);
        }

        std::vector<Triplet> triplet_list;
        for (unsigned int m = 0; m < m_SM.rows(); m++)
        { // for all rows

			for (unsigned int n = 0; n < m_SM.cols(); n++)
            { // and for all columns
                if ((m_SM(m, n) != 0) && m_constraints[m] == 0 && m_constraints[n] == 0)
                { // if the entry is not zero, and the 'm' and 'n'th dof are not constrained
                    triplet_list.push_back(Triplet(m_EFT[m], m_EFT[n], m_SM(m, n)));
                }
            }
        }
        return triplet_list;
    } // get_SM()

    void Element::calc_energies(const std::vector<unsigned int>& load_cases)
    {
        get_displacements(load_cases);
        m_total_energy = 0;
        for (auto lc : load_cases)
        { // for all load cases
            auto temp_disp = m_displacements[lc];

            double energy = 0.5 * temp_disp.transpose() * m_SM * temp_disp;
            m_energies[lc] = energy;
            m_total_energy += energy;
        }
    } // calc_energies()

    Eigen::VectorXd Element::get_nodal_element_coord() // NEW
    {
        Eigen::VectorXd coord(m_nodes.size() * 3);

        for (unsigned int i = 0; i < m_nodes.size(); i++)
        {
            coord.block<3,1>(i*3,0) = m_nodes[i].m_node_ptr->get_coord();
        }

        return coord;
    } // get_nodal_element_coord() NEW

    Eigen::VectorXd Element::get_nodal_element_forces(unsigned int lc) // NEWSJONNIE
    {
        Eigen::VectorXd displacements(m_nodes.size() * 6);

        for (unsigned int i = 0; i < m_nodes.size(); i++)
        {
            displacements.block<6,1>(i*6,0) = m_nodes[i].m_node_ptr->get_displacements(lc);
        }

        for (unsigned int i = 0; i < m_nodes.size(); i++)
        {
            for (unsigned int j = 0; j < 6; j++)
            {
                if (m_nodes[i].m_EFS[j] == 0)
                {
                    std::copy(displacements.data() + i + 1, displacements.data() + displacements.size(), displacements.data() + i);
                    displacements.resize(displacements.size()-1);

                    std::cerr << "Found inconvenient number of dof's (Element.hpp), reconsideration is required..." << std::endl;
                }
            }
        }

        Eigen::VectorXd forces = m_SM * displacements;

        return forces;

    } // get_nodal_element_forces() NEWSJONNIE

    void Element::get_displacements(const std::vector<unsigned int>& load_cases)
    {
        m_displacements.clear();
        for (auto lc : load_cases)
        { // for all load cases
            Eigen::VectorXd temp_disp(m_SM.cols());
            temp_disp.setZero();
            unsigned int count = 0;

            for (unsigned int j = 0; j < m_nodes.size(); j++)
            { // for all nodes of the elements (in the right order)

                for (int k = 0; k < 6; k++)
                { // for all dof's of the node
			
                    if (m_nodes[j].m_EFS[k] == 1)
                    { // if the node's dof is active for this element

                        temp_disp(count) = m_nodes[j].m_node_ptr->get_displacement(lc, k);
                        count++;
                    }
                }
            }
            if (count != m_SM.cols())
            {
                std::cout << "Error in calculating energies: did not find all displacements (Element.hpp), exiting now..." << std::endl;
                exit(1);
            }

            m_displacements[lc] = temp_disp;
        }
    }


    bool Element::is_truss()
    {
        return m_is_truss;
    } // is_truss()

    bool Element::is_beam()
    {
        return m_is_beam;
    } // is_beam()

    bool Element::is_flat_shell()
    {
        return m_is_flat_shell;
    } // is_flat_shell()

    bool Element::visualise()
    {
        return m_visualise;
    } // visualise()

    void Element::set_vis_transparancy(bool val)
    {
        m_visualisation_transparancy = val;
    } // set_vis_transparancy()

    bool Element::get_vis_transparancy()
    {
        return m_visualisation_transparancy;
    } // get_vis_transparancy()

    bool Element::is_active_in_compliance() //NEW
    {
        return m_active_in_compliance;
    } // NEW is_compliance_active()

    void Element::set_activity_in_compliance(bool active) //NEW
    {
        m_active_in_compliance = active;
    } // NEW deactivate_for_compliance()


    unsigned long Element::get_node_ID(int n)
    {
        return m_nodes[n].m_node_ptr->get_ID();
    } // get_node_ID()


    std::vector<Eigen::Vector3d> Element::get_vis_coords()
    {
        std::vector<Eigen::Vector3d> temp;

        if(this->is_flat_shell())
        {
            for (int i = 0; i < 4; i++)
            {
                temp.push_back(m_nodes[i].m_node_ptr->get_coord());
            }
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                temp.push_back(m_nodes[i].m_node_ptr->get_coord());
            }
        }

        return temp;
    }

    void Element::update_density(double& x, const double& p)
    {
        m_x = x;
        m_E = m_Emin + std::pow(x,p)*(m_E0 - m_Emin);
        m_SM = (m_E/m_E0) * m_original_SM;
    } // update_density()

    void Element::update_density_old(double& x, const double& p)
    {
        m_x = x;
        m_SM = pow(m_x, p) * m_original_SM;
    } // update_density()

    void Element::switch_visualisation()
    {
        m_visualise = !m_visualise;
    } // switch_visualisation()

    double Element::get_volume()
    {
        return m_vol;
    } // get_volume()

    double Element::get_energy()
    {
        return m_total_energy;
    } // get_compliance()

    double Element::get_normal_energy()
    {
        std::cerr << "Error, requested normal energy from non Flat shell, exiting....(Element.hpp)" << std::endl;
        exit(1);
    } // get_normal_energy() only for Flat shells!

    double Element::get_bending_energy()
    {
        std::cerr << "Error, requested bending energy from non Flat shell, exiting....(Element.hpp)" << std::endl;
        exit(1);
    } // get_bending_energy() only for flat shells!

    double Element::get_shear_energy()
    {
        std::cerr << "Error, requested bending energy from non Flat shell, exiting....(Element.hpp)" << std::endl;
        exit(1);
    } // get_shear_energy() only for flat shells!

    double Element::get_axial_energy()
    {
        std::cerr << "Error, requested bending energy from non Flat shell, exiting....(Element.hpp)" << std::endl;
        exit(1);
    } // get_axial_energy() only for flat shells!

    std::map<unsigned int, double> Element::get_energies()
    {
        return m_energies;
    }

    double Element::get_energy_sensitivity(const double& p)
    {
        return (-p / m_x) * m_total_energy;
    } // get_energy_sensitivity()

    double Element::get_volume_sensitivity()
    {
        return m_vol;
    } // get_volume_sensitivity()

    double Element::get_density()
    {
        return m_x;
    } // get_density()

    Eigen::Vector3d Element::get_center_coord()
    {
        return m_center;

    } // get_center_coord()


} // namespace Elements
} // namespace Structural_Design
} // namespace BSO



#endif // ELEMENT_HPP
