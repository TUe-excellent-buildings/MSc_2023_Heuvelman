#ifndef GRAMMAR_4_HPP
#define GRAMMAR_4_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <BSO/Spatial_Design/Conformation.hpp>

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Simulation.hpp>
#endif // BP_SIMULATION_HPP

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <Read_SD_Settings.hpp>
#include <SD_Rectangle_Rules.hpp>
#include <SD_Line_Rules.hpp>
#endif // SD_ANALYSIS_HPP

#include <BSO/Visualisation/Model_Module/Model.hpp>
#include <Read_Grammar_Settings.hpp>


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

    static std::string ga_design = "uninitialized";
	static const std::string valid_vars = "1234";

    // forward declarations
    #ifdef BP_SIMULATION_HPP
    void BP_grammar_4(Spatial_Design::MS_Conformal*, Building_Physics::BP_Simulation_Vars*);
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void SD_grammar_4(Spatial_Design::MS_Conformal*, Structural_Design::SD_Analysis_Vars*);
    #endif // SD_ANALYSIS_HPP

    Grammar_Settings grammar_settings; // will hold all the settings for assignment of BP and SD properties to the conformal model

    // function definition
    void grammar_4(Spatial_Design::MS_Conformal* MS) // assign geometric types to the building geometry in a conformal building model
    {
        grammar_settings = read_grammar_settings("Settings/Grammar_Settings.txt"); // read the grammar settings file

        #ifdef SD_ANALYSIS_HPP
        MS->add_grammars(&SD_grammar_4);
        #endif // SD_ANALYSIS_HPP

        #ifdef BP_SIMULATION_HPP
        MS->add_grammars(&BP_grammar_4);
        #endif // BP_SIMULATION_HPP

    } // grammar_4 geometry types



    #ifdef BP_SIMULATION_HPP
    void BP_grammar_4(Spatial_Design::MS_Conformal* CF, Building_Physics::BP_Simulation_Vars* BPS)
    { // if it is tried to make a building physics model with this grammar, then give an error
        std::cerr << "Selected grammar is intended for structural design only, exiting now..." << std::endl;
        exit(1);
    } // BP_grammar_4()
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void SD_grammar_4(Spatial_Design::MS_Conformal* CF, Structural_Design::SD_Analysis_Vars* SD)
    {
        using namespace Structural_Design;
        // read file with structural design settings

        BSO::Structural_Design::read_SD_settings("Settings/SD_Settings.txt", SD);

        // add all the points that make a rectangle in the conformal model to the structural model
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

        // pass all the flat shells to the SD model using the rectangles in the conformal model that are selected by the grammar
        std::map<Spatial_Design::Geometry::Rectangle*, SD_Rectangle_Rules*> rectangle_rule_sets;
		
		for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        { // for each rectangle in the conformal model

            Spatial_Design::Geometry::Rectangle* rectangle_ptr = CF->get_rectangle(i);
            unsigned int surface_count = CF->get_rectangle(i)->get_surface_count();

            if (surface_count == 0 || surface_count > 2)
            { // if no surfaces or too many surfaces are assigned to this rectangle
                continue; // do not add any structural properties to this rectangle
            }

            SD_Rectangle_Rules* rule_set_ptr = new SD_Rectangle_Rules;
            rule_set_ptr->m_point_map = &point_map;
            rule_set_ptr->m_SD = SD;
            rule_set_ptr->m_grammar_settings = grammar_settings;
            rule_set_ptr->m_rectangle = rectangle_ptr;
            rule_set_ptr->m_facing_side_1 = rectangle_ptr->get_surface_ptr(0)->get_facing(0);

            rule_set_ptr->m_space_info.first = rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)->get_space_type();
			if (surface_count == 2) rule_set_ptr->m_space_info.second = rectangle_ptr->get_surface_ptr(1)->get_space_ptr(0)->get_space_type();
            rule_set_ptr->m_surface_info.first = rectangle_ptr->get_surface_ptr(0)->get_surface_type();
			if (surface_count == 2) rule_set_ptr->m_surface_info.second = rectangle_ptr->get_surface_ptr(1)->get_surface_type();

			switch(ga_design[i]) // assign a structural type according to what it is assigned in the ga_string
			{
			case '1': // no structure
			{
				rule_set_ptr->m_conv_type = sd_conversion_type::NO_STRUCTURE;
				break;
			}
			case '2': // trusses
			{
				rule_set_ptr->m_conv_type = sd_conversion_type::TRUSS;
				break;
			}
			case '3': // beams
			{
				rule_set_ptr->m_conv_type = sd_conversion_type::BEAM;
				break;
			}
			case '4': // flat shells
			{
				rule_set_ptr->m_conv_type = sd_conversion_type::FLAT_SHELL;
				break;
			}
			};

			rule_set_ptr->apply_rules(); // apply the rule sets, this will create the specified components in the current SD model (SD)
			rectangle_rule_sets[rectangle_ptr] = rule_set_ptr; // store the rule set under the key value of this rectangle's pointer
		}

		std::vector<SD_Line_Rules*> line_rule_sets;
		
		for (unsigned int i = 0; i < CF->get_line_count(); i++)
        { // for each rectangle in the conformal model
            Spatial_Design::Geometry::Line* line_ptr = CF->get_line(i);
            SD_Line_Rules* rule_set_ptr = new SD_Line_Rules;

            unsigned int rect_count = line_ptr->get_rectangle_count();

            for (unsigned int j = 0; j < rect_count; j++)
            {
                Spatial_Design::Geometry::Rectangle* rect_ptr = line_ptr->get_rectangle_ptr(j);
                if (rect_ptr->get_surface_count() > 0 && rect_ptr->get_surface_count() <= 2)
                {
                    rule_set_ptr->m_rectangles.push_back(rect_ptr);
                }
            }
            if (!(rule_set_ptr->m_rectangles.size() > 0))
            { // if this line does not belong to any of the rectangles that own a rule set then stop considering this line
                continue;
            }


            rule_set_ptr->m_point_map = &point_map;
            rule_set_ptr->m_SD = SD;
            rule_set_ptr->m_line = line_ptr;
            rule_set_ptr->m_rect_rule_sets = &rectangle_rule_sets;


            rule_set_ptr->apply_rules();
			line_rule_sets.push_back(rule_set_ptr);
        }
		
		for (auto i : rectangle_rule_sets)
			delete i.second;
		for (auto i : line_rule_sets)
			delete i;
		rectangle_rule_sets.clear();
		line_rule_sets.clear();

    } // SD_grammar_4()
    #endif // SD_SIMULATION_HPP
} // Grammar
} // namespace BSO


#endif // GRAMMAR_4_HPP
