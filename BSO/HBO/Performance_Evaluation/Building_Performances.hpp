#ifndef BUILDING_PERFORMANCES_HPP
#define BUILDING_PERFORMANCES_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Structural_Design/SD_Results.hpp>
#include <BSO/Building_Physics/BP_Results.hpp>
#include <BSO/Data.hpp>
#include <BSO/Clustering.hpp>
#include <BSO/Trim_And_Cast.hpp>

#include <BSO/HBO/HBO_Settings.hpp>

#include <iostream>
#include <vector>
#include <cmath>

namespace BSO{ namespace HBO { namespace Performance_Evaluation
{
    /* Data structures to process performance evaluations */

    struct space_performance
    {
        std::vector<int> space_ID;
        std::vector<double> initial_performance;
        std::vector<double> modified_performance;

        bool id_in_space(int& id) // checks if an ID belongs to this space object
        {
            for ( unsigned int i = 0 ; i < space_ID.size() ; i++ )
            {
                if (space_ID[i] == id ) return true;
            }

            return false;
        }
    };

    class Building_Performances
    {
    private:



    public:
        std::vector<double> weights;
        Building_Performances(); // ctor, creates an empty object
        Building_Performances(BSO::Spatial_Design::MS_Building&, BSO::Structural_Design::SD_Building_Results&, BSO::Building_Physics::BP_Building_Results&, Settings&); //ctor creates an building performance object with initial performances
        Building_Performances(BSO::Spatial_Design::MS_Building&, BSO::Structural_Design::SD_Building_Results&, Settings&);
        Building_Performances(BSO::Spatial_Design::MS_Building&, BSO::Building_Physics::BP_Building_Results&, Settings&);
        ~Building_Performances(); // dtor

        std::vector<space_performance> groups; // contains performance information of the spaces

        int id_count();
        int id_belonging_group(int&); // searches the space vector for the id asked and returns the index of the object in the vector
        int best_space_initial(); //  returns the index with the best initial performance
        int best_space_modified(); //  returns the index with the best modified performance
        int worst_space_initial(); // returns the space index with worst initial performance
        int worst_space_modified(); // return the space index with worst modified performance
    };

    Building_Performances::Building_Performances()
    {

    } //ctor

    Building_Performances::Building_Performances(BSO::Spatial_Design::MS_Building& current_design, BSO::Structural_Design::SD_Building_Results& sd_results, BSO::Building_Physics::BP_Building_Results& bp_results, Settings& settings)
    {
        switch(settings.individual_or_clus)
        {
            case assessment_level::INDIVIDUAL:
                {
                    // for all spaces in ms_building
                for( int i = 0 ; i < current_design.obtain_space_count() ; i++ )
                {
                    space_performance temp_space;
                    temp_space.space_ID.push_back(current_design.obtain_space(i).ID);

                    // search the sd result belonging to it
                    for(unsigned int j = 0 ; j < sd_results.m_spaces.size() ; j++ )
                    {

                        if(temp_space.space_ID.back() == sd_results.m_spaces[j].m_ID)
                        {
                            temp_space.initial_performance.push_back(sd_results.m_spaces[j].m_rel_performance);
                            break;
                        }
                    } // end sd_result

                    // and the bp result belonging to it
                    for(unsigned int j = 0 ; j < bp_results.m_space_results.size() ; j++)
                    {

                        if ( temp_space.space_ID.back() == BSO::trim_and_cast_int(bp_results.m_space_results[j].m_space_ID))
                        {
                            temp_space.initial_performance.push_back(bp_results.m_space_results[j].m_rel_performance);
                            break;
                        }
                    } // end bp_result

                    groups.push_back(temp_space);
                }

                // add weights for each performance value
                for ( unsigned int i = 0 ; i < groups[0].initial_performance.size() ; i++ )
                {
                    weights.push_back(1 / groups[0].initial_performance.size() );
                }

                // check whether all groups have the same amount of performance values
                for ( unsigned int i = 0 ; i < groups.size() -1 ; i++ )
                {
                    if (groups[i].initial_performance.size() == groups[i+1].initial_performance.size() )
                    {
                        continue;
                    }
                    else if( groups[i].initial_performance.size() != groups[i+1].initial_performance.size() )
                    {
                        std::cout<< "Error, amount of performance values not equal... exiting now ( Building_Performances.hpp)";
                        exit(1);
                    }
                    else
                    {
                        std::cout<<"Error in amount of spaces... exiting now (Building_Performances.hpp)";
                        exit(1);
                    }
                }

                break;
                }

            case assessment_level::CLUSTERS:
                {
                 // get the results and put them in a vector (shared ptr for clustering purposes)
                std::shared_ptr<std::vector<BSO::data_point> > data_set = std::make_shared<std::vector<BSO::data_point> >();
                std::vector<int> space_id_list; // to track space ID's across result structures of different disciplines
                unsigned int number_of_spaces = current_design.obtain_space_count(); // determine how many spaces are in the design

                for (unsigned int i = 0; i < number_of_spaces; i++)
                { // for each space in the design
                    // initialise a data point and an ID
                    space_id_list.push_back(current_design.obtain_space(i).ID);
                    BSO::data_point temp = Eigen::Vector2d::Zero();
                    data_set->push_back(temp);
                }

                bool match_check[space_id_list.size()] = {false}; // this list will keep track if a performance has been found for each space

                for (unsigned int i = 0; i < bp_results.m_space_results.size(); i++)
                { // for each space result
                    for (unsigned int j = 0; j < number_of_spaces; j++)
                    { // and for each space in the design
                        if (space_id_list[j] == BSO::trim_and_cast_int(bp_results.m_space_results[i].m_space_ID))
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
                        std::cout << "Error could not find BP performance of space with ID: "
                                  << space_id_list[i] << " (Building_Performances.hpp), exiting now..." << std::endl;
                        exit(1);
                    }
                }

                for (unsigned int i = 0; i < sd_results.m_spaces.size(); i++)
                { // for each space result
                    for (unsigned int j = 0; j < space_id_list.size(); j++)
                    { // and for each space in the design
                        if (space_id_list[j] == sd_results.m_spaces[i].m_ID)
                        { // check if space_result matches with a space in the design
                            (*data_set)[j](1) = sd_results.m_spaces[i].m_total_compliance;
                            match_check[j] = false;
                        } //since the BP check set the values to true SD needs to switch them back to false
                    }
                }

                for (unsigned int i = 0; i < number_of_spaces; i++)
                { // for each space in the design
                    if (match_check[i])
                    { // check if a performance has been found for space i
                        std::cout << "Error could not find structural performance of space with ID: "
                                  << space_id_list[i] << " (Building_Performances.hpp), exiting now..." << std::endl;
                        exit(1);
                    }
                }

                // cluster the data set
                // make k clusters, where k lies between 25% and 75% of the number of spaces with a lower limit of 2 clusters
                //std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set = clustering(data_set, 50, (2 < 1+number_of_spaces/8)? 1+number_of_spaces/4 : 2, 3*number_of_spaces/4, 2);
                std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set = BSO::clustering(data_set, 50, 6, 10, 2);

                for (unsigned int i = 0 ; i < clustered_data_set.size() ; i++ )
                { // for all clusters

                    space_performance temp_group;

                    for(unsigned int j = 0 ; j < clustered_data_set[i]->m_space_id_list.size() ; j++ )
                    { // add space ids to the space performance
                        temp_group.space_ID.push_back(clustered_data_set[i]->m_space_id_list[j]);
                    }
                    for(unsigned int j = 0 ; j < clustered_data_set[i]->m_centroid.size() ; j++ )
                    { // add centroid to initial performance
                        temp_group.initial_performance.push_back(clustered_data_set[i]->m_centroid[j]);
                    }

                    groups.push_back(temp_space);
                }
                for (unsigned int i = 0 ; i < spaces[0].initial_performance.size() ; i++ )
                {
                    weights.push_back(1 / spaces[0].initial_performance.size() );
                }
                break;
                }

            case assessment_level::ARG_COUNT:
                {
                    std::cout<<"Error in Settings::assessment_level, exiting now.... (Building_Performances.hpp)";
                    exit(1);
                    break;
                }

            default:
                {
                    std::cout<<"Error in ctor Building_Performances, exiting now.... ( Building_Performances.hpp)";
                    exit(1);
                }
        } // switch(individual_or_cluster

    } //ctor (MS_Build, SD_results, BP_results, Settings)

    Building_Performances::Building_Performances(BSO::Spatial_Design::MS_Building& current_design, BSO::Structural_Design::SD_Building_Results& sd_results, Settings& settings)
    {
        switch(settings.individual_or_clus)
        {
            case assessment_level::INDIVIDUAL:
                {
                    for ( unsigned int i = 0 ; i < current_design.obtain_space_count() ; i++ )
                    {
                        space_performance temp_group;
                        temp_group.space_ID.push_back(current_design.obtain_space(i).ID);

                        for ( unsigned int j = 0 ; j < sd_results.m_spaces.size() ; j++ )
                        {
                            if ( sd_results.m_spaces[j].m_ID == temp_group.space_ID.back() )
                            {
                                temp_group.initial_performance.push_back( sd_results.m_spaces[j].m_rel_performance);
                                break;
                            }
                        }
                        groups.push_back(temp_group);
                    }
                    break;
                }
            case assessment_level::CLUSTERS:
                {
                    std::cout<<"Error, no clusters are written for solo SD discipline" << std::endl;
                    exit(1);
                }
            case assessment_level::ARG_COUNT:
                {
                    std::cout<<"Error in Settings::assessment_level, exiting now.... (Building_Performances.hpp)";
                    exit(1);
                    break;
                }
            default:
                {
                    std::cout<<"Error in ctor Building_Performances, exiting now.... ( Building_Performances.hpp)";
                    exit(1);
                    break;
                }
        } //switch(individual_or_cluster)

    } // ctor, (MS_build, SD_results, BP_results)

    Building_Performances::Building_Performances(BSO::Spatial_Design::MS_Building& current_design, BSO::Building_Physics::BP_Building_Results& bp_results, Settings& settings)
    {
        switch(settings.individual_or_clus)
        {
            case assessment_level::INDIVIDUAL:
                {
                    for ( unsigned int i = 0 ; i < current_design.obtain_space_count() ; i++ )
                    {
                        space_performance temp_group;
                        temp_group.space_ID.push_back(current_design.obtain_space(i).ID);

                        for ( unsigned int j = 0 ; j < bp_results.m_space_results.size() ; j++ )
                        {
                            if ( BSO::trim_and_cast_int(bp_results.m_space_results[j].m_space_ID) == temp_group.space_ID.back() )
                        }
                        groups.push_back(temp_group);
                    }
                    break;
                }
            case assessment_level::CLUSTERS:
                {
                        std::cout<<"Error in Settings::assessment_level, exiting now.... (Building_Performances.hpp)";
                        exit(1);
                        break;
                }
            case assessment_level::ARG_COUNT:
                {
                    std::cout<<"Error in Settings::assessment_level, exiting now.... (Building_Performances.hpp)";
                    exit(1);
                    break;
                }
            default:
                {
                        std::cout<<"Error in ctor Building_Performances, exiting now.... ( Building_Performances.hpp)";
                        exit(1);
                        break;
                }
        }
    }


     Building_Performances::~Building_Performances()
     {

     } //dtor

     int Building_Performances::id_count()
     {
         int id_count = 0;
         for(unsigned int i = 0 ; i < groups.size() ; i++ )
         {
             id_count += groups[i].space_ID.size();
         }

         return id_count;
     }

     int Building_Performances::id_belonging_group(int& id)
     {
         for ( unsigned int i = 0 ; i < groups.size() ; i++ )
         {
             for (unsigned int j = 0 ; j < groups[i].space_ID.size() ; j++ )
             {
                 if ( groups[i].space_ID[j] == id)
                 {
                     return i;
                 }
             }

             // if no space objects can be found with the id give error and exit
             std::cout<< "Error no space_ID :" << id <<" can be found, exiting now... ( Building_Performances.hpp)"<<std::endl;
             exit(1);
         }
     }

     int Building_Performances::best_space_initial()
     {
         int best_index = 0;
         double best_performance = groups[0].initial_performance[0];

         for ( unsigned int i = 0 ; i < groups.size() ; i++ )
         {
             for ( unsigned int j = 0 ; j < groups[i].initial_performance.size() ; j++ )
             {
                 if( groups[i].initial_performance[j] > best_performance )
                 {
                     best_performance = groups[i].initial_performance[j];
                     best_index = i;
                 }
             }
         }
         return best_index;
     }

     int Building_Performances::best_space_modified()
     {
        int best_index = 0;
        double best_performance = groups[0].modified_performance[0];

         for ( unsigned int i = 0 ; i < groups.size() ; i++ )
         {
             for ( unsigned int j = 0 ; j < groups[i].modified_performance.size() ; j++ )
             {
                if ( groups[i].modified_performance[j] > best_performance )
                {
                    best_performance = groups[i].modified_performance[j];
                    best_index = i;
                }
             }
         }
         return best_index;
     }

     int Building_Performances::worst_space_initial()
     {
         int worst_index = 0;
         double worst_performance = groups[0].initial_performance[0];

         for ( unsigned int i = 0 ; i < groups.size()  ; i++)
         {
             for ( unsigned int j = 0 ; j < groups[i].initial_performance.size() ; j++ )
             {
                 if ( groups[i].initial_performance[j] < worst_performance )
                 {
                     worst_performance = groups[i].initial_performance[j];
                     worst_index = i;
                 }
             }
         }
         return worst_index;
     }

     int Building_Performances::worst_space_modified()
     {
         int worst_index = 0;
         double worst_performance = groups[0].initial_performance[0];

         for ( unsigned int i = 0 ; i < groups.size() ; i++ )
         {
             for (unsigned int j = 0 ; groups[i].modified_performance.size() ; j++ )
             {
                 if ( groups[i].modified_performance[j] < worst_performance )
                 {
                     worst_performance = groups[i].modified_performance[j];
                     worst_index = i;
                 }
             }
         }
         return worst_index;
     }


} // namespace Performance_Evaluation
} // namespace HBO
} // namespace BSO

#endif // BUILDING_PERFORMANCES
