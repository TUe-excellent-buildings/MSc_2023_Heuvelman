#ifndef TOPOPT_SIMP_OLD_HPP
#define TOPOPT_SIMP_OLD_HPP

#include <BSO/Structural_Design/Analysis_Tools/FEA.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace BSO { namespace Structural_Design {

    void topopt_SIMP_old(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol)
    {
        unsigned int num_el = fea_ptr->get_element_count();
        double total_volume = 0; // initialised at 0, before each element's volume is added
        double c; // sum of all the elements compliances (objective value)

double min_diag = 10000;

for (unsigned int i = 0; i < num_el; i++)
{
    if (min_diag > fea_ptr->get_element_ptr(i)->get_diagonal())
        min_diag = fea_ptr->get_element_ptr(i)->get_diagonal();
}

r_min = min_diag;

        Eigen::VectorXd x(num_el), x_new(num_el), x_change(num_el),
                        volume(num_el), dc(num_el), dv(num_el); // initialising containers for element values

        // prepare filter
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
            fea_ptr->get_element_ptr(i)->update_density_old(f, penal);

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
        std::cout << "Total Volume: " << total_volume << std::endl;

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
                std::cout << std::endl
                          << std::setw(5)  << std::left << "loop"
                          << std::setw(15) << std::left << "Objective"
                          << std::setw(15) << std::left << "Volume"
                          << std::setw(15) << std::left << "Change"
                          << std::setw(10) << std::left << "Time" << std::endl;
            }


            loop++;
            c = 0;

            // FEA
            fea_ptr->generate_GSM();
            fea_ptr->solve();

            // objective function and sensitivity analysis (retrieve data from FEA)
            for (unsigned int i = 0; i < num_el; i++)
            {
                c += fea_ptr->get_element_ptr(i)->get_energy() * 2; // factor 2 to let the term 1/2 in 1/2F*U vanish
                dc(i) = fea_ptr->get_element_ptr(i)->get_energy_sensitivity(penal) * 2; // factor 2 to let the term 1/2 in 1/2F*U vanish
                dv(i) = fea_ptr->get_element_ptr(i)->get_volume_sensitivity();
            }

            dc = H * (dc * x.transpose()).diagonal();
            for (unsigned int i = 0; i < num_el; i++)
            {
                dc(i) /= (Hs(i) * std::max(1e-3, x(i)));
            }

            // optimality criteria update of design variables and physical densities
            double l1 = 0, l2 = 50000, lmid, upper, lower;
            while (((l2-l1)/(l1+l2))>1e-3)
            {
                lmid = (l1+l2)/2.0;

                for (unsigned int i = 0; i < num_el; i++)
                {
                    x_new(i) = x(i) * (std::sqrt(-dc(i)/(lmid*dv(i))));
                    upper = std::min(1.0, (x(i) + x_move));
                    lower = std::max(0.001, (x(i) - x_move));

                    if (x_new(i) > upper)
                    {
                        x_new(i) = upper;
                    }
                    else if (x_new(i) < lower)
                    {
                        x_new(i) = lower;
                    }
                }
((x_new.transpose()).sum()/num_el > f) ? l1 = lmid : l2 = lmid; // this is how it is done in the old toolbox
                //((volume * x_new.transpose()).trace() > f * total_volume) ? l1 = lmid : l2 = lmid;
            }

            for (unsigned int i = 0; i < num_el; i++)
            {
                fea_ptr->get_element_ptr(i)->update_density_old(x_new(i), penal);
            }

            // update change
            x_change = x_new - x;
            change = x_change.cwiseAbs().maxCoeff();

            time_end = clock();
            std::cout << std::setw(5)  << std::left << loop
                      << std::setw(15) << std::left << c
                      << std::setw(15) << std::left << (volume * x_new.transpose()).trace()
                      << std::setw(15) << std::left << change
                      << std::setw(10) << std::left << (time_end - iteration_start)/CLOCKS_PER_SEC << std::endl;

            x = x_new;
        } // end of loop
        std::cout << "Topology optimisation successfully finished after: "
                  << (time_end - loop_start)/CLOCKS_PER_SEC << " seconds."
                  << std::endl << std::endl;

std::ofstream out("Densities.txt");

typedef std::map<unsigned long, Elements::Node*>::iterator node_iterator;
for (node_iterator ite = fea_ptr->m_node_map.begin(); ite != fea_ptr->m_node_map.end(); ite++)
{
    out << "N,"
        << ite->first << ","
        << ite->second->get_coord()(0) << ","
        << ite->second->get_coord()(1) << ","
        << ite->second->get_coord()(2) << std::endl;

}
out << std::endl << std::endl;
for (int i = 0; i < num_el; i++)
{
    out << "DE,"
        << fea_ptr->m_elements[i]->get_node_ID(0) << ","
        << fea_ptr->m_elements[i]->get_node_ID(1) << ","
        << fea_ptr->m_elements[i]->get_node_ID(2) << ","
        << fea_ptr->m_elements[i]->get_node_ID(3) << ","
        << fea_ptr->m_elements[i]->get_density( ) << std::endl;
}

out.close();
    } // topopt_SIMP_old()

} // namespace Structural_Design
} // namespace BSO



#endif // TOPOPT_SIMP_OLD_HPP
