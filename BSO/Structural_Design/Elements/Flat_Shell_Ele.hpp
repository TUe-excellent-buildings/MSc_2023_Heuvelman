#ifndef FLAT_SHELL_ELE_HPP
#define FLAT_SHELL_ELE_HPP

#include <BSO/Structural_Design/Elements/Element.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <math.h>

// usefull typedef for node dof arrangements
namespace Eigen {typedef Matrix<int, 6, 1> Vector6i;}

namespace BSO { namespace Structural_Design { namespace Elements {

    class Flat_Shell : public Element
    {
    private:
        double m_t; // element thickness [mm]
        double m_A; // element surface area [mm³]
        double m_v; // poisson ratio [-]

        double m_bending_energy; // the shear energy over all load_cases
        double m_normal_energy; // the normal energy over all load_cases
        double m_shear_energy; // the normal energy over all load_cases
        double m_axial_energy; // the normal energy over all load_cases

        Eigen::MatrixXd m_SM_normal;
        Eigen::MatrixXd m_SM_axial;
        Eigen::MatrixXd m_SM_shear;

        Eigen::MatrixXd m_SM_bending;

        Eigen::MatrixXd m_T; // transformation matrix, for transformations between local and global coordinates
        Eigen::Vector3d m_loc_p1, m_loc_p2, m_loc_p3, m_loc_p4; // local coordinates of element points

        Eigen::Vector3d m_intersect; // intersection point of the element's diagonals

    public:
        Flat_Shell(double t, double E, double v, Node* n_1, Node* n_2, Node* n_3, Node* n_4);
        ~Flat_Shell();

        void calc_energies(const std::vector<unsigned int>& load_cases);
        double get_normal_energy();
        double get_bending_energy();
        double get_shear_energy();
        double get_axial_energy();
        double get_property(unsigned int n);
    }; // Flat_Shell

    Flat_Shell::Flat_Shell(double t, double E, double v, Node* n_1, Node* n_2, Node* n_3, Node* n_4) : Element()
    {
        m_is_flat_shell = true;
        m_t = t; // element thickness [mm]

        // store the node coordinates temporarily
        Eigen::Vector3d p1 = n_1->get_coord();
        Eigen::Vector3d p2 = n_2->get_coord();
        Eigen::Vector3d p3 = n_3->get_coord();
        Eigen::Vector3d p4 = n_4->get_coord();

        // initialise all possible vectors between p1, p2, p3, p4 (two of them are diagonals diagonal)
        BSO::Vectors::Vector v21 = p2 - p1;
        BSO::Vectors::Vector v31 = p3 - p1;
        BSO::Vectors::Vector v41 = p4 - p1;
        BSO::Vectors::Vector v32 = p3 - p2;
        BSO::Vectors::Vector v42 = p4 - p2;
        BSO::Vectors::Vector v43 = p4 - p3;

        if (BSO::Vectors::intersection(p1, v41, p2, v32, 0.1))
        { // v41 and v32 are the two diagonals
            std::swap(p4, p3);
            std::swap(n_4, n_3);
            std::swap(v31, v41);
            std::swap(v32, v42);
            v43 *= -1; // swaps 43 with 34
        }
        else if (BSO::Vectors::intersection(p1, v21, p3, v43, 0.1))
        { // v31 and v42 are the two diagonals
            std::swap(p3, p2);
            std::swap(n_3, n_2);
            std::swap(v31, v21);
            std::swap(v43, v42);
            v32 *= -1; // swaps 32 with 23
        } // endif v31 and v42 are now the diagonals, nodes are now ordered either clock wise or counter clockwise : 1,2,3,4 or 1,4,3,2

        m_A = BSO::Vectors::calc_area_quadri_lat(v31, v42); // element surface area [mm²]
        m_E0 = E; // youngs modulus [N/mm²]
        m_E = E; // youngs modulus [N/mm²]
        m_Emin = E * 1e-6; // minimal youngs modulus [N/mm²]
        m_v = v; // poisson ratio
        m_vol = (m_A * m_t) / 1.0e9; // element volume [m³]

        Eigen::Vector6i EFS; // initialise a temporary element freedom signature
        EFS << 1, 1, 1, 1, 1, 1; // EFS of a Quad element's node [ux, uy, uz, rx, ry, rz]
        Element_Node temp; // initialise a temporary Element_Node structure
        temp.m_EFS = EFS; // assign the truss's EFS

        temp.m_node_ptr = n_1; // assign first node
        n_1->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node information to this elements node list

        temp.m_node_ptr = n_2; // assign second node
        n_2->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node to this elements node list

        temp.m_node_ptr = n_3; // assign third node
        n_3->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node to this elements node list

        temp.m_node_ptr = n_4; // assign fourth node
        n_4->update_NFS(EFS); // update the node freedom signature with the EFS
        m_nodes.push_back(temp); // add the node to this elements node list

        if (BSO::Vectors::intersection(p1, v31, p2, v42, m_intersect, 0.1))
        { // diagonals intersect,
             // the intersection point has been stored in m_intersect
             // by the function in the if statement
        }
        else
        { // diagonals do not intersect, something is wrong!
            std::cout << "Diagonals in element do not intersect (Flat_Shell_Ele.hpp), exiting..." << std::endl;
            exit(1);
        }

        m_center = (p1 + p2 + p3 + p4) / 4.0;

        //
        BSO::Vectors::Vector vx, vy, vz;
        vx = BSO::Vectors::normalise((p2 + ((p3 - p2) / 2.0)) - m_intersect); // vector in plane with the element from the intersection point to the centre of a line(2-3)
        vz = BSO::Vectors::normalise(BSO::Vectors::cross(v21, v41)); // normal to the plane of the element
        vy = BSO::Vectors::normalise(BSO::Vectors::cross(vz, vx)); // normal to both vx and vz

        // construct transformation matrix from global to local matrix [xyx](local) = lambda.transpose()*[XYZ](global)
        Eigen::Matrix3d lambda;
        lambda << vx, vy, vz;

        // initialise the transformation matrix for element stiffness matrix from local to global coordinate system
        m_T.setZero(24,24);

        for (int i = 0; i < 4; i++)
        { // for each node
            for (int j = 0; j < 2; j++)
            { // and for both: displacements and rotations
                // add the transformation matrix lambda
                m_T.block<3,3>((2*i+j)*3,(2*i+j)*3) = lambda.transpose();
            }
        }

        m_loc_p1 = lambda.transpose() * p1;
        m_loc_p2 = lambda.transpose() * p2;
        m_loc_p3 = lambda.transpose() * p3;
        m_loc_p4 = lambda.transpose() * p4;

        // get local coordinates of the elements nodes:
        double x1loc = m_loc_p1(0);
        double y1loc = m_loc_p1(1);
        double x2loc = m_loc_p2(0);
        double y2loc = m_loc_p2(1);
        double x3loc = m_loc_p3(0);
        double y3loc = m_loc_p3(1);
        double x4loc = m_loc_p4(0);
        double y4loc = m_loc_p4(1);

        // initialising this elements stiffness matrix:
        Eigen::MatrixXd Kmem, Kmem_shear, Kmem_axial;
        Kmem.setZero(8,8);
        Kmem_shear = Kmem_axial = Kmem;
        m_SM_normal.setZero(24,24);
        m_SM_axial = m_SM_shear = m_SM_normal;
        for (int l = 0; l < 2; l++)
        {
            for (int m = 0; m < 2; m++)
            {
                // determine values for eta and ksi for numerical integration
                double ksi;
                double eta;
                if (l==0) {
                    ksi = -1*sqrt(1.0/3.0);
                } else {
                    ksi = 1*sqrt(1.0/3.0);
                }
                if (m==0) {
                    eta = -1*sqrt(1.0/3.0);
                } else {
                    eta = 1*sqrt(1.0/3.0);
                }


                // Finding matrix J following Kaushalkumar Kansara
                Eigen::MatrixXd J;
                J.setZero(2,2);
                J(0,0) = (-0.25+0.25*eta)*x1loc + (0.25-0.25*eta)*x2loc + (0.25+0.25*eta)*x3loc + (-0.25-0.25*eta)*x4loc;
                J(0,1) = (-0.25+0.25*eta)*y1loc + (0.25-0.25*eta)*y2loc + (0.25+0.25*eta)*y3loc + (-0.25-0.25*eta)*y4loc;
                J(1,0) = (-0.25+0.25*ksi)*x1loc + (-0.25-0.25*ksi)*x2loc + (0.25+0.25*ksi)*x3loc + (0.25-0.25*ksi)*x4loc;
                J(1,1) = (-0.25+0.25*ksi)*y1loc + (-0.25-0.25*ksi)*y2loc + (0.25+0.25*ksi)*y3loc + (0.25-0.25*ksi)*y4loc;


                // Finding matrix A following Kaushalkumar Kansara
                Eigen::MatrixXd As;
                As.setZero(3,4);
                As(0,0) = J(1,1);	As(0,1) = -J(0,1);	As(0,2) = 0;		    As(0,3) = 0;
                As(1,0) = 0;		As(1,1) = 0;		As(1,2) = -J(1,0);	    As(1,3) = J(0,0);
                As(2,0) = -J(1,0);	As(2,1) = J(0,0);	As(2,2) = J(1,1);	    As(2,3) = -J(0,1);

                As = As * (1/J.determinant());

                // Finding matrix G following Kaushalkumar Kansara
                Eigen::MatrixXd G;
                G.setZero(4,8);

                G(0,0)=(-0.25+0.25*eta); 	G(2,1)=G(0,0);
                G(0,2)=(0.25-0.25*eta);		G(2,3)=G(0,2);
                G(0,4)=(0.25+0.25*eta);		G(2,5)=G(0,4);
                G(0,6)=(-0.25-0.25*eta);	G(2,7)=G(0,6);
                //
                G(1,0)=(-0.25+0.25*ksi);	G(3,1)=G(1,0);
                G(1,2)=(-0.25-0.25*ksi);	G(3,3)=G(1,2);
                G(1,4)=(0.25+0.25*ksi);		G(3,5)=G(1,4);
                G(1,6)=(0.25-0.25*ksi);		G(3,7)=G(1,6);

                Eigen::MatrixXd B;
                B.setZero(3,8);
                B = As * G;


                // Matrix mE following mech 8 lecture notes
                Eigen::MatrixXd mE, mE_shear, mE_axial;
                mE.setZero(3,3);
                mE_shear.setZero(3,3);

                mE(0,0) = 1;    mE(0,1) = m_v;  mE(0,2) = 0;
                mE(1,0) = m_v;  mE(1,1) = 1;    mE(1,2) = 0;
                mE(2,0) = 0;    mE(2,1) = 0;    mE(2,2) = (1-m_v)/2;

                mE = mE * (m_E/(1-pow(m_v,2)));

                mE_axial = mE;
                mE_axial(2,2) = 0;

                mE_shear(2,2) = mE(2,2);

                Kmem += m_t*1*1* B.transpose() * mE * B * J.determinant();
                Kmem_shear += m_t*1*1* B.transpose() * mE_shear * B * J.determinant();
                Kmem_axial += m_t*1*1* B.transpose() * mE_axial * B * J.determinant();


            } // end for m (ksi/eta)
        } // end for l (ksi/eta)

        for (int m=0;m<4;m++)
        {
            for (int n=0;n<4;n++)
            {
                // for 2 x 2 matrix elements
                for (int p=0;p<2;p++)
                {
                    for (int q=0; q<2;q++)
                    {
                        m_SM_normal(6*m+p,6*n+q) = Kmem(2*m+p,2*n+q);
                        m_SM_shear(6*m+p,6*n+q) = Kmem_shear(2*m+p,2*n+q);
                        m_SM_axial(6*m+p,6*n+q) = Kmem_axial(2*m+p,2*n+q);
                    }
                }
            }
        }


        Eigen::MatrixXd Kben;
        Kben.setZero(12,12);
        m_SM_bending.setZero(24,24);
        for (int l=0;l<2;l++)
        {
            for (int m=0;m<2;m++)
            {
            // determine values for eta and ksi for numerical integration
            double ksi;
            double eta;
            if (l==0) {
                ksi=-1*sqrt(1.0/3.0);
            } else {
                ksi=1*sqrt(1.0/3.0);
            }
            if (m==0) {
            eta=-1*sqrt(1.0/3.0);
            } else {
            eta=1*sqrt(1.0/3.0);
            }

            // calculating matrix of Jacobi
            Eigen::MatrixXd J;
            J.setZero(2,2);

            J(0,0) = (-0.25+0.25*eta)*x1loc + (0.25-0.25*eta)*x2loc + (0.25+0.25*eta)*x3loc + (-0.25-0.25*eta)*x4loc;
            J(0,1) = (-0.25+0.25*eta)*y1loc + (0.25-0.25*eta)*y2loc + (0.25+0.25*eta)*y3loc + (-0.25-0.25*eta)*y4loc;
            J(1,0) = (-0.25+0.25*ksi)*x1loc + (-0.25-0.25*ksi)*x2loc + (0.25+0.25*ksi)*x3loc + (0.25-0.25*ksi)*x4loc;
            J(1,1) = (-0.25+0.25*ksi)*y1loc + (-0.25-0.25*ksi)*y2loc + (0.25+0.25*ksi)*y3loc + (0.25-0.25*ksi)*y4loc;


            // calculating matrix Db
            Eigen::MatrixXd Db;
            Db.setZero(3,3);
            Db(0,0) = 1;    Db(0,1) = m_v;  Db(0,2) = 0;
            Db(1,0) = m_v;  Db(1,1) = 1;    Db(1,2) = 0;
            Db(2,0) = 0;    Db(2,1) = 0;    Db(2,2) = (1-m_v)/2;

            Db = Db * ((m_E*pow(m_t,3))/(12*(1-pow(m_v,2))));


            // calculating matrix B
            double j11 = (1/J.determinant()) * (J(1,1));      double j12 = (-1/J.determinant()) * (J(0,1));
            double j21 = (-1/J.determinant()) * (J(1,0));     double j22 = (1/J.determinant()) * (J(0,0));


            // values of interpolation functions derived to ksi and eta respectively
            double N1ksi = 0.25 * (2*ksi+eta) * (1-eta);
            double N2ksi = 0.25 * (2*ksi-eta) * (1-eta);
            double N3ksi = 0.25 * (2*ksi+eta) * (1+eta);
            double N4ksi = 0.25 * (2*ksi-eta) * (1+eta);
            double N5ksi = -ksi * (1-eta);
            double N6ksi =  0.5 * (1-(eta*eta));
            double N7ksi = -ksi * (1+eta);
            double N8ksi = -0.5 * (1-(eta*eta));
            //

            double N1eta = 0.25 * ((2*eta)+ksi) * (1-ksi);
            double N2eta = 0.25 * ((2*eta)-ksi) * (1+ksi);
            double N3eta = 0.25 * ((2*eta)+ksi) * (1+ksi);
            double N4eta = 0.25 * ((2*eta)-ksi) * (1-ksi);
            double N5eta = -0.5 * (1-(ksi*ksi));
            double N6eta = -eta * (1+ksi);
            double N7eta =  0.5 * (1-(ksi*ksi));
            double N8eta = -eta * (1-ksi);


            // values for temporarily variable a
            double a5 = -(x1loc-x2loc) / (pow((x1loc-x2loc),2) + pow((y1loc-y2loc),2));
            double a6 = -(x2loc-x3loc) / (pow((x2loc-x3loc),2) + pow((y2loc-y3loc),2));
            double a7 = -(x3loc-x4loc) / (pow((x3loc-x4loc),2) + pow((y3loc-y4loc),2));
            double a8 = -(x4loc-x1loc) / (pow((x4loc-x1loc),2) + pow((y4loc-y1loc),2));

            // values for temporarily variable b
            double b5 = 0.75 * ((x1loc-x2loc)*(y1loc-y2loc) / (pow((x1loc-x2loc),2) + pow((y1loc-y2loc),2)));
            double b6 = 0.75 * ((x2loc-x3loc)*(y2loc-y3loc) / (pow((x2loc-x3loc),2) + pow((y2loc-y3loc),2)));
            double b7 = 0.75 * ((x3loc-x4loc)*(y3loc-y4loc) / (pow((x3loc-x4loc),2) + pow((y3loc-y4loc),2)));
            double b8 = 0.75 * ((x4loc-x1loc)*(y4loc-y1loc) / (pow((x4loc-x1loc),2) + pow((y4loc-y1loc),2)));

            // values for temporarily variable c
            double c5 = (0.25*pow((x1loc-x2loc),2) - 0.5*pow((y1loc-y2loc),2)) / (pow((x1loc-x2loc),2) + pow((y1loc-y2loc),2));
            double c6 = (0.25*pow((x2loc-x3loc),2) - 0.5*pow((y2loc-y3loc),2)) / (pow((x2loc-x3loc),2) + pow((y2loc-y3loc),2));
            double c7 = (0.25*pow((x3loc-x4loc),2) - 0.5*pow((y3loc-y4loc),2)) / (pow((x3loc-x4loc),2) + pow((y3loc-y4loc),2));
            double c8 = (0.25*pow((x4loc-x1loc),2) - 0.5*pow((y4loc-y1loc),2)) / (pow((x4loc-x1loc),2) + pow((y4loc-y1loc),2));

            // values for temporarily variable d
            double d5 = -(y1loc-y2loc) / (pow((x1loc-x2loc),2) + pow((y1loc-y2loc),2));
            double d6 = -(y2loc-y3loc) / (pow((x2loc-x3loc),2) + pow((y2loc-y3loc),2));
            double d7 = -(y3loc-y4loc) / (pow((x3loc-x4loc),2) + pow((y3loc-y4loc),2));
            double d8 = -(y4loc-y1loc) / (pow((x4loc-x1loc),2) + pow((y4loc-y1loc),2));

            // values for temporarily variable e
            double e5 = (-0.5*pow((x1loc-x2loc),2) + 0.25*pow((y1loc-y2loc),2)) / (pow((x1loc-x2loc),2) + pow((y1loc-y2loc),2));
            double e6 = (-0.5*pow((x2loc-x3loc),2) + 0.25*pow((y2loc-y3loc),2)) / (pow((x2loc-x3loc),2) + pow((y2loc-y3loc),2));
            double e7 = (-0.5*pow((x3loc-x4loc),2) + 0.25*pow((y3loc-y4loc),2)) / (pow((x3loc-x4loc),2) + pow((y3loc-y4loc),2));
            double e8 = (-0.5*pow((x4loc-x1loc),2) + 0.25*pow((y4loc-y1loc),2)) / (pow((x4loc-x1loc),2) + pow((y4loc-y1loc),2));



            // values for Hx derivatives as presented in the paper Batoz, Taher
            double Hx1ksi = (3.0/2) * ((a5*N5ksi)-(a8*N8ksi));
            double Hx2ksi = (b5*N5ksi) + (b8*N8ksi);
            double Hx3ksi = N1ksi - (c5*N5ksi) - (c8*N8ksi);

            double Hx4ksi = (3.0/2) * ((a6*N6ksi)-(a5*N5ksi));
            double Hx5ksi = (b6*N6ksi) + (b5*N5ksi);
            double Hx6ksi = N2ksi - (c6*N6ksi) - (c5*N5ksi);

            double Hx7ksi = (3.0/2) * ((a7*N7ksi)-(a6*N6ksi));
            double Hx8ksi = (b7*N7ksi) + (b6*N6ksi);
            double Hx9ksi = N3ksi - (c7*N7ksi) - (c6*N6ksi);

            double Hx10ksi = (3.0/2) * ((a8*N8ksi)-(a7*N7ksi));
            double Hx11ksi = (b8*N8ksi) + (b7*N7ksi);
            double Hx12ksi = N4ksi - (c8*N8ksi) - (c7*N7ksi);
            //
            double Hx1eta = (3.0/2) * ((a5*N5eta)-(a8*N8eta));
            double Hx2eta = (b5*N5eta) + (b8*N8eta);
            double Hx3eta = N1eta - (c5*N5eta) - (c8*N8eta);

            double Hx4eta = (3.0/2) * ((a6*N6eta)-(a5*N5eta));
            double Hx5eta = (b6*N6eta) + (b5*N5eta);
            double Hx6eta = N2eta - (c6*N6eta) - (c5*N5eta);

            double Hx7eta = (3.0/2) * ((a7*N7eta)-(a6*N6eta));
            double Hx8eta = (b7*N7eta) + (b6*N6eta);
            double Hx9eta = N3eta - (c7*N7eta) - (c6*N6eta);

            double Hx10eta = (3.0/2) * ((a8*N8eta)-(a7*N7eta));
            double Hx11eta = (b8*N8eta) + (b7*N7eta);
            double Hx12eta = N4eta - (c8*N8eta) - (c7*N7eta);

            // values for Hx derivatives as presented in the paper Batoz, Taher
            double Hy1ksi = (3.0/2) * ((d5*N5ksi)-(d8*N8ksi));
            double Hy2ksi = -N1ksi + (e5*N5ksi) + (e8*N8ksi);
            double Hy3ksi = -(b5*N5ksi) - (b8*N8ksi);

            double Hy4ksi = (3.0/2) * ((d6*N6ksi)-(d5*N5ksi));
            double Hy5ksi = -N2ksi + (e6*N6ksi) + (e5*N5ksi);
            double Hy6ksi = -(b6*N6ksi) - (b5*N5ksi);

            double Hy7ksi = (3.0/2) * ((d7*N7ksi)-(d6*N6ksi));
            double Hy8ksi = -N3ksi + (e7*N7ksi) + (e6*N6ksi);
            double Hy9ksi = -(b7*N7ksi) - (b6*N6ksi);

            double Hy10ksi = (3.0/2) * ((d8*N8ksi)-(d7*N7ksi));
            double Hy11ksi = -N4ksi + (e8*N8ksi) + (e7*N7ksi);
            double Hy12ksi = -(b8*N8ksi) - (b7*N7ksi);

            double Hy1eta = (3.0/2) * ((d5*N5eta)-(d8*N8eta));
            double Hy2eta = -N1eta + (e5*N5eta) + (e8*N8eta);
            double Hy3eta = -(b5*N5eta) - (b8*N8eta);

            double Hy4eta = (3.0/2) * ((d6*N6eta)-(d5*N5eta));
            double Hy5eta = -N2eta + (e6*N6eta) + (e5*N5eta);
            double Hy6eta = -(b6*N6eta) - (b5*N5eta);

            double Hy7eta = (3.0/2) * ((d7*N7eta)-(d6*N6eta));
            double Hy8eta = -N3eta + (e7*N7eta) + (e6*N6eta);
            double Hy9eta = -(b7*N7eta) - (b6*N6eta);

            double Hy10eta = (3.0/2) * ((d8*N8eta)-(d7*N7eta));
            double Hy11eta = -N4eta + (e8*N8eta) + (e7*N7eta);
            double Hy12eta = -(b8*N8eta) - (b7*N7eta);


            // matrix B
            Eigen::MatrixXd B;
            B.setZero(3,12);
            //
            B(0,0)  = (j11*Hx1ksi) + (j12*Hx1eta);
            B(0,1)  = (j11*Hx2ksi) + (j12*Hx2eta);
            B(0,2)  = (j11*Hx3ksi) + (j12*Hx3eta);
            B(0,3)  = (j11*Hx4ksi) + (j12*Hx4eta);
            B(0,4)  = (j11*Hx5ksi) + (j12*Hx5eta);
            B(0,5)  = (j11*Hx6ksi) + (j12*Hx6eta);
            B(0,6)  = (j11*Hx7ksi) + (j12*Hx7eta);
            B(0,7)  = (j11*Hx8ksi) + (j12*Hx8eta);
            B(0,8)  = (j11*Hx9ksi) + (j12*Hx9eta);
            B(0,9)  = (j11*Hx10ksi) + (j12*Hx10eta);
            B(0,10) = (j11*Hx11ksi) + (j12*Hx11eta);
            B(0,11) = (j11*Hx12ksi) + (j12*Hx12eta);
            //
            B(1,0)  = (j21*Hy1ksi) + (j22*Hy1eta);
            B(1,1)  = (j21*Hy2ksi) + (j22*Hy2eta);
            B(1,2)  = (j21*Hy3ksi) + (j22*Hy3eta);
            B(1,3)  = (j21*Hy4ksi) + (j22*Hy4eta);
            B(1,4)  = (j21*Hy5ksi) + (j22*Hy5eta);
            B(1,5)  = (j21*Hy6ksi) + (j22*Hy6eta);
            B(1,6)  = (j21*Hy7ksi) + (j22*Hy7eta);
            B(1,7)  = (j21*Hy8ksi) + (j22*Hy8eta);
            B(1,8)  = (j21*Hy9ksi) + (j22*Hy9eta);
            B(1,9)  = (j21*Hy10ksi) + (j22*Hy10eta);
            B(1,10) = (j21*Hy11ksi) + (j22*Hy11eta);
            B(1,11) = (j21*Hy12ksi) + (j22*Hy12eta);
            //
            B(2,0)  = (j11*Hy1ksi) + (j12*Hy1eta) + (j21*Hx1ksi) + (j22*Hx1eta);
            B(2,1)  = (j11*Hy2ksi) + (j12*Hy2eta) + (j21*Hx2ksi) + (j22*Hx2eta);
            B(2,2)  = (j11*Hy3ksi) + (j12*Hy3eta) + (j21*Hx3ksi) + (j22*Hx3eta);
            B(2,3)  = (j11*Hy4ksi) + (j12*Hy4eta) + (j21*Hx4ksi) + (j22*Hx4eta);
            B(2,4)  = (j11*Hy5ksi) + (j12*Hy5eta) + (j21*Hx5ksi) + (j22*Hx5eta);
            B(2,5)  = (j11*Hy6ksi) + (j12*Hy6eta) + (j21*Hx6ksi) + (j22*Hx6eta);
            B(2,6)  = (j11*Hy7ksi) + (j12*Hy7eta) + (j21*Hx7ksi) + (j22*Hx7eta);
            B(2,7)  = (j11*Hy8ksi) + (j12*Hy8eta) + (j21*Hx8ksi) + (j22*Hx8eta);
            B(2,8)  = (j11*Hy9ksi) + (j12*Hy9eta) + (j21*Hx9ksi) + (j22*Hx9eta);
            B(2,9)  = (j11*Hy10ksi) + (j12*Hy10eta) + (j21*Hx10ksi) + (j22*Hx10eta);
            B(2,10) = (j11*Hy11ksi) + (j12*Hy11eta) + (j21*Hx11ksi) + (j22*Hx11eta);
            B(2,11) = (j11*Hy12ksi) + (j12*Hy12eta) + (j21*Hx12ksi) + (j22*Hx12eta);


            // stiffness matrix for bending
            Kben += B.transpose() * Db * B * J.determinant();
            } // end for m (ksi/eta)
        } // end for l (ksi/eta)

        // fill the found values in Kben into the stiffness matrix for bending action
        for (int m=0;m<4;m++)
        {
            for (int n=0;n<4;n++)
            {
                // for 3 x 3 matrix elements
                for (int p=0;p<3;p++)
                {
                    for (int q=0; q<3;q++)
                    {
                        m_SM_bending(2+6*m+p,2+6*n+q) = Kben(3*m+p,3*n+q);
                    }
                }
            }
        }

        // compose stiffness matrix out of normal and bending stiffness matrices
        m_SM.setZero(24,24);
        m_SM = m_SM_bending + m_SM_normal;

        // add drilling stiffness to the element
        m_SM(5,5)   = m_SM.mean(); // add drilling terms to the 6th dof of the local stiffness matrix
        m_SM(11,11) = m_SM(5,5);
        m_SM(17,17) = m_SM(5,5);
        m_SM(23,23) = m_SM(5,5);

        // transform element stiffness matrices to global coordinate system
        m_SM = m_T.transpose() * m_SM * m_T;
        m_original_SM = m_SM;

        // also transform the bending and normal action stiffness amtrices
        m_SM_bending = m_T.transpose() * m_SM_bending * m_T;
        m_SM_normal = m_T.transpose() * m_SM_normal * m_T;
        m_SM_axial = m_T.transpose() * m_SM_axial * m_T;
        m_SM_shear = m_T.transpose() * m_SM_shear * m_T;

    } // ctor

    Flat_Shell::~Flat_Shell()
    {

    } // dtor

    void Flat_Shell::calc_energies(const std::vector<unsigned int>& load_cases)
    {
        Element::calc_energies(load_cases);
        m_bending_energy = 0;
        m_normal_energy = 0;
        m_shear_energy = 0;
        m_axial_energy = 0;

        for (auto lc : load_cases)
        {
            m_bending_energy += 0.5 * m_displacements[lc].transpose() * m_SM_bending * m_displacements[lc];
            m_normal_energy += 0.5 * m_displacements[lc].transpose() * m_SM_normal * m_displacements[lc];
            m_shear_energy += 0.5 * m_displacements[lc].transpose() * m_SM_shear * m_displacements[lc];
            m_axial_energy += 0.5 * m_displacements[lc].transpose() * m_SM_axial * m_displacements[lc];
        }

    } // calc_energies

    double Flat_Shell::get_normal_energy()
    {
        return m_normal_energy;
    } // get_normal_energy()

    double Flat_Shell::get_bending_energy()
    {
        return m_bending_energy;
    } // get_bending_energy()

    double Flat_Shell::get_shear_energy()
    {
        return m_shear_energy;
    } // get_shear_energy()

    double Flat_Shell::get_axial_energy()
    {
        return m_axial_energy;
    } // get_axial_energy()

    double Flat_Shell::get_property(unsigned int n)
    {
        switch(n)
        {
        case 1:
            return m_t;
            break;
        case 2:
            return m_E;
            break;
        case 3:
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



#endif // FLAT_SHELL_ELE_HPP
