#ifndef TRUSS_ELE_HPP
#define TRUSS_ELE_HPP

#include <BSO/Structural_Design/Elements/Element.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <math.h>

namespace Eigen {typedef Matrix<int, 6, 1> Vector6i;}

namespace BSO { namespace Structural_Design { namespace Elements {

    class Truss : public Element
    {
    private:
        double m_A; // cross sectional area [mm³]
        double m_L; // element length [mm]

    public:
        Truss(double A, double E, Node* n_1, Node* n_2);
        ~Truss();

        double get_property(unsigned int n);

    }; // Truss

    Truss::Truss(double A, double E, Node* n_1, Node* n_2) : Element()
    {
        m_is_truss = true;
        m_A = A; // cross sectional area [mm³]
        m_E = E; // youngs modulus [N/mm²]
        m_E0 = E; // youngs modulus [N/mm²]
        m_Emin = E * 1e-6; // minimal youngs modulus [N/mm²]

        BSO::Vectors::Vector c = n_2->get_coord() - n_1->get_coord(); // vector moving from node 1 to node 2
        m_L = BSO::Vectors::length(c); // length of the truss [mm]
        m_center = n_1->get_coord() + 0.5*c; // center coordinate of the truss

        m_vol = (m_A * m_L) / 1.0e9; // element volume [m³]

        Eigen::Vector6i EFS; // initialise a temporary element freedom signature
        EFS << 1, 1, 1, 0, 0, 0; // EFS of a truss [ux, uy, uz, rx, ry, rz]
        Element_Node temp; // initialise a temporary Element_Node structure
        temp.m_EFS = EFS; // assign the truss's EFS
        temp.m_node_ptr = n_1; // assign first node
        n_1->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node information to this elements node list
        temp.m_node_ptr = n_2; // assign second node
        n_2->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node to this elements node list

        // initialising this elements stiffness matrix:
        m_SM.setZero(6,6);

        // generate element stiffness matrix
        c = BSO::Vectors::normalise(c);

        m_SM(0,0) =  pow(c(0),2);
        m_SM(0,1) =  c(0)*c(1);
        m_SM(0,2) =  c(0)*c(2);
        m_SM(0,3) = -pow(c(0),2);
        m_SM(0,4) = -c(0)*c(1);
        m_SM(0,5) = -c(0)*c(2);

        m_SM(1,1) =  pow(c(1),2);
        m_SM(1,2) =  c(1)*c(2);
        m_SM(1,3) = -c(0)*c(1);
        m_SM(1,4) = -pow(c(1),2);
        m_SM(1,5) = -c(1)*c(2);

        m_SM(2,2) =  pow(c(2),2);
        m_SM(2,3) = -c(0)*c(2);
        m_SM(2,4) = -c(1)*c(2);
        m_SM(2,5) = -pow(c(2),2);

        m_SM(3,3) =  pow(c(0),2);
        m_SM(3,4) =  c(0)*c(1);
        m_SM(3,5) =  c(0)*c(2);

        m_SM(4,4) =  pow(c(1),2);
        m_SM(4,5) =  c(1)*c(2);

        m_SM(5,5) =  pow(c(2),2);

        m_SM *= ((m_A*m_E) / m_L);

        // m_SM is symmetric, this algorithm mirrors the above entries along the matrix diagonal
        for (unsigned int j=0; j<6; j++)
        {
            for (unsigned int k=j+1; k<6; k++)
            {
                m_SM(k,j) = m_SM(j,k);
            }
        }

        m_original_SM = m_SM;
    } // ctor

    Truss::~Truss()
    {

    } // dtor

    double Truss::get_property(unsigned int n)
    {
        switch(n)
        {
        case 1:
            return m_A;
            break;
        case 2:
            return m_E;
            break;
        default:
            std::cout << "Requesting invalid property from truss element, exiting now..." << std::endl;
            exit(1);
        }
    } // get_property()


} // namespace Elements
} // namespace Structural_Design
} // namespace BSO



#endif // TRUSS_ELE_HPP
