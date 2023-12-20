#ifndef MODIFY_PERFORMANCES_HPP
#define MODIFY_PERFORMANCES_HPP

#include <BSO/HBO/HBO_Settings.hpp>
#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>

#include <iostream>
#include <vector>
#include <cmath>

namespace BSO{ namespace HBO { namespace Performance_Evaluation
{

    HBO::Performance_Evaluation::Building_Performances modify_performances( HBO::Performance_Evaluation::Building_Performances& build_perform, HBO::Settings& settings)
    {
        HBO::Performance_Evaluation::Building_Performances temp_build_perform = build_perform;

        for ( unsigned int i = 0 ; i < build_perform.groups[0].initial_performance.size()  ; i++ )
        {
            std::cout<<build_perform.groups[0].initial_performance.size()<<std::endl;
            double weight = 1.0 / build_perform.groups[0].initial_performance.size();
            settings.weights.push_back(  weight );
            std::cout<< 1.0 / build_perform.groups[0].initial_performance.size()<<", "<< weight <<std::endl;
            std::cout<<settings.weights[i]<< std::endl;
        }

        for( unsigned int i = 0 ; i < temp_build_perform.groups.size() ; i++ )
        {
            for ( unsigned int j = 0 ; j < temp_build_perform.groups[i].initial_performance.size() ; j++)
            {
                temp_build_perform.groups[i].modified_performance.push_back(temp_build_perform.groups[i].initial_performance[j] * settings.weights[j]);
                std::cout<<temp_build_perform.weights[j];
            }
        }

        return temp_build_perform;
    }

} // namespace Performance_Evaluation
} // namespace HBO
} // namespace BSO

#endif // MODIFY_PERFORMANCES_HPP
