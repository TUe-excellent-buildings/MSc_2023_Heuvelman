#ifndef BEAM_ELE_HPP
#define BEAM_ELE_HPP

#include <BSO/Structural_Design/Elements/Element.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <math.h>

// usefull typedef for node dof arrangements
namespace Eigen {typedef Matrix<int, 6, 1> Vector6i;}

namespace BSO { namespace Structural_Design { namespace Elements {

    class Beam : public Element
    {
    private:
        double m_b; // beam width [mm]
        double m_h; // beam height [mm]
        double m_A; // cross sectional area [mm³]
        double m_v; // poisson ratio [-]
        double m_L; // element length [mm]
        double m_Iz; // moment of inertia around z-axis [mm4]
        double m_Iy; // moment of inertia around y-axis [mm4]
        double m_J; // torsional moment of inertia [mm4]
        double m_G; // shear modulus of elasticity [N/mm²]

        Eigen::MatrixXd m_T;
    public:
        Beam(double b, double h, double E, double v, Node* n_1, Node* n_2);
        ~Beam();

        double get_property(unsigned int n);

    }; // Beam

    Beam::Beam(double b, double h, double E, double v, Node* n_1, Node* n_2) : Element()
    {
        m_is_beam = true;
        m_b = b; // beam width [mm]
        m_h = h; // beam height [mm]
        m_E = E; // youngs modulus [N/mm²]
        m_E0 = E; // youngs modulus [N/mm²]
        m_Emin = E * 1e-6; // minimal youngs modulus [N/mm²]
        m_v = v; // poisson ratio

        BSO::Vectors::Vector c = n_2->get_coord() - n_1->get_coord(); // vector moving from node 1 to node 2
        m_L = BSO::Vectors::length(c); // truss length [mm]
        m_center = n_1->get_coord() + 0.5*c; // center coordinate of the truss

        Eigen::Vector6i EFS; // initialise a temporary element freedom signature
        EFS << 1, 1, 1, 1, 1, 1; // EFS of a truss [ux, uy, uz, rx, ry, rz]
        Element_Node temp; // initialise a temporary Element_Node structure
        temp.m_EFS = EFS; // assign the truss's EFS
        temp.m_node_ptr = n_1; // assign first node
        n_1->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node information to this elements node list
        temp.m_node_ptr = n_2; // assign second node
        n_2->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node to this elements node list

        /*
         * Generate Transformation matrix
         */

        // step 1: find length values:
        BSO::Vectors::Vector vx = BSO::Vectors::normalise(c); // local x-vector

        BSO::Vectors::Vector vy, vz;

        if (BSO::Vectors::is_vert(vx,0.001))
        {
            vy(0) = 0.0;
            vy(1) = 1.0;
            vy(2) = 0.0;
            vz(0) =-vx(2);
            vz(1)= 0.0;
            vz(2)= 0.0;
        }
        else
        {
        // step 2.3: check if local x lies in global YZ
        // step 2.4: local y-axis decomposed

            if (BSO::Vectors::is_zero(vx(0),0.001))
            {
                vy(0) = 1.0;
                vy(1) = 0.0;
                vy(2) = 0.0;
            }
            else
            {

                /* inproduct of local x-vector with global Z-vector and with local y vector should be zero
                 * and length should be 1, now system of equations can be solved
                 * note: for sqrt the first (+) value has been taken, maybe this has to be checked in future
                 */

                vy(1) = sqrt(1.0 /(1.0 + pow((vx(1) / vx(0)), 2)));
                vy(0) = -vy(1) * (vx(1) / vx(0));
                vy(2) = 0.0;
            }

            // step 2.5: local z-axis decomposed
            // lz is the outproduct of lx and ly, length is set to 1:

            vz = BSO::Vectors::cross(vx, vy);
            vz = BSO::Vectors::normalise(vz);
        }

        // step 3.1 write local directional cosine
        Eigen::Matrix3d lambda;
        lambda << vx, vy, vz;

        // step 3.2 expand the transformation matrix to the size of the beam element's stiffness matrix
        m_T.setZero(12,12);

        for (int i = 0; i < 2; i++)
        { // for each node
            for (int j = 0; j < 2; j++)
            { // and for both: displacements and rotations
                // add the transformation matrix lambda
                m_T.block<3,3>((2*i+j)*3,(2*i+j)*3) = lambda.transpose();
            }
        }

        // generate element stiffness matrix
        m_A = m_b * m_h; // cross sectional area [mm³]
        m_Iz = ((m_h * pow(m_b,3))/12); // moment of inertia around z-axis [mm4]
        m_Iy = ((m_b * pow(m_h,3))/12); // moment of inertia around y-axis [mm4]
        m_J = m_Iy + m_Iz; // torsional moment of inertia [mm4]
        m_G = m_E / (2 * (1+ m_v)); // shear modulus of elasticity
        m_vol = (m_A * m_L) / 1.0e9; // element volume [m³]

        // initialising this elements stiffness matrix:
        m_SM.setZero(12,12);

        double ael = (m_A * m_E) / m_L; // normal strength
        double gjl = (m_G * m_J) / m_L; // shear strength
        double ay  = (12.0 * m_E * m_Iy) / pow(m_L, 3);
        double az  = (12.0 * m_E * m_Iz) / pow(m_L ,3);
        double cy  = (6.0 * m_E * m_Iy) / pow(m_L, 2);
        double cz  = (6.0 * m_E * m_Iz) / pow(m_L, 2);
        double ey  = (4.0 * m_E * m_Iy) / m_L;
        double ez  = (4.0 * m_E * m_Iz) / m_L;
        double fy  = (2.0 * m_E * m_Iy) / m_L;
        double fz  = (2.0 * m_E * m_Iz) / m_L;

        m_SM(0,0) = ael;   // row 0: F(x,1) : normal force
        m_SM(1,1) = az;    // row 1: F(y,1) : shear  force
        m_SM(2,2) = ay;    // row 2: F(z,1) : shear  force
        m_SM(3,3) = gjl;   // row 3: M(xy,1): torsional moment
        m_SM(4,2) = -cy;   // row 4: M(yz,1): bending   moment
        m_SM(4,4) = ey;
        m_SM(5,1) = cz;    // row 5: M(zx,1): bending   moment
        m_SM(5,5) = ez;
        m_SM(6,0) = -ael;  // row 6: F(x,2)
        m_SM(6,6) = ael;
        m_SM(7,1) = -az;   // row 7: F(y,2)
        m_SM(7,5) = -cz;
        m_SM(7,7) = az;
        m_SM(8,2) = -ay;   // row 8: F(z,2)
        m_SM(8,4) = cy;
        m_SM(8,8) = ay;
        m_SM(9,3) = -gjl;  // row 9: M(xy,2)
        m_SM(9,9) = gjl;
        m_SM(10,2) = -cy;  // row 10:M(yz,2)
        m_SM(10,4) = fy;
        m_SM(10,8) = cy;
        m_SM(10,10) = ey;
        m_SM(11,1) = cz;   // row 11:M(zx,2)
        m_SM(11,5) = fz;
        m_SM(11,7) = -cz;
        m_SM(11,11) = ez;

        // m_SM is symmetric, this algorithm mirrors the above entries along the matrix diagonal
        for (unsigned int j=0; j<12; j++)
        {
            for (unsigned int k=j+1; k<12; k++)
            {
                m_SM(j,k) = m_SM(k,j);
            }
        }

        // transform element stiffness matrix to global coordinate system
        m_SM = m_T.transpose() * m_SM * m_T;
        m_original_SM = m_SM;

    } // ctor

    Beam::~Beam()
    {

    } // dtor

    double Beam::get_property(unsigned int n)
    {
        switch(n)
        {
        case 1:
            return m_b;
            break;
        case 2:
            return m_h;
            break;
        case 3:
            return m_A;
            break;
        case 4:
            return m_E;
            break;
        case 5:
            return m_v;
            break;
        case 6:
            return m_Iy;
            break;
        case 7:
            return m_Iz;
            break;
        default:
            std::cout << "Requesting invalid property from beam element, exiting now..." << std::endl;
            exit(1);
        }
    } // get_property()


} // namespace Elements
} // namespace Structural_Design
} // namespace BSO



#endif // BEAM_ELE_HPP
