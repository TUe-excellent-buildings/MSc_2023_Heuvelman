#ifndef NODE_ELE_HPP
#define NODE_ELE_HPP

#include <boost/algorithm/string.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <map>
#include <stdexcept>

namespace Eigen {typedef Matrix<int, 6, 1> Vector6i;}
namespace Eigen {typedef Matrix<double, 6, 1> Vector6d;}

namespace BSO { namespace Structural_Design { namespace Elements {


    class Node
    {
    private:
        static unsigned long m_count;
        unsigned long m_ID; // node ID
        unsigned long m_NFM; // node freedom mapping (the number of dof's counted before this node's dof's)
        std::map<int, unsigned long> m_NFT; // node freedom table, keyvariable corresponds with node freedom arrangement
        Eigen::Vector3d m_coord; // node coordinates, arrangement: [x, y, z]
        Eigen::Vector6i m_NFS; // node freedom signature, node freedom arrangement: [ux, uy, uz, rx, ry, rz]
        Eigen::Vector6i m_constraints; // node constraints, arrangement: [ux, uy, uz, rx, ry, rz]
        std::map<unsigned int, Eigen::Vector6d> m_loads; // load vector mapped to each load case, arrangement [fx, fy, fz, mx, my, mz]
        std::map<unsigned int, Eigen::Vector6d> m_displacements; // displacement vector mapped to each load case, arrangement [ux, uy, uz, rx, ry, rz]

    public:
        Node(unsigned long ID, double x, double y, double z);
        Node(double x, double y, double z);
        ~Node();

        void update_NFS(Eigen::Vector6i EFS);
        void add_constraint(std::string dir);
        void add_constraint(unsigned int dof);
        void add_load_case(unsigned int lc);
        void add_load(unsigned int lc, std::string dir, double load);
        void add_load(unsigned int lc, unsigned int dir, double load);
        void add_displacements(std::map<unsigned int, Eigen::VectorXd> displacements);
        void set_NFT(unsigned long NFM);

        Eigen::Vector6i get_NFS();
        Eigen::Vector6i get_constraints();
        Eigen::Vector6d get_loads(unsigned int n);
        Eigen::Vector3d get_coord();
        unsigned long get_ID();
        int get_freedom_count();
        unsigned long get_dof(int n);
        double get_load(unsigned int lc, int n);
        double get_displacement(unsigned int lc, int dof);
        Eigen::Vector6d get_displacements(unsigned int lc);
        static unsigned long get_count();

        bool check_constraint(int n);
        bool check_load(unsigned int lc, unsigned int n, double& load);

    }; // Node

    // initialisation of static variables
    unsigned long Node::m_count = 0;


    Node::Node(unsigned long ID, double x, double y, double z)
    {   // initialiser which uses a pre-specified ID for nodes
        ++m_count;
        m_ID = ID;
        m_coord[0] = x;
        m_coord[1] = y;
        m_coord[2] = z;

        m_NFS.setZero();
        m_constraints.setZero();
    } // ctor

    Node::~Node()
    {
        --m_count;
    } //dtor

    void Node::update_NFS(Eigen::Vector6i EFS)
    {
        for (int i = 0; i < 6; i++)
        {
            if(EFS[i] == 1)
            {
                m_NFS[i] = 1;
            }
        }
    } // update_EFS()

    void Node::add_constraint(std::string dof)
    {
        boost::to_upper(dof);
        if (dof == "UX")
            {m_constraints(0) = 1;}
        else if (dof == "UY")
            {m_constraints(1) = 1;}
        else if (dof == "UZ")
            {m_constraints(2) = 1;}
        else if (dof == "RX")
            {m_constraints(3) = 1;}
        else if (dof == "RY")
            {m_constraints(4) = 1;}
        else if (dof == "RZ")
            {m_constraints(5) = 1;}
        else
        {
            std::cout << "Error in assigning constraint to node: " << m_ID << " exiting now..." << std::endl;
            exit(1);
        }
    } // add_constraint(std::string)

    void Node::add_constraint(unsigned int dof)
    {
        m_constraints(dof) = 1;
    } // add_constraint(unsigned int)

    void Node::add_load_case(unsigned int lc)
    {
        Eigen::Vector6d temp;
        temp.setZero();

        m_loads[lc] = temp;
    } // add_load_case()

    void Node::add_load(unsigned int lc, std::string dir, double load)
    {
        boost::to_upper(dir);
        if (dir == "FX")
            {m_loads[lc](0) += load;}
        else if (dir == "FY")
            {m_loads[lc](1) += load;}
        else if (dir == "FZ")
            {m_loads[lc](2) += load;}
        else if (dir == "MX")
            {m_loads[lc](3) += load;}
        else if (dir == "MY")
            {m_loads[lc](4) += load;}
        else if (dir == "MZ")
            {m_loads[lc](5) += load;}
        else
        {
            std::cout << "Error in assigning load to node: " << m_ID << " exiting now..." << std::endl;
            exit(1);
        }
    } // add_load()

    void Node::add_load(unsigned int lc, unsigned int dir, double load)
    {
        m_loads[lc](dir) += load;
    } // add_load()

    void Node::add_displacements(std::map<unsigned int, Eigen::VectorXd> displacements)
    {
        typedef std::map<unsigned int, Eigen::VectorXd>::iterator ite;
        for (ite it = displacements.begin(); it != displacements.end(); it++)
        { // for all load cases
            Eigen::Vector6d temp_disp;
            temp_disp.setZero();

            for (int i = 0; i < 6; i++)
            { // for all dof's of the node
                if (m_NFS[i] == 1)
                { // if the dof is active
					if (m_constraints[i] == 1) temp_disp(i) = 0; // if the node is constrained, then it should be zero
                    else temp_disp(i) = it->second(m_NFT[i]); // if not, then it is safe to get it from here
                }

            }

            m_displacements[it->first] = temp_disp;
        }
    } // add_displacement

    void Node::set_NFT(unsigned long NFM)
    {
        m_NFM = NFM;
        unsigned long counter = NFM;
        for (int i = 0; i < 6; i++)
        {
            if (m_NFS[i] == 1)
            {
				if (m_constraints[i] == 1) m_NFT[i] = 0;
                else m_NFT[i] = counter++;
            }
        }
    } // set_NFM()


    Eigen::Vector6i Node::get_NFS()
    {
        return m_NFS;
    } // get_NFS()

    Eigen::Vector6i Node::get_constraints()
    {
        return m_constraints;
    } // get_constraints

    Eigen::Vector6d Node::get_loads(unsigned int n)
    {
        if (m_loads.find(n) != m_loads.end())
        {
            return m_loads[n];
        }
        else
        {
            std::cout << "Error in requesting non existing loadcase: " << n << " (in Node.hpp) exiting now..." << std::endl;
            exit(1);
        }
    } // get_loads()

    Eigen::Vector3d Node::get_coord()
    {
        return m_coord;
    } // get_coord()

    unsigned long Node::get_ID()
    {
        return m_ID;
    } // get_ID()

    int Node::get_freedom_count()
    {
        int counter = 0;
        for (int i = 0; i < 6; i++)
        {
            if (m_NFS(i) == 1 && m_constraints[i] == 0)
            {
                counter++;
            }
        }
        return counter;
    }

    unsigned long Node::get_dof(int n)
    {
        if (m_NFS[n] == 0 || m_constraints[n] == 1)
        {
            std::string error_message = "Error, requesting non-existing dof (Node.hpp) node: " + std::to_string(m_ID) + "; dof: " + std::to_string(n) + "; exiting now...\n";
			throw std::invalid_argument(error_message);
        }
        return m_NFT[n];
    }

    double Node::get_load(unsigned int lc, int n)
    {
        if (m_NFS[n] == 0)
        {
            std::cout << "Error, requesting non-existing load (Node.hpp) node: " << m_ID << "; load: " << n << "; exiting now..." << std::endl;
            exit(1);
        }
        return m_loads[lc](n);
    }

    double Node::get_displacement(unsigned int lc, int dof)
    {
        return m_displacements[lc](dof);
    } // get_displacement()

    unsigned long Node::get_count()
    {
        return m_count;
    } // get_count();

    Eigen::Vector6d Node::get_displacements(unsigned int lc)
    {
        return m_displacements[lc];
    } // get_displacements()

    bool Node::check_constraint(int n)
    { // checks if dof 'n' is constrained
        if (m_NFS(n) == 1)
        { // if the dof is active
            if(m_constraints(n) == 1)
            { // and the dof is constrained
                return true;
            }
        }// if not then the node's dof is not constrained
        return false;
    }

    bool Node::check_load(unsigned int lc, unsigned int n, double& load)
    { // checks if the node has a load acting in the direction of dof 'n' if so it passes it by reference via the last argument
        if (m_NFS(n) == 1)
        { // if the dof is active
            if(m_constraints(n) == 0)
            { // and the dof is not constrained
                load = m_loads[lc](n);

                if (load != 0)
                { // and a load is acting on the dof
                    // then return the load value via the variable passed by reference
                    return true;
                }
            }
        }// if not then there is no load acting on the node in the direction of the dof
        return false;
    }


} // namespace Elements
} // namespace Structural_Design
} // namespace BSO



#endif // NODE_ELE_HPP
