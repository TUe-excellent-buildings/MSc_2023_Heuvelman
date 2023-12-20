#ifndef SD_LINE_RULES_HPP
#define SD_LINE_RULES_HPP

#include <iostream>
#include <vector>
#include <cstdlib>

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>
#include <Read_Grammar_Settings.hpp>
#include <SD_Rectangle_Rules.hpp>

namespace BSO { namespace Grammar {

struct SD_Line_Rules
{
    sd_conversion_type m_conv_type;
    std::string m_constr_ID;
    std::vector<Spatial_Design::Geometry::Rectangle*> m_rectangles;
    std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>* m_point_map;
    Structural_Design::SD_Analysis_Vars* m_SD;
    Spatial_Design::Geometry::Line* m_line;
    std::map<Spatial_Design::Geometry::Rectangle*, SD_Rectangle_Rules*>* m_rect_rule_sets;

    Components::Component* m_comp_ptr;

    SD_Line_Rules()
    {
        m_comp_ptr  = nullptr;
        m_conv_type = sd_conversion_type::UNDEFINED;
        m_constr_ID = "";
    }

    void apply_rules();
    void apply_line_constraint(unsigned int dof);
};

void SD_Line_Rules::apply_rules()
{
    bool rect_floor = false;

    for (auto rect_ptr : m_rectangles)
    {
        if ((*m_rect_rule_sets)[rect_ptr]->m_struct_type == structural_type::FLOOR) rect_floor = true;
        sd_conversion_type conv_type = (*m_rect_rule_sets)[rect_ptr]->m_conv_type;
        std::string temp_cID = (*m_rect_rule_sets)[rect_ptr]->m_constr_ID;

        if (conv_type == sd_conversion_type::FLAT_SHELL)
        {
            if (m_conv_type != sd_conversion_type::FLAT_SHELL)
                m_constr_ID = "";

            m_conv_type = conv_type;

            if (m_constr_ID != "")
                m_constr_ID = (temp_cID > m_constr_ID) ? m_constr_ID : temp_cID;
            else
                m_constr_ID = temp_cID;
        }
        else if (conv_type == sd_conversion_type::BEAM && m_conv_type != sd_conversion_type::FLAT_SHELL)
        {
            if (m_conv_type != sd_conversion_type::BEAM)
                m_constr_ID = "";

            m_conv_type = conv_type;

            if (m_constr_ID != "")
                m_constr_ID = (temp_cID > m_constr_ID) ? m_constr_ID : temp_cID;
            else
                m_constr_ID = temp_cID;
        }
        else if (conv_type == sd_conversion_type::TRUSS &&(m_conv_type != sd_conversion_type::FLAT_SHELL && m_conv_type != sd_conversion_type::BEAM))
        {
            if (m_conv_type != sd_conversion_type::TRUSS)
                m_constr_ID = "";

            m_conv_type = conv_type;

            if (m_constr_ID != "")
                m_constr_ID = (temp_cID > m_constr_ID) ? m_constr_ID : temp_cID;
            else
                m_constr_ID = temp_cID;
        }
        else if (conv_type == sd_conversion_type::UNSTABLE_TRUSS &&(m_conv_type != sd_conversion_type::FLAT_SHELL && m_conv_type != sd_conversion_type::BEAM))
        {
            if (m_conv_type != sd_conversion_type::UNSTABLE_TRUSS)
                m_constr_ID = "";

            m_conv_type = conv_type;

            if (m_constr_ID != "")
                m_constr_ID = (temp_cID > m_constr_ID) ? m_constr_ID : temp_cID;
            else
                m_constr_ID = temp_cID;
        }
        else if (m_conv_type == sd_conversion_type::UNDEFINED || m_conv_type == sd_conversion_type::GHOST_FLAT_SHELL)
        {
            m_conv_type = sd_conversion_type::NO_STRUCTURE;
        }

    }

    switch(m_conv_type)
    {
    case sd_conversion_type::BEAM:
    {
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
            std::cerr << "Error: did not find property set when converting a line to a BEAM structure" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_Line_rules.hpp)" << std::endl;
            exit(1);
        }
        if (m_line->get_thickness() == 0)
        {
            m_SD->m_components.push_back( new Components::Beam(props.m_b, props.m_h, props.m_E, props.m_v, m_line, *m_point_map));
            m_comp_ptr = m_SD->m_components.back();
            break;
        }
        else
        {
            m_SD->m_components.push_back( new Components::Beam(m_line->get_thickness(), m_line->get_thickness(), props.m_E, props.m_v, m_line, *m_point_map));
            m_comp_ptr = m_SD->m_components.back();
            break;
        }
    } // end of case
    case sd_conversion_type::TRUSS:
    {
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
            std::cerr << "Error: did not find property set when converting a line to a TRUSS structure" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_Line_Rules.hpp)" << std::endl;
            exit(1);
        }
        m_SD->m_components.push_back( new Components::Truss(props.m_E, props.m_A, m_line, *m_point_map));
        m_SD->m_components.back()->set_mesh_switch(false);
        m_comp_ptr = m_SD->m_components.back();
        break;
    } // end of case
    case sd_conversion_type::UNSTABLE_TRUSS:
    {
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
            std::cerr << "Error: did not find property set when converting a line to a TRUSS structure" << std::endl
                      << "Error: did not find constr_ID: " << m_constr_ID << ", exiting now... (SD_Line_Rules.hpp)" << std::endl;
            exit(1);
        }

		if (m_line->get_vertex_ptr(0)->get_coords()(2) >= 0.001 || m_line->get_vertex_ptr(1)->get_coords()(2) >= 0.001)
		{
	        m_SD->m_components.push_back( new Components::Truss(props.m_E, props.m_A, m_line, *m_point_map));
	        m_SD->m_components.back()->set_mesh_switch(false);
	        m_comp_ptr = m_SD->m_components.back();
		}
        break;
    } // end of case
    case sd_conversion_type::UNDEFINED:
    {
        std::cout << m_line->get_vertex_ptr(0)->get_coords().transpose() << std::endl;
        std::cout << m_line->get_vertex_ptr(1)->get_coords().transpose() << std::endl;
        std::cerr << "Error, undefined conversion when converting line to structural component. Exiting now .. (SD_Line_Rules.hpp)" << std::endl;
        exit(1);
        break;
    }
    default:
    {
        break;
    }
    }; // end of switch statement
	
    if (m_line->get_vertex_ptr(0)->get_coords()(2) < 0.001 && m_line->get_vertex_ptr(1)->get_coords()(2) < 0.001 && rect_floor && m_line->get_constraint() == true)
    {
        apply_line_constraint(0);
        apply_line_constraint(1);
        apply_line_constraint(2);
    }

} // apply_rules()

void SD_Line_Rules::apply_line_constraint(unsigned int dof)
{
    Components::Constraint constraint(dof);
    if (m_comp_ptr != nullptr)
        m_comp_ptr->add_constraint(constraint);
    else
    {
        m_SD->m_components.push_back(new Components::Line_Constraint(constraint, m_line, *m_point_map));
    }
} // apply_line_constraint


} // namespace Grammar
} // namespace BSO


#endif // SD_LINE_RULES_HPP
