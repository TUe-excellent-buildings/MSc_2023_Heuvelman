#ifndef GRAMMAR_ZONING_HPP
#define GRAMMAR_ZONING_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>

#include <boost/bind.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <BSO/Spatial_Design/Conformation.hpp>
#include <Read_Grammar_Settings.hpp>

#ifdef BP_SIMULATION_HPP
#include <Read_BP_Settings.hpp>
#include <BP_Space_Rules.hpp>
#include <BP_Rectangle_Rules.hpp>
#endif // BP_SIMULATION_HPP

#ifdef SD_ANALYSIS_HPP
#include <Read_SD_Settings.hpp>
#include <SD_Rectangle_Rules.hpp>
#include <SD_Line_Rules.hpp>
#endif // SD_ANALYSIS_HPP



namespace BSO { namespace Grammar {
    /*
     *
     */

    // forward declarations
    #ifdef BP_SIMULATION_HPP
    void BP_grammar_zoning(Spatial_Design::MS_Conformal*, Building_Physics::BP_Simulation_Vars*);
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void SD_grammar_zoning(Spatial_Design::MS_Conformal*, Structural_Design::SD_Analysis_Vars*);
    #endif // SD_ANALYSIS_HPP

    Grammar_Settings grammar_settings; // will hold all the settings for assignment of BP and SD properties to the conformal model

    // function definition
    void grammar_zoning(Spatial_Design::MS_Conformal* MS) // assign geometric types to the building geometry in a conformal building model
    {

        grammar_settings = read_grammar_settings("Settings/Grammar_Settings.txt");

        #ifdef SD_ANALYSIS_HPP
        MS->add_grammars(&SD_grammar_zoning);
        #endif // SD_ANALYSIS_HPP

        #ifdef BP_SIMULATION_HPP
        MS->add_grammars(&BP_grammar_zoning);
        #endif // BP_SIMULATION_HPP
    } // grammar_1 geometry types



    #ifdef BP_SIMULATION_HPP
    void BP_grammar_zoning(Spatial_Design::MS_Conformal* CF, Building_Physics::BP_Simulation_Vars* BPS)
    {
        read_BP_settings("Settings/BP_Settings.txt" , BPS);
        Building_Physics::BP_Simulation* BP_ptr = static_cast<Building_Physics::BP_Simulation*>(BPS);

        std::map<Spatial_Design::Geometry::Space*, BP_Space_Rules> space_rule_sets;

        for (unsigned int i = 0; i < CF->get_space_count(); i++)
        { // for each space
            Spatial_Design::Geometry::Space* space_ptr = CF->get_space(i);
            BP_Space_Rules temp_space_rules;

            temp_space_rules.m_BP_vars = BPS;
            temp_space_rules.m_BP_sim = BP_ptr;
            temp_space_rules.m_space_ptr = space_ptr;
            temp_space_rules.m_space_ID = space_ptr->get_ID();
            temp_space_rules.m_volume = space_ptr->get_encasing_cuboid().get_volume()/(1e9); // volume in m cube
            temp_space_rules.m_grammar_settings = &grammar_settings;

            temp_space_rules.m_type_ID = space_ptr->get_space_type();


            temp_space_rules.apply_rules();
            space_rule_sets[space_ptr] = temp_space_rules;
        }

        std::map<Spatial_Design::Geometry::Rectangle*, BP_Rectangle_Rules> rectangle_rule_sets;
        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        {
            Spatial_Design::Geometry::Rectangle* rect_ptr = CF->get_rectangle(i);
            unsigned int surface_count = rect_ptr->get_surface_count();

            if (surface_count == 0 || surface_count > 2)
            {
                continue;
            }

            BP_Rectangle_Rules temp_rect_rules;
            temp_rect_rules.m_rectangle_ptr = rect_ptr;
            temp_rect_rules.m_area = rect_ptr->get_area()/(1e6); // area in m squared
            temp_rect_rules.m_space_rules = &space_rule_sets;
            temp_rect_rules.m_grammar_settings = &grammar_settings;
            temp_rect_rules.m_BP_vars = BPS;
            temp_rect_rules.m_BP_sim = BP_ptr;
			temp_rect_rules.m_space_info.first = rect_ptr->get_surface_ptr(0)->get_space_ptr(0)->get_space_type();
            if(surface_count == 2) temp_rect_rules.m_space_info.second = rect_ptr->get_surface_ptr(1)->get_space_ptr(0)->get_space_type();


            temp_rect_rules.apply_rules();
            rectangle_rule_sets[rect_ptr] = temp_rect_rules;
        }
    } // BP_grammar_zoning()
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    void SD_grammar_zoning(Spatial_Design::MS_Conformal* CF, Structural_Design::SD_Analysis_Vars* SD)
    {std::cout << "0" << std::endl;
        using namespace Structural_Design;
        // read file with structural design settings

        BSO::Structural_Design::read_SD_settings("Settings/SD_Settings.txt", SD);

        // add all the points that make a rectangle in the conformal model to the structural model
        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        { // tag ghost-rectangle vertices
            Spatial_Design::Geometry::Rectangle* rectangle_ptr = CF->get_rectangle(i);
            if (rectangle_ptr->get_zoned() == false && rectangle_ptr->get_horizontal() == true && rectangle_ptr->get_surface_count() == 2)
            {
                for (unsigned int j = 0; j < 4; j++)
                {
                    rectangle_ptr->get_vertex_ptr(j)->tag_zoned();
                }
            }
        }

        std::map<Spatial_Design::Geometry::Vertex*, Components::Point*> point_map;
        for (unsigned int i = 0; i < CF->get_vertex_count(); i++)
        { // and for each vertex on that surface
            Spatial_Design::Geometry::Vertex* temp_vertex = CF->get_vertex(i);
            if(temp_vertex->get_zoned() == true)
            {
                if (point_map.find(temp_vertex) == point_map.end())
                { // if the point is not added to the structural design yet
                    Components::Point* temp_point = new Components::Point(temp_vertex->get_coords());

                    point_map[CF->get_vertex(i)] = temp_point;

                    SD->m_points.push_back(temp_point); // add to structural design
                }
            }
        }

        // pass all the flat shells to the SD model using the rectangles in the conformal model that are selected by the grammar
        std::map<Spatial_Design::Geometry::Rectangle*, SD_Rectangle_Rules*> rectangle_rule_sets;

        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
        { // for each rectangle in the conformal model

            Spatial_Design::Geometry::Rectangle* rectangle_ptr = CF->get_rectangle(i);

            if (rectangle_ptr->get_zoned() == true)
            {
                SD_Rectangle_Rules* rule_set = new SD_Rectangle_Rules;
                rule_set->m_point_map = &point_map;
                rule_set->m_SD = SD;
                rule_set->m_grammar_settings = grammar_settings;
                rule_set->m_rectangle = rectangle_ptr;
                unsigned int surface_count = rectangle_ptr->get_surface_count();
                rule_set->m_thickness = rectangle_ptr->get_thickness();
                rule_set->m_loading = rectangle_ptr->get_loading();

                if (surface_count == 0)
                {
                    rule_set->m_space_info.first = "Z";
                    rule_set->m_space_info.second = "Z";
                    rule_set->m_surface_info.first = "Z";
                    rule_set->m_surface_info.second = "Z";
                    if (rectangle_ptr->get_horizontal() == false)
                        rule_set->m_struct_type = structural_type::WALL;
                    else
                        rule_set->m_struct_type = structural_type::FLOOR;
                }
                else if (surface_count == 1)
                {
                    rule_set->m_facing_side_1 = rectangle_ptr->get_surface_ptr(0)->get_facing(0);
                    rule_set->m_surface_info.first = rectangle_ptr->get_surface_ptr(0)->get_surface_type();
                    rule_set->m_space_info.first = rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)->get_space_type();
                }
                else if (surface_count == 2)
                {
                    rule_set->m_facing_side_1 = rectangle_ptr->get_surface_ptr(0)->get_facing(0);
                    rule_set->m_surface_info.first = rectangle_ptr->get_surface_ptr(0)->get_surface_type();
                    rule_set->m_space_info.first = rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)->get_space_type();
                    rule_set->m_surface_info.second = rectangle_ptr->get_surface_ptr(1)->get_surface_type();
                    rule_set->m_space_info.second = rectangle_ptr->get_surface_ptr(1)->get_space_ptr(0)->get_space_type();
                }
                rule_set->apply_rules();
                rectangle_rule_sets[rule_set->m_rectangle] = rule_set;
            }
            // ghost elements (floors within zones)
            else if (rectangle_ptr->get_zoned() == false && rectangle_ptr->get_horizontal() == true && rectangle_ptr->get_surface_count() == 2)
            {
                SD_Rectangle_Rules* rule_set = new SD_Rectangle_Rules;
                rule_set->m_point_map = &point_map;
                rule_set->m_SD = SD;
                rule_set->m_grammar_settings = grammar_settings;
                rule_set->m_rectangle = rectangle_ptr;

                rule_set->m_space_info.first = "G"; // G refers to grammar settings and defines a ghost element
                rule_set->m_space_info.second = "G";
                rule_set->m_surface_info.first = "G";
                rule_set->m_surface_info.second = "G";
                rule_set->m_struct_type = structural_type::FLOOR;

                rule_set->apply_rules();
                rectangle_rule_sets[rule_set->m_rectangle] = rule_set;
            }
        }

        std::map<Spatial_Design::Geometry::Line*, SD_Line_Rules*> line_rule_sets;

        for (unsigned int i = 0; i < CF->get_line_count(); i++)
        { // for each rectangle in the conformal model
            Spatial_Design::Geometry::Line* line_ptr = CF->get_line(i);
            SD_Line_Rules* rule_set = new SD_Line_Rules;

            unsigned int rect_count = line_ptr->get_rectangle_count();

            for (unsigned int j = 0; j < rect_count; j++)
            {
                Spatial_Design::Geometry::Rectangle* rect_ptr = line_ptr->get_rectangle_ptr(j);
                if (rect_ptr->get_zoned() == true)
                {
                    rule_set->m_rectangles.push_back(rect_ptr);
                }
                else if (rect_ptr->get_zoned() == false && rect_ptr->get_horizontal() == true && rect_ptr->get_surface_count() == 2)
                {
                    rule_set->m_rectangles.push_back(rect_ptr); // ghost element
                }
            }
            if (!(rule_set->m_rectangles.size() > 0))
            { // if this line does not belong to any of the rectangles that own a rule set then stop considering this line
                continue;
            }


            rule_set->m_point_map = &point_map;
            rule_set->m_SD = SD;
            rule_set->m_line = line_ptr;
            rule_set->m_rect_rule_sets = &rectangle_rule_sets;

            rule_set->apply_rules();
			line_rule_sets[line_ptr] = rule_set;
        }
		for (auto i : rectangle_rule_sets)
		{
			delete i.second;
		}
		for (auto i : line_rule_sets)
		{
			delete i.second;
		}std::cout << "v" << std::endl;
		rectangle_rule_sets.clear();
		line_rule_sets.clear();
    } // SD_grammar_zoning()
    #endif // SD_SIMULATION_HPP
} // Grammar
} // namespace BSO


#endif // GRAMMAR_ZONING_HPP
