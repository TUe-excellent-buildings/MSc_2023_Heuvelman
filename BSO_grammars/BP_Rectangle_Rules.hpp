#ifndef BP_RECTANGLE_RULES_HPP
#define BP_RECTANGLE_RULES_HPP

#include <cstdlib>

namespace BSO { namespace Grammar {
using namespace Building_Physics;

enum class bp_type {UNDEFINED, WALL, FLOOR, ARG_COUNT};
enum class bp_conversion_type{UNDEFINED, GLASS, CONSTRUCTION, ARG_COUNT};

struct BP_Rectangle_Rules
{
    Spatial_Design::Geometry::Rectangle* m_rectangle_ptr;
    double m_area;
    std::map<Spatial_Design::Geometry::Space*, BP_Space_Rules*>* m_space_rules;
    Grammar_Settings* m_grammar_settings;
    std::vector<Rectangle_Setting>* m_rect_settings_set;
    BP_Simulation_Vars* m_BP_vars;
    BP_Simulation* m_BP_sim;
    bp_conversion_type m_conv;
    bp_type m_bp_type;
    Rectangle_Setting m_rect_conv_setting;
    BP_Dep_State* m_state_ptr;
    std::pair<std::string, std::string> m_space_info;
    bool m_settings_found;

    static unsigned int m_floor_counter;
    static unsigned int m_wall_counter;
    static unsigned int m_window_counter;

    BP_Rectangle_Rules()
    {
        m_rectangle_ptr = nullptr;
        m_space_rules = nullptr;
        m_grammar_settings = nullptr;
        m_BP_vars = nullptr;
        m_BP_sim = nullptr;
        m_rect_settings_set = nullptr;
        m_area = 0;
        m_conv = bp_conversion_type::UNDEFINED;
        m_bp_type = bp_type::UNDEFINED;
        m_settings_found = false;
    }

    void apply_rules();
};

unsigned int BP_Rectangle_Rules::m_floor_counter = 0;
unsigned int BP_Rectangle_Rules::m_wall_counter = 0;
unsigned int BP_Rectangle_Rules::m_window_counter = 0;

void BP_Rectangle_Rules::apply_rules()
{
    using namespace Building_Physics;

    // depending on the grammar settings, decide what this rectangle will be
    // and find the states that are present on both sides of the rectangle
    BP_State* state_side_1 = nullptr, * state_side_2 = nullptr;
    std::string side_1, side_2;
    if (m_rectangle_ptr->get_surface_count() == 2)
    {
        side_1 = (*m_space_rules)[m_rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)]->m_type_ID;
        state_side_1 = (*m_space_rules)[m_rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)]->m_state_ptr;
        side_2 = (*m_space_rules)[m_rectangle_ptr->get_surface_ptr(1)->get_space_ptr(0)]->m_type_ID;
        state_side_2 = (*m_space_rules)[m_rectangle_ptr->get_surface_ptr(1)->get_space_ptr(0)]->m_state_ptr;
    }
    else if (m_rectangle_ptr->get_surface_count() == 1)
    {
        side_1 = (*m_space_rules)[m_rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)]->m_type_ID;
        state_side_1 = (*m_space_rules)[m_rectangle_ptr->get_surface_ptr(0)->get_space_ptr(0)]->m_state_ptr;

        if (m_rectangle_ptr->get_center_vertex_ptr()->get_coords()(2) < 0.01) // if the center coordinate is below z-coordinate: 0.01
        { // then there will be soil
            side_2 = "G";
            state_side_2 = m_BP_vars->m_ground_profile;
        }
        else
        {
            side_2 = "E";
            state_side_2 = m_BP_vars->m_weather_profile;
        }
    }
    if (m_space_info.first == "") m_space_info.first = side_1;
	if (m_space_info.second == "") m_space_info.second = side_2;

    if (m_bp_type == bp_type::UNDEFINED)
    {
        if (std::abs(m_rectangle_ptr->get_surface_ptr(0)->get_facing(0).second) > 45)
        {
            m_bp_type = bp_type::FLOOR;
            m_rect_settings_set = &m_grammar_settings->m_BP_rectangles_floor;
        }
        else
        {
            m_bp_type = bp_type::WALL;
            m_rect_settings_set = &m_grammar_settings->m_BP_rectangles_wall;
        }
    }

    // find the specified construction settings ID
    if (!m_settings_found)
    {
        for (Rectangle_Setting setting_ite : *m_rect_settings_set)
        {
            if (setting_ite.compare(m_space_info))
            {
                m_settings_found = true;
                m_rect_conv_setting = setting_ite;
                break;
            }
        }
    }
    if (!m_settings_found)
    {
        std::cerr << "Error: did not find conversion settings when converting a wall rectangle to a FLAT_SHELL" << std::endl
                  << "Error: type set: " << m_space_info.first << " - " <<  m_space_info.second << ", exiting now... (SD_rules.hpp)" << std::endl;
        exit(1);
    }

    if (m_rect_conv_setting.m_type == "Glass")
    {
        m_conv = bp_conversion_type::GLASS;
    }
    else if (m_rect_conv_setting.m_type == "Construction")
    {
        m_conv = bp_conversion_type::CONSTRUCTION;
    }

    //convert to either a wall, glazing or a floor.
    switch (m_conv)
    {
    case bp_conversion_type::CONSTRUCTION:
    {
        // find the settings for this rectangle
        BP_Construction construction;
        bool constr_found = false;
        for (auto ite : m_BP_vars->m_constructions)
        {
            if (ite.m_construction_ID == m_rect_conv_setting.m_type_ID)
            {
                construction = ite;
                constr_found = true;
            }
        }
        if (!constr_found)
        {
            std::cerr << "Did not find the BP construction with ID: " << m_rect_conv_setting.m_type_ID << ". exiting now... (BP_Rectangle_Rules.hpp)" << std::endl;
            exit(1);
        }

        if (m_bp_type == bp_type::WALL)
        {
            BP_Wall* wall_ptr = new BP_Wall(m_BP_sim, std::to_string(++m_wall_counter), m_area, construction, state_side_1, state_side_2);
            wall_ptr->set_rect_ptr(m_rectangle_ptr);
            m_BP_vars->m_wall_ptrs.push_back(wall_ptr);
            m_BP_vars->m_dep_states.push_back(wall_ptr);
            m_BP_vars->m_states.push_back(wall_ptr);
        }
        else if (m_bp_type == bp_type::FLOOR)
        {
            BP_Floor* floor_ptr = new BP_Floor(m_BP_sim, std::to_string(++m_floor_counter), m_area, construction, state_side_1, state_side_2);
            floor_ptr->set_rect_ptr(m_rectangle_ptr);
            m_BP_vars->m_floor_ptrs.push_back(floor_ptr);
            m_BP_vars->m_dep_states.push_back(floor_ptr);
            m_BP_vars->m_states.push_back(floor_ptr);
        }

        //floor_ptr->add_vis_settings(m_rectangle_ptr, temp_vis_setting.m_vis_settings);

        break;
    }
    case bp_conversion_type::GLASS:
    {
        BP_Glazing glazing;
        bool glazing_found = false;
        for (auto ite : m_BP_vars->m_glazings)
        {
            if (ite.m_glazing_ID == m_rect_conv_setting.m_type_ID)
            {
                glazing = ite;
                glazing_found = true;
            }
        }
        if (!glazing_found)
        {
            std::cerr << "Did not find the BP glazing with ID: " << m_rect_conv_setting.m_type_ID << ". exiting now... (BP_Rectangle_Rules.hpp)" << std::endl;
            exit(1);
        }

        BP_Window* window_ptr = new BP_Window(m_BP_sim, std::to_string(++m_window_counter), m_area, glazing, state_side_1, state_side_2);
        window_ptr->set_rect_ptr(m_rectangle_ptr);
        m_BP_vars->m_window_ptrs.push_back(window_ptr);
        m_BP_vars->m_dep_states.push_back(window_ptr);
        m_BP_vars->m_states.push_back(window_ptr);

        break;
    }
    default:
    {
        break;
    }
    }
}

} // namespace Grammar
} // namespace BSO

#endif // BP_RECTANGLE_RULES_HPP
