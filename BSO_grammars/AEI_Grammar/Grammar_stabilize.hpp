#ifndef GRAMMAR_STABILIZE_HPP
#define GRAMMAR_STABILIZE_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Clustering.hpp>

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Simulation.hpp>
#endif // BP_SIMULATION_HPP

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <Read_SD_Settings.hpp>
#include <SD_Rectangle_Rules.hpp>
#include <SD_Line_Rules.hpp>
#endif // SD_ANALYSIS_HPP

#ifdef ZONING_HPP
#include <BSO/Spatial_Design/Zoning.hpp>
#endif // ZONING_HPP

#ifdef STABILIZE_HPP
#include <BSO/Structural_Design/Stabilization/Stabilize.hpp>
#include <Read_Stabilize_Settings.hpp>
#endif // STABILIZE_HPP

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

	double eta_bend  = 0.3;
	double eta_ax    = 0.05;
	double eta_shear = 0.1;
	double eta_noise = 0.01;
	int eta_converge = 4;
	std::string checking_order = "231"; // these are the default values, a user can change them in the main (before initialisation of the structural design model).

    // forward declarations
    #ifdef BP_SIMULATION_HPP
    void BP_grammar_3(Spatial_Design::MS_Conformal*, Building_Physics::BP_Simulation_Vars*);
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
	
		#ifndef ZONING_HPP
	    void SD_grammar_3(Spatial_Design::MS_Conformal*, Structural_Design::SD_Analysis_Vars*);
		#endif // ndef ZONING_HPP
	
		#ifdef ZONING_HPP
		Zoning_Settings zoning_settings = read_zoning_settings("JH_Stabilization_Assignment_GUI_new/Settings/Zoning_Settings.txt"); // read the zoning settings file
		bool unzoned = zoning_settings.unzoned;
		void SD_grammar_zoning(Spatial_Design::MS_Conformal*, Structural_Design::SD_Analysis_Vars*);
		#endif // ZONING_HPP
	
    #endif // SD_ANALYSIS_HPP

	#ifdef STABILIZE_HPP
		Stabilize_Settings stabilize_settings = read_stabilize_settings("JH_Stabilization_Assignment_GUI_new/Settings/Stabilize_Settings.txt"); // read the stabilize settings file
		unsigned int method = stabilize_settings.method;
		double singular = stabilize_settings.singular;
		unsigned int point_it_unzoned = stabilize_settings.point_it_unzoned;
		unsigned int zone_it = stabilize_settings.zone_it;
		unsigned int point_it_zoned = stabilize_settings.point_it_zoned;
	
		void SD_grammar_stabilize(Structural_Design::SD_Analysis_Vars* SD, Spatial_Design::MS_Conformal* CF);
		
		#ifdef ZONING_HPP
		void SD_grammar_stabilize_zoning(Structural_Design::SD_Analysis_Vars* SD, Spatial_Design::MS_Conformal* CF, Spatial_Design::Zoning::Zoned_Design* Zoned);
		std::vector<double> m_compliance; // total compliances of each stabilised zoned design (for output)
		std::vector<double> m_added_volume; // added volume of each stabilised zoned design (for output)
		#endif // ZONING_HPP

	#endif // STABILIZE_HPP

    Grammar_Settings grammar_settings; // will hold all the settings for assignment of BP and SD properties to the conformal model

    // function definition
    void grammar_stabilize(Spatial_Design::MS_Conformal* MS) // assign geometric types to the building geometry in a conformal building model
    {
        grammar_settings = read_grammar_settings("JH_Stabilization_Assignment_GUI_new/Settings/Grammar_Settings.txt"); // read the grammar settings file
        #ifdef SD_ANALYSIS_HPP
		
			#ifndef ZONING_HPP
			MS->add_grammars(&SD_grammar_3);
			#endif // ndef ZONING_HPP

			#ifdef ZONING_HPP
			MS->add_grammars(&SD_grammar_zoning);
			#endif // ZONING_HPP
			
        #endif // SD_ANALYSIS_HPP

        #ifdef BP_SIMULATION_HPP
        MS->add_grammars(&BP_grammar_3);
        #endif // BP_SIMULATION_HPP

    } // grammar_3 geometry types


    #ifdef BP_SIMULATION_HPP
    void BP_grammar_3(Spatial_Design::MS_Conformal* CF, Building_Physics::BP_Simulation_Vars* BPS)
    { // if it is tried to make a building physics model with this grammar, then give an error
        std::cerr << "Selected grammar is intended for structural design only, exiting now..." << std::endl;
        exit(1);
    } // BP_grammar_3()
    #endif // BP_SIMULATION_HPP

	#ifdef SD_ANALYSIS_HPP
	
		#ifndef ZONING_HPP
	    void SD_grammar_3(Spatial_Design::MS_Conformal* CF, Structural_Design::SD_Analysis_Vars* SD)
	    {
			using namespace Structural_Design;
	        // read file with structural design settings
	        BSO::Structural_Design::read_SD_settings("JH_Stabilization_Assignment_GUI_new/Settings/SD_Settings.txt", SD);
	
			// initialise some containers and variables
	        std::map<Spatial_Design::Geometry::Vertex*, Components::Point*> point_map;
	        std::map<Spatial_Design::Geometry::Rectangle*, SD_Rectangle_Rules*> rectangle_rule_sets;
	        std::map<Spatial_Design::Geometry::Line*, SD_Line_Rules*> line_rule_sets;

	        bool initialised = false;
			int init_number_subs = 0;
			double init_mean_tot = 0;
	std::cout << "i" << std::endl;
			for (int i = 0; i < eta_converge + 1; i++)
			{
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
				if (!initialised)
				{ // if rule sets have not yet been initialised, then do so
					//rectangle rule sets first
					for (unsigned int j = 0; j < CF->get_rectangle_count(); j++)
					{ // for each rectangle in the conformal model
						Spatial_Design::Geometry::Rectangle* rect_ptr = CF->get_rectangle(j);
						unsigned int surface_count = rect_ptr->get_surface_count();
	
						if (surface_count != 1 && surface_count != 2)
						{ // if the rectangle does not belong to one or two surfaces in the conformal model
							continue; // continue to the next rectangle in the conformal model
						}
	
						// create and initialise a new rectangle rule_set
						auto rule_set = new SD_Rectangle_Rules;
						++init_number_subs;
						rule_set->m_point_map = &point_map;
						rule_set->m_SD = SD;
						rule_set->m_grammar_settings = grammar_settings;
						rule_set->m_rectangle = rect_ptr;
						rule_set->m_facing_side_1 = rect_ptr->get_surface_ptr(0)->get_facing(0);
	
						rule_set->m_space_info.first = rect_ptr->get_surface_ptr(0)->get_space_ptr(0)->get_space_type();
						if (surface_count == 2) rule_set->m_space_info.second = rect_ptr->get_surface_ptr(1)->get_space_ptr(0)->get_space_type();
						rule_set->m_surface_info.first = rect_ptr->get_surface_ptr(0)->get_surface_type();
						if (surface_count == 2) rule_set->m_surface_info.second = rect_ptr->get_surface_ptr(1)->get_surface_type();
	
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
							unsigned int surface_count = rect_ptr->get_surface_count();
	
							if (surface_count == 1 || surface_count == 2)
							{ // if it the rectangle was assigned a rectangle rule_set, then add it to the line rule set
								rule_set->m_rectangles.push_back(rect_ptr);
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
				}
				else
				{ // else reset some values that point to the old SD model before applying the rules again
					for (auto rect_rule_set : rectangle_rule_sets)
					{
						rect_rule_set.second->m_comp_ptr = nullptr; // as it will be designing a new component in a new structural design, remove the pointer to the old one
	                    rect_rule_set.second->m_constr_ID = ""; // for the same reason, remove the old construction ID, in case it has changed
						rect_rule_set.second->apply_rules(); // apply the rules
					}
					for (auto line_rule_set : line_rule_sets)
					{
						line_rule_set.second->m_comp_ptr = nullptr; // as it will be designing a new component in a new structural design, remove the pointer to the old one
						line_rule_set.second->m_constr_ID = ""; // for the same reason, remove the old construction ID, in case it has changed
						line_rule_set.second->m_conv_type = sd_conversion_type::UNDEFINED; // reset the type of the structural conversion for this line
						line_rule_set.second->apply_rules(); // apply the rules
					}
				}
	
				SD->mesh(SD->m_mesh_division);
				std::cout << "Meshed" << std::endl;
				SD->analyse();
				std::cout << "Analysed" << std::endl;
				SD_Building_Results sd_results;
				sd_results = SD->get_results();
	
				std::vector<Spatial_Design::Geometry::Rectangle*> subs_rectangles; // contains the results of each substitute rectangle
				std::vector<Structural_Design::SD_Component_Results*> subs_results;
				std::shared_ptr<std::vector<BSO::data_point> > data_set = std::make_shared<std::vector<BSO::data_point> >(); // for clustering
	
				for (auto rect_rule_set : rectangle_rule_sets)
				{ // for all rectangle rule sets
					if (rect_rule_set.second->m_conv_type != sd_conversion_type::GHOST_FLAT_SHELL)
					{ // if the rule set is not of type ghost flat shell, then continue
						continue;
					}
	
					bool match_found = false;
					for (auto result : sd_results.m_ghost_components)
					{ // for all ghost components in the results
						auto rect_comp_ptr = rect_rule_set.second->m_comp_ptr;
	
						if (result->m_component_ptr == rect_comp_ptr)
						{ // check if the component pointers match, if so, then they are linked to each other via the order of data_set and subs_rectangles
							match_found = true;
							subs_rectangles.push_back(rect_rule_set.first);
							subs_results.push_back(result);
							data_point temp = Eigen::VectorXd(1); // create a data point for clustering
	                        temp(0) = result->m_total_compliance/result->m_struct_volume; // assign the ghost compliance of the current ghost component to the data point
	                        data_set->push_back(temp); // add the data point to the data set for clustering
							break; // stop looking for the match
						}
					}
					if (!match_found)
					{
						std::cerr << "Error, could not match substitute component results with a rule_set. Exiting... (grammar_3.hpp)" << std::endl;
						exit(1);
					}
				}
	
				// cluster the data
				std::vector<std::shared_ptr<BSO::Cluster> > clustered_data_set;
				bool do_clustering = false;
				unsigned int k_min = 4;
				if (i < eta_converge && k_min < subs_rectangles.size())
				{ // perform clustering
					do_clustering = true;
					int k_max = subs_rectangles.size(); // select minimal and maximal number of clusters
					k_max = (k_max < 10) ? k_max : 10; // set an upper limit of 10 clusters to the number of clusters
					clustered_data_set = clustering(data_set, 50, k_min, k_max, 1);
				}
	
				// check the convergence criterium
				unsigned int left_over = std::max(0,init_number_subs - (int)std::ceil(init_number_subs/(double)eta_converge)*(i+1));
				while (left_over >= subs_rectangles.size() && left_over != 0)
				{ // if the target has already been reached, then skip the this iteration (++i) and set the next target
					left_over = std::max(0,(init_number_subs - (int)std::ceil(init_number_subs/(double)eta_converge)*((++i)+1)));
				}
	
				unsigned int current_subs_rect = subs_rectangles.size();
				while (left_over < current_subs_rect && current_subs_rect > 0)
				{ // start replacing selected substitute rectangles
					// if clustering is applied, then select the cluster with the maximum centroid
					std::shared_ptr<BSO::Cluster> max_cluster;
					if (do_clustering)
					{
						max_cluster = *(clustered_data_set.begin());
						for (auto cluster : clustered_data_set)
						{
							if (cluster->m_dist_from_origin > max_cluster->m_dist_from_origin)
							{
								max_cluster = cluster;
							}
						}
						clustered_data_set.erase(std::find(clustered_data_set.begin(), clustered_data_set.end(), max_cluster));
					}
	
					// get the initial mean of the substitute compliance per volume
					if (!initialised)
					{
						for (auto j : subs_results)
							init_mean_tot += j->m_total_compliance/j->m_struct_volume;
						init_mean_tot /= subs_results.size();
					}
	
					for (unsigned int j = 0; j < subs_rectangles.size(); j++)
					{ // for each substitute rectangle
						// check if it should be replaced
						bool must_replace = false;
						if (!do_clustering || max_cluster->m_bit_mask[j])
						{
							must_replace = true;
						}
	
						// do the replacements
						if (must_replace)
						{
							--current_subs_rect;
							auto res_ptr = subs_results[j];
							auto rect_ptr = subs_rectangles[j];
							auto rule_set_ptr = rectangle_rule_sets[rect_ptr];
							double total = res_ptr->m_total_compliance/res_ptr->m_struct_volume;
							double bend  = res_ptr->m_total_bending_compliance/res_ptr->m_struct_volume;
							double axial = res_ptr->m_total_axial_compliance/res_ptr->m_struct_volume;
							double shear = res_ptr->m_total_shear_compliance/res_ptr->m_struct_volume;
	
							if (total >= eta_noise*init_mean_tot)
							{
								for (int k = 0; k < 3 && (rule_set_ptr->m_conv_type == sd_conversion_type::GHOST_FLAT_SHELL); k++)
								{
									switch(checking_order[k])
									{
									case '1':
									{
										if (shear/total >= eta_shear)
										{
											rule_set_ptr->m_conv_type = sd_conversion_type::TRUSS;
										}
										break;
									}
									case '2':
									{
										if (axial/total >= eta_ax)
										{
											rule_set_ptr->m_conv_type = sd_conversion_type::BEAM;
										}
										break;
									}
									case '3':
									{
										if (bend/total >= eta_bend)
										{
											rule_set_ptr->m_conv_type = sd_conversion_type::FLAT_SHELL;
										}
										break;
									}
									default:
									{
										std::cerr << "Found an invalid character in the checking order, exiting now (grammar_3.hpp)" << std::endl;
										exit(1);
									}
									}
								}
							}
							if (rule_set_ptr->m_conv_type == sd_conversion_type::GHOST_FLAT_SHELL)
							{
								rule_set_ptr->m_conv_type = sd_conversion_type::NO_STRUCTURE;
							}
						}
					}
				}
	
				if (i != eta_converge)
				{ // if there is still an iteration coming
					SD->m_previous_designs.push_back(new SD_Analysis()); // push create a new and empty SD model in a vector within this SD model
					SD->transfer_model(*(SD->m_previous_designs.back())); // transfer this model to the newly created (empty) model
				}
				initialised = true;
			}
			for (auto rect_rule_set_ptr : rectangle_rule_sets)
			{
				delete rect_rule_set_ptr.second;
			}
			for (auto line_rule_set_ptr : line_rule_sets)
			{
				delete line_rule_set_ptr.second;
			}

			#ifdef STABILIZE_HPP
			SD_grammar_stabilize(SD, CF);
			#endif // STABILIZE_HPP

	    } // SD_grammar_3()
		#endif // ndef ZONING_HPP
	
		#ifdef ZONING_HPP
	    void SD_grammar_zoning(Spatial_Design::MS_Conformal* CF, Structural_Design::SD_Analysis_Vars* SD)
	    {
			BSO::Spatial_Design::Zoning::Zoned_Design Zoned(CF);
		    Zoned.make_zoning();
			for (unsigned int l = 0; l < Zoned.get_designs().size(); l++)
		    {
		        if (unzoned == false)
				{
					std::cout << std::endl << "Zoned design " << l + 1 << ":" << std::endl;
					std::cout << "Creating structural model..."<< std::endl;
					
					#ifndef STABILIZE_HPP
					Zoned.reset_SD_model();
			        Zoned.prepare_zoned_SD_model(l);
					#endif // ndef STABILIZE_HPP
					
					#ifdef STABILIZE_HPP
					if (method == 1) // partially zoned
					{
						Zoned.reset_SD_model();
				        Zoned.prepare_unzoned_SD_model();
					}
					else if (method == 2) // fully zoned
					{
						Zoned.reset_SD_model();
				        Zoned.prepare_zoned_SD_model(l);
					}
					#endif // STABILIZE_HPP
				}
				else if (unzoned == true)
				{
					std::cout << "Creating structural model for the unzoned design..."<< std::endl;
					Zoned.reset_SD_model();
			        Zoned.prepare_unzoned_SD_model();
				}
				
				using namespace Structural_Design;
		        BSO::Structural_Design::read_SD_settings("JH_Stabilization_Assignment_GUI_new/Settings/SD_Settings.txt", SD); // read file with structural design settings
		
		        /*
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
		
    */
				// add all the points that make a rectangle in the conformal model to the structural model
		        std::map<Spatial_Design::Geometry::Vertex*, Components::Point*> point_map;
				point_map.clear();
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
		        for (unsigned int i = 0; i < CF->get_rectangle_count(); i++)
		        { // add ghost-rectangle vertices
		            Spatial_Design::Geometry::Rectangle* rectangle_ptr = CF->get_rectangle(i);
					if (rectangle_ptr->get_zoned() == false && rectangle_ptr->get_horizontal() == true && rectangle_ptr->get_surface_count() == 2)
		            {
		                for (unsigned int j = 0; j < 4; j++)
		                {
			                Spatial_Design::Geometry::Vertex* temp_vertex = rectangle_ptr->get_vertex_ptr(j);
							if (point_map.find(temp_vertex) == point_map.end())
			                { // if the point is not added to the structural design yet
			                    Components::Point* temp_point = new Components::Point(temp_vertex->get_coords());
		
			                    point_map[temp_vertex] = temp_point;
		
			                    SD->m_points.push_back(temp_point); // add to structural design
			                }
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
		                    rule_set->m_surface_info.second = "E";
		                    rule_set->m_space_info.second = "E";
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
		                if (/*rect_ptr->get_surface_count() == 0 && */rect_ptr->get_zoned() == true /*&& line_ptr->get_zoned() == true*/)
		                {
		                    rule_set->m_rectangles.push_back(rect_ptr); // space-intersecting rectangle
		                    //break;
		                }
						/*
		                else if (rect_ptr->get_surface_count() <= 2 && rect_ptr->get_zoned() == true && line_ptr->get_zoned() == false)
		                {
		                    rule_set->m_rectangles.push_back(rect_ptr); // space-coinciding rectangle
		                }
						*/
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
		        }
				
				#ifdef STABILIZE_HPP
				if (unzoned == false && method == 1) // partially zoned
				{
					Zoned.reset_zoned_SD_model();
			        Zoned.prepare_zoned_SD_model(l);
					std::cout << "Commencing stabilization..." << std::endl;
					//SD_grammar_stabilize_zoning(SD, CF, Zoned.get_designs()[l]);
					SD_grammar_stabilize(SD, CF);
				}
				if (unzoned == false && method == 2) // fully zoned
				{
					std::cout << "Commencing stabilization..." << std::endl;
					SD_grammar_stabilize_zoning(SD, CF, Zoned.get_designs()[l]);
				}
				#endif // STABILIZE_HPP
				
				if (unzoned == true)
				{
					#ifdef STABILIZE_HPP
					SD_grammar_stabilize(SD, CF);
					#endif // STABILIZE_HPP
					break;
				}
				
				SD->m_previous_designs.push_back(new SD_Analysis()); // push create a new and empty SD model in a vector within this SD model
				SD->transfer_model(*(SD->m_previous_designs.back())); // transfer this model to the newly created (empty) model
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
				if (unzoned == false)
				{
					std::cout << std::endl << "Compliances:" << std::endl;
					for (unsigned int i = 0 ; i < m_compliance.size(); i++)
					{
						std::cout << m_compliance[i] << std::endl;
					}
					std::cout << std::endl << "Added volumes:" << std::endl;
					for (unsigned int i = 0 ; i < m_added_volume.size(); i++)
					{
						std::cout << m_added_volume[i] << std::endl;
					}
				}
				#endif // STABILIZE_HPP
				
			}
	    } // SD_grammar_zoning()
		#endif // ZONING_HPP
		
	#endif // SD_ANALYSIS_HPP

	#ifdef STABILIZE_HPP
		
		
		void SD_grammar_stabilize(Structural_Design::SD_Analysis_Vars* SD, Spatial_Design::MS_Conformal* CF)
		{
			#ifdef AUTOSTABILIZE
	        std::cout << "Commencing Stabilization..." << std::endl << std::endl;
			std::map<Components::Point*, std::vector<unsigned int> > free_dofs = SD->get_points_with_free_dofs(singular);
			unsigned int free_dof_points = free_dofs.size();
			unsigned int free_nodes = 0;
			//unsigned int prev_free_nodes;
			for (auto i : free_dofs)
			{
				free_nodes += i.second.size();
			}
			SD->remesh();
			SD->analyse();
			SD_Building_Results SD_results = SD->get_results();
			BSO::SD_compliance_indexing(SD_results);

			unsigned int iterations = 0;
			double initial_volume = SD_results.m_struct_volume;
			//double initial_compliance = SD_results.m_total_compliance;
			//std::cout << "Total compliance: " << initial_compliance << std::endl;
			//std::cout << "Structural volume: " << initial_volume << std::endl;
			BSO::Visualisation::visualise(SD, 1);
		
			if (free_dof_points == 0)
				std::cout << "No free DOF's in Structural model" << std::endl;
			else
			{
				std::cout << "Number of points with free DOF's: " << free_dof_points;
				Structural_Design::Stabilization::Stabilize Stab(SD, CF);
		        while (free_dof_points > 0)
		        {
					iterations++;

					//std::cout << std::endl << "Stabilization, round " << iterations << "..." << std::endl;

					Stab.update_free_dofs(free_dofs);
					bool stabilization_possible = Stab.stabilize_free_dofs(point_it_unzoned);
					//Stab.show_free_dofs();

					//prev_free_nodes = free_nodes;
					SD->remesh();
					free_dofs = SD->get_points_with_free_dofs(singular);
					free_nodes = 0;
					for (auto i : free_dofs)
					{
						free_nodes += i.second.size();
					}
					free_dof_points = free_dofs.size();
					if (stabilization_possible == false)
					{
						//Stab.show_free_dofs();
						break;
					}

					//std::cout << "Remaining points with free DOF's: " << free_dof_points << std::endl;
					//std::cout << "Remaining free DOF's: " << free_nodes << std::endl;

					/*
					if (free_nodes >= prev_free_nodes)
					{
						std::cout << "Rod addition has no (positive) effect, deleting rod..." << std::endl;
						SD->m_components.pop_back();
						std::cout << "Deleted rod" << std::endl;
						SD->remesh();
						std::cout << "Remeshed" << std::endl;
						free_dofs = SD->get_points_with_free_dofs(singular);
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
				SD->remesh();
				SD->analyse();
				SD_Building_Results sd_results = SD->get_results();
				BSO::SD_compliance_indexing(sd_results);
				std::cout << "Total compliance: " << sd_results.m_total_compliance << std::endl;
				std::cout << "Total structural volume: " << sd_results.m_struct_volume << std::endl;
				std::cout << "Structural volume added for stabilization: " << sd_results.m_struct_volume - initial_volume << std::endl;
				m_compliance.push_back(sd_results.m_total_compliance);
				m_added_volume.push_back(sd_results.m_struct_volume - initial_volume);
				BSO::Visualisation::visualise(SD, 1);
			}
			#endif // AUTOSTABILIZE
		} // SD_grammar_stabilize()
		
	
		#ifdef ZONING_HPP
		void SD_grammar_stabilize_zoning(Structural_Design::SD_Analysis_Vars* SD, Spatial_Design::MS_Conformal* CF, Spatial_Design::Zoning::Zoned_Design* Zoned)
		{
			std::map<Components::Point*, std::vector<unsigned int> > free_dofs = SD->get_zoned_points_with_free_dofs(singular);
			unsigned int free_dof_points = free_dofs.size();
			unsigned int free_nodes = 0;
			//unsigned int prev_free_nodes;
			for (auto i : free_dofs)
			{
				free_nodes += i.second.size();
			}
			SD->remesh();
			SD->analyse();
			SD_Building_Results SD_results = SD->get_results();
			BSO::SD_compliance_indexing(SD_results);

			double initial_volume = SD_results.m_struct_volume;
			//double initial_compliance = SD_results.m_total_compliance;
			//std::cout << "Total compliance: " << initial_compliance << std::endl;
			//std::cout << "Structural volume: " << initial_volume << std::endl;
			//BSO::Visualisation::visualise(SD, 1);

			if (free_dof_points == 0)
				std::cout << "No free DOF's in Structural model" << std::endl;
			else
			{
				std::cout << "Stabilizing " << free_dof_points << " points with free DOF's..." << std::endl;
				Structural_Design::Stabilization::Stabilize Stab(SD, CF, Zoned);
				Stab.update_free_dofs(free_dofs);
				Stab.stabilize_free_dofs_zoned(zone_it);
				
				SD->remesh();
				free_dofs = SD->get_zoned_points_with_free_dofs(singular);
				free_dof_points = free_dofs.size();
				while (free_dof_points > 0)
		        {
					std::cout << "Zoned point iteration needed.." << std::endl;
					//Stab.update_free_dofs(free_dofs);
					//Stab.show_free_dofs();
					bool stabilization_possible = Stab.stabilize_free_dofs(point_it_zoned);
					if (stabilization_possible == false)
					{
						//Stab.show_free_dofs();
						break;
					}
					
					SD->remesh();
					free_dofs = SD->get_zoned_points_with_free_dofs(singular);
					free_dof_points = free_dofs.size();
		        }
				
				SD->remesh();
				free_dofs = SD->get_points_with_free_dofs(singular);
				free_dof_points = free_dofs.size();
				while (free_dof_points > 0)
		        {
					std::cout << "Unzoned point iteration needed.." << std::endl;
					//Stab.update_free_dofs(free_dofs);
					//Stab.show_free_dofs();
					bool stabilization_possible = Stab.stabilize_free_dofs(point_it_unzoned);
					if (stabilization_possible == false)
					{
						Stab.show_free_dofs();
						break;
					}
					
					SD->remesh();
					free_dofs = SD->get_points_with_free_dofs(singular);
					free_dof_points = free_dofs.size();
		        }
				
				std::cout << "Finished stabilization" << std::endl;
				//std::cout << "Iterations: " << iterations << std::endl;
				SD->remesh();
				SD->analyse();
				SD_Building_Results sd_results = SD->get_results();
				BSO::SD_compliance_indexing(sd_results);
				std::cout << "Total compliance: " << sd_results.m_total_compliance << std::endl;
				std::cout << "Total structural volume: " << sd_results.m_struct_volume << std::endl;
				std::cout << "Structural volume added for stabilization: " << sd_results.m_struct_volume - initial_volume << std::endl;
				m_compliance.push_back(sd_results.m_total_compliance);
				m_added_volume.push_back(sd_results.m_struct_volume - initial_volume);
				BSO::Visualisation::visualise(SD, 1);
			}
		} // SD_grammar_stabilize_zoning()
		#endif // ZONING_HPP
		
	#endif // STABILIZE_HPP

} // Grammar
} // namespace BSO


#endif // GRAMMAR_STABILIZE_HPP

