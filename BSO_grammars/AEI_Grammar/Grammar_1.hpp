#ifndef GRAMMAR_1_HPP
#define GRAMMAR_1_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <BSO/Spatial_Design/Conformation.hpp>

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Simulation.hpp>
#endif // BP_SIMULATION_HPP

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Analysis.hpp>
#endif // SD_ANALYSIS_HPP

#include <BSO/Visualisation/Model_Module/Model.hpp>


#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

namespace BSO { namespace Grammar
{
    /*
     *
     */

    // forward declarations
    #ifdef BP_SIMULATION_HPP
    void BP_grammar_1(Spatial_Design::MS_Conformal*, Building_Physics::BP_Simulation_Vars*);
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void SD_grammar_1(Spatial_Design::MS_Conformal*, Structural_Design::SD_Analysis_Vars*);
    #endif // SD_ANALYSIS_HPP

    // function definition
    void grammar_1(Spatial_Design::MS_Conformal* MS) // assign geometric types to the building geometry in a conformal building model
    {
        for (unsigned int i = 0; i < MS->get_space_count(); i++)
        {
            MS->get_space(i)->assign_space_type(1); // there is only one space type in this grammar
        }

        for (unsigned int i = 0; i < MS->get_surface_count(); i++)
        {
            Vectors::Vector pos_y;
            pos_y << 0,1,0;
            Vectors::Vector surf_normal = *MS->get_surface(i)->get_encasing_rectangle().get_normal_ptr();

            if (Vectors::is_paral(pos_y, surf_normal, 0.01))
            {
                MS->get_surface(i)->assign_surface_type(2); // non structural surface type
            }
            else
            {
                MS->get_surface(i)->assign_surface_type(2); // structural surface type
            }
        }

        for (unsigned int i = 0; i < MS->get_edge_count(); i++)
        {
            MS->get_edge(i)->assign_edge_type(2); // non structural surface type
        }

        #ifdef SD_ANALYSIS_HPP
        MS->add_grammars(&SD_grammar_1);
        #endif // SD_ANALYSIS_HPP

        #ifdef BP_SIMULATION_HPP
        MS->add_grammars(&BP_grammar_1);
        #endif // BP_SIMULATION_HPP

    } // grammar_1 geometry types








    #ifdef BP_SIMULATION_HPP
    void BP_grammar_1(Spatial_Design::MS_Conformal* CF, Building_Physics::BP_Simulation_Vars* BPS)
    {
        Building_Physics::BP_Simulation* BP_ptr = static_cast<Building_Physics::BP_Simulation*>(BPS);

        struct Vis_Setting
        {
            std::string m_ID;
            std::shared_ptr<BSO::Visualisation::polygon_props> m_vis_settings;
        };

        std::vector<Vis_Setting> vis_settings;

        BPS->m_visualisation_model = true; // this grammar will add visualisation settings to the BP_Model
        std::string input_file = "0_Settings/Grammars/BP_Settings.txt";
        std::fstream input(input_file.c_str()); // open a file stream
        if (!input)
        {
            std::cout << "Error, could not open file "
                      << "\"" << input_file << "\""
                      << ", exiting..." << std::endl;
        }
        std::string line; // initialise a string to hold lines being read from file
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
        char type_ID; // holds information about what type of information is described by the line currently read

        while(!input.eof())
        {
            getline(input, line); // get the next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }
            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            type_ID = trim_and_cast_char(*token); // interpret first token as type ID

            switch (type_ID)
            {
            case 'B':
                {
                    token++; // next token holds number of warm up days
                    BPS->m_warm_up_days = trim_and_cast_int(*token);
                    break;
                }
            case 'C':
                {
                    token++; //next token holds number of time steps per hour
                    BPS->m_time_step_hour = trim_and_cast_int(*token);
                    break;
                }
            case 'D':
                {
                    Building_Physics::BP_Space_Settings space_settings;
                    token++; // space_set_ID
                    space_settings.m_space_set_ID = *token;
                    boost::algorithm::trim(space_settings.m_space_set_ID);
                    token++; // heating capacity
                    space_settings.m_heating_capacity = trim_and_cast_double(*token);
                    token++; // cooling capacity
                    space_settings.m_cooling_capacity = trim_and_cast_double(*token);
                    token++; // heating set point
                    space_settings.m_heat_set_point = trim_and_cast_double(*token);
                    token++; // cooling set point
                    space_settings.m_cool_set_point = trim_and_cast_double(*token);
                    token++; // air changes per hour
                    space_settings.m_ACH = trim_and_cast_double(*token);

                    BPS->m_space_settings.push_back(space_settings);
                    break;
                }
            case 'E':
                {
                   int year, month, day, hour; // initialize variables to temporarily hold data
                    std::string day_name; // initialize string to temporarily hold day name

                    token++; // start year
                    year = trim_and_cast_int(*token);
                    token++; // start month
                    month = trim_and_cast_int(*token);
                    token++; // start day
                    day = trim_and_cast_int(*token);
                    token++; // start day name
                    day_name = *token;
                    boost::algorithm::trim(day_name); // remove white space in front and at end of string
                    token++; // start hour
                    hour = trim_and_cast_int(*token);
                    BPS->m_begin = boost::posix_time::ptime(boost::gregorian::date(year, month, day), boost::posix_time::hours(hour));

                    token++; // finish year
                    year = trim_and_cast_int(*token);
                    token++; // finish month
                    month = trim_and_cast_int(*token);
                    token++; // finish day
                    day = trim_and_cast_int(*token);
                    token++; // finish hour
                    hour = trim_and_cast_int(*token);
                    BPS->m_end = boost::posix_time::ptime(boost::gregorian::date(year, month, day), boost::posix_time::hours(hour));

                    BPS->m_weather_profile = new Building_Physics::BP_Weather_Profile(BP_ptr, ""); // it is important that the nr of warm up days is declared before the weather profile (consider change in future)
                    BPS->m_indep_states.push_back(BPS->m_weather_profile);
                    BPS->m_states.push_back(BPS->m_weather_profile);
                    break;
                }
            case 'F':
                {
                    double temperature;
                    token++; // Constant ground temperature
                    temperature = trim_and_cast_double(*token);
                    BPS->m_ground_profile = new Building_Physics::BP_Ground_Profile(BP_ptr, temperature);
                    BPS->m_indep_states.push_back(BPS->m_ground_profile);
                    BPS->m_states.push_back(BPS->m_ground_profile);
                    break;
                }
            case 'G':
                {
                    Building_Physics::BP_Material material; // initialize to temporarily hold data
                    token++; // Material_ID
                    material.m_material_ID = *token;
                    boost::algorithm::trim(material.m_material_ID);
                    token++; // material name
                    material.m_name = *token;
                    boost::algorithm::trim(material.m_name);
                    token++; // specific weight
                    material.m_spec_weight = trim_and_cast_double(*token);
                    token++; // specific heat
                    material.m_spec_heat = trim_and_cast_double(*token);
                    token++; // thermal conductivity
                    material.m_therm_conductivity = trim_and_cast_double(*token);
                    BPS->m_materials.push_back(material);
                    break;
                }
            case 'H':
                {
                    Building_Physics::BP_Construction construction; // initialize to temporarily hold data
                    token++; // Construction_ID
                    construction.m_construction_ID = *token;
                    boost::algorithm::trim(construction.m_construction_ID); // remove white space at front and end of string
                    std::vector<std::string> layer_mat_ID; // will hold material ID of all layers
                    std::vector<double> layer_thicknesses; // will hold thicknesses of all layers
                    token++; // material ID
                    while (token != tok.end()) // read indefinite number of layers into the construction
                    {
                        std::string mat_ID = *token;
                        boost::algorithm::trim(mat_ID);
                        layer_mat_ID.push_back(mat_ID);
                        token++; // layer thickness
                        layer_thicknesses.push_back(trim_and_cast_double(*token));
                        token++; // Material ID or final token
                    }

                    if (layer_mat_ID.size() != layer_thicknesses.size())
                    {
                        std::cout << "Error in reading construction info, exiting..." << std::endl;
                        exit(1);
                    }
                    Building_Physics::BP_Con_Layer layer_entry; // initialize a layer structure to temporarily hold data

                    construction.m_total_thickness = 0; // initialise to zero
                    for (unsigned int i = 0; i < layer_mat_ID.size(); i++)
                    {
                        for (unsigned int j = 0; j < BPS->m_materials.size(); j++)
                        {
                            if (layer_mat_ID[i] == BPS->m_materials[j].m_material_ID)
                            {
                                layer_entry.m_material = BPS->m_materials[j];
                                layer_entry.m_thickness = layer_thicknesses[i];
                                construction.m_layers.push_back(layer_entry);
                                construction.m_total_thickness += layer_thicknesses[i];
                                break;
                            }
                            else if (j == (BPS->m_materials.size() - 1))
                            {
                                std::cout << "Error in processing construction info, exiting..." << std::endl;
                            }
                        }
                    }

                    //compute capacitance
                    construction.m_capacitance_per_area = 0; // initialise value to zero
                    for (unsigned int i = 0; i < construction.m_layers.size(); i++) // add each layer's contribution
                    {
                        construction.m_capacitance_per_area += (construction.m_layers[i].m_thickness/1000) * construction.m_layers[i].m_material.m_spec_heat * construction.m_layers[i].m_material.m_spec_weight; // capacitance equals thickness*specific heat* speciic weight
                    }

                    //location of measure point
                    construction.m_measure_point = 0.5; // relative location, initialise to centre of wall, may need to be with respect to equal capacitance at each side?
                    double abs_measure_point = construction.m_measure_point * construction.m_total_thickness; // the absolute value of the distance from surface on side one to the temperature measure point

                    construction.m_resistance_to_side_1 = 0; //initialise value to zero
                    construction.m_resistance_to_side_2 = 0; //initialise value to zero

                    // compute resistances (per square metre) to each side
                    if (construction.m_measure_point >= 0.0 && construction.m_measure_point <= 1.0) // see if the measure point lies within the construction
                    {
                        for (unsigned int i = 0; i < construction.m_layers.size(); i++) // cycle through each layer
                        {
                            // resistance to side 1
                            if (construction.m_layers[i].m_thickness <= abs_measure_point) // if true, this layer contributes to the resistance to flux to side 1
                            {
                                construction.m_resistance_to_side_1 += (construction.m_layers[i].m_thickness/1000)/(construction.m_layers[i].m_material.m_therm_conductivity); // resistance per square metre equals thickness(m)/thermal conductivity
                                abs_measure_point -= construction.m_layers[i].m_thickness; // decrement the distance to the temperature measure point
                            }
                            else if (abs_measure_point > 0) // if true, part of this layer contributes to resistance to flux to side 1 and the other to the resistance of flux to side 2
                            {
                                construction.m_resistance_to_side_1 += (abs_measure_point/1000)/(construction.m_layers[i].m_material.m_therm_conductivity); // add resistance at side 1 of the measure point
                                abs_measure_point -= construction.m_layers[i].m_thickness; // decrement distance to measure point by layer thickness to add remainder of the current layer to the resistance to flux to side two
                                construction.m_resistance_to_side_2 += ((-1.0*abs_measure_point)/1000)/(construction.m_layers[i].m_material.m_therm_conductivity); // add resistance at side 2 of the measure point
                            }
                            else // otherwise this layer (and the remainder) contributes to the resistance to flux to side 2
                            {
                                construction.m_resistance_to_side_2 += (construction.m_layers[i].m_thickness/1000)/(construction.m_layers[i].m_material.m_therm_conductivity); // the remaining layers contribute to the resistance to flux to side two of the measure point
                            }
                        }
                    }
                    else
                    {
                        std::cout << "Temperature measure point is outside construction, exiting...." << std::endl;
                        exit(1);
                    }

                    BPS->m_constructions.push_back(construction);
                    break;
                }
            case 'I':
                {
                    Building_Physics::BP_Glazing glazing;
                    token++; // glazing_ID
                    glazing.m_glazing_ID = *token;
                    boost::algorithm::trim(glazing.m_glazing_ID);
                    token++; // U-value
                    glazing.m_U_value = trim_and_cast_double(*token);
                    token++; // Capacitance per area
                    glazing.m_capacitance_per_area = trim_and_cast_double(*token);
                    BPS->m_glazings.push_back(glazing);
                    break;
                }
            case 'J':
                {
                    Vis_Setting temp;

                    token++; // vis_ID
                    temp.m_ID = *token;
                    boost::algorithm::trim(temp.m_ID);
                    token++; // red
                    double red = trim_and_cast_double(*token);
                    token++; // green
                    double green = trim_and_cast_double(*token);
                    token++; // blue
                    double blue = trim_and_cast_double(*token);
                    token++; // alpha
                    double alpha = trim_and_cast_double(*token);

                    temp.m_vis_settings = std::shared_ptr<BSO::Visualisation::polygon_props>(std::make_shared<BSO::Visualisation::polygon_props>());
                    temp.m_vis_settings->ambient = BSO::Visualisation::rgba(red, green, blue, alpha);
                    temp.m_vis_settings->diffuse = BSO::Visualisation::rgba(std::min(1.0, 2*red), std::min(1.0, 2*green), std::min(1.0, 2*blue), alpha);
                    temp.m_vis_settings->specular = BSO::Visualisation::rgba(0.0, 0.0, 0.0, alpha);
                    temp.m_vis_settings->emission = BSO::Visualisation::rgba(0.0, 0.0, 0.0, alpha);
                    temp.m_vis_settings->shininess = 0.0;
                    temp.m_vis_settings->translucent = true;
                    temp.m_vis_settings->twosided = true;
                    vis_settings.push_back(temp);
                    break;
                }
            default:
                break;
            } // end switch statement
        } // end while (finished reading file)



        for (unsigned int i = 0; i < CF->get_space_count(); i++)
        {
            std::string space_ID = std::to_string(CF->get_space(i)->get_ID());
            double volume = CF->get_space(i)->get_encasing_cuboid().get_volume()/1e9 ;
            Building_Physics::BP_Space_Settings space_settings;

            for (unsigned int j = 0; j < BPS->m_space_settings.size(); j++)
            {
                if (BPS->m_space_settings[j].m_space_set_ID == "1")
                {
                    space_settings = BPS->m_space_settings[j];
                }
            }
            Building_Physics::BP_Space* space_ptr = new Building_Physics::BP_Space(BP_ptr, space_ID, volume, space_settings, BPS->m_weather_profile); // initialise a ptr to an object of the space class with the values read from the file
            BPS->m_space_ptrs.push_back(space_ptr); // add the space pointer to the space vector
            BPS->m_dep_states.push_back(space_ptr);
            BPS->m_states.push_back(space_ptr);
        }

        unsigned int wall_counter = 0;
        unsigned int window_counter = 0;
        unsigned int floor_counter = 0;

        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        {
            unsigned int surface_count = CF->get_rectangle(i)->get_surface_count();

            if (surface_count == 0 || surface_count > 2)
            {
                continue;
            }

            double area = CF->get_rectangle(i)->get_area()/(1e6);

            Vectors::Vector pos_x; pos_x << 1,0,0;
            Vectors::Vector pos_y; pos_y << 0,1,0;
            Vectors::Vector pos_z; pos_z << 0,0,1;

            Building_Physics::BP_State* state_ptr_side_1 = nullptr;
            Building_Physics::BP_State* state_ptr_side_2 = nullptr;

            int geo_type_1 = CF->get_rectangle(i)->get_surface_ptr(0)->get_surface_type();
            if (geo_type_1 == 0)
            {
                std::cout << "Error in BP_grammar_1 (a), exiting..." << std::endl;
                exit(1);
            }

            std::string Space_ID_1 = std::to_string(CF->get_rectangle(i)->get_surface_ptr(0)->get_space_ptr(0)->get_ID());
            bool side_1_found = false;

            if (surface_count == 1)
            {
                for (unsigned int j = 0; j < BPS->m_space_ptrs.size(); j++)
                {
                    if (BPS->m_space_ptrs[j]->get_ID() == Space_ID_1)
                    {
                        state_ptr_side_1 = BPS->m_space_ptrs[j];
                        side_1_found = true;
                    }
                }

                if (!side_1_found)
                {
                    std::cout << "Error in BP_grammar_1 (b), exiting..." << std::endl;
                    exit(1);
                }

                if (Vectors::is_paral(pos_z, *CF->get_rectangle(i)->get_normal_ptr(), 0.01))
                { // floor

                    Building_Physics::BP_Construction construction;
                    bool construction_found = false;
                    for (unsigned int j = 0; j < BPS->m_constructions.size(); j++)
                    {
                        if (BPS->m_constructions[j].m_construction_ID == "1")
                        {
                            construction = BPS->m_constructions[j];
                            construction_found = true;
                        }
                    }
                    if (!construction_found)
                    {
                        std::cout << "Error in BP_grammar (c), exiting..." << std::endl;
                        exit(1);
                    }

                    if ((int)CF->get_rectangle(i)->get_vertex_ptr(0)->get_coords()(2) <= 0)
                    { // ground floor

                        state_ptr_side_2 = BPS->m_ground_profile;

                        Building_Physics::BP_Floor* floor_ptr = new Building_Physics::BP_Floor(BP_ptr, std::to_string(++floor_counter), area, construction, state_ptr_side_1, state_ptr_side_2);

                        Vis_Setting temp_vis_setting;
                        bool vis_settings_found = false;

                        for (unsigned int i = 0; i < vis_settings.size(); i++)
                        {
                            if (vis_settings[i].m_ID == "1")
                            {
                                vis_settings_found = true;
                                temp_vis_setting = vis_settings[i];
                            }
                        }

                        if (!vis_settings_found)
                        {
                            std::cout << "Could not find the vis_settings profile for floor, exiting now..." << std::endl;
                            exit(1);
                        }

                        floor_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                        BPS->m_floor_ptrs.push_back(floor_ptr); // add the floor pointer to the floor vector
                        BPS->m_dep_states.push_back(floor_ptr);
                        BPS->m_states.push_back(floor_ptr);
                    }
                    else
                    { // roof

                        state_ptr_side_2 = BPS->m_weather_profile;

                        Building_Physics::BP_Floor* floor_ptr = new Building_Physics::BP_Floor(BP_ptr, std::to_string(++floor_counter), area, construction, state_ptr_side_1, state_ptr_side_2);

                        Vis_Setting temp_vis_setting;
                        bool vis_settings_found = false;

                        for (unsigned int i = 0; i < vis_settings.size(); i++)
                        {
                            if (vis_settings[i].m_ID == "1")
                            {
                                vis_settings_found = true;
                                temp_vis_setting = vis_settings[i];
                            }
                        }

                        if (!vis_settings_found)
                        {
                            std::cout << "Could not find the vis_settings profile for floor, exiting now..." << std::endl;
                            exit(1);
                        }

                        floor_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                        BPS->m_floor_ptrs.push_back(floor_ptr); // add the floor pointer to the floor vector
                        BPS->m_dep_states.push_back(floor_ptr);
                        BPS->m_states.push_back(floor_ptr);
                    }
                }
                else
                { // exterior wall or window
                    state_ptr_side_2 = BPS->m_weather_profile;

                    if (geo_type_1 == 1)
                    { // window

                        Building_Physics::BP_Glazing glazing;
                        bool glazing_found = false;
                        for (unsigned int j = 0; j < BPS->m_constructions.size(); j++)
                        {
                            if (BPS->m_glazings[j].m_glazing_ID == "1")
                            {
                                glazing = BPS->m_glazings[j];
                                glazing_found = true;
                            }
                        }
                        if (!glazing_found)
                        {
                            std::cout << "Error in BP_grammar (d), exiting..." << std::endl;
                            exit(1);
                        }

                        Building_Physics::BP_Window* window_ptr = new Building_Physics::BP_Window(BP_ptr, std::to_string(++window_counter), area, glazing, state_ptr_side_1, state_ptr_side_2);

                        Vis_Setting temp_vis_setting;
                        bool vis_settings_found = false;

                        for (unsigned int i = 0; i < vis_settings.size(); i++)
                        {
                            if (vis_settings[i].m_ID == "2")
                            {
                                vis_settings_found = true;
                                temp_vis_setting = vis_settings[i];
                            }
                        }

                        if (!vis_settings_found)
                        {
                            std::cout << "Could not find the vis_settings profile for window, exiting now..." << std::endl;
                            exit(1);
                        }

                        window_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                        BPS->m_window_ptrs.push_back(window_ptr); // add the wall pointer to the wall vector
                        BPS->m_dep_states.push_back(window_ptr);
                        BPS->m_states.push_back(window_ptr);
                    }
                    else if (geo_type_1 == 2)
                    { // wall

                        Building_Physics::BP_Construction construction;
                        bool construction_found = false;
                        for (unsigned int j = 0; j < BPS->m_constructions.size(); j++)
                        {
                            if (BPS->m_constructions[j].m_construction_ID == "1")
                            {
                                construction= BPS->m_constructions[j];
                                construction_found = true;
                            }
                        }
                        if (!construction_found)
                        {
                            std::cout << "Error in BP_grammar (e), exiting..." << std::endl;
                            exit(1);
                        }

                        Building_Physics::BP_Wall* wall_ptr = new Building_Physics::BP_Wall(BP_ptr, std::to_string(++wall_counter), area, construction, state_ptr_side_1, state_ptr_side_2);

                        Vis_Setting temp_vis_setting;
                        bool vis_settings_found = false;

                        for (unsigned int i = 0; i < vis_settings.size(); i++)
                        {
                            if (vis_settings[i].m_ID == "1")
                            {
                                vis_settings_found = true;
                                temp_vis_setting = vis_settings[i];
                            }
                        }

                        if (!vis_settings_found)
                        {
                            std::cout << "Could not find the vis_settings profile for wall, exiting now..." << std::endl;
                            exit(1);
                        }

                        wall_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                        BPS->m_wall_ptrs.push_back(wall_ptr); // add the wall pointer to the wall vector
                        BPS->m_dep_states.push_back(wall_ptr);
                        BPS->m_states.push_back(wall_ptr);
                    }
                    else
                    {
                        std::cout << "Error in BP_grammar (f), exiting..." << std::endl;
                        exit(1);
                    }
                }
                continue;
            }

            int geo_type_2 = CF->get_rectangle(i)->get_surface_ptr(1)->get_surface_type();
            if (geo_type_2 == 0)
            {

                std::cout << "Error in BP_grammar_1 (f), exiting..." << std::endl;
                exit(1);
            }

            std::string Space_ID_2 = std::to_string(CF->get_rectangle(i)->get_surface_ptr(1)->get_space_ptr(0)->get_ID());
            bool side_2_found = false;

            if (surface_count == 2)
            {
                for (unsigned int j = 0; j < BPS->m_space_ptrs.size(); j++)
                {
                    if (BPS->m_space_ptrs[j]->get_ID() == Space_ID_1)
                    {
                        state_ptr_side_1 = BPS->m_space_ptrs[j];
                        side_1_found = true;
                    }
                    if (BPS->m_space_ptrs[j]->get_ID() == Space_ID_2)
                    {
                        state_ptr_side_2 = BPS->m_space_ptrs[j];
                        side_2_found = true;
                    }
                }

                if (!(side_1_found && side_2_found))
                {
                    std::cout << "Error in BP_grammar_1 (g), exiting..." << std::endl;
                    exit(1);
                }

                if (Vectors::is_paral(pos_z, *CF->get_rectangle(i)->get_normal_ptr(), 0.01))
               { // floor

                    Building_Physics::BP_Construction construction;
                    bool construction_found = false;
                    for (unsigned int j = 0; j < BPS->m_constructions.size(); j++)
                    {
                        if (BPS->m_constructions[j].m_construction_ID == "1")
                        {
                            construction = BPS->m_constructions[j];
                            construction_found = true;
                        }
                    }
                    if (!construction_found)
                    {
                        std::cout << "Error in BP_grammar (h), exiting..." << std::endl;
                        exit(1);
                    }

                    Building_Physics::BP_Floor* floor_ptr = new Building_Physics::BP_Floor(BP_ptr, std::to_string(++floor_counter), area, construction, state_ptr_side_1, state_ptr_side_2);

                    Vis_Setting temp_vis_setting;
                    bool vis_settings_found = false;

                    for (unsigned int i = 0; i < vis_settings.size(); i++)
                    {
                        if (vis_settings[i].m_ID == "1")
                        {
                            vis_settings_found = true;
                            temp_vis_setting = vis_settings[i];
                        }
                    }

                    if (!vis_settings_found)
                    {
                        std::cout << "Could not find the vis_settings profile for floor, exiting now..." << std::endl;
                        exit(1);
                    }

                    floor_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                    BPS->m_floor_ptrs.push_back(floor_ptr); // add the floor pointer to the floor vector
                    BPS->m_dep_states.push_back(floor_ptr);
                    BPS->m_states.push_back(floor_ptr);
                }
                else
                { // wall
                    if (geo_type_1 == 1 &&
                        geo_type_2 == 1)
                    { // separation wall
                        Building_Physics::BP_Construction construction;
                        bool construction_found = false;
                        for (unsigned int j = 0; j < BPS->m_constructions.size(); j++)
                        {
                            if (BPS->m_constructions[j].m_construction_ID == "1")
                            {
                                construction = BPS->m_constructions[j];
                                construction_found = true;
                            }
                        }
                        if (!construction_found)
                        {
                            std::cout << "Error in BP_grammar (i), exiting..." << std::endl;
                            exit(1);
                        }

                        Building_Physics::BP_Wall* wall_ptr = new Building_Physics::BP_Wall(BP_ptr, std::to_string(++wall_counter), area, construction, state_ptr_side_1, state_ptr_side_2);

                        Vis_Setting temp_vis_setting;
                        bool vis_settings_found = false;

                        for (unsigned int i = 0; i < vis_settings.size(); i++)
                        {
                            if (vis_settings[i].m_ID == "1")
                            {
                                vis_settings_found = true;
                                temp_vis_setting = vis_settings[i];
                            }
                        }

                        if (!vis_settings_found)
                        {
                            std::cout << "Could not find the vis_settings profile for wall, exiting now..." << std::endl;
                            exit(1);
                        }

                        wall_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                        BPS->m_wall_ptrs.push_back(wall_ptr); // add the wall pointer to the wall vector
                        BPS->m_dep_states.push_back(wall_ptr);
                        BPS->m_states.push_back(wall_ptr);
                    }
                    else if (geo_type_1 == 2 ||
                             geo_type_2 == 2)
                    { // structural wall
                        Building_Physics::BP_Construction construction;
                        bool construction_found = false;
                        for (unsigned int j = 0; j < BPS->m_constructions.size(); j++)
                        {
                            if (BPS->m_constructions[j].m_construction_ID == "1")
                            {
                                construction = BPS->m_constructions[j];
                                construction_found = true;
                            }
                        }
                        if (!construction_found)
                        {
                            std::cout << "Error in BP_grammar (j), exiting..." << std::endl;
                            exit(1);
                        }

                        Building_Physics::BP_Wall* wall_ptr = new Building_Physics::BP_Wall(BP_ptr, std::to_string(++wall_counter), area, construction, state_ptr_side_1, state_ptr_side_2);

                        Vis_Setting temp_vis_setting;
                        bool vis_settings_found = false;

                        for (unsigned int i = 0; i < vis_settings.size(); i++)
                        {
                            if (vis_settings[i].m_ID == "1")
                            {
                                vis_settings_found = true;
                                temp_vis_setting = vis_settings[i];
                            }
                        }

                        if (!vis_settings_found)
                        {
                            std::cout << "Could not find the vis_settings profile for wall, exiting now..." << std::endl;
                            exit(1);
                        }

                        wall_ptr->add_vis_settings(CF->get_rectangle(i), temp_vis_setting.m_vis_settings);

                        BPS->m_wall_ptrs.push_back(wall_ptr); // add the wall pointer to the wall vector
                        BPS->m_dep_states.push_back(wall_ptr);
                        BPS->m_states.push_back(wall_ptr);
                    }
                }
                continue;
            }
            else
            {
                std::cout << "Error (Grammar_1.hpp), too many surfaces assigned to a rectangle, exiting now..." << std::endl;
                exit(1);
            }
        }
    } // BP_grammar_1()
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void SD_grammar_1(Spatial_Design::MS_Conformal* CF, Structural_Design::SD_Analysis_Vars* SD)
    {
        using namespace Structural_Design;
        // read file with structural design settings
        std::string input_file = "0_Settings/Grammars/SD_Settings.txt";
        std::fstream input(input_file.c_str()); // open a file stream
        if (!input)
        {
            std::cout << "Error, could not open file "
                      << "\"" << input_file << "\""
                      << ", exiting..." << std::endl;
        }
        std::string line; // initialise a string to hold lines being read from file
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
        char type_ID; // holds information about what type of information is described by the line currently read

        Components::Load live_load(1,2,0.0);

        struct Wind_Load
        {
            unsigned int m_number_directions;
            double m_pressure;
            double m_suction;
            double m_shear;

            Wind_Load()
            {
                m_number_directions = 0;
                m_pressure = 0;
                m_suction = 0;
                m_shear = 0;
            }
        };
        Wind_Load wind_load;

        while(!input.eof())
        {
            getline(input, line); // get the next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }
            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            type_ID = trim_and_cast_char(*token); // interpret first token as type ID

            switch (type_ID)
            {
            case 'A':
            {
                token++;
                SD->m_mesh_division = trim_and_cast_uint(*token);
                break;
            }
            case 'B':
            {
                token++;
                live_load.m_value = -trim_and_cast_double(*token);
                break;
            }
            case 'C':
            {
                token++;
                wind_load.m_pressure = trim_and_cast_double(*token);
                token++;
                wind_load.m_suction = trim_and_cast_double(*token);
                token++;
                wind_load.m_shear = trim_and_cast_double(*token);
                token++;
                wind_load.m_number_directions = trim_and_cast_int(*token);
                token++;
                break;
            }
            case 'D':
            {
                Truss_Props temp;
                token++;
                temp.m_ID = *token;
                boost::algorithm::trim(temp.m_ID);
                token++;
                temp.m_A = trim_and_cast_double(*token);
                token++;
                temp.m_E = trim_and_cast_double(*token);

                SD->m_truss_props.push_back(temp);
                break;
            }
            case 'E':
            {
                Beam_Props temp;
                token++;
                temp.m_ID = *token;
                boost::algorithm::trim(temp.m_ID);
                token++;
                temp.m_b = trim_and_cast_double(*token);
                token++;
                temp.m_h = trim_and_cast_double(*token);
                token++;
                temp.m_E = trim_and_cast_double(*token);
                token++;
                temp.m_v = trim_and_cast_double(*token);

                SD->m_beam_props.push_back(temp);
                break;
            }
            case 'F':
            {
                Flat_Shell_Props temp;
                token++;
                temp.m_ID = *token;
                boost::algorithm::trim(temp.m_ID);
                token++;
                temp.m_t = trim_and_cast_double(*token);
                token++;
                temp.m_E = trim_and_cast_double(*token);
                token++;
                temp.m_v = trim_and_cast_double(*token);

                SD->m_flat_shell_props.push_back(temp);
                break;
            }
            default:
                break;
            }
        }

        Vectors::Vector pos_x; pos_x << 1,0,0;
        Vectors::Vector pos_y; pos_y << 0,1,0;
        Vectors::Vector pos_z; pos_z << 0,0,1;

        std::map<Spatial_Design::Geometry::Vertex*, Components::Point*> point_map;

        for (unsigned int i = 0; i < CF->get_surface_count(); i++)
        { // for each surface in the conformal model
            for (unsigned int j = 0; j < CF->get_surface(i)->get_vertex_count(); j++)
            { // and for each vertex on that surface
                Spatial_Design::Geometry::Vertex* temp_vertex = CF->get_surface(i)->get_vertex_ptr(j);
                if (point_map.find(temp_vertex) == point_map.end())
                { // if the point is not added to the structural design yet
                    Components::Point* temp_point = new Components::Point(temp_vertex->get_coords());

                    point_map[CF->get_surface(i)->get_vertex_ptr(j)] = temp_point;

                    SD->m_points.push_back(temp_point); // add to structural design
                }
            }
        }

        SD->m_all_points = SD->m_points;

        std::map<Spatial_Design::Geometry::Rectangle*, Components::Component*> flat_shell_map;

        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        { // for each rectangle in the conformal model

            unsigned int surface_count = CF->get_rectangle(i)->get_surface_count();

            if (surface_count == 0 || surface_count > 2)
            { // if no surfaces or too many surfaces are assigned this rectangle
                continue; // do not add any structural properties to this rectangle
            }

            int geo_type_1 = CF->get_rectangle(i)->get_surface_ptr(0)->get_surface_type();
            if (geo_type_1 == 0)
            {

                std::cout << "Error in SD_grammar_1 (a), exiting..." << std::endl;
                exit(1);
            }

            if (surface_count == 1 && geo_type_1 == 2)
            { // if the geometry is an exterior and structural
                CF->get_rectangle(i)->make_structural();

                if (Vectors::is_paral(pos_z, *CF->get_rectangle(i)->get_normal_ptr(), 0.01))
                { // rectangle is horizontal: floor
                    if ((int)CF->get_rectangle(i)->get_vertex_ptr(0)->get_coords()(2) <= 0)
                    { // ground floor (foundation)
                        // add floor slab
                        Flat_Shell_Props props;
                        bool props_found = false;
                        for (unsigned int j = 0; j < SD->m_flat_shell_props.size(); j++)
                        {
                            if (SD->m_flat_shell_props[j].m_ID == "1")
                            {
                                props = SD->m_flat_shell_props[j];
                                props_found = true;
                            }
                        }

                        if (!props_found)
                        {
                            std::cout << "Error when assigning structural properties to a floor (b), exiting now..." << std::endl;
                            exit(1);
                        }

                        SD->m_components.push_back(
                            new Components::Flat_Shell(props.m_t, props.m_E, props.m_v,
                                                       CF->get_rectangle(i), point_map));

                        SD->m_flat_shells.push_back(SD->m_components.back());
                        flat_shell_map[CF->get_rectangle(i)] = SD->m_components.back();

                        // add foundation constraints to each of the
                        Components::Point* p1, * p2;
                        for (int j = 0; j < 4; j++)
                        {
                            if (CF->get_rectangle(i)->get_line_ptr(j)->get_edge_count() > 0)
                            { // if the line belongs to the edge of a surface/space
                                p1 = point_map[CF->get_rectangle(i)->get_line_ptr(j)->get_vertex_ptr(0)];
                                p2 = point_map[CF->get_rectangle(i)->get_line_ptr(j)->get_vertex_ptr(1)];
                                SD->m_components.back()->add_line_constraint(Components::Constraint(0), p1, p2); // ux
                                SD->m_components.back()->add_line_constraint(Components::Constraint(1), p1, p2); // uy
                                SD->m_components.back()->add_line_constraint(Components::Constraint(2), p1, p2); // uz
                            }
                        }
                    }
                    else
                    { // roof or overhang
                        // add slab
                        Flat_Shell_Props props;
                        bool props_found = false;
                        for (unsigned int j = 0; j < SD->m_flat_shell_props.size(); j++)
                        {
                            if (SD->m_flat_shell_props[j].m_ID == "1")
                            {
                                props = SD->m_flat_shell_props[j];
                                props_found = true;
                            }
                        }

                        if (!props_found)
                        {
                            std::cout << "Error when assigning structural properties to a floor (c), exiting now..." << std::endl;
                            exit(1);
                        }

                        SD->m_components.push_back(
                            new Components::Flat_Shell(props.m_t, props.m_E, props.m_v,
                                                       CF->get_rectangle(i), point_map));

                        SD->m_flat_shells.push_back(SD->m_components.back());
                        flat_shell_map[CF->get_rectangle(i)] = SD->m_components.back();
                    }

                }
                else
                { // rectangle is a vertical: wall
                    // add slab
                    Flat_Shell_Props props;
                    bool props_found = false;
                    for (unsigned int j = 0; j < SD->m_flat_shell_props.size(); j++)
                    {
                        if (SD->m_flat_shell_props[j].m_ID == "1")
                        {
                            props = SD->m_flat_shell_props[j];
                            props_found = true;
                        }
                    }

                    if (!props_found)
                    {
                        std::cout << "Error when assigning structural properties to a floor (e), exiting now..." << std::endl;
                        exit(1);
                    }

                    SD->m_components.push_back(
                        new Components::Flat_Shell(props.m_t, props.m_E, props.m_v,
                                                   CF->get_rectangle(i), point_map));

                    SD->m_flat_shells.push_back(SD->m_components.back());
                    flat_shell_map[CF->get_rectangle(i)] = SD->m_components.back();
                }
            }

            if (surface_count == 1)
            {
                continue;
            }

            int geo_type_2 = CF->get_rectangle(i)->get_surface_ptr(1)->get_surface_type();
            if (geo_type_2 == 0)
            {

                std::cout << "Error in SD_grammar_1 (d), exiting..." << std::endl;
                exit(1);
            }

            if (surface_count == 2 && (geo_type_1 == 2 || geo_type_2 == 2))
            { // if the geometry is an interior and structural
                CF->get_rectangle(i)->make_structural();

                if (Vectors::is_paral(pos_z, *CF->get_rectangle(i)->get_normal_ptr(), 0.01))
                { // rectangle is horizontal: floor
                    // add slab
                    Flat_Shell_Props props;
                    bool props_found = false;
                    for (unsigned int j = 0; j < SD->m_flat_shell_props.size(); j++)
                    {
                        if (SD->m_flat_shell_props[j].m_ID == "1")
                        {
                            props = SD->m_flat_shell_props[j];
                            props_found = true;
                        }
                    }

                    if (!props_found)
                    {
                        std::cout << "Error when assigning structural properties to a floor (e), exiting now..." << std::endl;
                        exit(1);
                    }

                    SD->m_components.push_back(
                        new Components::Flat_Shell(props.m_t, props.m_E, props.m_v,
                                                   CF->get_rectangle(i), point_map));

                    SD->m_flat_shells.push_back(SD->m_components.back());
                    flat_shell_map[CF->get_rectangle(i)] = SD->m_components.back();
                }
                else
                { // rectangle is a vertical: wall
                    // add_slab
                    Flat_Shell_Props props;
                    bool props_found = false;
                    for (unsigned int j = 0; j < SD->m_flat_shell_props.size(); j++)
                    {
                        if (SD->m_flat_shell_props[j].m_ID == "1")
                        {
                            props = SD->m_flat_shell_props[j];
                            props_found = true;
                        }
                    }

                    if (!props_found)
                    {
                        std::cout << "Error when assigning structural properties to a floor (f), exiting now..." << std::endl;
                        exit(1);
                    }

                    SD->m_components.push_back(
                        new Components::Flat_Shell(props.m_t, props.m_E, props.m_v,
                                                   CF->get_rectangle(i), point_map));

                    SD->m_flat_shells.push_back(SD->m_components.back());
                    flat_shell_map[CF->get_rectangle(i)] = SD->m_components.back();
                }
            }
        }

        std::vector<Components::Load> wind_loads;
        Components::Load temp_load(0,0,0.0);

        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        { // for each surface in the conformal model
            double surface_count = CF->get_rectangle(i)->get_surface_count();

            if (surface_count == 1)
            { // if the rectangle is assigned to an external surface
                // add wind loads
                CF->get_rectangle(i)->get_surface_ptr(0)->calc_azimuths(); // calculates te surface's azimuths towards each space that are assigned this surface
                double azimuth = CF->get_rectangle(i)->get_surface_ptr(0)->get_azimuth(0); // wall orientation to inside of building
                azimuth = ((round(100*azimuth)))/100.0;
                azimuth += (azimuth < 180) ? 180 : -180; // azimuth is now the outside face of the surface (instead of face to inside of space)

                for (unsigned int j = 0; j < wind_load.m_number_directions; j++)
                { // for each wind direction (pos_y=N=0/360, pos_x=W=90, neg_y=S=180, neg_x=E=270)
                    double wind_dir = 360 * (j/(double)wind_load.m_number_directions); // angle of wind direction
                    double angle = azimuth - wind_dir; // angle between wind and wall orientation
                    angle = (angle >  180) ? angle - 360 : angle;
                    angle = (angle < -180) ? angle + 360 : angle;
                    double abs_angle = std::abs(angle);

                    // calculate x- and y- components of the wind loads
                    double y_comp = cos(wind_dir*PI/180);
                    y_comp = (round(10000*y_comp))/10000.0;
                    double x_comp = sin(wind_dir*PI/180);
                    x_comp = (round(10000*x_comp))/10000.0;

                    double shear_y = y_comp * wind_load.m_shear;
                    double shear_x = x_comp * wind_load.m_shear;
                    double pressure_y = y_comp * wind_load.m_pressure;
                    double pressure_x = x_comp * wind_load.m_pressure;
                    double suction_y = y_comp * wind_load.m_suction;
                    double suction_x = x_comp * wind_load.m_suction;

                    temp_load.m_lc = j + 2; // live load will be lc 1

                    if (Vectors::is_paral(pos_z, *CF->get_rectangle(i)->get_normal_ptr(), 0.01))
                    { // rectangle is horizontal: floor
                        if ((int)CF->get_rectangle(i)->get_vertex_ptr(0)->get_coords()(2) > 0)
                        { // if the rectangle is not at the ground floor level
                            // add shear forces to the roof floor
                            temp_load.m_value = shear_y;
                            temp_load.m_dof = 1;
                            wind_loads.push_back(temp_load);

                            temp_load.m_value = shear_x;
                            temp_load.m_dof = 0;
                            wind_loads.push_back(temp_load);
                        }
                    }
                    else
                    { // rectangle is vertical: wall
                        // add pressure, suction and shear forces
                        double y_red = abs(cos(azimuth*PI/180));
                        y_red = (round(10000*y_red))/10000.0;
                        double x_red = abs(sin((azimuth)*PI/180));
                        x_red = (round(10000*x_red))/10000.0;

                        double shear_y_red = shear_y * x_red;
                        double shear_x_red = shear_x * y_red;
                        double suction_y_red = suction_y * y_red;
                        double suction_x_red = suction_x * x_red;
                        double pressure_y_red = pressure_y * y_red;
                        double pressure_x_red = pressure_x * x_red;

                        if ((abs_angle >= 90) && (abs_angle < 180))
                        { // add shear forces to the rectangle
                            temp_load.m_value = shear_y_red;
                            temp_load.m_dof = 1;
                            wind_loads.push_back(temp_load);

                            temp_load.m_value = shear_x_red;
                            temp_load.m_dof = 0;
                            wind_loads.push_back(temp_load);
                        }

                        if (abs_angle < 90)
                        { // add suction forces to the rectangle
                            temp_load.m_value = suction_y_red;
                            temp_load.m_dof = 1;
                            wind_loads.push_back(temp_load);

                            temp_load.m_value = suction_x_red;
                            temp_load.m_dof = 0;
                            wind_loads.push_back(temp_load);
                        }

                        if (abs_angle > 90)
                        { // add pressure forces to the rectangle
                            temp_load.m_value = pressure_y_red;
                            temp_load.m_dof = 1;
                            wind_loads.push_back(temp_load);

                            temp_load.m_value = pressure_x_red;
                            temp_load.m_dof = 0;
                            wind_loads.push_back(temp_load);
                        }
                    }
                }

                if (CF->get_rectangle(i)->is_structural())
                {   // add surface load
                    for (unsigned int j = 0; j < wind_loads.size(); j++)
                    {
                        flat_shell_map[CF->get_rectangle(i)]->add_load(wind_loads[j]);
                    }
                }
                else
                { // add line loads to surrounding construction
                    /*for (unsigned int j = 0; j < wind_loads.size(); j++)
                    {
                        flat_shell_map[rectanglesside]->add_line_load(wind_loads[j]*(), p1, p2)
                    }*/
                }
            }

            if (Vectors::is_paral(pos_z, *CF->get_rectangle(i)->get_normal_ptr(), 0.01))
            { // if the rectangle is floor
                // add live load to the floor
                if (CF->get_rectangle(i)->is_structural() && live_load.m_value != 0)
                {
                    flat_shell_map[CF->get_rectangle(i)]->add_load(live_load);
                }
            }

            wind_loads.clear();
        }


    } // SD_grammar_1()
    #endif // BP_SIMULATION_HPP
} // Grammar
} // namespace BSO


#endif // GRAMMAR_1_HPP
