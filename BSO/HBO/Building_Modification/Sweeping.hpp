#ifndef SWEEPING_HPP
#define SWEEPING_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/HBO/HBO_Settings.hpp>
#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>

#include <iostream>
#include <vector>
#include <cmath>

namespace BSO { namespace HBO { namespace Building_Modification
{

    BSO::Spatial_Design::MS_Building z_sweep_building_design(BSO::Spatial_Design::MS_Building& current_design, std::vector<int>& spaces_for_removal, HBO::Performance_Evaluation::Building_Performances& build_perform)
    {
        BSO::Spatial_Design::MS_Building temp_design;

        double initial_volume = current_design.get_volume();
        int initial_space_count = current_design.obtain_space_count();
        int spaces_added = 0;

        // delete all spaces marked
        for ( unsigned int i = 0 ; i < current_design.obtain_space_count() ; i++ )
        {
            bool space_on_list = false;

            // check if the space is on the removal list
            for ( unsigned int j = 0 ; j < spaces_for_removal.size() ; j++ )
            {
                if ( current_design.obtain_space(i) == spaces_for_removal[j] )
                {
                    space_on_list = true;
                }
            }

            // if the space is not on the list add it to the new design
            if(!space_on_list)
            {
                temp_design.add_space(current_design.obtain_space(i));
            }
        }

        // make sure the building stands at z = 0
        temp_design.reset_z_zero();


        // sweep over z axis by adding a space in the negative space direction then using the reset z function
        for ( unsigned int i = 0 ; i < temp_design.obtain_space_count() ; i++ )
        {
            // if the z coordinate is zero add a space beneath it
            if ( temp_design.obtain_space(i).z == 0 )
            {
                BSO::Spatial_Design::MS_Space temp_space = temp_design.obtain_space(i);
                temp_space.z -= 3000;
                temp_space.ID = temp_design.get_last_space_id();

                temp_design.add_space(temp_space);
                temp_design.search_last_space_id();
                spaces_added++;
            }
        }

        //make sure the volume is still correct
        temp_design.scale_x(sqrt(initial_volume/temp_design.get_volume()));
        temp_design.scale_y(sqrt(initial_volume/temp_design.get_volume()));

        // reset the z dimension to 0
        temp_design.reset_z_zero();

        return temp_design;
    }  // z_sweep_building_design()

    BSO::Spatial_Design::MS_Building x_sweep_building_design( BSO::Spatial_Design::MS_Building& current_design, std::vector<int>& spaces_for_removal, HBO::Performance_Evaluation::Building_Performances& build_perform, Settings& settings)
    {
        BSO::Spatial_Design::MS_Building temp_design;

        double initial_volume = current_design.get_volume();
        int initial_space_count = current_design.obtain_space_count();

        // delete all spaces
        for (unsigned int i = 0 ; i < current_design.obtain_space_count() ; i++ )
        {
            bool space_on_list = false;

            for (unsigned int j = 0 ; j < spaces_for_removal.size() ; j++ )
            {
                if( current_design.obtain_space(i).ID == spaces_for_removal[j])
                {
                    space_on_list = true;
                }
            }

            if ( !space_on_list )
            {
                temp_design.add_space(current_design.obtain_space(i))
            }
        }

        // find the best space and sweep it over the x axis
        std::vector<int> ids_to_sweep;
        double coordinate_to_sweep = temp_design.obtain_space(0).x;

        switch(settings.direction)
            {
            case sweep_direction::POSITIVE:
                for( unsigned int i = 0 ; i < temp_design.obtain_space_count() ; i++ )
                {
                    if ( coordinate_to_sweep < temp_design.obtain_space(i).x)
                    {
                        coordinate_to_sweep = temp_design.obtain_space(i).x;
                    }
                }
                break;
            case sweep_direction::NEGATIVE:
                for ( unsigned int i = 0 ; i < temp_design.obtain_space_count() ; i++ )
                {
                    if( coordinate_to_sweep > temp_design.obtain_space(i).x )
                    {
                        coordinate_to_sweep = temp_design.obtain_space(i).x;
                    }
                }
                break;
            case sweep_direction::ARG_COUNT:
                std::cout<<"Error in sweep direction x axis, exiting now... (Sweeping.hpp)"<<std::endl;
                exit(1);
                break;

            default:
                std::cout<<"Error in sweep direction x axis, exiting now... (Sweeping.hpp)"<<std::endl;
                exit(1);
                break;
            }
        // create a vector of pointer to sort the performances
        std::vector<int&> sorted_ids;

        for( unsigned int i = 0 ; i < temp_design.obtain_space_count() ; i++ )
        {
            if ( temp_design.obtain_space(i).x == coordinate_to_sweep )
            {
                for(unsigned int j = 0 ; j < sorted_ids.size() ; j++)
                {
                    if ( sorted_ids.empty())
                    {
                        sorted_ids.push_back(temp_design.obtain_space(i));
                    }
                    else
                    {
                        if ( build_perform.spaces[build_perform.id_belonging_space(temp_design.obtain_space(i).ID)].modified_performance[0]
                    }
                }
            }
        }

            //////////////////////////////////// 26-7-2017

        for ( unsigned int i = 0 ; i < temp_design.obtain_space_count() ; i++ )
        {
            if ( temp_design.obtain_space(i).x == coordinate_to_sweep && settings.direction == sweep_direction::POSITIVE )
            {
                BSO::Spatial_Design::MS_Space temp_space = temp_design.obtain_space(i);
                temp_space.width *= 2;

                temp_design.delete_space(i);
                temp_design.add_space(temp_space);
            }
            else if( temp_design.obtain_space(i).x == coordinate_to_sweep && settings.direction == sweep_direction::NEGATIVE )
            {
                BSO::Spatial_Design::MS_Space temp_space = temp_design.obtain_space(i);
                temp_space.x -= temp_space.width;
                temp_space.width *= 2;

                temp_design.delete_space(i);
                temp_design.add_space(temp_space);
            }

        }

        // for each space in the removal list sweep a space over the x axis
        for ( unsigned i = 0 ; i < spaces_for_removal.size() ; i++ )
        {

            for ( unsigned int j = 0 ; j < temp_design.obtain_space_count() ; j++ )
            {
                int best_peforming_id;
                double best_performance = 1;


                if ( temp_design.obtain_space(j).x ==0 )
                {
                    if( )
                }
            }

        }



    } // x_sweep_building_design()



} // namespace Building_Modification
} // namespace HBO
} // namespace BSO

#endif // SWEEPING_HPP
