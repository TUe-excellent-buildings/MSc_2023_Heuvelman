#ifndef HEURISTICS_HPP
#define HEURISTICS_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Clustering.hpp>

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Results.hpp>
#endif

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Results.hpp>
#endif

#include <iostream>
#include <map>
#include <vector>
#include <cmath>

namespace BSO {
    /*
     * Structures to store results of a BP_simulation
     */
	
	#ifdef SD_ANALYSIS_HPP
    Spatial_Design::MS_Building scale_and_subdivide_1(Spatial_Design::MS_Building& current_design,
                                                      Structural_Design::SD_Building_Results& sd_results)
    {
        Spatial_Design::MS_Building new_design = current_design;

        int space_total = current_design.obtain_space_count(); // amount of spaces in the current design
        int space_delete_number = 0;
        double initial_volume = current_design.get_volume();

        std::map<int, unsigned int> sd_performances;
        typedef std::map<int, unsigned int>::iterator space_ite;

        for (unsigned int i = 0; i < sd_results.m_spaces.size(); i++)
        {
            sd_performances[sd_results.m_spaces[i].m_ID] = sd_results.m_spaces[i].m_deletion_count;
        }

        std::vector<int> space_performances_ordered;
        // although described in the thesis, below vectors are not used
        //std::vector<int> space_floor_areas_ordered;
        //std::vector<int> space_coords_ordered;
        //std::vector<int> space_ratios_ordered;

        // create an ordered list that maps space performances to their space ID
        for (space_ite ite = sd_performances.begin(); ite != sd_performances.end(); ite++)
        {
            bool space_inserted = false;
            for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
            {
                if (ite->second > sd_performances[space_performances_ordered[i] ])
                {
                    space_performances_ordered.insert(space_performances_ordered.begin() + i, ite->first);
                    space_inserted = true;
                    break;
                }
            }
            if (!space_inserted)
            { // if the space's performance is not smaller than that of any of the already inserted spaces, then add it to the back of the vector
                space_performances_ordered.push_back(ite->first);
            }
            space_inserted = false;
        }


        unsigned int delete_pos = space_total - space_total/2;
        unsigned int deleted_elements = 0;

        for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
        { // delete the worst performing spaces
            if (i < delete_pos - 1)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (i == delete_pos - 1)
            {
                deleted_elements = sd_performances[space_performances_ordered[i] ];
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (deleted_elements == sd_performances[space_performances_ordered[i] ])
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else
            {
                break;
            }
        }

        // all though described in the thesis, the sorted lists below are not used
/*        // create ordered lists of space ID's
        for (int i = 0; i < new_design.obtain_space_count(); i++)
        { // for each space in the new design
            bool space_inserted = false;
            for (unsigned int j = 0; j < space_floor_areas_ordered.size(); j++)
            { // for each space in this list
                if(new_design.obtain_space(i).get_area() > new_design.obtain_space(space_floor_areas_ordered[j]).get_area())
                {
                    space_floor_areas_ordered.insert(space_floor_areas_ordered.begin() + j, i);
                    space_inserted = true;
                    break;
                }
            }
            if (!space_inserted)
            {
                space_floor_areas_ordered.push_back(i);
            }

            space_inserted = false;
            for (unsigned int j = 0; j < space_ratios_ordered.size(); j++)
            { // for each space in this list
                if(new_design.obtain_space(i).get_aspect_ratio() > new_design.obtain_space(space_floor_areas_ordered[j]).get_aspect_ratio())
                {
                    space_ratios_ordered.insert(space_ratios_ordered.begin() + j, i);
                    space_inserted = true;
                    break;
                }
            }
            if (!space_inserted)
            {
                space_ratios_ordered.push_back(i);
            }

            space_inserted = false;
            for (unsigned int j = 0; j < space_coords_ordered.size(); j++)
            { // for each space in this list
                if (new_design.obtain_space(i).x < new_design.obtain_space(new_design.get_space_index(space_coords_ordered[j])).x)
                { // if the x coordinate of space 'i' is smaller than that of 'j'
                    space_coords_ordered.insert(space_coords_ordered.begin()+j, new_design.obtain_space(i).ID);
                    space_inserted = true;
                    break;
                }
                else if ((new_design.obtain_space(i).x == new_design.obtain_space(new_design.get_space_index(space_coords_ordered[j])).x) &&
                         (new_design.obtain_space(i).y <  new_design.obtain_space(new_design.get_space_index(space_coords_ordered[j])).y))
                { // if the y coordinate of space 'i' is smaller than that of 'j' and the x-coordinates are equal
                    space_coords_ordered.insert(space_coords_ordered.begin()+j, new_design.obtain_space(i).ID);
                    space_inserted = true;
                    break;
                }
                else if ((new_design.obtain_space(i).x == new_design.obtain_space(new_design.get_space_index(space_coords_ordered[j])).x) &&
                         (new_design.obtain_space(i).y == new_design.obtain_space(new_design.get_space_index(space_coords_ordered[j])).y) &&
                         (new_design.obtain_space(i).z <  new_design.obtain_space(new_design.get_space_index(space_coords_ordered[j])).z))
                { // if the z coordinate of space 'i' is smaller than that of 'j' and both the y and the x-coordinates are equal
                    space_coords_ordered.insert(space_coords_ordered.begin()+j, new_design.obtain_space(i).ID);
                    space_inserted = true;
                    break;
                }
            }

            if (!space_inserted)
            {
                space_coords_ordered.push_back(new_design.obtain_space(i).ID);
            }
        }*/

        // calculate 'D' the number of divisions
        int D = (int)((1/(1-(double)space_delete_number/(double)space_total))+0.5);

        int x_div, y_div;
        bool plan_A = true;
        if ((D == 4) || (D == 9) || (D == 16) || (D == 25) || (D == 36))
        {
            x_div = sqrt(D);
            y_div = sqrt(D);
        }
        else if ((D == 6) || (D == 8) || (D == 10) || (D == 14) || (D == 22) || (D == 26) || (D == 34) || (D == 38))
        {
            x_div = 2;
            y_div = D/x_div;
        }
        else if ((D == 12) || (D == 15) || (D == 18))
        {
            x_div = 3;
            y_div = D/x_div;
        }
        else if ((D == 20) || (D == 24) || (D == 28) || (D == 32) || (D == 36))
        {
            x_div = 4;
            y_div = D/x_div;
        }
        else if ((D == 30) || (D == 40))
        {
            x_div = 5;
            y_div = D/x_div;
        }
        else
        { // plan B
            plan_A = false;
        }

        if (plan_A)
        {
            // split all spaces in x and y axis in the determined amount of divisions
            int space_count = new_design.obtain_space_count();

            for (int i = 0; i < space_count; i++)
            { // split all spaces in 'x_div' equal divisions across the x-axis
                new_design.split_space_n(0,0,x_div); //always pops off the first in the list
            }

            space_count = new_design.obtain_space_count();

            for (int i = 0; i < new_design.obtain_space_count(); i++)
            { // split all spaces in 'y_div' equal divisions across the y-axis
                new_design.split_space_n(0,1,y_div); //always pops off the first in the list
            }
        }
        else
        {
            // split spaces by size or spatial position
            for (int i = 0; new_design.obtain_space_count() < 2*(space_total-space_delete_number); i++)
            { // for the 'i'th division
                // divide the spaces as listed (by ascending ID)
                new_design.split_space(0); //always pops off the first in the list
            }
        }

        // scale the building up to its old volume
        double new_volume = new_design.get_volume();

        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));

        return new_design;

    } // scale_and_subdivide()
	#endif

	#ifdef SD_ANALYSIS_HPP
    Spatial_Design::MS_Building scale_and_subdivide_2(Spatial_Design::MS_Building& current_design,
                                                      Structural_Design::SD_Building_Results& sd_results)
    {
        Spatial_Design::MS_Building new_design = current_design;

        int space_total = current_design.obtain_space_count(); // amount of spaces in the current design
        int space_delete_number = 0;
        double initial_volume = current_design.get_volume();

        std::map<int, double> sd_performances;
        typedef std::map<int, double>::iterator space_ite;

        for (unsigned int i = 0; i < sd_results.m_spaces.size(); i++)
        {
            sd_performances[sd_results.m_spaces[i].m_ID] = sd_results.m_spaces[i].m_rel_performance;
        }

        std::vector<int> space_performances_ordered;

        // create an ordered list that maps space performances to their space ID
        for (space_ite ite = sd_performances.begin(); ite != sd_performances.end(); ite++)
        {
            bool space_inserted = false;
            for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
            {
                if (ite->second < sd_performances[space_performances_ordered[i] ])
                {
                    space_performances_ordered.insert(space_performances_ordered.begin() + i, ite->first);
                    space_inserted = true;
                    break;
                }
            }
            if (!space_inserted)
            { // if the space's performance is not smaller than that of any of the already inserted spaces, then add it to the back of the vector
                space_performances_ordered.push_back(ite->first);
            }
            space_inserted = false;
        }

        unsigned int delete_pos = space_total - space_total/2;
        double critical_performance = 0;

        for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
        { // delete the worst performing spaces
            if (i < delete_pos - 1)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (i == delete_pos - 1)
            {
                critical_performance = sd_performances[space_performances_ordered[i] ];
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (sd_performances[space_performances_ordered[i] ] - critical_performance < 0.0001)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else
            {
                break;
            }
        }

        // calculate 'D' the number of divisions
        int D = (int)((1/(1-(double)space_delete_number/(double)space_total))+0.5);

        int x_div, y_div;
        bool plan_A = true;
        if ((D == 4) || (D == 9) || (D == 16) || (D == 25) || (D == 36))
        {
            x_div = sqrt(D);
            y_div = sqrt(D);
        }
        else if ((D == 6) || (D == 8) || (D == 10) || (D == 14) || (D == 22) || (D == 26) || (D == 34) || (D == 38))
        {
            x_div = 2;
            y_div = D/x_div;
        }
        else if ((D == 12) || (D == 15) || (D == 18))
        {
            x_div = 3;
            y_div = D/x_div;
        }
        else if ((D == 20) || (D == 24) || (D == 28) || (D == 32) || (D == 36))
        {
            x_div = 4;
            y_div = D/x_div;
        }
        else if ((D == 30) || (D == 40))
        {
            x_div = 5;
            y_div = D/x_div;
        }
        else
        { // plan B
            plan_A = false;
        }

        if (plan_A)
        {
            // split all spaces in x and y axis in the determined amount of divisions
            int space_count = new_design.obtain_space_count();

            for (int i = 0; i < space_count; i++)
            { // split all spaces in 'x_div' equal divisions across the x-axis
                new_design.split_space_n(0,0,x_div); //always pops off the first in the list
            }

            space_count = new_design.obtain_space_count();

            for (int i = 0; i < new_design.obtain_space_count(); i++)
            { // split all spaces in 'y_div' equal divisions across the y-axis
                new_design.split_space_n(0,1,y_div); //always pops off the first in the list
            }
        }
        else
        {
            // split spaces by size or spatial position
            for (int i = 0; new_design.obtain_space_count() < 2*(space_total-space_delete_number); i++)
            { // for the 'i'th division
                // divide the spaces as listed (by ascending ID)
                new_design.split_space(0); //always pops off the first in the list
            }
        }

        // scale the building up to its old volume
        double new_volume = new_design.get_volume();

        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));

        return new_design;
    }
	#endif

	#ifdef BP_SIMULATION_HPP
    Spatial_Design::MS_Building BP_scale_and_subdivide(Spatial_Design::MS_Building& current_design,
                                                      Building_Physics::BP_Building_Results& bp_results)
    {
        Spatial_Design::MS_Building new_design = current_design;

        int space_total = current_design.obtain_space_count(); // amount of spaces in the current design
        int space_delete_number = 0;
        double initial_volume = current_design.get_volume();

        std::map<int, double> bp_performances;
        typedef std::map<int, double>::iterator space_ite;

        for (unsigned int i = 0; i < bp_results.m_space_results.size(); i++)
        {
            bp_performances[trim_and_cast_int(bp_results.m_space_results[i].m_space_ID)] = bp_results.m_space_results[i].m_rel_performance;
        }

        std::vector<int> space_performances_ordered;

        // create an ordered list that maps space performances to their space ID
        for (space_ite ite = bp_performances.begin(); ite != bp_performances.end(); ite++)
        {
            bool space_inserted = false;
            for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
            {
                if (ite->second < bp_performances[space_performances_ordered[i] ])
                {
                    space_performances_ordered.insert(space_performances_ordered.begin() + i, ite->first);
                    space_inserted = true;
                    break;
                }
            }
            if (!space_inserted)
            { // if the space's performance is not smaller than that of any of the already inserted spaces, then add it to the back of the vector
                space_performances_ordered.push_back(ite->first);
            }
            space_inserted = false;
        }

        unsigned int delete_pos = space_total - space_total/2;
        double critical_performance = 0;

        for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
        { // delete the worst performing spaces
            if (i < delete_pos - 1)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (i == delete_pos - 1)
            {
                critical_performance = bp_performances[space_performances_ordered[i] ];
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (bp_performances[space_performances_ordered[i] ] - critical_performance < 0.01)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else
            {
                break;
            }
        }

        // calculate 'D' the number of divisions
        int D = (int)((1/(1-(double)space_delete_number/(double)space_total))+0.5);

        int x_div, y_div;
        bool plan_A = true;
        if ((D == 4) || (D == 9) || (D == 16) || (D == 25) || (D == 36))
        {
            x_div = sqrt(D);
            y_div = sqrt(D);
        }
        else if ((D == 6) || (D == 8) || (D == 10) || (D == 14) || (D == 22) || (D == 26) || (D == 34) || (D == 38))
        {
            x_div = 2;
            y_div = D/x_div;
        }
        else if ((D == 12) || (D == 15) || (D == 18))
        {
            x_div = 3;
            y_div = D/x_div;
        }
        else if ((D == 20) || (D == 24) || (D == 28) || (D == 32) || (D == 36))
        {
            x_div = 4;
            y_div = D/x_div;
        }
        else if ((D == 30) || (D == 40))
        {
            x_div = 5;
            y_div = D/x_div;
        }
        else
        { // plan B
            plan_A = false;
        }

        if (plan_A)
        {
            // split all spaces in x and y axis in the determined amount of divisions
            int space_count = new_design.obtain_space_count();

            for (int i = 0; i < space_count; i++)
            { // split all spaces in 'x_div' equal divisions across the x-axis
                new_design.split_space_n(0,0,x_div); //always pops off the first in the list
            }

            space_count = new_design.obtain_space_count();

            for (int i = 0; i < new_design.obtain_space_count(); i++)
            { // split all spaces in 'y_div' equal divisions across the y-axis
                new_design.split_space_n(0,1,y_div); //always pops off the first in the list
            }
        }
        else
        {
            // split spaces by size or spatial position
            for (int i = 0; new_design.obtain_space_count() < 2*(space_total-space_delete_number); i++)
            { // for the 'i'th division
                // divide the spaces as listed (by ascending ID)
                new_design.split_space(0); //always pops off the first in the list
            }
        }

        // scale the building up to its old volume
        double new_volume = new_design.get_volume();

        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));
        new_design.reset_z_zero();
        return new_design;
    }
	#endif
	
	#ifdef SD_ANALYSIS_HPP
	#ifdef BP_SIMULATION_HPP
    Spatial_Design::MS_Building combined_scale_and_subdivide(Spatial_Design::MS_Building& current_design,
                                                          Building_Physics::BP_Building_Results& bp_results,
                                                          Structural_Design::SD_Building_Results& sd_results)
    {
        Spatial_Design::MS_Building new_design = current_design;

        int space_total = current_design.obtain_space_count(); // amount of spaces in the current design
        int space_delete_number = 0;
        double initial_volume = current_design.get_volume();

        std::map<int, double> bp_sd_performances;
        typedef std::map<int, double>::iterator space_ite;

        // below misses error catch, case: when an ID does not get a performance (or no performance at some disciplines)
        for (unsigned int i = 0; i < bp_results.m_space_results.size(); i++)
        {
            bp_sd_performances[trim_and_cast_int(bp_results.m_space_results[i].m_space_ID)] = bp_results.m_space_results[i].m_rel_performance;
        }

        for (unsigned int i = 0; i < sd_results.m_spaces.size(); i++)
        {
            bp_sd_performances[sd_results.m_spaces[i].m_ID] += sd_results.m_spaces[i].m_rel_performance;
            bp_sd_performances[sd_results.m_spaces[i].m_ID] /= 2.0;
        }

        std::vector<int> space_performances_ordered;

        // create an ordered list that maps space performances to their space ID
        for (space_ite ite = bp_sd_performances.begin(); ite != bp_sd_performances.end(); ite++)
        {
            bool space_inserted = false;
            for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
            {
                if (ite->second < bp_sd_performances[space_performances_ordered[i] ])
                {
                    space_performances_ordered.insert(space_performances_ordered.begin() + i, ite->first);
                    space_inserted = true;
                    break;
                }
            }
            if (!space_inserted)
            { // if the space's performance is not smaller than that of any of the already inserted spaces, then add it to the back of the vector
                space_performances_ordered.push_back(ite->first);
            }
            space_inserted = false;
        }

        unsigned int delete_pos = space_total - space_total/2;
        double critical_performance = 0;

        for (unsigned int i = 0; i < space_performances_ordered.size(); i++)
        { // delete the worst performing spaces
            if (i < delete_pos - 1)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (i == delete_pos - 1)
            {
                critical_performance = bp_sd_performances[space_performances_ordered[i] ];
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else if (bp_sd_performances[space_performances_ordered[i] ] - critical_performance < 0.01)
            {
                new_design.delete_space(new_design.get_space_index(space_performances_ordered[i]));
                space_delete_number++;
            }
            else
            {
                break;
            }
        }

        // calculate 'D' the number of divisions
        int D = (int)((1/(1-(double)space_delete_number/(double)space_total))+0.5);

        int x_div, y_div;
        bool plan_A = true;
        if ((D == 4) || (D == 9) || (D == 16) || (D == 25) || (D == 36))
        {
            x_div = sqrt(D);
            y_div = sqrt(D);
        }
        else if ((D == 6) || (D == 8) || (D == 10) || (D == 14) || (D == 22) || (D == 26) || (D == 34) || (D == 38))
        {
            x_div = 2;
            y_div = D/x_div;
        }
        else if ((D == 12) || (D == 15) || (D == 18))
        {
            x_div = 3;
            y_div = D/x_div;
        }
        else if ((D == 20) || (D == 24) || (D == 28) || (D == 32) || (D == 36))
        {
            x_div = 4;
            y_div = D/x_div;
        }
        else if ((D == 30) || (D == 40))
        {
            x_div = 5;
            y_div = D/x_div;
        }
        else
        { // plan B
            plan_A = false;
        }

        if (plan_A)
        {
            // split all spaces in x and y axis in the determined amount of divisions
            int space_count = new_design.obtain_space_count();

            for (int i = 0; i < space_count; i++)
            { // split all spaces in 'x_div' equal divisions across the x-axis
                new_design.split_space_n(0,0,x_div); //always pops off the first in the list
            }

            space_count = new_design.obtain_space_count();

            for (int i = 0; i < new_design.obtain_space_count(); i++)
            { // split all spaces in 'y_div' equal divisions across the y-axis
                new_design.split_space_n(0,1,y_div); //always pops off the first in the list
            }
        }
        else
        {
            // split spaces by size or spatial position
            for (int i = 0; new_design.obtain_space_count() < 2*(space_total-space_delete_number); i++)
            { // for the 'i'th division
                // divide the spaces as listed (by ascending ID)
                new_design.split_space(0); //always pops off the first in the list
            }
        }

        // scale the building up to its old volume
        double new_volume = new_design.get_volume();

        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));

        return new_design;
    }
	#endif
	#endif
	
	#ifdef SD_ANALYSIS_HPP
	#ifdef BP_SIMULATION_HPP
    Spatial_Design::MS_Building cluster_scale_and_subdivide(Spatial_Design::MS_Building& current_design,
                                                            Building_Physics::BP_Building_Results& bp_results,
                                                            Structural_Design::SD_Building_Results& sd_results)
    {
        Spatial_Design::MS_Building new_design = current_design;
        double initial_volume = current_design.get_volume();

        // get the results and put them in a vector (shared ptr for clustering purposes)
        std::shared_ptr<std::vector<BSO::data_point> > data_set = std::make_shared<std::vector<BSO::data_point> >();
        std::vector<unsigned int> space_id_list; // to track space ID's across result structures of different disciplines
        unsigned int number_of_spaces = current_design.obtain_space_count(); // determine how many spaces are in the design

        for (unsigned int i = 0; i < number_of_spaces; i++)
        { // for each space in the design
            // initialise a data point and an ID
            space_id_list.push_back(current_design.obtain_space(i).ID);
            data_point temp = Eigen::Vector2d::Zero();
            data_set->push_back(temp);
        }

        std::vector<bool> match_check(space_id_list.size(), false); // this list will keep track if a performance has been found for each space

        for (unsigned int i = 0; i < bp_results.m_space_results.size(); i++)
        { // for each space result
            for (unsigned int j = 0; j < number_of_spaces; j++)
            { // and for each space in the design
                if (space_id_list[j] == BSO::trim_and_cast_uint(bp_results.m_space_results[i].m_space_ID))
                { // check if space_result matches with a space in the design
                    (*data_set)[j](0) = bp_results.m_space_results[i].m_total_energy;
                    match_check[j] = true;
                }
            }
        }

        for (unsigned int i = 0; i < number_of_spaces; i++)
        { // for each space in the design
            if (!match_check[i])
            { // check if a performance has been found for space i
                std::cerr << "Error could not find BP performance of space with ID: "
                          << space_id_list[i] << " (heuristics.hpp), exiting now..." << std::endl;
                exit(1);
            }
        }

        for (unsigned int i = 0; i < sd_results.m_spaces.size(); i++)
        { // for each space result
            for (unsigned int j = 0; j < space_id_list.size(); j++)
            { // and for each space in the design
                if (space_id_list[j] == (unsigned int)sd_results.m_spaces[i].m_ID)
                { // check if space_result matches with a space in the design
                    (*data_set)[j](1) = sd_results.m_spaces[i].m_total_compliance;
                    match_check[j] = false;
                }
            }
        }

        for (unsigned int i = 0; i < number_of_spaces; i++)
        { // for each space in the design
            if (match_check[i])
            { // check if a performance has been found for space i
                std::cerr << "Error could not find structural performance of space with ID: "
                          << space_id_list[i] << " (heuristics.hpp), exiting now..." << std::endl;
                exit(1);
            }
        }

        // cluster the data set
        // make k clusters, where k lies between 25% and 75% of the number of spaces with a lower limit of 2 clusters
        //std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set = clustering(data_set, 50, (2 < 1+number_of_spaces/8)? 1+number_of_spaces/4 : 2, 3*number_of_spaces/4, 2);
        std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set = clustering(data_set, 50, 6, 10, 2);

        // remove cluster by cluster the worst performing cluster until half of the spaces has been removed
        // here the distance from a cluster's centroid to the origin is used to asses a cluster
        unsigned int removed_spaces = 0;
        std::vector<std::shared_ptr<BSO::Cluster> > cp_cluster_set = clustered_data_set; // copy the clustered set of data for

        while (removed_spaces < number_of_spaces/2)
        { // as long as not half of all spaces are removed
            double max_distance = 0;
            unsigned int furthest_cluster = 0;
            for (unsigned int i = 0; i < cp_cluster_set.size(); i++)
            { // for each cluster
                if (max_distance < cp_cluster_set[i]->m_dist_from_origin)
                { // find cluster that has the centroid with the largest distance from the origin
                    max_distance = cp_cluster_set[i]->m_dist_from_origin;
                    furthest_cluster = i;
                }
            }

            // remove all spaces that are in the cluster with the furthest distance
            for (unsigned int i = 0; i < number_of_spaces; i++)
            { // for each space in the design
                if (cp_cluster_set[furthest_cluster]->m_bit_mask[i])
                { // check if it belongs to the cluster
                    new_design.delete_space(new_design.get_space_index(space_id_list[i])); // remove the space in this cluster
                    removed_spaces++;
                }
            }

            // remove the cluster from the (copied) clustered data set
            cp_cluster_set.erase(cp_cluster_set.begin()+furthest_cluster);
        }

        // scale the building back to its initial volume
        double new_volume = new_design.get_volume();

        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));

        // split the remaining spaces
        unsigned int number_of_remaining_spaces = new_design.obtain_space_count();
        for (unsigned int i = 0; i < number_of_remaining_spaces; i++)
        { // for each remaining space
            new_design.split_space(0); // splits the first listed space in the design
        }

        std::ofstream output("Bit_Mask.txt");


        for (unsigned int i = 0; i < clustered_data_set.size(); i++)
        { // for each cluster
                output << "Cluster: " << i+1 << std::endl;
                for (unsigned int j = 0; j < data_set->size(); j++)
                { // for each data point
                    if (clustered_data_set[i]->m_bit_mask[j])
                    { //  if that data point blongs to the cluster
                        output << space_id_list[j];
                        for (unsigned int k = 0; k < 2; k++)
                        { // plot the performances of this data point
                            output << "," << (*data_set)[j](k);
                        }
                        output << std::endl;
                    }
                }
                output << std::endl << std::endl;
        }

        std::ofstream output2("Bit_Mask2.txt");


        for (unsigned int i = 0; i < cp_cluster_set.size(); i++)
        {// for each remaining cluster
                output2 << "Cluster: " << i+1 << std::endl;
                for (unsigned int j = 0; j < data_set->size(); j++)
                { // for each data point
                    if (cp_cluster_set[i]->m_bit_mask[j])
                    { // if data point belongs to the cluster
                        output2 << space_id_list[j];
                        for (unsigned int k = 0; k < 2; k++)
                        { // plot the performances of the remaining cluster
                            output2 << "," << (*data_set)[j](k);
                        }
                        output2 << std::endl;
                    }
                }
                output2 << std::endl << std::endl;
        }

        return new_design;
    }
	#endif
	#endif
	
	#ifdef BP_SIMULATION_HPP
    Spatial_Design::MS_Building cluster_scale_and_subdivide(Spatial_Design::MS_Building& current_design,
                                                            Building_Physics::BP_Building_Results& bp_results)
    {
        Spatial_Design::MS_Building new_design = current_design;
        double initial_volume = current_design.get_volume();

        // get the results and put them in a vector (shared ptr for clustering purposes)
        std::shared_ptr<std::vector<BSO::data_point> > data_set = std::make_shared<std::vector<BSO::data_point> >();
        std::vector<unsigned int> space_id_list; // to track space ID's across result structures of different disciplines
        unsigned int number_of_spaces = current_design.obtain_space_count(); // determine how many spaces are in the design

        for (unsigned int i = 0; i < number_of_spaces; i++)
        { // for each space in the design
            // initialise a data point and an ID
            space_id_list.push_back(current_design.obtain_space(i).ID);
            data_point temp = Eigen::Vector2d::Zero();
            data_set->push_back(temp);
        }

        std::vector<bool> match_check(space_id_list.size(), false); // this list will keep track if a performance has been found for each space

        for (unsigned int i = 0; i < bp_results.m_space_results.size(); i++)
        { // for each space result
            for (unsigned int j = 0; j < number_of_spaces; j++)
            { // and for each space in the design
                if (space_id_list[j] == BSO::trim_and_cast_uint(bp_results.m_space_results[i].m_space_ID))
                { // check if space_result matches with a space in the design
                    (*data_set)[j](0) = bp_results.m_space_results[i].m_total_energy;
                    match_check[j] = true;
                }
            }
        }

        for (unsigned int i = 0; i < number_of_spaces; i++)
        { // for each space in the design
            if (!match_check[i])
            { // check if a performance has been found for space i
                std::cerr << "Error could not find BP performance of space with ID: "
                          << space_id_list[i] << " (heuristics.hpp), exiting now..." << std::endl;
                exit(1);
            }
        }

        // cluster the data set
        // make k clusters, where k lies between 25% and 75% of the number of spaces with a lower limit of 2 clusters
        std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set = clustering(data_set, 50, (2 < 1+number_of_spaces/8)? 1+number_of_spaces/4 : 2, 3*number_of_spaces/4, 2);
        //std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set = clustering(data_set, 50, 2, 6, 2);

        // remove cluster by cluster the worst performing cluster until half of the spaces has been removed
        // here the distance from a cluster's centroid to the origin is used to asses a cluster
        unsigned int removed_spaces = 0;
        std::vector<std::shared_ptr<BSO::Cluster> > cp_cluster_set = clustered_data_set; // copy the clustered set of data for

        while (removed_spaces < number_of_spaces/2)
        { // as long as not half of all spaces are removed
            double max_distance = 0;
            unsigned int furthest_cluster = 0;
            for (unsigned int i = 0; i < cp_cluster_set.size(); i++)
            { // for each cluster
                if (max_distance < cp_cluster_set[i]->m_dist_from_origin)
                { // find cluster that has the centroid with the largest distance from the origin
                    max_distance = cp_cluster_set[i]->m_dist_from_origin;
                    furthest_cluster = i;
                }
            }

            // remove all spaces that are in the cluster with the furthest distance
            for (unsigned int i = 0; i < number_of_spaces; i++)
            { // for each space in the design
                if (cp_cluster_set[furthest_cluster]->m_bit_mask[i])
                { // check if it belongs to the cluster
                    new_design.delete_space(new_design.get_space_index(space_id_list[i])); // remove the space in this cluster
                    removed_spaces++;
                }
            }

            // remove the cluster from the (copied) clustered data set
            cp_cluster_set.erase(cp_cluster_set.begin()+furthest_cluster);
        }

        // scale the building back to its initial volume
        double new_volume = new_design.get_volume();

        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));

        // split the remaining spaces
        unsigned int number_of_remaining_spaces = new_design.obtain_space_count();
        for (unsigned int i = 0; i < number_of_remaining_spaces; i++)
        { // for each remaining space
            new_design.split_space(0); // splits the first listed space in the design
        }

        std::ofstream output("Bit_Mask.txt");


        for (unsigned int i = 0; i < clustered_data_set.size(); i++)
        { // for each cluster
                output << "Cluster: " << i+1 << std::endl;
                for (unsigned int j = 0; j < data_set->size(); j++)
                { // for each data point
                    if (clustered_data_set[i]->m_bit_mask[j])
                    { //  if that data point blongs to the cluster
                        output << space_id_list[j];
                        for (unsigned int k = 0; k < 2; k++)
                        { // plot the performances of this data point
                            output << "," << (*data_set)[j](k);
                        }
                        output << std::endl;
                    }
                }
                output << std::endl << std::endl;
        }

        std::ofstream output2("Bit_Mask2.txt");


        for (unsigned int i = 0; i < cp_cluster_set.size(); i++)
        {// for each remaining cluster
                output2 << "Cluster: " << i+1 << std::endl;
                for (unsigned int j = 0; j < data_set->size(); j++)
                { // for each data point
                    if (cp_cluster_set[i]->m_bit_mask[j])
                    { // if data point belongs to the cluster
                        output2 << space_id_list[j];
                        for (unsigned int k = 0; k < 2; k++)
                        { // plot the performances of the remaining cluster
                            output2 << "," << (*data_set)[j](k);
                        }
                        output2 << std::endl;
                    }
                }
                output2 << std::endl << std::endl;
        }

        return new_design;
    }
	#endif
	
	#ifdef SD_ANALYSIS_HPP
	#ifdef BP_SIMULATION_HPP
    Spatial_Design::MS_Building change_1_space(Spatial_Design::MS_Building& current_design,
                                               Building_Physics::BP_Building_Results& bp_results,
                                               Structural_Design::SD_Building_Results& sd_results,
                                               unsigned int mode_switch)
    {
        Spatial_Design::MS_Building new_design = current_design;
        double initial_volume = current_design.get_volume();

        // get the results and put them in a vector (shared ptr for clustering purposes)
        std::vector<BSO::data_point> data_set;
        std::vector<unsigned int> space_id_list; // to track space ID's across result structures of different disciplines
        unsigned int number_of_spaces = current_design.obtain_space_count(); // determine how many spaces are in the design

        for (unsigned int i = 0; i < number_of_spaces; i++)
        { // for each space in the design
            // initialise a data point and an ID
            space_id_list.push_back(current_design.obtain_space(i).ID);
            data_point temp = Eigen::Vector2d::Zero();
            data_set.push_back(temp);
        }


        std::vector<bool> match_check(space_id_list.size(), false); // this list will keep track if a performance has been found for each space

        if (mode_switch == 0 || mode_switch == 1)
        {
            for (unsigned int i = 0; i < bp_results.m_space_results.size(); i++)
            { // for each space result
                for (unsigned int j = 0; j < number_of_spaces; j++)
                { // and for each space in the design
                    if (space_id_list[j] == BSO::trim_and_cast_uint(bp_results.m_space_results[i].m_space_ID))
                    { // check if space_result matches with a space in the design
                        data_set[j](0) = bp_results.m_space_results[i].m_total_energy;
                        match_check[j] = true;
                    }
                }
            }

            for (unsigned int i = 0; i < number_of_spaces; i++)
            { // for each space in the design
                if (!match_check[i])
                { // check if a performance has been found for space i
                    std::cerr << "Error could not find BP performance of space with ID: "
                              << space_id_list[i] << " (heuristics.hpp), exiting now..." << std::endl;
                    exit(1);
                }
            }
        }

        if (mode_switch == 0 || mode_switch == 2)
        {
            for (unsigned int i = 0; i < sd_results.m_spaces.size(); i++)
            { // for each space result
                for (unsigned int j = 0; j < space_id_list.size(); j++)
                { // and for each space in the design
                    if (space_id_list[j] == (unsigned int)sd_results.m_spaces[i].m_ID)
                    { // check if space_result matches with a space in the design
                        data_set[j](1) = sd_results.m_spaces[i].m_total_compliance;
                        match_check[j] = false;
                    }
                }
            }

            for (unsigned int i = 0; i < number_of_spaces; i++)
            { // for each space in the design
                if (match_check[i])
                { // check if a performance has been found for space i
                    std::cerr << "Error could not find structural performance of space with ID: "
                              << space_id_list[i] << " (heuristics.hpp), exiting now..." << std::endl;
                    exit(1);
                }
            }
        }

        data_point min = data_set[0];
        data_point max = data_set[0];

        for (unsigned int i = 0; i < data_set.size(); i++)
        {
            for (unsigned int j = 0; j < data_set[i].rows(); j++)
            {
                if (min(j) > data_set[i](j) )
                {
                    min(j) = data_set[i](j);
                }
                if (max(j) < data_set[i](j) )
                {
                    max(j) = data_set[i](j);
                }
            }
        }

        // find
        dist_function d_func; // by default initialised to Euclidian distance
        d_func.m_tag = dist_function::Normalised; // initialised to normalised distances
        d_func.m_min = min;
        d_func.m_max = max;
        unsigned int selected_space = find_closest_to(data_set,max,d_func); // saves the index of data_set that points to the point closes to the origin

        // remove the worst space
        new_design.delete_space(new_design.get_space_index(space_id_list[selected_space]));
        data_set.erase(data_set.begin() + selected_space);
        space_id_list.erase(space_id_list.begin() + selected_space);

        // scale up the design to its original volume
        double new_volume = new_design.get_volume();
        new_design.scale_x(sqrt(initial_volume/new_volume));
        new_design.scale_y(sqrt(initial_volume/new_volume));
        new_design.reset_z_zero();

        double min_w = 1000, min_d = 1000; // min_h = 3000;

        bool found_valid_space = false;
        for (unsigned int i = 0; i < data_set.size(); i++)
        {
            unsigned int selected_space = find_closest_to(data_set,min,d_func); // saves the index of data_set that points to the point closes to the origin
            Spatial_Design::MS_Space considered_space = new_design.obtain_space(selected_space);
            if ((considered_space.width  >= considered_space.depth) && (considered_space.width  >= min_w)) // if the dimension in x-direction is largest and larger than the minimal required value
            { // split parallel to y-axis
                new_design.split_space(new_design.get_space_index(space_id_list[selected_space]), 0);
                found_valid_space = true;
                break;
            }
            else if ((considered_space.width  < considered_space.depth) && (considered_space.depth  >= min_d)) // or if the dimension in y-direction is largest and larger than the minimal required value
            { // split parallel to x-axis
                new_design.split_space(new_design.get_space_index(space_id_list[selected_space]), 1);
                found_valid_space = true;
                break;
            }
            else
            { // splitting will lead to a constraint violation, look at the next space
                data_set.erase(data_set.begin() + selected_space);
                space_id_list.erase(space_id_list.begin() + selected_space);
                i = 0;
                continue;
            }
        }

        if (!found_valid_space)
        {
            std::cerr << "Did not find a valid space for splitting, exiting now..." << std::endl;
            exit(1);
        }

        return new_design;
    } // remove_one_space()
	#endif
	#endif

} // namespace BSO

#endif // HEURISTICS_HPP
