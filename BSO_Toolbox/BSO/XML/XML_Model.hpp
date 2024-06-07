#ifndef XML_MODEL_HPP
#define XML_MODEL_HPP

#include <BSO/Spatial_Design/Conformation.hpp>

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Analysis.hpp>
#endif // SD_ANALYSIS_HPP

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Simulation.hpp>
#endif // BP_SIMULATION_HPP

#include <BSO/XML/XML_Vector_Translator.hpp>

#include <map>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace BSO { namespace XML {

class XML_Model
{
private:
    std::map<Spatial_Design::Geometry::Rectangle*, unsigned int> m_rectangle_ptrs;
    std::map<Spatial_Design::Geometry::Space*, unsigned int> m_space_ptrs;

    boost::property_tree::ptree m_pt;
public:
    XML_Model(Spatial_Design::MS_Conformal&, std::string, int);
    XML_Model(std::string);
    ~XML_Model();

    #ifdef SD_ANALYSIS_HPP
    void add_structural_design(Structural_Design::SD_Analysis&);
    #endif // SD_ANALYSIS_HPP

    #ifdef BP_SIMULATION_HPP
    void add_building_physics_design(Building_Physics::BP_Simulation&);
    #endif // BP_SIMULATION_HPP

    void init_MS_building(Spatial_Design::MS_Building&);

    void write_xml_file(std::string);
    void read_xml_file(std::string);

}; // class XML_Model



} // namespace XML
} // namespace BSO



#include <BSO/XML/XML_Model.cpp>

#endif // XML_MODEL_HPP
