#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Supercube.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Building_Physics/BP_Simulation.hpp>
#include <BSO/Data.hpp>

#include <BSO/Performance_Indexing.hpp>
#include <BSO/Heuristics.hpp>

#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>

namespace BSO {

//bool check_constraint(std::string);

template<typename stream_type>
void Opt_func_1(std::string input_file, unsigned int n_ite, Spatial_Design::Grammar_Ptr grammar, std::string output_file, stream_type &output)
{
    std::vector<Spatial_Design::MS_Building> building_designs;
    std::vector<Structural_Design::SD_Building_Results> sd_results(n_ite+1);
    std::vector<Building_Physics::BP_Building_Results> bp_results(n_ite+1);

    // init MS-file
    building_designs.push_back(Spatial_Design::MS_Building(input_file.c_str()));

    for (unsigned int i = 0; i < n_ite+1; i++)
    {
        Spatial_Design::MS_Conformal CF(building_designs[i], grammar);
        CF.make_conformal();

        Structural_Design::SD_Analysis SD_building(CF);
        SD_building.analyse();
        sd_results[i] = SD_building.get_results();

        SD_compliance_indexing(sd_results[i]);

        Building_Physics::BP_Simulation BP_building(CF);
        BP_building.sim_period();

        bp_results[i] = BP_building.get_results();
        BP_thermal_demand_indexing(bp_results[i]);

        if (i != n_ite)
        {
            building_designs.push_back(change_1_space(building_designs[i], bp_results[i], sd_results[i], 0));
        }

        output << i << " ";
        output << sd_results[i].m_total_compliance << " ";
        output << bp_results[i].m_total_energy << " ";
        output << Spatial_Design::SC_Building(building_designs[i]).write_string() << std::endl;
    }

    output << n_ite << " ";
    output << sd_results.back().m_total_compliance << " ";
    output << bp_results.back().m_total_energy << " ";
    output << Spatial_Design::SC_Building(building_designs.back()).write_string() << std::endl;

    // select the best design now
    // put all data points in a vector
    std::vector<BSO::data_point> data_set;
    for (unsigned int i = 0; i < building_designs.size(); i++)
    {
        BSO::data_point temp_point = Eigen::Vector2d::Zero();
        temp_point(0) = sd_results[i].m_total_compliance;
        temp_point(1) = bp_results[i].m_total_energy;
        data_set.push_back(temp_point);
    }

    // find out what the minimum and maximum point are
    BSO::data_point min = data_set[0];
    BSO::data_point max = data_set[0];

    for (unsigned int i = 0; i < data_set.size(); i++)
    {
        for (unsigned int j = 0; j < data_set[i].rows();j++)
        {
            if (min(j) > data_set[i](j))
            {
                min(j) = data_set[i](j);
            }
            if (max(j) < data_set[i](j))
            {
                max(j) = data_set[i](j);
            }
        }
    }

    // initialise the distance function
    BSO::dist_function d_func;
    d_func.m_min = min;
    d_func.m_max = max;
    d_func.m_tag = BSO::dist_function::Normalised;

    std::string design;
    unsigned int selected_design;
    /*do
    {*/
        // find the point that lies closest to the minimum point ánd satisfies the constraints
        selected_design = BSO::find_closest_to(data_set, min, d_func);

        Spatial_Design::SC_Building Selected_SC(building_designs[selected_design]);
        Selected_SC.add_padding(1,1,1,1000);
        design = Selected_SC.write_string();
        //data_set.erase(data_set.begin() + selected_design);
    //} while (check_constraint(design));

    //Spatial_Design::SC_Building Selected_SC(building_designs[selected_design]);
    //Selected_SC.add_padding(1,1,1,1000);
    Selected_SC.write_file(output_file);
}
/*
bool check_constraint(std::string design)
{
    std::ofstream p_infile("opt.out");
    int space_count = 0;
    for (unsigned int i = 0; i < design.size(); i++)
    {
        if (design[i] == ' ')
        {
            space_count++;
            if (space_count > 3)
            {
                design[i] = '\n';
            }
        }
    }
    design = "0 " + design;
    p_infile << design;

    p_infile.close();
    //(void) freopen ("myfile.txt","w",stdout);
    (void) system("./opt2sc_cnt_cnstrnt opt.out cnstrnt.out -c");
    //fclose (stdout);

    bool check = true;

    std::ifstream check_file("a.cnstrnt");
    std::string check_line;
    getline(check_file, check_line);

    for (unsigned int i = 0; i < check_line.size(); i++)
    {
        char c = check_line[i];
        if (!(c == ' ' || c == '\t' || c == '0'))
        {
            check = false;
        }
    }
    return check;
}
*/
} // namespace BSO
