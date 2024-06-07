#ifndef SD_RECTANGLE_RULES_HPP
#define SD_RECTANGLE_RULES_HPP

#include <iostream>
#include <cstdlib>

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>
#include <Read_Grammar_Settings.hpp>


namespace BSO { namespace Grammar {

enum class structural_type {UNDEFINED, WALL, FLOOR, ARG_COUNT};
enum class sd_conversion_type {UNDEFINED, TRUSS, UNSTABLE_TRUSS, BEAM, FLAT_SHELL, GHOST_FLAT_SHELL, NO_STRUCTURE, ARG_COUNT};

using namespace Structural_Design;

struct SD_Rectangle_Rules
{
    structural_type m_struct_type;
    sd_conversion_type m_conv_type;
    std::string m_constr_ID;
    std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>* m_point_map; // pointer to the map that contains all the points in the SD model
    std::vector<Rectangle_Setting>* m_rect_settings_set;
    Structural_Design::SD_Analysis_Vars* m_SD; // Holds the Structural Design model
    Grammar_Settings m_grammar_settings; // holds the conversion tables for BP and SD design
    Spatial_Design::Geometry::Rectangle* m_rectangle; // pointer to the rectangle in the conformal model (to be determined if it will be added and if it will which properties it will get)
    std::pair<std::string, std::string> m_space_info; // holds the type information of the two adjacent spaces
    std::pair<std::string, std::string> m_surface_info; // holds the type information of the two adjacent spaces
	std::pair<std::string, std::string> m_surf_conv_info;
    std::pair<double, double> m_facing_side_1; // holds the facing of the rectangle <azimuth, altitude angle> (facing is positive at the first entry of m_space_info and thus negative at the second)

    unsigned int m_thickness = 0;
    double m_loading = 0;

    bool m_settings_found;
    Rectangle_Setting m_rect_conv_setting;
    Components::Component* m_comp_ptr;

    SD_Rectangle_Rules()
    {
        m_comp_ptr    = nullptr;
        m_struct_type = structural_type::UNDEFINED;
        m_conv_type   = sd_conversion_type::UNDEFINED;
        m_constr_ID   = "";
        m_settings_found = false;
    }

    void apply_rules();
    void apply_surface_load(unsigned int lc, unsigned int dof, double magnitude);

};

void SD_Rectangle_Rules::apply_rules()
{
    using namespace Structural_Design;
    //m_rectangle->make_structural();
    if(m_struct_type == structural_type::UNDEFINED && (m_facing_side_1.second < 45 && m_facing_side_1.second > -45))
    { // if the rectangle has not been defined yet, and has an altitude angle between -45 and 45 degrees, then assign it as a wall
        m_struct_type = structural_type::WALL;
        m_rect_settings_set = &m_grammar_settings.m_SD_rectangles_wall;
    }
    else if (m_struct_type == structural_type::UNDEFINED)
    { // if the rectangle has not been defined yet, and has an altitude angle smaller than -45 greater than 45 degrees, then assign it as a floor
        m_struct_type = structural_type::FLOOR;
        m_rect_settings_set = &m_grammar_settings.m_SD_rectangles_floor;
    }
    else if (m_struct_type == structural_type::WALL)
    {
        m_rect_settings_set = &m_grammar_settings.m_SD_rectangles_wall;
    }
    else if (m_struct_type == structural_type::FLOOR)
    {
        m_rect_settings_set = &m_grammar_settings.m_SD_rectangles_floor;
    }

    if (!m_settings_found)
    {
		if (m_space_info.first == "" && m_surface_info.first == "")
		{ // assign default value using space types
			m_surf_conv_info = m_space_info;
			m_surf_conv_info.first = "A";
			if (m_surf_conv_info.second == "") m_surf_conv_info.second = "A";
		}
		else if (m_space_info.first == "")
		{ // use the surface type info if no space type info has been defined
			m_surf_conv_info = m_surface_info;
			if (m_surf_conv_info.second == "") m_surf_conv_info.second = "A";
		}
		else
		{ // by default use the space type info
			m_surf_conv_info = m_space_info;
			if (m_surf_conv_info.second == "") m_surf_conv_info.second = "A";
		}

		if (m_rectangle->get_surface_count() == 1)
		{ // override the default value of "A"
			m_surf_conv_info.second = "E";
		}

        for (Rectangle_Setting setting_ite : *m_rect_settings_set)
        {
            if (setting_ite.compare(m_surf_conv_info))
            {
                m_settings_found = true;
                m_rect_conv_setting = setting_ite;
                break;
            }
        }
    }
    if (!m_settings_found)
    {
        std::cerr << "Error: did not find conversion settings" << std::endl
                  << "Error: type set: " << m_surf_conv_info.first << " - " <<  m_surf_conv_info.second << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
        exit(1);
    }


    if (m_conv_type == sd_conversion_type::UNDEFINED && m_rect_conv_setting.m_type == "Flat_Shell")
    {
        m_conv_type = sd_conversion_type::FLAT_SHELL;
    }
    else if (m_conv_type == sd_conversion_type::UNDEFINED && m_rect_conv_setting.m_type == "Beam")
    {
        m_conv_type = sd_conversion_type::BEAM;
    }
    else if (m_conv_type == sd_conversion_type::UNDEFINED && m_rect_conv_setting.m_type == "Truss")
    {
        m_conv_type = sd_conversion_type::TRUSS;
    }
    else if (m_conv_type == sd_conversion_type::UNDEFINED && m_rect_conv_setting.m_type == "Unstable_Truss")
    {
        m_conv_type = sd_conversion_type::UNSTABLE_TRUSS;
    }
    else if (m_conv_type == sd_conversion_type::UNDEFINED && m_rect_conv_setting.m_type == "Ghost_Flat_Shell")
    {
        m_conv_type = sd_conversion_type::GHOST_FLAT_SHELL;
    }
    else if (m_conv_type == sd_conversion_type::UNDEFINED && m_rect_conv_setting.m_type == "None")
    {
        m_conv_type = sd_conversion_type::NO_STRUCTURE;
    }

    if (m_struct_type == structural_type::UNDEFINED || m_conv_type == sd_conversion_type::UNDEFINED)
    {
        std::cerr << "Error in assigning a structural typology to a rectangle, exiting... (SD_rectangle_rules.hpp" << std::endl;
		std::cerr << "Error: Cannot find conversion type: " << m_rect_conv_setting.m_type << std::endl;
        exit(1);
    }

    // creating structural elements
    switch(m_conv_type)
    {
    case sd_conversion_type::TRUSS:
    {
        m_rectangle->make_structural();
		// the rectangle will be converted to two structural trusses i.e. in a cross of
        if (m_constr_ID == "")
        {
            m_constr_ID = m_rect_conv_setting.m_type_ID;
        }
        Truss_Props props;
        bool properties_found = false;
        for (Truss_Props prop_ite : m_SD->m_truss_props)
        {
            if (m_constr_ID == prop_ite.m_ID)
            {
                properties_found = true;
                props = prop_ite;
                break;
            }
        }
        if (!properties_found)
        {
            std::cerr << "Error: did not find property set when converting a wall rectangle to a TRUSS structure" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
            exit(1);
        }

        // apply the cross of the two trusses
        Components::Point* structural_points[4];
        for (int i = 0; i < 4; i++)
        {
            structural_points[i] = (*m_point_map)[m_rectangle->get_vertex_ptr(i)];
        }

        // assuming the points are ordered:
        m_SD->m_components.push_back( new Components::Truss(props.m_E, props.m_A, structural_points[0], structural_points[2]));
        m_SD->m_components.back()->set_mesh_switch(false);
        m_SD->m_components.push_back( new Components::Truss(props.m_E, props.m_A, structural_points[1], structural_points[3]));
        m_SD->m_components.back()->set_mesh_switch(false);

        break;
    }

    case sd_conversion_type::UNSTABLE_TRUSS:
    {
        // the rectangle will only get the properties of a truss
        if (m_constr_ID == "")
        {
            m_constr_ID = m_rect_conv_setting.m_type_ID;
        }
        Truss_Props props;
        bool properties_found = false;
        for (Truss_Props prop_ite : m_SD->m_truss_props)
        {
            if (m_constr_ID == prop_ite.m_ID)
            {
                properties_found = true;
                props = prop_ite;
                break;
            }
        }
        if (!properties_found)
        {
            std::cerr << "Error: did not find property set when converting a wall rectangle to a TRUSS structure" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
            exit(1);
        }

        break;
    }

    case sd_conversion_type::BEAM:
    {
        m_rectangle->make_structural();
		// the rectangle will be converted to four beams on each of the rectangle's sides
        if (m_constr_ID == "")
        {
            m_constr_ID = m_rect_conv_setting.m_type_ID;
        }
        Beam_Props props;
        bool properties_found = false;
        for (Beam_Props prop_ite : m_SD->m_beam_props)
        {
            if (m_constr_ID == prop_ite.m_ID)
            {
                properties_found = true;
                props = prop_ite;
                break;
            }
        }
        if (!properties_found)
        {
            std::cerr << "Error: did not find property set when converting a wall rectangle to a BEAM structure" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
            exit(1);
        }

        // assigning the components will occur in the SD_Line_Rules.hpp

        break;
    }
    case sd_conversion_type::FLAT_SHELL:
    {
		m_rectangle->make_structural();
        // the rectangle will be converted to a structural flat shell
        if (m_constr_ID == "")
        {
            m_constr_ID = m_rect_conv_setting.m_type_ID;
        }
        Flat_Shell_Props props;
        bool properties_found = false;
        for (Flat_Shell_Props prop_ite : m_SD->m_flat_shell_props)
        {
            if (m_constr_ID == prop_ite.m_ID)
            {
                properties_found = true;
                props = prop_ite;
                break;
            }
        }
        if (!properties_found)
        {
            std::cerr << "Error: did not find property set when converting a wall rectangle to a FLAT_SHELL" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
            exit(1);
        }
        if(m_thickness == 0)
        {
            m_SD->m_components.push_back( new Components::Flat_Shell(props.m_t, props.m_E, props.m_v, m_rectangle, *m_point_map));
            m_comp_ptr = m_SD->m_components.back();
            break;
        }
        else
        {
            m_SD->m_components.push_back( new Components::Flat_Shell(m_thickness, props.m_E, props.m_v, m_rectangle, *m_point_map));
            m_comp_ptr = m_SD->m_components.back();
            break;
        }
    }
    case sd_conversion_type::GHOST_FLAT_SHELL:
    {
        // the rectangle will be converted to a structural flat shell
        if (m_constr_ID == "")
        {
            m_constr_ID = m_rect_conv_setting.m_type_ID;
        }
        Flat_Shell_Props props;
        bool properties_found = false;
        for (Flat_Shell_Props prop_ite : m_SD->m_ghost_flat_shell_props)
        {
            if (m_constr_ID == prop_ite.m_ID)
            {
                properties_found = true;
                props = prop_ite;
                break;
            }
        }
        if (!properties_found)
        {
            std::cerr << "Error: did not find property set when converting a wall rectangle to a FLAT_SHELL" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
            exit(1);
        }
        if(m_thickness == 0)
        {
            m_SD->m_components.push_back( new Components::Flat_Shell(props.m_t, props.m_E, props.m_v, m_rectangle, *m_point_map));
            m_comp_ptr = m_SD->m_components.back();
            m_comp_ptr->set_ghost_component(true);
            m_comp_ptr->set_vis_transparancy(true);
            break;
        }
        else
        {
            m_SD->m_components.push_back( new Components::Flat_Shell(m_thickness, props.m_E, props.m_v, m_rectangle, *m_point_map));
            m_comp_ptr = m_SD->m_components.back();
            m_comp_ptr->set_ghost_component(true);
            m_comp_ptr->set_vis_transparancy(true);
            break;
        }
    }
    case sd_conversion_type::NO_STRUCTURE:
    { // do nothing
        break;
    }
    default:
        std::cerr << "Error in converting rectangle element to wall rectangle, exiting" << std::endl;
        exit(1);
        break;
    } // end of switch

    double max_z = m_rectangle->get_vertex_ptr(0)->get_coords()(2);
    for (int i = 1; i < 4; i++)
    { // find the maximum z-coordinate of this rectangle
        double z_coord = m_rectangle->get_vertex_ptr(i)->get_coords()(2);
        if (z_coord > max_z) max_z = z_coord;
    }

    // wind loads
    if ((m_surf_conv_info.first == "E" || m_surf_conv_info.second == "E") && max_z > 0)
    { // if the rectangle is has a surface on the exteriror

        // add the wind load to rectangle (depening on what structural conversion type is given)
        for (auto ite : m_SD->m_abstract_loads)
        {
            auto abstr_load = ite.second;
            double azi_side_2 = (m_facing_side_1.first > 180) ? m_facing_side_1.first - 180 : m_facing_side_1.first + 180; // getting the azimuth of the facing of side 2 (0-360 degrees)
            double alt_side_2 = -1 * m_facing_side_1.second;
            double attack_angle = std::abs(azi_side_2 - abstr_load.m_azimuth); // getting the absolute value between the angle of the wind and the facing of the exterior
            attack_angle = (attack_angle > 180) ? 360 - attack_angle : attack_angle; // transform to the smallest angle between azi_side_2 and abstr_load.m_azimuth

            Vectors::Vector direction = Vectors::calc_direction(azi_side_2, alt_side_2);

            if (abstr_load.m_type == "wind_pressure" && (attack_angle > 90 && attack_angle <= 180))
            { // if the angle of attack by the wind is over 90 degrees, then the surface and wind point in opposing directions, thus pressure occurs
                this->apply_surface_load(abstr_load.m_lc, 0, std::abs(direction(0))*abstr_load.m_direction(0)*abstr_load.m_magnitude);
                this->apply_surface_load(abstr_load.m_lc, 1, std::abs(direction(1))*abstr_load.m_direction(1)*abstr_load.m_magnitude);
            }
            else if (abstr_load.m_type == "wind_suction" && (attack_angle >= 0 && attack_angle < 90))
            { // if the angle of attack by the wind is between zero and 90 degrees, then the surface and wind point in the same direction, thus suction occurs
                this->apply_surface_load(abstr_load.m_lc, 0, std::abs(direction(0))*abstr_load.m_direction(0)*abstr_load.m_magnitude);
                this->apply_surface_load(abstr_load.m_lc, 1, std::abs(direction(1))*abstr_load.m_direction(1)*abstr_load.m_magnitude);
            }
            else if (abstr_load.m_type == "wind_shear" && (std::abs(alt_side_2) > 88 && m_rectangle->get_vertex_ptr(0)->get_coords()(2) > 0))
            { // if the surface is nearly horizontal, put the full shear load on it, independent of azimuth
                this->apply_surface_load(abstr_load.m_lc, 0, abstr_load.m_direction(0)*abstr_load.m_magnitude);
                this->apply_surface_load(abstr_load.m_lc, 1, abstr_load.m_direction(1)*abstr_load.m_magnitude);
                this->apply_surface_load(abstr_load.m_lc, 2, abstr_load.m_direction(2)*abstr_load.m_magnitude);
            }
            else if (abstr_load.m_type == "wind_shear" && (attack_angle >= 90 && attack_angle < 180))
            { // if the angle of attack by the wind is over 90 degrees and under 180, then the surface and wind point in opposing directions, thus pressure occurs, and with that also shear (if under an angle)
                this->apply_surface_load(abstr_load.m_lc, 0, std::abs(direction(1))*abstr_load.m_direction(0)*abstr_load.m_magnitude);
                this->apply_surface_load(abstr_load.m_lc, 1, std::abs(direction(0))*abstr_load.m_direction(1)*abstr_load.m_magnitude);
            }
        }
    }

    // live loads
    if (m_struct_type == structural_type::FLOOR)
    { // add a live loading
        Abstract_Load live_load = m_SD->m_abstract_loads[1];
        for (int i = 0; i < 3; i++)
        { // for each dof
            if (live_load.m_direction(i) > 0.01 || live_load.m_direction(i) < -0.01)
            {
                this->apply_surface_load(live_load.m_lc, i, live_load.m_direction(i)*(live_load.m_magnitude + m_loading));
            }
        }
    }

} // apply_rules()

void SD_Rectangle_Rules::apply_surface_load(unsigned int lc, unsigned int dof, double magnitude)
{
    if (magnitude < 1e-5 && magnitude > -1e-5) return; // is the load is close to zero or zero, then dont bother putting it on the surface

    if (m_conv_type == sd_conversion_type::FLAT_SHELL || m_conv_type == sd_conversion_type::GHOST_FLAT_SHELL)
    { // if the rectangle is a flat shell, then just put the load on that component
        m_comp_ptr->add_load(Components::Load(lc, dof, magnitude));
    }
    else if (m_conv_type == sd_conversion_type::TRUSS ||
			 m_conv_type == sd_conversion_type::UNSTABLE_TRUSS ||
             m_conv_type == sd_conversion_type::BEAM  ||
             m_conv_type == sd_conversion_type::NO_STRUCTURE)
    { // if there is a wind cross in this rectangle, put a ghost flat shell here, to bring the loading to the appropriate places
        // create the load
        Components::Load load_struct(lc, dof, magnitude);

        // create a ghost flat shell at this rectangle
        if (m_constr_ID == "")
        {
            m_constr_ID = m_rect_conv_setting.m_type_ID;
        }
        Flat_Shell_Props props;
        bool properties_found = false;
        for (Flat_Shell_Props prop_ite : m_SD->m_ghost_flat_shell_props)
        {
            if (m_constr_ID == prop_ite.m_ID)
            {
                properties_found = true;
                props = prop_ite;
                break;
            }
        }
        if (!properties_found)
        {
            std::cerr << "Error: did not find property set when converting a truss wall rectangle to a GHoST_FLAT_SHELL" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_rectangle_rules.hpp)" << std::endl;
            exit(1);
        }

        m_SD->m_components.push_back( new Components::Flat_Shell(props.m_t, props.m_E, props.m_v, m_rectangle, *m_point_map));
        m_comp_ptr = m_SD->m_components.back();
        m_comp_ptr->set_ghost_component(true);
        m_comp_ptr->set_vis_transparancy(true);
        m_comp_ptr->add_load(load_struct);
    }
    else
    {
        std::cerr << "Was unable to apply a surface load to rectangle, exiting (SD_Rules.hpp)" << std::endl;
        exit(1);
    }

} // apply_surface_load ()

} // namespace Grammar
} // namespace BSO

#endif // SD_RECTANGLE_RULES_HPP
