#ifndef AGGREGATE_DISCIPLINES_HPP
#define AGGREGATE_DISCIPLINES_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Structural_Design/SD_Results.hpp>
#include <BSO/Building_Physics/BP_Results.hpp>
#include <BSO/Trim_And_Cast.hpp>

#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>
#include <BSO/HBO/HBO_Settings.hpp>


#include <iostream>
#include <vector>
#include <cmath>

namespace BSO { namespace HBO { namespace Performance_Evaluation {

        /*
        Functions that aggregate space performances
        */

        Performance_Evaluation::Building_Performances aggregate_performances(Performance_Evaluation::Building_Performances& build_perform, Settings& settings)
        {
            int space_count;
            Performance_Evaluation::Building_Performances temp_build_perform = build_perform;

            for (unsigned int i = 0 ; i < temp_build_perform.groups.size() ; i++)
            {
                switch(settings.aggregate_options)
                {
                case aggregate_disciplines::SUMMATION:
                    for(unsigned int j = 0 ; j < temp_build_perform.groups[i].initial_performance.size() ; j++ )
                    {
                        if( j == 0)
                        {
                            temp_build_perform.groups[i].modified_performance.push_back(temp_build_perform.groups[i].initial_performance[j]);
                        }
                        else if(j != 0)
                        {
                            temp_build_perform.groups[i].modified_performance[0] += temp_build_perform.groups[i].initial_performance[j] ;
                        }
                        else{ std::cout<<"Error in summation, exiting now... (Aggregate_Performances.hpp)";  exit(1);  }
                    }
                    break;

                case aggregate_disciplines::PRODUCT:
                    for( unsigned int j = 0 ; j < temp_build_perform.groups[i].initial_performance.size() ; j++ )
                    {
                        if( j == 0 )
                        {
                            temp_build_perform.groups[i].modified_performance.push_back(temp_build_perform.groups[i].initial_performance[j]);
                        }
                        else if( j != 0)
                        {
                            temp_build_perform.groups[i].modified_performance[0] *= temp_build_perform.groups[i].initial_performance[j];
                        }
                        else { std::cout<< "Error in product, exiting now... (Aggregate_Performances.hpp)"; exit(1);}
                    }
                    break;

                case aggregate_disciplines::DISTANCE:
                    for(unsigned int j = 0 ; j < temp_build_perform.groups[i].initial_performance.size() ; j++)
                    {
                        if( j == 0 )
                        {
                            temp_build_perform.groups[i].modified_performance.push_back(pow(temp_build_perform.groups[i].initial_performance[j], 2)) ;
                        }
                        else if(j != 0 )
                        {
                            temp_build_perform.groups[i].modified_performance[0] += pow(temp_build_perform.groups[i].initial_performance[j], 2);
                        }
                        else { std::cout<< "Error in distance, exiting now... (Aggregate_Performances.hpp)"; exit(1);}
                    }
                    sqrt(temp_build_perform.groups[i].modified_performance[0]);
                    break;
                case aggregate_disciplines::ARG_COUNT:
                    std::cout<<"Error ARG_COUNT in aggregate options, exiting now... (Aggregate_Performances.hpp)";
                    exit(1);
                    break;
                default:
                    std::cout<<"Error in aggregate options, exiting now... (Aggregate_Performances.hpp)";
                    exit(1);
                }

            } // for all temp_build.groups

            return temp_build_perform;

        } // aggregate_performances


}   // namespace Performance_evaluation
}   // namespace HBO
} // namespace BSO

#endif // AGGREGATE_DISCIPLINES
