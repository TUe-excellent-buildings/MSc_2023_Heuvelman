#ifndef SD_POINT_RULES_HPP
#define SD_POINT_RULES_HPP

#include <iostream>
#include <cstdlib>

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>
#include <Read_Grammar_Settings.hpp>


namespace BSO { namespace Grammar {
	
	using namespace Structural_Design;
	
	struct SD_Point_Rules
	{
	    Structural_Design::SD_Analysis_Vars* m_SD; // Holds the Structural Design model
	    std::pair<Components::Point*, Components::Point*> dof_key;

	    SD_Point_Rules(std::pair<Components::Point*, Components::Point*> points, Structural_Design::SD_Analysis_Vars* SD)
	    {
	        dof_key = points;
			m_SD = SD;
	    }
	    void apply_rules();
		void detach_rules();
	};
	
	void SD_Point_Rules::apply_rules()
	{
	    Truss_Props props = m_SD->m_truss_props[0];
		m_SD->m_components.push_back( new Components::Truss(props.m_E, props.m_A, dof_key.first, dof_key.second));
	    m_SD->m_components.back()->set_mesh_switch(false);
	}
	
	void SD_Point_Rules::detach_rules()
	{
	    m_SD->m_components.pop_back();
		m_SD->m_components.back()->set_mesh_switch(false);
	}
} // namespace Grammar
} // namespace BSO

#endif // SD_POINT_RULES_HPP
