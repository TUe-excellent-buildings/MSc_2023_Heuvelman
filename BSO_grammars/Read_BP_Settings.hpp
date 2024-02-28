#ifndef READ_BP_SETTINGS_HPP
#define READ_BP_SETTINGS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include <boost/algorithm/string.hpp>

#include <BSO/Trim_And_Cast.hpp>

namespace BSO { namespace Building_Physics {

void read_BP_settings(std::string input_file, Building_Physics::BP_Simulation_Vars* BPS)
{ // reads "input_file" and stores the BP model it describes in "BP"

    Building_Physics::BP_Simulation* BP_ptr = static_cast<Building_Physics::BP_Simulation*>(BPS); // to be ble to call functions of the child class

    std::fstream input(input_file.c_str()); // open a file stream
    if (!input)
    {
        std::cerr << "Error, could not open file "
                  << "\"" << input_file << "\""
                  << ", exiting..." << std::endl;
        exit(1);
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

        case 'A':
        { // switch daylight saving time on or off (not implemented)

            break;
        }
        case 'B':
        { // add the number of warm up days
            token++; // next token holds number of warm up days
            BPS->m_warm_up_days = trim_and_cast_int(*token);
            break;
        }
        case 'C':
        { // add the number of time steps per hour
            token++; //next token holds number of time steps per hour
            BPS->m_time_step_hour = trim_and_cast_int(*token);
            break;
        }
        case 'D':
        { // add space settings
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
        { // add a weather temperature profile (from KNMI here)
            std::string file_format, file_location; // initialize string to temporarily hold day name

			token++; // file format (unused)
			file_format =*token;
			boost::algorithm::trim(file_format); // remove white space in front and at end of string
			token++; // file location
			file_location =*token;
			boost::algorithm::trim(file_location); // remove white space in front and at end of string

			BPS->m_weather_profile = new Building_Physics::BP_Weather_Profile(BP_ptr, file_location); // it is important that the nr of warm up days is declared before the weather profile (consider change in future)
			BPS->m_indep_states.push_back(BPS->m_weather_profile);
			BPS->m_states.push_back(BPS->m_weather_profile);
            break;
        }
		case 'F':
        { // add a weather temperature data
			// a weather temperature profile must be initialised beforehand! (see case: 'E')
            int year, month, day, hour; // initialize variables to temporarily hold data
			std::string sim_ID;
            std::string day_name; // initialize string to temporarily hold day name
			boost::posix_time::ptime begin, end;
			
			token++; // simulation ID
			sim_ID = *token;
			boost::algorithm::trim(sim_ID);

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
            begin = boost::posix_time::ptime(boost::gregorian::date(year, month, day), boost::posix_time::hours(hour));

            token++; // finish year
            year = trim_and_cast_int(*token);
            token++; // finish month
            month = trim_and_cast_int(*token);
            token++; // finish day
            day = trim_and_cast_int(*token);
            token++; // finish hour
            hour = trim_and_cast_int(*token);
            end = boost::posix_time::ptime(boost::gregorian::date(year, month, day), boost::posix_time::hours(hour));

			BPS->m_simulation_periods[sim_ID] = std::make_pair(begin, end);
			BPS->m_weather_profile->add_weather_data(begin,end);

            break;
        }
        case 'G':
        { // add a ground temperature profile (constant here)
            double temperature;
            token++; // Constant ground temperature
            temperature = trim_and_cast_double(*token);
            BPS->m_ground_profile = new Building_Physics::BP_Ground_Profile(BP_ptr, temperature);
            BPS->m_indep_states.push_back(BPS->m_ground_profile);
            BPS->m_states.push_back(BPS->m_ground_profile);
            break;
        }
        case 'H':
        { // add material properties
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
        case 'I':
        { // add construction properties
            Building_Physics::BP_Construction construction; // initialize to temporarily hold data
            token++; // Construction_ID
            construction.m_construction_ID = *token;
            boost::algorithm::trim(construction.m_construction_ID); // remove white space at front and end of string
            std::vector<std::string> layer_mat_ID; // will hold material ID of all layers
            std::vector<double> layer_thicknesses; // will hold thicknesses of all layers
            token++; // visualisation ID
            construction.m_vis_ID = *token;
            boost::algorithm::trim(construction.m_vis_ID);
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
        case 'J':
        { // add glazing properties
            Building_Physics::BP_Glazing glazing;
            token++; // glazing_ID
            glazing.m_glazing_ID = *token;
            boost::algorithm::trim(glazing.m_glazing_ID);
            token++; // U-value
            glazing.m_U_value = trim_and_cast_double(*token);
            token++; // Capacitance per area
            glazing.m_capacitance_per_area = trim_and_cast_double(*token);
            token++; // visualisation ID
            glazing.m_vis_ID = *token;
            boost::algorithm::trim(glazing.m_vis_ID);

            BPS->m_glazings.push_back(glazing);
            break;
        }
        case 'K':
        { // add visualisation settings (this can only be used when the BP model is initialised with a conformal model
            BP_Vis_Setting temp;

            token++; // vis_ID
            temp.m_ID = *token;
            boost::algorithm::trim(temp.m_ID);
            token++; // red
            temp.m_r = trim_and_cast_double(*token);
            token++; // green
            temp.m_g = trim_and_cast_double(*token);
            token++; // blue
            temp.m_b = trim_and_cast_double(*token);
            token++; // alpha
            temp.m_alpha = trim_and_cast_double(*token);

            BPS->m_vis_settings.push_back(temp);
            break;
        }
        case 'L':
        { // add a space
            BP_Space_Settings space_settings;
            token++; //space_ID
            std::string space_ID = *token;
            boost::algorithm::trim(space_ID);
            token++; //volume
            double volume = trim_and_cast_double(*token);
            token++; // Space_Set_ID
            std::string space_set_ID = *token;
            boost::algorithm::trim(space_set_ID);

            for (unsigned int i = 0; i < BPS->m_space_settings.size(); i++)
            {
                if (BPS->m_space_settings[i].m_space_set_ID == space_set_ID)
                {
                    space_settings = BPS->m_space_settings[i];
                }
            }



            BP_Space* space_ptr = new BP_Space(BP_ptr, space_ID, volume, space_settings, BPS->m_weather_profile); // initialise a ptr to an object of the space class with the values read from the file
            BPS->m_space_ptrs.push_back(space_ptr); // add the space pointer to the space vector
            BPS->m_dep_states.push_back(space_ptr);
            BPS->m_states.push_back(space_ptr);
            break;
        }
        case 'M':
        { // add a wall
            token++; // Wall_ID
            std::string wall_ID = *token;
            boost::algorithm::trim(wall_ID);
            token++; // Construction_ID
            std::string construction_ID = *token;
            boost::algorithm::trim(construction_ID);
            BP_Construction construction;
            for (unsigned int i = 0; i < BPS->m_constructions.size(); i++)
            {
                if (construction_ID == BPS->m_constructions[i].m_construction_ID)
                {
                    construction = BPS->m_constructions[i];
                }
            }
            token++; // Surface area
            double area = trim_and_cast_double(*token);
            token++; // Surface Orientation, skipped for now
            token++; // Space ID side one
            std::string space_ID_1 = *token;
            boost::algorithm::trim(space_ID_1);
            token++; // SPace Id side two
            std::string space_ID_2 = *token;
            boost::algorithm::trim(space_ID_2);

            BP_State* state_ptr_side_1 = nullptr;
            BP_State* state_ptr_side_2 = nullptr;
            bool side_1_found = false;
            bool side_2_found = false;

            for (unsigned int i = 0; i < BPS->m_space_ptrs.size(); i ++)
            {
                if(space_ID_1 == BPS->m_space_ptrs[i]->get_ID())
                {
                    state_ptr_side_1 = BPS->m_space_ptrs[i];
                    side_1_found = true;
                }
                else if (space_ID_1 == "E")
                {
                    state_ptr_side_1 = BPS->m_weather_profile;
                    side_1_found = true;
                }
                else if (space_ID_1 == "G")
                {
                    state_ptr_side_1 = BPS->m_ground_profile;
                    side_1_found = true;
                }
                if(space_ID_2 == BPS->m_space_ptrs[i]->get_ID())
                {
                    state_ptr_side_2 = BPS->m_space_ptrs[i];
                    side_2_found = true;
                }
                else if (space_ID_2 == "E")
                {
                    state_ptr_side_2 = BPS->m_weather_profile;
                    side_2_found = true;
                }
                else if (space_ID_2 == "G")
                {
                    state_ptr_side_2 = BPS->m_ground_profile;
                    side_2_found = true;
                }
            }

            if (!side_1_found || !side_2_found)
            {
                std::cout << "Error in assigning neighbouring states to wall: " << wall_ID << ". Exiting now..." << std::endl;
                exit(1);
            }

            BP_Wall* wall_ptr = new BP_Wall(BP_ptr, wall_ID, area, construction, state_ptr_side_1, state_ptr_side_2);
            BPS->m_wall_ptrs.push_back(wall_ptr); // add the wall pointer to the wall vector
            BPS->m_dep_states.push_back(wall_ptr);
            BPS->m_states.push_back(wall_ptr);
            break;
        }
        case 'N':
        { // add a window
            token++; // window_ID
            std::string window_ID = *token;
            boost::algorithm::trim(window_ID);
            token++; // area
            double area = trim_and_cast_double(*token);
            token++; // glazing_ID
            std::string glazing_id = *token;
            boost::algorithm::trim(glazing_id);
            BP_Glazing glazing;
            for (unsigned int i = 0; i < BPS->m_glazings.size(); i++)
            {
                if (BPS->m_glazings[i].m_glazing_ID == glazing_id)
                {
                    glazing = BPS->m_glazings[i];
                }
            }
            token++; // Orientation, skipped for now
            token++; // Space ID side one
            std::string space_ID_1 = *token;
            boost::algorithm::trim(space_ID_1);
            token++; // SPace Id side two
            std::string space_ID_2 = *token;
            boost::algorithm::trim(space_ID_2);

            BP_State* state_ptr_side_1 = nullptr;
            BP_State* state_ptr_side_2 = nullptr;
            bool side_1_found = false;
            bool side_2_found = false;

            for (unsigned int i = 0; i < BPS->m_space_ptrs.size(); i ++)
            {
                if(space_ID_1 == BPS->m_space_ptrs[i]->get_ID())
                {
                    state_ptr_side_1 = BPS->m_space_ptrs[i];
                    side_1_found = true;
                }
                else if (space_ID_1 == "E")
                {
                    state_ptr_side_1 = BPS->m_weather_profile;
                    side_1_found = true;
                }
                else if (space_ID_1 == "G")
                {
                    state_ptr_side_1 = BPS->m_ground_profile;
                    side_1_found = true;
                }

                if(space_ID_2 == BPS->m_space_ptrs[i]->get_ID())
                {
                    state_ptr_side_2 = BPS->m_space_ptrs[i];
                    side_2_found = true;
                }
                else if (space_ID_2 == "E")
                {
                    state_ptr_side_2 = BPS->m_weather_profile;
                    side_2_found = true;
                }
                else if (space_ID_2 == "G")
                {
                    state_ptr_side_2 = BPS->m_ground_profile;
                    side_2_found = true;
                }
            }

            if (!side_1_found || !side_2_found)
            {
                std::cout << "Error in assigning neighbouring states to wall, exiting now..." << std::endl;
                exit(1);
            }

            BP_Window* window_ptr = new BP_Window(BP_ptr, window_ID, area, glazing, state_ptr_side_1, state_ptr_side_2);
            BPS->m_window_ptrs.push_back(window_ptr); // add the wall pointer to the wall vector
            BPS->m_dep_states.push_back(window_ptr);
            BPS->m_states.push_back(window_ptr);
            break;
        }
        case 'O':
        { // add a floor
            token++; // floor_ID
            std::string floor_ID = *token;
            boost::algorithm::trim(floor_ID);
            token++; // Construction_ID
            std::string construction_ID = *token;
            boost::algorithm::trim(construction_ID);
            BP_Construction construction;
            for (unsigned int i = 0; i < BPS->m_constructions.size(); i++)
            {
                if (construction_ID == BPS->m_constructions[i].m_construction_ID)
                {
                    construction = BPS->m_constructions[i];
                }
            }
            token++; // Surface area
            double area = trim_and_cast_double(*token);
            token++; // Space ID side one
            std::string space_ID_1 = *token;
            boost::algorithm::trim(space_ID_1);
            token++; // SPace Id side two
            std::string space_ID_2 = *token;
            boost::algorithm::trim(space_ID_2);

            BP_State* state_ptr_side_1 = nullptr;
            BP_State* state_ptr_side_2 = nullptr;
            bool side_1_found = false;
            bool side_2_found = false;

            for (unsigned int i = 0; i < BPS->m_space_ptrs.size(); i ++)
            {
                if(space_ID_1 == BPS->m_space_ptrs[i]->get_ID())
                {
                    state_ptr_side_1 = BPS->m_space_ptrs[i];
                    side_1_found = true;
                }
                else if (space_ID_1 == "E")
                {
                    state_ptr_side_1 = BPS->m_weather_profile;
                    side_1_found = true;
                }
                else if (space_ID_1 == "G")
                {
                    state_ptr_side_1 = BPS->m_ground_profile;
                    side_1_found = true;
                }
                if(space_ID_2 == BPS->m_space_ptrs[i]->get_ID())
                {
                    state_ptr_side_2 = BPS->m_space_ptrs[i];
                    side_2_found = true;
                }
                else if (space_ID_2 == "E")
                {
                    state_ptr_side_2 = BPS->m_weather_profile;
                    side_2_found = true;
                }
                else if (space_ID_2 == "G")
                {
                    state_ptr_side_2 = BPS->m_ground_profile;
                    side_2_found = true;
                }
            }

            if (!side_1_found || !side_2_found)
            {
                std::cout << "Error in assigning neighbouring states to floor, exiting now..." << std::endl;
                exit(1);
            }

            BP_Floor* floor_ptr = new BP_Floor(BP_ptr, floor_ID, area, construction, state_ptr_side_1, state_ptr_side_2);
            BPS->m_floor_ptrs.push_back(floor_ptr); // add the floor pointer to the floor vector
            BPS->m_dep_states.push_back(floor_ptr);
            BPS->m_states.push_back(floor_ptr);
            break;
        }
        default:
        {
            // type_ID not recognized, do nothing
            break;
        }
        } // end of switch statement

    } // end of while (reading file)

} // read_BP_settings

} // namespace Building_Physics
} // namespace BSO

#endif // READ_BP_SETTINGS_HPP
