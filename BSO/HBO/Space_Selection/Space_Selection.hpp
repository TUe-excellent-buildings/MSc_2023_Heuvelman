#ifndef SPACE_SELECTION_HPP
#define SPACE_SELECTION_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Trim_And_Cast.hpp>
#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>
#include <BSO/HBO/HBO_Settings.hpp>

#include <iostream>
#include <vector>
#include <cmath>

namespace BSO{ namespace HBO{ namespace Space_Selection
{

    std::vector<int>  select_worst_spaces( Performance_Evaluation::Building_Performances& build_perform, Settings& settings)
    {
        Performance_Evaluation::Building_Performances temp_build_perform = build_perform;
        unsigned int requested_deletion_count = settings.space_removal_requested;
        std::vector<int> id_removal_list;

        // check if the modified performance vector exists
        for ( unsigned int i = 0 ; i < temp_build_perform.groups.size() ; i++ )
        {
            if ( temp_build_perform.groups[i].modified_performance.empty() )
            {
                std::cout<<"Error, modified performance is empty. Exiting now... (Space_Selection.hpp)"<<std::endl;
                exit(1);
            }
        }


        while( id_removal_list.size() < requested_deletion_count )
        {
            // the space and its performances on the zeroth index are the initial values
            int critical_space_index =0;
            double critical_performance = temp_build_perform.groups[0].modified_performance[0];

            std::cout<< critical_performance << std::endl;


            // cycle trough all groups
            for (unsigned int i = 0 ; i < temp_build_perform.groups.size() ; i++)
            {
                // cycle through all performances
                for (unsigned int j = 0 ; j < temp_build_perform.groups[i].modified_performance.size() ; j++)
                {
                    if (temp_build_perform.groups[i].modified_performance[j] < critical_performance )
                    {
                        critical_performance = temp_build_perform.groups[i].modified_performance[j];
                        critical_space_index = i;
                    }
                }
            } // end space loop


            // add all space IDs to the removal list
            for ( unsigned int i =0 ; i < temp_build_perform.groups[critical_space_index].space_ID.size()  ;  i++ )
            {
                id_removal_list.push_back(temp_build_perform.groups[critical_space_index].space_ID[i]);
            }

            // remove the critical space from the vector so i cannot be found again
            temp_build_perform.groups.erase(temp_build_perform.groups.begin() + critical_space_index);

            // if the removal amount is reached check if no groups with similar performances remain
            if ( id_removal_list.size() >= requested_deletion_count )  // equal or greater since groups can contain multiple IDs
            {
                for ( unsigned int i = 0 ; i < temp_build_perform.groups.size() ; i++ )
                {
                    for (unsigned int j = 0 ; j < temp_build_perform.groups[i].modified_performance.size() ; j++ )
                    {
                        if ( temp_build_perform.groups[i].modified_performance[j]  == critical_performance || temp_build_perform.groups[i].modified_performance[j] - critical_performance <= 0.01 )
                        {

                            for ( unsigned int k = 0 ; k < temp_build_perform.groups[i].space_ID.size() ; k++ )
                            {
                                id_removal_list.push_back(temp_build_perform.groups[i].space_ID[k]);
                            }
                        }
                    }
                }
            } // end check for similar performances


        } // end while loop

        settings.space_removal_selected = id_removal_list.size(); // gives the settings object the amount of spaces removed

        return id_removal_list;
    } // select_worst_spaces()


    std::vector<int> select_best_spaces( Performance_Evaluation::Building_Performances& build_perform, Settings& settings)
    {
        Performance_Evaluation::Building_Performances temp_build_perform = build_perform;
        unsigned int spaces_to_keep = build_perform.id_count() - settings.space_removal_requested;
        std::vector<int> id_best_spaces;

        // check if the modified list is not empty
        for ( unsigned int i = 0 ; i < build_perform.groups.size() ; i++ )
        {
            if ( build_perform.groups[i].modified_performance.empty() )
            {
                std::cout<<"Error, modified performances is empty. Exiting now... (Space_Selection.hpp)"<<std::endl;
                exit(1);
            }
        }

        while( id_best_spaces.size() < spaces_to_keep )
        {
            // the space and its performances on the zeroth index are the initial values
            int critical_space_index = 0;
            double critical_space_performance = temp_build_perform.groups[0].modified_performance[0];

            std::cout<< critical_space_performance<< std::endl;

            // cycle through all groups
            for ( unsigned int i = 0 ; i < temp_build_perform.groups.size() ; i++ )
            {
                // cycle through all performances
                for (unsigned int j = 0 ; j < temp_build_perform.groups[i].modified_performance.size() ; j++ )
                {
                    if( temp_build_perform.groups[i].modified_performance[j]  > critical_space_performance )
                    {
                        critical_space_performance = temp_build_perform.groups[i].modified_performance[j];
                        critical_space_index = i;
                    }

                }
            }

            //add IDs of selected space to the keep vector
            for(unsigned int i = 0 ; i < temp_build_perform.groups[critical_space_index].space_ID.size() ; i++ )
            {
                if ( temp_build_perform.groups[critical_space_index].space_ID.size() + id_best_spaces.size() >= spaces_to_keep)
                {
                    goto stop_loop;
                }
                id_best_spaces.push_back(temp_build_perform.groups[critical_space_index].space_ID[i]);
            }

            // remove the selected space form the build_perform
            temp_build_perform.groups.erase(temp_build_perform.groups.begin() + critical_space_index);


        } // end while loop


        stop_loop:
        std::vector<int> id_removal_list;

        // inverse the keep list

        // for all groups in build_perform
        for ( unsigned int i = 0 ; i < build_perform.groups.size() ; i++ )
        {
            // for all IDs in the space
            for ( unsigned int j = 0 ; j < build_perform.groups[i].space_ID.size() ; j++ )
            {
                bool keep_space = false;
                // for all IDs in the keep
                for(unsigned int k = 0 ; k < id_best_spaces.size() ; k++ )
                {
                    if ( id_best_spaces[k] ==build_perform.groups[i].space_ID[j] )
                    {
                        keep_space = true;
                    }
                }
                if(!keep_space)
                {
                    id_removal_list.push_back(build_perform.groups[i].space_ID[j]);
                }
            }
        } // end inverse loop

        settings.space_removal_selected = id_removal_list.size();

        return id_removal_list;

    } // select_best_spaces()


    std::vector<int> select_one_or_another_worst(Performance_Evaluation::Building_PerformancesU& build_perform, Settings& settings)
    {
        Performance_Evaluation::Building_Performances temp_perform = build_perform;
        std::vector<int> selected_spaces;

        // check if the modified performances list is not empty
        for ( unsigned int i = 0 ; i < temp_perform.groups.size() ; i++ )
        {
            if (temp_perform.groups[i].modified_performance.empty() )
            {
                std::cout<<"Error modified performances are empty, exiting now... ( Space_Selection.hpp)"<<std::endl;
                exit(1);
            }
        }

        while ( selected_spaces < settings.space_removal_requested )
        {
            int critical_space_index = 0;
            double critical_performance;

            static int selected_performance = 0;

            if ( selected_performance == temp_perform.groups[0].modified_performance.size() )
            {
                selected_performance = 0;
            }

            for ( unsigned int i = 0 ; i < temp_perform.groups.size() ; i++ )
            {
                if ( i == 0)
                {
                    critical_performance = temp_perform.groups[critical_space_index].modified_performance[selected_performance];
                }

                if ( critical_performance > temp_perform.groups[i].modified_performance[selected_performance] )
                {
                    critical_performance = temp_perform.groups[i].modified_performance[selected_performance];
                    critical_space_index = i;
                }
            }

            selected_performance++;

            // add the selected space to the vector
            for ( unsigned int i = 0 ; i < temp_perform.groups[critical_space_index].space_ID.size() ; i++ )
            {
                selected_spaces.push_back(temp_perform.groups[critical_space_index].space_ID[i]);
            }

            // remove the selected space from the temp vector so it cannot be selected again
            temp_perform.groups.erase(temp_perform.groups.begin() + critical_space_index);
        }

        settings.space_removal_selected = selected_spaces.size();
        return selected_spaces;
    } // select_one_or_another_worst()


    std::vector<int> select_one_or_another_best( Performance_Evaluation::Building_Performances& build_perform, Settings& settings)
    {
        Performance_Evaluation::Building_Performances temp_perform = build_perform;
        std::vector<int> selected_spaces;

        while (selected_spaces.size() < build_perform.id_count() - settings.space_removal_requested )
        {
            int critical_space_index = 0;
            double critical_performance;

            static int selected_performance = 0;
            if ( selected_performance == temp_perform.groups[0].modified_performance.size() )
            {
                selected_performance = 0;
            }

            for( unsigned int i = 0 ; i < temp_perform.groups.size() ; i++ )
            {
                if ( i == 0 )
                {
                    selected_performance = temp_perform.groups[0].modified_performance[selected_performance];
                }

                if ( critical_performance < temp_perform.groups[i].modified_performance[selected_performance] )
                {
                    critical_performance = temp_perform.groups[i].modified_performance[selected_performance];
                    critical_space_index = i;
                }
            }

            selected_performance++;

            // add the selected space to the vector
            for ( unsigned int i= 0 ; i < temp_perform.groups[critical_space_index].space_ID.size() ; i++ )
            {
                selected_spaces.push_back( temp_perform.groups[critical_space_index].space_ID[i]);
            }

            temp_perform.groups.erase( temp_perform.groups.begin() + critical_space_index );
        }

        return selected_spaces;
    } //select_one_or_another_best()


} // namespace Space_Selection
} // namespace HBO
} // namespace BSO

#endif // SPACE_SELECTION_HPP
