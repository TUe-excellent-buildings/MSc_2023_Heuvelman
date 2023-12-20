#ifndef TOPOPT_ROBUST_HPP
#define TOPOPT_ROBUST_HPP

#include <BSO/Structural_Design/Analysis_Tools/FEA.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>

namespace BSO { namespace Structural_Design {

    double density_projection(double beta, double eta, double x)
    {
        return (tanh(beta * eta) + tanh(beta * (x - eta))) /
               (tanh(beta * eta) + tanh(beta * (1 - eta)));
    }

    void topopt_robust(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol)
    {
        unsigned int num_el = fea_ptr->get_element_count();
        double Mnd;
        double beta = 1.0;
        double eta_e = 0.8;
        double eta_n = 0.5;
        double total_volume = 0; // initialised at 0, before each element volumes are added
        double c; // sum of all the elements compliances (objective value)
        Eigen::VectorXd xe(num_el), xn(num_el), x(num_el), x_tilde(num_el), x_new(num_el),
                        x_change(num_el), volume(num_el), dc(num_el), dv(num_el); // initialise containers for element values

        // prepare filter and initialise densities
        Eigen::SparseMatrix<double> H(num_el, num_el); // contains filter vectors for each element
        Eigen::VectorXd Hs; // contains sums of filter vectors of each element
        Hs.setZero(num_el); // initialise to size and all zeros
        typedef Eigen::Triplet<double> T;
        std::vector<T> triplet_list;

        for (unsigned int i = 0; i < num_el; i++)
        { // for each element i
            double r_ij; // distance from center of element i to center of element j

            volume(i) = fea_ptr->get_element_ptr(i)->get_volume();

            x(i) = f;
            fea_ptr->get_element_ptr(i)->update_density(xe(i), penal);

            for (unsigned int j = 0; j < num_el; j++)
            { // and for each element j
                // calculate distance center to center distance r_ij between element i and j
                r_ij = BSO::Vectors::length(fea_ptr->get_element_ptr(j)->get_center_coord()
                                            - fea_ptr->get_element_ptr(i)->get_center_coord());

                if (r_ij < r_min)
                {
                    triplet_list.push_back(T(i, j, (r_min - r_ij)));
                    Hs(i) += r_min - r_ij;
                }
            }
        }
        H.setFromTriplets(triplet_list.begin(), triplet_list.end());
        total_volume = volume.sum();
        triplet_list.clear();

        x_tilde = x;

        for (unsigned int i = 0; i < num_el; i++)
        {
            xn(i) = density_projection(beta, eta_n, x_tilde(i));
            xe(i) = density_projection(beta, eta_e, x_tilde(i));
        }

        // initialise iteration
        double change = 1;
        double x_move_beta = (x_move*tanh(0.5*beta))/(0.5 * beta);
        int loop = 0;
        int loopbeta = 0;

        double loop_start = clock(), iteration_start, time_end = 0.0;

        std::cout << std::endl
                  << std::setw(5)  << std::left << "loop"
                  << std::setw(10) << std::left << "loop_beta"
                  << std::setw(15) << std::left << "Objective"
                  << std::setw(15) << std::left << "Volume"
                  << std::setw(10)  << std::left << "Change"
                  << std::setw(10)  << std::left << "Mnd"
                  << std::setw(10)  << std::left << "Time" << std::endl;

        // start iteration
        while (change > tol)
        {
            iteration_start = clock();

            loop++;
            loopbeta++;
            c = 0;

            // FEA
            fea_ptr->generate_GSM();
            fea_ptr->solve();

            // objective function and sensitivity analysis (retrieve data from FEA)
            for (unsigned int i = 0; i < num_el; i++)
            {
                c += fea_ptr->get_element_ptr(i)->get_energy() * 2; // factor 2 to let the term 1/2 in 1/2F*U vanish
                // dc will here become dc/dxe (same of r dv)
                dc(i) = fea_ptr->get_element_ptr(i)->get_energy_sensitivity(penal) * 2; // factor 2 to let the term 1/2 in 1/2F*U vanish
                dv(i) = fea_ptr->get_element_ptr(i)->get_volume_sensitivity();
            }

            for (unsigned int i = 0; i < num_el; i++)
            {
                // dc is now dc/dx_tilde and part of dx_tilde/dx is accounted for (same for dv)
                dc(i) *= ((beta*pow(1/cosh(beta*(x_tilde(i)-eta_e)),2))/(tanh(beta*eta_e)+tanh(beta*(1-eta_e)))) / Hs(i);
                dv(i) *= ((beta*pow(1/cosh(beta*(x_tilde(i)-eta_n)),2))/(tanh(beta*eta_n)+tanh(beta*(1-eta_n)))) / Hs(i);
            }

            // rest of dx_tilde/dx is accounted for
            dc = H * dc;
            dv = H * dv;

            // chain rule is now complete --> dc is now dc/dx and dv is now dv/dx

            // optimality criteria update of design variables and physical densities
            double l1 = 0, l2 = 1e9, lmid, upper, lower;
            while (((l2-l1)/(l1+l2))>1e-3)
            {
                lmid = (l1+l2)/2.0;

                // calculate new densities (within constraints)
                for (unsigned int i = 0; i < num_el; i++)
                {
                    x_new(i) = x(i) * (std::sqrt(-dc(i)/(lmid*dv(i))));
                    upper = std::min(1.0, (x(i) + x_move_beta));
                    lower = std::max(0.0, (x(i) - x_move_beta));

                    if (x_new(i) > upper)
                    {
                        x_new(i) = upper;
                    }
                    else if (x_new(i) < lower)
                    {
                        x_new(i) = lower;
                    }
                }

                // filter the new densities
                x_tilde = H * x_new;

                for (unsigned int i = 0; i < num_el; i++)
                {// for each element
                    // finish filtering
                    x_tilde(i) /= Hs(i);
                    // calculate nominal and erode densities
                    xn(i) = density_projection(beta, eta_n, x_tilde(i));
                    xe(i) = density_projection(beta, eta_e, x_tilde(i));
                }

                // check volume constraint
                ((volume * xn.transpose()).trace() > f * total_volume) ? l1 = lmid : l2 = lmid;
            }

            // update densities in the finite element calculation
            for (unsigned int i = 0; i < num_el; i++)
            {
                fea_ptr->get_element_ptr(i)->update_density(xe(i), penal);
            }

            // update change
            x_change = x_new - x;
            change = x_change.cwiseAbs().maxCoeff();

            x = x_new;

            // calculate measure of non-discreteness
            Mnd = 0.0;
            for (unsigned int i = 0; i < num_el; i++)
            {
                Mnd += xn(i)*(1.0-xn(i));
            }
            Mnd *= ((1.0/(f*(1.0-f)))*(100.0/num_el));

            time_end = clock();
            std::cout << std::setw(5)  << std::left << loop
                      << std::setw(10)  << std::left << loopbeta
                      << std::setw(15) << std::left << c
                      << std::setw(15) << std::left << (volume * x_new.transpose()).trace()
                      << std::setw(10) << std::left << change
                      << std::setw(10) << std::left << Mnd
                      << std::setw(10) << std::left << (time_end - iteration_start)/CLOCKS_PER_SEC << std::endl;

            // update beta
            if ((beta < 256) && ((loopbeta >= 50) || (change <= tol)))
            {
                beta *= 2;
                loopbeta = 0;
                change = 1;
                x_move_beta = (x_move*tanh(0.5*beta))/(0.5 * beta);
                std::cout << "beta increased to: " << beta << std::endl << std::endl;
                std::cout << std::setw(5)  << std::left << "loop"
                          << std::setw(10) << std::left << "loop_beta"
                          << std::setw(15) << std::left << "Objective"
                          << std::setw(15) << std::left << "Volume"
                          << std::setw(10) << std::left << "Change"
                          << std::setw(10) << std::left << "Mnd"
                          << std::setw(10) << std::left << "Time" << std::endl;
            }

        } // end of iteration

        std::cout << "Topology optimisation successfully finished after: "
                  << (time_end - loop_start)/CLOCKS_PER_SEC << " seconds."
                  << std::endl << std::endl;

        for (unsigned int i = 0; i < num_el; i++)
        {
            fea_ptr->get_element_ptr(i)->update_density(xn(i), penal);
        }

    } // topopt_robust()

} // namespace Structural_Design
} // namespace BSO



#endif // TOPOPT_SIMP_HPP
