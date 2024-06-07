#ifndef BP_SPACE_RULES_HPP
#define BP_SPACE_RULES_HPP

#include <iostream>
#include <cstdlib>

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Building_Physics/States/Dep_States/BP_Wall.hpp>
#include <Read_Grammar_Settings.hpp>

namespace BSO { namespace Grammar {

using namespace Building_Physics;

struct BP_Space_Rules
{
    unsigned int m_space_ID;
    std::string m_type_ID;
    std::string m_space_set_ID;
    Spatial_Design::Geometry::Space* m_space_ptr;
    double m_volume;
    BP_Simulation_Vars* m_BP_vars;
    BP_Simulation* m_BP_sim;
    Grammar_Settings* m_grammar_settings;
    BP_Space* m_state_ptr;


    BP_Space_Rules()
    {
        m_space_ptr = nullptr;
        m_BP_vars = nullptr;
        m_BP_sim = nullptr;
        m_grammar_settings = nullptr;
        m_type_ID = "";
        m_space_ID = 0;
    }

    void apply_rules();
};

void BP_Space_Rules::apply_rules()
{
	
    if (m_type_ID == "") m_type_ID = "A";

    bool settings_found = false;
    for (auto ite : m_grammar_settings->m_BP_space_types)
    {
        if (ite.m_type_ID == m_type_ID)
        {
            m_space_set_ID = ite.m_BP_space_type;
            settings_found = true;
            break;
        }
    }

    if (!settings_found)
    {
        std::cerr << "Error could not find BP conversion for space " << m_space_ID << " with type_ID " << m_type_ID << " exiting now... (BP_Space_Rules.hpp)"  << std::endl;
        exit(1);
    }

    settings_found = false;

    Building_Physics::BP_Space_Settings space_settings;
    for (unsigned int j = 0; j < m_BP_vars->m_space_settings.size(); j++)
    {
        if (m_BP_vars->m_space_settings[j].m_space_set_ID == m_space_set_ID)
        {
            space_settings = m_BP_vars->m_space_settings[j];
            settings_found = true;
            break;
        }
    }

    if (!settings_found)
    {
        std::cerr << "Error could not find space settings for space " << m_space_ID << " with space set ID " << m_space_set_ID << " exiting now... (BP_Space_Rules.hpp)"  << std::endl;
        exit(1);
    }

    m_state_ptr = new Building_Physics::BP_Space(m_BP_sim, std::to_string(m_space_ID), m_volume, space_settings, m_BP_vars->m_weather_profile); // initialise a ptr to an object of the space class with the values read from the file
    m_BP_vars->m_space_ptrs.push_back(m_state_ptr); // add the space pointer to the space vector
    m_BP_vars->m_dep_states.push_back(m_state_ptr);
    m_BP_vars->m_states.push_back(m_state_ptr);
}

}// namespace Grammar
}// namespace BSO

#endif // BP_SPACE_RULES_HPP
