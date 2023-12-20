#ifndef FEEDBACK_HPP
#define FEEDBACK_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Structural_Design/SD_Results.hpp>
#include <BSO/Building_Physics/BP_Results.hpp>
#include <BSO/Trim_And_Cast.hpp>

#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>

#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

namespace BSO { namespace HBO { namespace Feedback
{

    void print_building_performances(Performance_Evaluation::Building_Performances& build_perform, std::string& outputfile)
    {
        int amount_of_spaces = 0;

        for ( unsigned int i = 0 ; i < build_perform.groups.size() ; i++)
        {
            amount_of_spaces += build_perform.groups[i].space_ID.size();
        }

        std::cout<<"Properties of a Building Performance Object, outputfile: "<< outputfile<< std::endl;
        std::cout<<"Amount of spaces" << amount_of_spaces<< std::endl;

        std::ofstream logfiles;

        // append the building performance to the logfiles
        logfiles.open(outputfile, std::ios_base::app);
        logfiles<<"Properties of a Building Performance Object: "<< std::endl;
        logfiles<<"Amount of spaces: "<< amount_of_spaces<<std::endl;

        for ( unsigned int i = 0 ; i < build_perform.groups.size() ; i++)
        {
            for( unsigned int j = 0 ; j < build_perform.groups[i].space_ID.size(); j++)
            { // print all IDs belonging to this space
                std::cout<<"Space number(s): "<< build_perform.groups[i].space_ID[j];
                logfiles<<"Space number(s): "<< build_perform.groups[i].space_ID[j];

                if(j == build_perform.groups[i].space_ID.size() - 1)
                {
                    std::cout<< std::endl;
                    logfiles<<std::endl;
                }
            } // end space IDs

            for (unsigned int j = 0 ; j < build_perform.groups[i].initial_performance.size() ; j++ )
            { // print the initial performance of the space
                std::cout<< "Performance number: "<< j +1 << ", value: "<< build_perform.groups[i].initial_performance[j]<< ", ";
                logfiles<<"Performance number: "<< j +1 << ", value: "<< build_perform.groups[i].initial_performance[j] << ", ";

                if(j == build_perform.groups[i].initial_performance.size() - 1 )
                {
                    std::cout<<std::endl;
                    logfiles<< std::endl;
                }
            } // end intitial performances

            if(!build_perform.groups[i].modified_performance.empty())
            { // if a modifed performance exists, print it
                for( unsigned int j = 0 ; j < build_perform.groups[i].modified_performance.size() ; j++ )
                {
                    std::cout<<"Modified performance number: "<< j+1 <<", value: "<< build_perform.groups[i].modified_performance[j]<<", ";
                    logfiles<<"Modified performance number: "<< j+1 <<", value: "<< build_perform.groups[i].modified_performance[j]<< ", ";

                    if( j == build_perform.groups[i].modified_performance.size() - 1 )
                    {
                        std::cout<< std::endl;
                        logfiles<< std::endl;
                    }
                }
            } // end modified performances

        } // end for loop build perform

        return;
    } // end print building performances

    void print_space_id_list(std::vector<int>& space_id_list, std::string& outputfile)
    {
        std::cout<< "List of the spaces selected for removal, amount selected: "<< space_id_list.size()<<"."<< std::endl;

        std::ofstream logfiles;

        logfiles.open(outputfile, std::ios_base::app);
        logfiles << "List of spaces selected for removal, amount selected: "<< space_id_list.size()<< std::endl;

        for ( unsigned int i = 0 ; i < space_id_list.size() ; i++ )
        {
            std::cout<<"ID number: "<< space_id_list[i] << std::endl;
            logfiles <<"ID number: "<< space_id_list[i]<< std::endl;
        }

        return;
    } // end print_space_id_list


}   // namespace feedback
}   // namespace HBO
} // namespace BSO
#endif // FEEDBACK_HPP
