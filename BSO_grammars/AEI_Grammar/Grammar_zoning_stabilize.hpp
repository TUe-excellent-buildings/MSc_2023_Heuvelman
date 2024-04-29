#ifndef GRAMMAR_ZONING_STABILIZE_HPP
#define GRAMMAR_ZONING_STABILIZE_HPP

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

#ifdef STABILIZE_HPP

#include <BSO/Structural_Design/Stabilization/Stabilize.hpp>
#endif // STABILIZE_HPP




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

	#ifdef STABILIZE_HPP
	void SD_grammar_stabilize(Structural_Design::SD_Analysis_Vars* SD, Spatial_Design::MS_Conformal* CF);

	#endif // STABILIZE_HPP


    Grammar_Settings grammar_settings; // will hold all the settings for assignment of BP and SD properties to the conformal model

    // function definition
    void grammar_zoning(Spatial_Design::MS_Conformal* MS) // assign geometric types to the building geometry in a conformal building model
    {

        grammar_settings = read_grammar_settings("files_stabilization/Settings/Grammar_Settings.txt");

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
    {
        using namespace Structural_Design;
        // read file with structural design settings

        BSO::Structural_Design::read_SD_settings("Settings/SD_Settings.txt", SD);

        std::map<Spatial_Design::Geometry::Vertex*, Components::Point*> point_map;
        std::map<Spatial_Design::Geometry::Rectangle*, SD_Rectangle_Rules*> rectangle_rule_sets;
        std::map<Spatial_Design::Geometry::Line*, SD_Line_Rules*> line_rule_sets;

		point_map.clear(); // clear the points of the previous structural design model (if there were any)

        // add all the vertices that belong to a surface in the conformal model to the structural model
        for (unsigned int j = 0; j < CF->get_surface_count(); j++)
        { // for each surface in the conformal model
            for (unsigned int k = 0; k < CF->get_surface(j)->get_vertex_count(); k++)
            { // and for each vertex on that surface
                Spatial_Design::Geometry::Vertex* temp_vertex = CF->get_surface(j)->get_vertex_ptr(k);
                if (point_map.find(temp_vertex) == point_map.end())
                { // if the point is not added to the structural design yet, then add it
                    Components::Point* temp_point = new Components::Point(temp_vertex->get_coords()); // create a new point
                    point_map[CF->get_surface(j)->get_vertex_ptr(k)] = temp_point; // add it to the point map that will be used in this grammar
                    SD->m_points.push_back(temp_point); // add to structural design
                }
            }
        }

		// Manage the rule sets that generate an SD model from the CF model
		//rectangle rule sets first
		for (unsigned int j = 0; j < CF->get_rectangle_count(); j++)
		{ // for each rectangle in the conformal model
			Spatial_Design::Geometry::Rectangle* rect_ptr = CF->get_rectangle(j);
			unsigned int surface_count = rect_ptr->get_surface_count();

			if (rect_ptr->get_zoned() == false)
			{ // if the rectangle does not belong to a zoned design
				continue; // continue to the next rectangle in the conformal model
			}

			// create and initialise a new rectangle rule_set
			auto rule_set = new SD_Rectangle_Rules;
			rule_set->m_point_map = &point_map;
			rule_set->m_SD = SD;
			rule_set->m_grammar_settings = grammar_settings;
			rule_set->m_rectangle = rect_ptr;
			rule_set->m_facing_side_1 = rect_ptr->get_surface_ptr(0)->get_facing(0);

            if (surface_count == 0)
            {
                rule_set->m_space_info.first = "Z";
                rule_set->m_space_info.second = "Z";
                rule_set->m_surface_info.first = "Z";
                rule_set->m_surface_info.second = "Z";
                
				if (rect_ptr->get_horizontal() == false)
                    rule_set->m_struct_type = structural_type::WALL;
                else
                    rule_set->m_struct_type = structural_type::FLOOR;
				
            }
			else
			{
				if (surface_count == 1) rule_set->m_space_info.first = rect_ptr->get_surface_ptr(0)->get_space_ptr(0)->get_space_type();
				if (surface_count == 2) rule_set->m_space_info.second = rect_ptr->get_surface_ptr(1)->get_space_ptr(0)->get_space_type();
				rule_set->m_surface_info.first = rect_ptr->get_surface_ptr(0)->get_surface_type();
				if (surface_count == 2) rule_set->m_surface_info.second = rect_ptr->get_surface_ptr(1)->get_surface_type();
			}
			rule_set->apply_rules();
			rectangle_rule_sets[rect_ptr] = rule_set;
		}

		// line rule sets next
		for (unsigned int j = 0; j < CF->get_line_count(); j++)
		{ // for each line segment j in the conformal model
			Spatial_Design::Geometry::Line* line_ptr = CF->get_line(j);

			// create a new line rule set
			auto rule_set = new SD_Line_Rules;

			// check for each of the rectangles if it is assigned a rectangle_rule_set
			unsigned int rect_count = line_ptr->get_rectangle_count();
			for (unsigned int k = 0; k < rect_count; k++)
			{
				// for each rectangle that line segment k belongs to
				Spatial_Design::Geometry::Rectangle* rect_ptr = line_ptr->get_rectangle_ptr(k);

				if (rect_ptr->get_zoned() == true)
				{ // if it the rectangle was assigned a rectangle rule_set, then add it to the line rule set
					rule_set->m_rectangles.push_back(rect_ptr);
					break;
				}
			}
			if (!(rule_set->m_rectangles.size() > 0))
			{ // if this line does not belong to any of the rectangles that was assigned a rectangle rule set then stop considering this line
				delete rule_set;
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
		}
		rectangle_rule_sets.clear();
		line_rule_sets.clear();

		#ifdef STABILIZE_HPP
		SD_grammar_stabilize(SD, CF);
		#endif // STABILIZE_HPP


    } // SD_grammar_zoning()
    #endif // SD_SIMULATION_HPP

	#ifdef STABILIZE_HPP
	void SD_grammar_stabilize(Structural_Design::SD_Analysis_Vars* SD, Spatial_Design::MS_Conformal* CF)
	{
	    std::cout << "Commencing Stabilization" << std::endl << std::endl;
		std::map<Components::Point*, std::vector<unsigned int> > free_dofs = SD->get_points_with_free_dofs(2);
		unsigned int free_dof_points = free_dofs.size();
		unsigned int free_nodes = 0;
		//unsigned int prev_free_nodes;
		for (auto i : free_dofs)
		{
			free_nodes += i.second.size();
		}

		SD->analyse();
		SD_Building_Results SD_results = SD->get_results();
		BSO::SD_compliance_indexing(SD_results);

		unsigned int iterations = 0;
		double initial_volume = SD_results.m_struct_volume;
		//double initial_compliance = SD_results.m_total_compliance;
		//std::cout << "Total compliance: " << initial_compliance << std::endl;
		//std::cout << "Structural volume: " << initial_volume << std::endl;
		//BSO::Visualisation::visualise(SD, 1);

		if (free_dof_points == 0)
			std::cout << "No free DOF's in Structural model" << std::endl;
		else
		{
			std::cout << "Number of points with free DOF's: " << free_dof_points;
			Structural_Design::Stabilization::Stabilize Stab(SD, CF);
	        while (free_dof_points > 0)
	        {
				iterations++;

				std::cout << std::endl << "Stabilization, round " << iterations << "..." << std::endl;

				Stab.update_free_dofs(free_dofs);
				Stab.stabilize_free_dofs(0);
				//Stab.show_free_dofs();

				//prev_free_nodes = free_nodes;
				SD->remesh();
				free_dofs = SD->get_points_with_free_dofs(2);
				free_nodes = 0;
				for (auto i : free_dofs)
				{
					free_nodes += i.second.size();
				}
				free_dof_points = free_dofs.size();

				std::cout << "Remaining points with free DOF's: " << free_dof_points << std::endl;
				std::cout << "Remaining free DOF's: " << free_nodes << std::endl;

				/*
				if (free_nodes >= prev_free_nodes)
				{
					std::cout << "Rod addition has no (positive) effect, deleting rod..." << std::endl;
					SD->m_components.pop_back();
					std::cout << "Deleted rod" << std::endl;
					SD->remesh();
					std::cout << "Remeshed" << std::endl;
					free_dofs = SD->get_points_with_free_dofs(2);
					free_nodes = 0;
					for (auto i : free_dofs)
					{
						free_nodes += i.second.size();
					}
					free_dof_points = free_dofs.size();
				}
				*/
				//BSO::Visualisation::visualise(SD, 1);
	        }
			std::cout << std::endl << "Finished stabilization" << std::endl;
			std::cout << "Iterations: " << iterations << std::endl;
			SD->analyse();
			SD_Building_Results sd_results = SD->get_results();
			BSO::SD_compliance_indexing(sd_results);
			std::cout << "Structural volume added: " << sd_results.m_struct_volume - initial_volume << std::endl;
			std::cout << "Total compliance: " << sd_results.m_total_compliance << std::endl;
			BSO::Visualisation::visualise(SD, 1);
		}
	} // SD_grammar_stabilize()

	#endif // STABILIZE_HPP


} // Grammar
} // namespace BSO


#endif // GRAMMAR_ZONING_STABILIZE_HPP
