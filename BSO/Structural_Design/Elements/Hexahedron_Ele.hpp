#ifndef HEXAHEDRON_ELE_HPP
#define HEXAHEDRON_ELE_HPP

#include <BSO/Structural_Design/Elements/Element.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <math.h>

// usefull typedef for node dof arrangements
namespace Eigen {typedef Matrix<int, 6, 1> Vector6i;}

namespace BSO { namespace Structural_Design { namespace Elements {

    class Hexahedron : public Element
    {
    private:
        double m_v; // poisson ratio [-]

    public:
        Hexahedron(double E, double v, Node* n_1, Node* n_2, Node* n_3, Node* n_4
                                 , Node* n_5, Node* n_6, Node* n_7, Node* n_8);
        ~Hexahedron();

        double get_property(unsigned int n);
    }; // Flat_Shell

    Hexahedron::Hexahedron(double E, double v, Node* n_1, Node* n_2, Node* n_3, Node* n_4,
                           Node* n_5, Node* n_6, Node* n_7, Node* n_8)
    {
        /*
                Local coordinate system of a hexahedron element, the position of the origin is
                computed as the sum of all node coordinates divided by the number of nodes.

                    5********8                      signum representation of each node:
                   **       **                      1: (-1,-1,-1)
                  * *      * *                      2: ( 1,-1,-1)
                 *  *     *  *                      3: ( 1, 1,-1)
                6********7   *       z              4: (-1, 1,-1)
                *   *    *   *       *              5: (-1,-1, 1)
                *   1********4       *              6: ( 1,-1, 1)
                *  *     *  *        *****y         7: ( 1, 1, 1)
                * *      * *        *               8: (-1, 1, 1)
                **       **        *
                2********3        x

        */

        // put the nodes in the right order
        std::vector<Node*> global_nodes; // ordered list of the

error still need to order the nodes

        // store the nodes in m_nodes, and update their freedom signature with this element's freedom table
        Eigen::Vector6i EFS; // initialise a temporary element freedom signature
        EFS << 1, 1, 1, 0, 0, 0; // EFS of a hexahedron's node [ux, uy, uz, rx, ry, rz] (no rotation dofs)
        Element_Node temp; // initialise a temporary Element_Node structure
        temp.m_EFS = EFS; // assign the truss's EFS

        m_nodes.clear();
        for (unsigned int i = 0; i < 8; i++)
        { // for each node of this element
            // store the nodes in an ordered list
            temp.m_node_ptr = global_nodes[i];
            m_nodes.push_back(temp);
        }

        //initialise some properties of the element
        m_v = v; // poisson ratio [-]
        m_E0 = E; // youngs modulus [N/mm²]
        m_E = E; // youngs modulus [N/mm²]
        m_center << 0, 0, 0; // initialise center point of the element to zero(coordinates x,y,z type: BSO::Vectors::Point)
        for (unsigned int i = 0; i < 8; i++)
        { // for each node
            // calculate the sum of all coordinates (to calculate the mean value of the center x, y and z value
            m_center += global_nodes[i]->get_coord();
        }
        m_center /= 8.0; // divide by the number of nodes
        m_Emin = E * 1e-6; // minimal youngs modulus [N/mm²]
        m_vol = BSO::Vectors::hexahedron_volume(global_nodes[0]->get_coord(), global_nodes[1]->get_coord(),
                                                global_nodes[2]->get_coord(), global_nodes[3]->get_coord(),
                                                global_nodes[4]->get_coord(), global_nodes[5]->get_coord(),
                                                global_nodes[6]->get_coord(), global_nodes[7]->get_coord(), true); // element volume [m³]

        // compute the unit vectors of the local coordinate system
        BSO::Vectors::Vector vx, vy, vz, v_temp;
        vx = BSO::Vectors::normalise(((global_nodes[3]->get_coords() + global_nodes[2]->get_coords() +
                                       global_nodes[6]->get_coords() + global_nodes[7]->get_coords())/4) - m_center); // vector from m_center to center of the surface in positive x-direction (local)
        v_temp = BSO::Vectors::normalise(((global_nodes[4]->get_coords() + global_nodes[3]->get_coords() +
                                           global_nodes[7]->get_coords() + global_nodes[8]->get_coords())/4) - m_center); // vector from m_center to center of the surface in positive y-direction (local)
        vz = BSO::Vectors::normalise(BSO::Vectors::cross(vx, v_temp)); // orthogonal to vx and v_temp in positive z direction (order vx --> vtemp is important!)
        vy = BSO::Vectors::normalise(BSO::Vectors::cross(vz, vx)); // orthogonal to vz and vx in positive y-direction (order vz --> vx is important)

        // compute the transformation matrix lambda to transform a point between the local and global coordinate systems
        Eigen::Matrix3d lambda;
        lambda << vx, vy, vz;

        // compute the transformation matrix T for the whole element
        m_T.setZero(24,24);
        for (int i = 0; i < 8; i++)
        { // for each node (only translation dof's present, i.e. no rotation)
            // add the transformation matrix lambda
            m_T.block<3,3>(3*i,3*i) = lambda.transpose();
        }

        // transform the global coordinates to local coordinates
        Eigen::MatrixXd loc_coords;
        loc_coords.setZero(8,3);

        for (unsigned int i = 0; i < 8; i++)
        { // for each node
            loc_coords.row(i) = lambda.transpose() * global_nodes[i]->get_coord();
        }

        // initialise the stiffness term [E] (from the constitutive relation: [sigma] = [E]*[epsilon]) for integral over element volume of [B]^T [E] [B] det([J])
        Eigen::MatrixXd mE;
        mE.setZero(6,6);

        mE(0,0) = 2*(pow(m_v,2)+1);   mE(0,1) = -2*m_v*(m_v-1); mE(0,2) = -2*m_v*(m_v+1); // first 3 elements of the first row
        mE(1,0) = mE(0,1);            mE(1,1) = mE(0,0);        mE(1,2) = mE(0,2); // first 3 elements of the second row
        mE(2,0) = mE(0,2);            mE(2,1) = mE(1,2);        mE(2,2) = 2*(pow(m_v,2)-1); // first 3 elements of the third row
        mE(3,3) = 2*pow(m_v,2)-m_v+1; mE(4,4) = mE(3,3);        mE(5,5) = mE(3,3); // elements of the diagonal on the last 3 rows

        mE *= (m_E/(2.0*(1.0+m_v)*(1.0-m_v+2.0*pow(m_v,2))));

        // initialise the element stiffness matrix and start numerical integration of the contribution of every node to the element's stiffness
        m_SM.setZero(24,24);
        double ksi, eta, zeta; // natural coordinates (will hold the values of the Gauss integration points)
        double w_ksi, w_eta, w_zeta; // weight values for each integration point
        for (unsigned int n = 0; n < 2; n++)
        { // for each integration point in ksi-direction
            for(unsigned int m = 0; m < 2; m++)
            { // for each integration point in eta-direction
                for (unsigned int l = 0; l < 2; l++)
                { // for each integration point in zeta-direction
                    // set the values for the current integration point (i.e. natural coordinates and weight factors)
                    if   (n == 0)
                    { // first integration point in ksi-direction
                        ksi = -1 * sqrt(1.0 / 3.0); // integration point
                        w_ksi = 1.0;
                    }
                    else (n == 1)
                    { // second integration point in ksi-direction
                        ksi =  1 * sqrt(1.0 / 3.0);
                        w_ksi = 1.0;
                    }

                    if   (m == 0)
                    { // first integration point in eta-direction
                        eta = -1 * sqrt(1.0 / 3.0);
                        w_eta = 1.0;
                    }
                    else (m == 1)
                    { // second integration point in eta-direction
                        eta =  1 * sqrt(1.0 / 3.0);
                        w_eta = 1.0;
                    }

                    if   (l == 0)
                    { // first integration point in zeta-direction
                        zeta = -1 * sqrt(1.0 / 3.0);
                        w_zeta = 1.0;
                    }
                    else (l == 1)
                    { // second integration point in zeta-direction
                        zeta =  1 * sqrt(1.0 / 3.0);
                        w_zeta = 1.0;
                    }

                    // compute the derivatives of the displacements with respect to the natural coordinates (ksi, eta and zeta)
                    Eigen::MatrixXd dN;
                    dN.setZero(3,8);

                    dN(0,0) = (-1.0/8.0)*(1-eta)*(1-zeta);   dN(1,0) = (-1.0/8.0)*(1-ksi)*(1-zeta);   dN(2,0) = (-1.0/8.0)*(1-ksi)*(1-eta);
                    dN(0,1) = ( 1.0/8.0)*(1-eta)*(1-zeta);   dN(1,1) = (-1.0/8.0)*(1+ksi)*(1-zeta);   dN(2,1) = (-1.0/8.0)*(1+ksi)*(1-eta);
                    dN(0,2) = ( 1.0/8.0)*(1+eta)*(1-zeta);   dN(1,2) = ( 1.0/8.0)*(1+ksi)*(1-zeta);   dN(2,2) = (-1.0/8.0)*(1+ksi)*(1+eta);
                    dN(0,3) = (-1.0/8.0)*(1+eta)*(1-zeta);   dN(1,3) = ( 1.0/8.0)*(1-ksi)*(1-zeta);   dN(2,3) = (-1.0/8.0)*(1-ksi)*(1+eta);
                    dN(0,4) = (-1.0/8.0)*(1-eta)*(1+zeta);   dN(1,4) = (-1.0/8.0)*(1-ksi)*(1+zeta);   dN(2,4) = ( 1.0/8.0)*(1-ksi)*(1-eta);
                    dN(0,5) = ( 1.0/8.0)*(1-eta)*(1+zeta);   dN(1,5) = (-1.0/8.0)*(1+ksi)*(1+zeta);   dN(2,5) = ( 1.0/8.0)*(1+ksi)*(1-eta);
                    dN(0,6) = ( 1.0/8.0)*(1+eta)*(1+zeta);   dN(1,6) = ( 1.0/8.0)*(1+ksi)*(1+zeta);   dN(2,6) = ( 1.0/8.0)*(1+ksi)*(1+eta);
                    dN(0,7) = (-1.0/8.0)*(1+eta)*(1+zeta);   dN(1,7) = ( 1.0/8.0)*(1-ksi)*(1+zeta);   dN(2,7) = ( 1.0/8.0)*(1-ksi)*(1+eta);

                    // compute the matrix of Jacobi to map between derivatives of the element shape with respect to natural and local coordinates (ksi, eta, zeta versus x_loc, y_loc, z_loc)
                    Eigen::MatrixXd J, J_i;
                    J = dN * loc_coords; // 3 by 3 matrix, matrix of Jacobi
                    J_i = J.inverse(); // also 3 by 3 matrix, the inverse of the matrix of Jacobi

                    // compute the constitutive relation between strain and nodal displacements in the natural coordinate system (ksi, eta, zeta)
                    Eigen::MatrixXd A;
                    A.setZero(6,9);

                    A(0,0) = J_i(0,0); A(0,1) = J_i(0,1); A(0,2) = J_i(0,2); // du/dx --> epsilon[x]
                    A(1,3) = J_i(1,0); A(1,4) = J_i(1,1); A(1,5) = J_i(1,2); // dv/dy --> epsilon[y]
                    A(2,6) = J_i(2,0); A(2,7) = J_i(2,1); A(2,8) = J_i(2,2); // dw/dz --> epsilon[z]

                    A(3,0) = J_i(1,0); A(3,1) = J_i(1,1); A(3,2) = J_i(1,2); // du/dy +
                    A(3,3) = J_i(0,0); A(3,4) = J_i(0,1); A(3,5) = J_i(0,2); // dv/dx --> gamma[xy]

                    A(4,3) = J_i(2,0); A(4,4) = J_i(2,1); A(4,5) = J_i(2,2); // dv/dz +
                    A(4,6) = J_i(1,0); A(4,7) = J_i(1,1); A(4,8) = J_i(1,2); // dw/dy --> gamma[yz]

                    A(5,6) = J_i(0,0); A(5,7) = J_i(0,1); A(5,8) = J_i(0,2); // dw/dx +
                    A(5,0) = J_i(2,0); A(5,1) = J_i(2,1); A(5,2) = J_i(2,2); // du/dz --> gamma[zx]

                    // compute the relation between displacements in the local coordinate system (x_loc, y_loc, z_loc) and the natural coordinate system (ksi, eta, zeta)
                    Eigen::MatrixXd G;
                    G.setZero(9,24);

                    for (unsigned int i = 0; i < 8; i++)
                    { // for each node
                        for (unsigned int j = 0; j < 3; j++)
                        { // for each dof in that node
                            for (unsigned int k = 0; k < 3; k++)
                            { // for each contribution of the dof 'j' of node 'i' to the dof 'k' of the current integration integration point
                                G(3*k+j,3*i+k) = dN(j,i);
                            }
                        }
                    }

                    // compute the derivatives of the displacement with respect to the local coordinates (x_loc, y_loc, z_loc) i.e. the strains in the element
                    Eigen::MatrixXd B;
                    B = A*G; // 6 by 24 matrix

                    m_SM += w_ksi*w_eta*w_zeta*B.transpose()*mE*B*J.determinant(); // sum for all integration points (Gauss Quadrature)
                } // end for each integration point in zeta-direction
            } // end for each integration point in eta-direction
        } // end for each integration point in ksi-direction

        // transform the element stiffness matrix from local to global coordinate system
        m_SM = m_T.transpose() * m_SM * m_T;
        m_original_SM = m_SM;
    } // ctor

    Hexahedron::~Hexahedron()
    {

    } // dtor

    double Flat_Shell::get_property(unsigned int n)
    {
        switch(n)
        {
        case 1:
            return m_E;
            break;
        case 2:
            return m_v;
            break;
        default:
            std::cout << "Requesting invalid property from flat shell element, exiting now..." << std::endl;
            exit(1);
        }
    } // get_property()


} // namespace Elements
} // namespace Structural_Design
} // namespace BSO



#endif // HEXAHEDRON_ELE_HPP
