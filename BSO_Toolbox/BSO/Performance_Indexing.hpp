#ifndef PERFORMANCE_INDEXING_HPP
#define PERFORMANCE_INDEXING_HPP

#include <iostream>

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Results.hpp>
#endif

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Results.hpp>
#endif

namespace BSO {
    /*
     * Structures to store results of a BP_simulation
     */
	#ifdef BP_SIMULATION_HPP
    void BP_thermal_demand_indexing(Building_Physics::BP_Building_Results& BP)
    {
        double max_performance = BP.m_space_results[0].m_total_energy / BP.m_space_results[0].m_floor_area;
        double min_performance = max_performance;

        for (unsigned int i = 1; i < BP.m_space_results.size(); i++)
        {
            double current_performance = BP.m_space_results[i].m_total_energy / BP.m_space_results[i].m_floor_area;
            if(current_performance > max_performance)
            {
                max_performance = current_performance;
            }
            else if (current_performance < min_performance)
            {
                min_performance = current_performance;
            }
        }

        if (max_performance != min_performance)
        {
            for (unsigned int i = 0; i < BP.m_space_results.size(); i++)
            {
                double current_performance = BP.m_space_results[i].m_total_energy / BP.m_space_results[i].m_floor_area;
                BP.m_space_results[i].m_rel_performance = ((max_performance - current_performance) / (max_performance - min_performance));
            }
        }
        else
        {
            for (unsigned int i = 0; i < BP.m_space_results.size(); i++)
            {
                BP.m_space_results[i].m_rel_performance = 1.0;
            }
        }
    }
	#endif
	
	#ifdef SD_ANALYSIS_HPP
    void SD_removed_mass_indexing(Structural_Design::SD_Building_Results& SD)
    {
        double sum_volume; // sum of element volumes encountered in a space so far
        double element_volume; // temp container for element volume
        double sum_reduced_volume; // sum of reduced (density) element volumes encountered in a space so far
        for (unsigned int i = 0; i < SD.m_spaces.size(); i++)
        { // for each space
            sum_volume = 0; // reset counters
            sum_reduced_volume = 0;

            for (unsigned int j = 0; j < SD.m_spaces[i].m_components.size(); j++)
            { // for each component in space 'i'
                for (unsigned int k = 0; k < SD.m_spaces[i].m_components[j]->m_elements.size(); k++)
                { // for each element in component 'j'
                    element_volume = SD.m_spaces[i].m_components[j]->m_elements[k].m_volume; // the element's volume
                    sum_volume += element_volume; // the sum of volumes of all elements encountered so far
                    sum_reduced_volume += element_volume * SD.m_spaces[i].m_components[j]->m_elements[k].m_density; // the sum of reduced element volumes encountered so far
                }
            }
            SD.m_spaces[i].m_rel_performance = sum_reduced_volume / sum_volume; // the relative amount of remaining mass in the space after topology optimisation
        }
    } // relative_removed_mass_indexing()
	#endif
	
	#ifdef SD_ANALYSIS_HPP
    void SD_compliance_indexing(Structural_Design::SD_Building_Results& SD)
    {
        double max_compliance = SD.m_spaces[0].m_total_compliance / SD.m_spaces[0].m_floor_area;
        double min_compliance = max_compliance;

        for (unsigned int i = 1; i < SD.m_spaces.size(); i++)
        {
            double current_compliance = SD.m_spaces[i].m_total_compliance / SD.m_spaces[i].m_floor_area;
            if(current_compliance > max_compliance)
            {
                max_compliance = current_compliance;
            }
            else if (current_compliance < min_compliance)
            {
                min_compliance = current_compliance;
            }
        }

        if (max_compliance != min_compliance)
        {
            for (unsigned int i = 0; i < SD.m_spaces.size(); i++)
            {
                double current_compliance = SD.m_spaces[i].m_total_compliance / SD.m_spaces[i].m_floor_area;
                SD.m_spaces[i].m_rel_performance = ((current_compliance - min_compliance) / (max_compliance - min_compliance));
            }
        }
        else
        {
            for (unsigned int i = 0; i < SD.m_spaces.size(); i++)
            {
                SD.m_spaces[i].m_rel_performance = 1.0;
            }
        }
    } // compliance_indexing()
	#endif
	
	#ifdef SD_ANALYSIS_HPP
    void SD_removed_element_indexing(Structural_Design::SD_Building_Results& SD, unsigned int n)
    {
        // remove first 'n' element clusters
        SD.delete_clusters(n);

        SD.m_spaces[0].get_deletion_count();
        unsigned int min = SD.m_spaces[0].m_deletion_count; // to hold min number of deleted elements of 1 of all spaces
        unsigned int max = min; // to hold max number of deleted elements of 1 of all spaces

        // count the amount of deleted elements in each space and find the min and max counts
        for (unsigned int i = 1; i < SD.m_spaces.size(); i++)
        {
            SD.m_spaces[i].get_deletion_count();
            double count = SD.m_spaces[i].m_deletion_count;

            if (min > count)
            {
                min = count;
            }
            else if (max < count)
            {
                max = count;
            }
        }

        // compute the relative indices of each space
        for (unsigned int i = 0; i < SD.m_spaces.size(); i++)
        {
            SD.m_spaces[i].m_rel_performance = (double)(max - SD.m_spaces[i].m_deletion_count) / (double)(max - min);
        }

    } // removed_element_indexing()
	#endif


} // namespace BSO

#endif // PERFORMANCE_INDEXING_HPP
