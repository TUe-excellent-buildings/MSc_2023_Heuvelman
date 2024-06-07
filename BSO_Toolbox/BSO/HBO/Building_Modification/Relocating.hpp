#ifndef RELOCATING_HPP
#define RELOCATING_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Supercube.hpp>

#include <BSO/HBO/HBO_Settings.hpp>
#include <BSO/HBO/Performance_Evaluation/Building_Performances.hpp>

#include <iostream>
#include <vector>
#include <cmath>

namespace BSO{ namespace HBO { namespace Building_Modification
{

    BSO::Spatial_Design::MS_Building relocating_spaces( BSO::Spatial_Design::MS_Building& current_design, HBO::Performance_Evaluation::Building_Performances& build_perform, std::vector<int>& spaces_for_removal, Settings& settings)
    {
        BSO::Spatial_Design::MS_Building temp_design = current_design;
        HBO::Performance_Evaluation::Building_Performances temp_build_perform = build_perform;

        int initial_space_count = current_design.obtain_space_count();
        double initial_volume = current_design.get_volume();

        // remove the spaces indicated
        for ( unsigned int i = 0 ; i < spaces_for_removal.size() ; i++ )
        {
            temp_design.delete_space(temp_design.get_space_index(spaces_for_removal[i]));
        }

        // convert to a SC_building and add a layer of cells around it
        BSO::Spatial_Design::SC_Building sc_build = temp_design;
        int amount_of_layers = 1; // 1 layers around the entire building
        sc_build.add_padding(amount_of_layers * 2);

        // get the ids with the best performance
        std::vector<int> best_ids;
        for ( unsigned int i = 0 ; i < temp_build_perform.spaces[temp_build_perform.best_space_modified()].space_ID.size() ; i++ )
        {
            best_ids.push_back(temp_build_perform.spaces[temp_build_perform.best_space_modified()].space_ID[i]);
        }

        // check cells around the best spaces

        std::vector<int> adjacent_empty_cells; // stores all empty adjacent cells

        std::map<int, double> adjacent_empty_cells; // stores all cells and their performance indicator
        typedef std::map<int, double>::iterator adjacent_ite;

        for ( unsigned int i = 0 ; i < best_ids.size() ; i++ )
        {
            for ( unsigned int j = 1 ; j < sc_build.b_row_size(best_ids[i]) ; j++ )
            {
                    if ( sc_build.request_b(best_ids[i], j )  == 1 )
                    {
                        std::vector<int> temp_adjacent_cells = sc_build.adjacent_cells(j);

                        for (unsigned int k = 0 ; k < temp_adjacent_cells.size() ; k++ )
                        {
                            if ( sc_build.empty_cell( temp_adjacent_cells[k] ) )
                            {
                                adjacent_empty_cells[temp_adjacent_cells[k]] = 0;
                            }
                        }
                    }
            }
        }

        // find the best empty cell

        for ( adjacent_ite ite = adjacent_empty_cells.begin() ; ite != adjacent_empty_cells.end() ; ite++ )
        {
            int amount_adjacent = sc_build.adjacent_cells(ite->first).size();

            for ( unsigned int i = 0 ; i < amount_adjacent ; i++ )
            {
                if ( !sc_build.empty_cell(ite->first) ) // if the cell is not empty
                {
                    // search the for the space id
                    int id = sc_build.cell_space_id(ite->first);
                    // get the index from the building performance this id belongs to
                    int index = build_perform.id_belonging_space(id);
                    // get the performance of the space
                    double performance = build_perform.spaces[index].modified_performance;

                    // add the performance to the second part of the adjacent map
                    adjacent_empty_cells[ite->first] += performance;
                }
                else { continue; } // if the cell is empty add nothing to the mapped performance
            }
        }

        int best_cell_index;
        double best_cell_performance = 0;

        for ( adjacent_ite ite = adjacent_empty_cells.begin() ; ite != adjacent_empty_cells.end() ; ite++ )
        {
            if ( ite->second > best_cell_performance)
            {
                best_cell_performance = ite->second;
                best_cell_index = ite->first;
            }
        }






    }




} // namespace Building_Modification
} // namespace HBO
} // namespace BSO

#endif // RELOCATING_HPP
