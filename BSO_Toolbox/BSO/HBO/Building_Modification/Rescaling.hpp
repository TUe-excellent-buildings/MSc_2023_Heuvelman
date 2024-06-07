#ifndef RESCALING_HPP
#define RESCALING_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>
#include <BSO/HBO/HBO_Settings.hpp>

#include <iostream>
#include <vector>
#include <cmath>


namespace BSO { namespace HBO { namespace Building_Modification
{

    BSO::Spatial_Design::MS_Building rescale_building_design( BSO::Spatial_Design::MS_Building& current_design, std::vector<int>& spaces_for_removal, HBO::Settings& settings, HBO::Performance_Evaluation::Building_Performances& build_perform)
    {
        BSO::Spatial_Design::MS_Building new_design = current_design;
        double initial_volume = current_design.get_volume();
        double initial_space_count = current_design.obtain_space_count();

        // delete the spaces as indicated by the space selection algorithm

        for ( unsigned int i = 0 ; i < spaces_for_removal.size() ; i++ )
        {
            new_design.delete_space(new_design.get_space_index(spaces_for_removal[i]));
        }

        double new_volume = new_design.get_volume();
        double ratio_cur_new = initial_volume / new_volume;

        // rescale the new design
        switch (settings::rescaling_options)
        {
            // options for scaling of a single axis
        case rescaling_options::X:
            new_design.scale_x(ratio_cur_new);
            break;
        case rescaling_options::Y:
            new_design.scale_y(ratio_cur_new);
            break;
        case rescaling_options::Z:
            new_design.scale_z(ratio_cur_new);
            break;
            // options for scaling over two axis, square root of the ratio is used
        case rescaling_options::XY:
            new_design.scale_x(sqrt(ratio_cur_new));
            new_design.scale_y(sqrt(ratio_cur_new));
            break;
        case rescaling_options::XZ:
            new_design.scale_x(sqrt(ratio_cur_new));
            new_design.scale_z(sqrt(ratio_cur_new));
        case rescaling_options::YZ:
            new_design.scale_y(sqrt(ratio_cur_new));
            new_design.scale_z(sqrt(ratio_cur_new));
            // option scaling over three axis, cubic root of the ratio is used
        case rescaling_options::XYZ:
            new_design.scale_x(cbrt(ratio_cur_new));
            new_design.scale_y(cbrt(ratio_cur_new));
            new_design.scale_z(cbrt(ratio_cur_new));
            break;

        case rescaling_options::ARG_COUNT:
            std::cout<<"Error in rescaling options (ARG_COUNT), exiting now... (Rescaling.hpp)" << std::endl;
            exit(1);
        default:
            std::cout<<"Eroor in rescaling options (default), exiting now... (Rescaling.hpp)"<<std::endl;
            exit(1);
        } // end switch rescaling

        // the best performing spaces in the building will be split

        new_design.search_last_space_id(); // update the last_space_id of the new design

        for ( unsigned int i = 0 ; i < spaces_for_removal.size() ; i++ )
        {
            Performance_Evaluation::Building_Performances temp_build_perform = build_perform;
            int best_space_index = 0;
            double best_space_performance = temp_build_perform.spaces[0].modified_performance[0];
            int spaces_split = 0;

            for ( unsigned int j = 0 ; j < temp_build_perform.spaces.size() ; j++ )
            {
                for ( unsigned int k = 0 ; k < temp_build_perform.spaces[j].modified_performance.size() ; k ++)
                {
                    if ( temp_build_perform.spaces[j].modified_performance[k] > best_space_performance )
                    {
                        best_space_performance = temp_build_perform.spaces[j].modified_performance[k];
                        best_space_index = j;
                    }
                }
            }

            for ( unsigned j = 0 ; j < temp_build_perform.spaces[best_space_index].space_ID.size() ; j++ )
            {
                new_design.split_space(new_design.get_space_index( temp_build_perform.spaces[best_space_index].space_ID[j] ) ) ;
                spaces_split++;

                // check if the amount of IDs split is reached, since spaces can contain multiple IDs
                if( spaces_split >= spaces_for_removal.size() )
                {
                    goto skip;
                }
            }
        }

        skip:

            return new_design;
    }



}   // namespace Building_Modification
}   // namespace HBO
} // namespace BSO

#endif // RESCALING_HPP
