#ifndef TOPOPT_SIMP_DIFF_ELEMENTS_HPP
#define TOPOPT_SIMP_DIFF_ELEMENTS_HPP

#include <BSO/Structural_Design/Analysis_Tools/FEA.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include <fstream>

namespace BSO { namespace Structural_Design {

    void H_init(Eigen::SparseMatrix<double>& H, Eigen::VectorXd& Hs, std::vector<Elements::Element*>& elements, Eigen::VectorXd& x, Eigen::VectorXd& volume, double f, double r_min, double penal);
    void obj_and_sens(std::vector<Elements::Element*>& elements, double& c, Eigen::VectorXd& x,  Eigen::VectorXd& dc, Eigen::VectorXd& dv, Eigen::SparseMatrix<double>& H, Eigen::VectorXd& Hs, double penal);
    void opt_crit_upd(double l1, double l2, double x_move, double& total_volume, double f, Eigen::VectorXd& volume, Eigen::VectorXd& x, Eigen::VectorXd& x_new, Eigen::VectorXd& dv, Eigen::VectorXd& dc, bool min_density_non_zero);

    void topopt_SIMP_diff_elements(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol)
    {
        unsigned int num_el = fea_ptr->get_element_count();
        std::vector<Elements::Element*> element_ptrs;
        std::vector<Elements::Element*> elements_f, elements_b, elements_t;
        double tot_vol_f = 0, tot_vol_b = 0, tot_vol_t = 0; // initialised at 0, before each element volumes are added
        double c_f, c_t, c_b; // sum of all the elements compliances (objective value)

        for (unsigned int i = 0; i < num_el; i++)
        {
            Elements::Element* temp_ptr = fea_ptr->get_element_ptr(i);
            element_ptrs.push_back(temp_ptr);
            if (temp_ptr->is_flat_shell() && temp_ptr->is_active_in_compliance())
            {
                elements_f.push_back(temp_ptr);
                tot_vol_f += temp_ptr->get_volume();
            }
            else if (temp_ptr->is_beam() && temp_ptr->is_active_in_compliance())
            {
                elements_b.push_back(temp_ptr);
                tot_vol_b += temp_ptr->get_volume();
            }
            else if (temp_ptr->is_truss() && temp_ptr->is_active_in_compliance())
            {
                elements_t.push_back(temp_ptr);
                tot_vol_t += temp_ptr->get_volume();
            }
        }
        unsigned int num_f = elements_f.size(), num_b = elements_b.size(), num_t = elements_t.size();


        Eigen::VectorXd x_f(num_f),         x_b(num_b),         x_t(num_t),
                        x_f_new(num_f),     x_b_new(num_b),     x_t_new(num_t),
                        x_f_change(num_f),  x_b_change(num_b),  x_t_change(num_t),
                        vol_f(num_f),       vol_b(num_b),       vol_t(num_t),
                        dc_f(num_f),        dc_b(num_b),        dc_t(num_t),
                        dv_f(num_f),        dv_b(num_b),        dv_t(num_t); // initialise containers for element values
                        x_f_change.setZero();   x_b_change.setZero();   x_t_change.setZero();

        // prepare filter
        Eigen::SparseMatrix<double> H_f(num_f, num_f),      H_b(num_b, num_b),      H_t(num_t, num_t); // contains filter vectors for each element
        Eigen::VectorXd             Hs_f(num_f),            Hs_b(num_b),            Hs_t(num_t); // contains sums of filter vectors of each element
                                    Hs_f.setZero();         Hs_b.setZero();         Hs_t.setZero(); // initialise to size and all zeros

        H_init(H_f, Hs_f, elements_f, x_f, vol_f, f, r_min, penal);
        H_init(H_b, Hs_b, elements_b, x_b, vol_b, f, r_min, penal);
        H_init(H_t, Hs_t, elements_t, x_t, vol_t, f, r_min, penal);

        // initialise iteration
        double change = 1;
        int loop = 0;

        double loop_start = clock(), iteration_start, time_end = 0.0;

        // start iteration
        while (change > tol)
        {
            iteration_start = clock();
            if (loop%20 == 0)
            {
                std::cout << std::endl << "Starting topology optimisation with separate volume constraints" << std::endl;
                std::cout << "Design volume: " << tot_vol_f + tot_vol_t + tot_vol_b << std::endl;
                std::cout << std::setw(5)  << std::left << "loop"
                          << std::setw(15) << std::left << "Objective"
                          << std::setw(15) << std::left << "Volume"
                          << std::setw(15) << std::left << "Change"
                          << std::setw(10) << std::left << "Time" << std::endl;
            }

            loop++;
            c_f = c_b = c_t = 0;

            // FEA
            fea_ptr->generate_GSM();
            fea_ptr->solve();

            double volume = 0;
            if (elements_f.size() > 0)
            {
                obj_and_sens(elements_f, c_f, x_f,  dc_f, dv_f, H_f, Hs_f, penal);
                opt_crit_upd(0, 1e9, x_move, tot_vol_f, f, vol_f, x_f, x_f_new, dv_f, dc_f, false);
                for (unsigned int i = 0; i < elements_f.size(); i++)
                    elements_f[i]->update_density(x_f_new(i), penal);
                x_f_change = x_f_new - x_f;
                volume += (vol_f* x_f_new.transpose()).trace();
            }
            if (elements_b.size() > 0)
            {
                obj_and_sens(elements_b, c_b, x_b,  dc_b, dv_b, H_b, Hs_b, penal);
                opt_crit_upd(0, 1e9, x_move, tot_vol_b, f, vol_b, x_b, x_b_new, dv_b, dc_b, false);
                for (unsigned int i = 0; i < elements_b.size(); i++)
                    elements_b[i]->update_density(x_b_new(i), penal);
                x_b_change = x_b_new - x_b;
                volume += (vol_b* x_b_new.transpose()).trace();
            }
            if (elements_t.size() > 0)
            {
                obj_and_sens(elements_t, c_t, x_t,  dc_t, dv_t, H_t, Hs_t, penal);
                opt_crit_upd(0, 1e9, x_move, tot_vol_t, f, vol_t, x_t, x_t_new, dv_t, dc_t, true);
                for (unsigned int i = 0; i < elements_t.size(); i++)
                    elements_t[i]->update_density(x_t_new(i), penal);
                x_t_change = x_t_new - x_t;
                volume += (vol_t* x_t_new.transpose()).trace();
            }

            // update change
            change = 0;
            if (elements_f.size() > 0) change = std::max(change, x_f_change.cwiseAbs().maxCoeff());
            if (elements_b.size() > 0) change = std::max(change, x_b_change.cwiseAbs().maxCoeff());
            if (elements_t.size() > 0) change = std::max(change, x_t_change.cwiseAbs().maxCoeff());


            time_end = clock();
            std::cout << std::setw(5)  << std::left << loop
                      << std::setw(15) << std::left << c_f+c_b+c_t
                      << std::setw(15) << std::left << volume
                      << std::setw(15) << std::left << change
                      << std::setw(10) << std::left << (time_end - iteration_start)/CLOCKS_PER_SEC << std::endl;

            x_f = x_f_new;
            x_b = x_b_new;
            x_t = x_t_new;
        } // end of iteration
        std::cout << "Topology optimisation successfully finished after: "
                  << (time_end - loop_start)/CLOCKS_PER_SEC << " seconds."
                  << std::endl << std::endl;

    } // topopt_SIMP()

    void H_init(Eigen::SparseMatrix<double>& H, Eigen::VectorXd& Hs, std::vector<Elements::Element*>& elements, Eigen::VectorXd& x, Eigen::VectorXd& volume, double f, double r_min, double penal)
    {
        typedef Eigen::Triplet<double> T;
        std::vector<T> triplet_list;

        for (unsigned int i = 0; i < elements.size(); i++)
        { // for each element i
            double r_ij; // distance from center of element i to center of element j

            volume(i) = elements[i]->get_volume();

            x(i) = f;
            elements[i]->update_density(f, penal);

            for (unsigned int j = 0; j < elements.size(); j++)
            { // and for each element j
                // calculate distance center to center distance r_ij between element i and j
                r_ij = BSO::Vectors::length(elements[j]->get_center_coord()
                                            - elements[i]->get_center_coord());

                if (r_ij < r_min)
                {
                    triplet_list.push_back(T(i, j, (r_min - r_ij)));
                    Hs(i) += r_min - r_ij;
                }
            }
        }
        H.setFromTriplets(triplet_list.begin(), triplet_list.end());
        triplet_list.clear();
    }

    void obj_and_sens(std::vector<Elements::Element*>& elements, double& c, Eigen::VectorXd& x,  Eigen::VectorXd& dc, Eigen::VectorXd& dv, Eigen::SparseMatrix<double>& H, Eigen::VectorXd& Hs, double penal)
    {
        for (unsigned int i = 0; i < elements.size(); i++)
        {
            c += elements[i]->get_energy() * 2; // factor 2 to let the term 1/2 in 1/2F*U vanish
            dc(i) = elements[i]->get_energy_sensitivity(penal) * 2; // factor 2 to let the term 1/2 in 1/2F*U vanish
            dv(i) = elements[i]->get_volume_sensitivity();
        }

        dc = (dc * x.transpose()).diagonal();
        dc = H * dc;
        for (unsigned int i = 0; i < elements.size(); i++)
        {
            dc(i) /= (Hs(i) * std::max(1e-3, x(i)));
        }
    }

    void opt_crit_upd(double l1, double l2, double x_move, double& total_volume, double f, Eigen::VectorXd& volume, Eigen::VectorXd& x, Eigen::VectorXd& x_new, Eigen::VectorXd& dv, Eigen::VectorXd& dc, bool min_density_non_zero)
    {
        double lmid, upper, lower;
        while (((l2-l1)/(l1+l2))>1e-3)
        {
            lmid = (l1+l2)/2.0;

            for (unsigned int i = 0; i < x.size(); i++)
            {
                x_new(i) = x(i) * (std::sqrt(-dc(i)/(lmid*dv(i))));
                upper = std::min(1.0, (x(i) + x_move));
                if (!min_density_non_zero) lower = std::max(0.0, (x(i) - x_move));
				else lower = std::max(0.01, (x(i) - x_move));

                if (x_new(i) > upper)
                {
                    x_new(i) = upper;
                }
                else if (x_new(i) < lower)
                {
                    x_new(i) = lower;
                }
            }

            ((volume * x_new.transpose()).trace() > f * total_volume) ? l1 = lmid : l2 = lmid;
        }
    }

} // namespace Structural_Design
} // namespace BSO



#endif // TOPOPT_SIMP_DIFF_ELEMENTS_HPP
