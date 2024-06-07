#ifndef XML_MODEL_CPP
#define XML_MODEL_CPP

#include <BSO/Vectors.hpp>

namespace BSO { namespace XML {

XML_Model::XML_Model(Spatial_Design::MS_Conformal& CF, std::string version, int language)
{
    // constructor to initialise the XML data model with a conformal spatial design
    unsigned int space_count = CF.get_space_count(); // number of spaces in the conformal design
    unsigned int rectangle_count = CF.get_rectangle_count(); // number of rectangles in the design
    m_pt.clear();
    if (language == 1)
    {

        using boost::property_tree::ptree;
        m_pt.put("building_model.<xmlattr>.version", version);
        m_pt.put("building_model.<xmlattr>.name", "");

        // initialise every construction including its geometry
        for (unsigned int i = 0; i < rectangle_count; i++)
        { // for each rectangle in the conformal design
            if (CF.get_rectangle(i)->get_surface_count() > 0)
            { // if the rectangle belongs to a surface
                m_rectangle_ptrs[CF.get_rectangle(i)] = i+1; // add an ID to that rectangle and link it to the pointer of the rectangle
                ptree& constr_node = m_pt.add("building_model.constructions.construction", "");
                constr_node.put("<xmlattr>.name", "rectangle_" + std::to_string(i+1));
                constr_node.put("<xmlattr>.id", i+1);

                std::vector<double> coordinates;

                for (int j = 0; j < 4; j++)
                { // for each vertex of the rectangle
                    Vectors::Point temp_coords = CF.get_rectangle(i)->get_vertex_ptr(j)->get_coords(); // get the coordinates of vertex j
                    coordinates.push_back(temp_coords(0)); // x-coordinate of vertex j temp_coords(0)
                    coordinates.push_back(temp_coords(1)); // y-coordinate of vertex j
                    coordinates.push_back(temp_coords(2)); // z-coordinate of vertex j
                } // end all vertices of rectangle i


                ptree& geo_node = constr_node.put("geometry", coordinates);
                geo_node.put("<xmlattr>.type", "plane");
            }
        } // end for each rectangle

        // add the construction ids for each space
        for (unsigned int i = 0; i < space_count; i++)
        { // for each space in the conformal design
            m_space_ptrs[CF.get_space(i)] = i+1; // add an ID to that space and link it to the pointer of the space
            ptree& space_node = m_pt.add("building_model.spaces.space", "");
            space_node.put("<xmlattr>.name", "simple");
            space_node.put("<xmlattr>.id", i+1);

            std::vector<int> construction_id_list;

            for (unsigned int j = 0; j < 6; j++)
            { // for each surface
                for (unsigned int k = 0; k < CF.get_space(i)->get_surface_ptr(j)->get_rectangle_count(); k++)
                { // and for each rectangle in that surface
                    Spatial_Design::Geometry::Rectangle* temp_ptr = CF.get_space(i)->get_surface_ptr(j)->get_rectangle_ptr(k);
                    construction_id_list.push_back(m_rectangle_ptrs[temp_ptr]); // check if it exists first!
                }
            }

            space_node.put("construction_ids", construction_id_list);

        } // end for each space
    }
    else if (language == 2)
    {
                using boost::property_tree::ptree;
        m_pt.put("gebouwmodel.<xmlattr>.versie", version);
        m_pt.put("gebouwmodel.<xmlattr>.naam", "");

        // initialise every construction including its geometry
        for (unsigned int i = 0; i < rectangle_count; i++)
        { // for each rectangle in the conformal design
            if (CF.get_rectangle(i)->get_surface_count() > 0)
            { // if the rectangle belongs to a surface
                m_rectangle_ptrs[CF.get_rectangle(i)] = i+1; // add an ID to that rectangle and link it to the pointer of the rectangle
                ptree& constr_node = m_pt.add("gebouwmodel.constructies.constructie", "");
                constr_node.put("<xmlattr>.naam", "rechthoek_" + std::to_string(i+1));
                constr_node.put("<xmlattr>.id", i+1);

                std::vector<double> coordinates;

                for (int j = 0; j < 4; j++)
                { // for each vertex of the rectangle
                    Vectors::Point temp_coords = CF.get_rectangle(i)->get_vertex_ptr(j)->get_coords(); // get the coordinates of vertex j
                    coordinates.push_back(temp_coords(0)); // x-coordinate of vertex j temp_coords(0)
                    coordinates.push_back(temp_coords(1)); // y-coordinate of vertex j
                    coordinates.push_back(temp_coords(2)); // z-coordinate of vertex j
                } // end all vertices of rectangle i


                ptree& geo_node = constr_node.put("geometrie", coordinates);
                geo_node.put("<xmlattr>.type", "vlak");
            }
        } // end for each rectangle

        // add the construction ids for each space
        for (unsigned int i = 0; i < space_count; i++)
        { // for each space in the conformal design
            m_space_ptrs[CF.get_space(i)] = i+1; // add an ID to that space and link it to the pointer of the space
            ptree& space_node = m_pt.add("gebouwmodel.ruimtes.ruimte", "");
            space_node.put("<xmlattr>.naam", "simpel");
            space_node.put("<xmlattr>.id", i+1);

            std::vector<int> construction_id_list;

            for (unsigned int j = 0; j < 6; j++)
            { // for each surface
                for (unsigned int k = 0; k < CF.get_space(i)->get_surface_ptr(j)->get_rectangle_count(); k++)
                { // and for each rectangle in that surface
                    Spatial_Design::Geometry::Rectangle* temp_ptr = CF.get_space(i)->get_surface_ptr(j)->get_rectangle_ptr(k);
                    construction_id_list.push_back(m_rectangle_ptrs[temp_ptr]); // check if it exists first!
                }
            }

            space_node.put("constructieids", construction_id_list);

        } // end for each space

    }
} // ctor

XML_Model::XML_Model(std::string file_name)
{
    read_xml_file(file_name);
} // ctor

XML_Model::~XML_Model()
{

} // dtor

#ifdef SD_ANALYSIS_HPP
void XML_Model::add_structural_design(Structural_Design::SD_Analysis& SD)
{
    // check if this SD_model has been initiated with the same conformal design
    unsigned int n_components = SD.get_component_count();
    for (unsigned int i = 0; i < n_components; i++)
    {
        //Structural_Design::Components::Component* temp_ptr = SD.get_component_ptr(i);
        //if (temp_ptr->is_flat_shell())
    }

    using boost::property_tree::ptree;
    ptree& SD_node = m_pt.put("building_model.structural_design", "");

    for (unsigned int i = 0; i < SD.get_flat_shell_props().size(); i++)
    { // for each structural property set
        ptree& SD_prop_node = SD_node.add("structural_property", "");
        SD_prop_node.put("<xmlattr>.id", SD.get_flat_shell_props()[i].m_ID);
        SD_prop_node.put("<xmlattr>.type", "flat_shell_props");
        SD_prop_node.put("thickness", SD.get_flat_shell_props()[i].m_t);
        SD_prop_node.put("youngs_modulus", SD.get_flat_shell_props()[i].m_E);
        SD_prop_node.put("poisson_ratio", SD.get_flat_shell_props()[i].m_v);

        std::vector<int> construction_ids;
        for (unsigned int j = 0; j < 3; j++)
        { // for each construction id to which this property set is assigned
            //construction_ids.push_back(1); // add this
        }

        if(construction_ids.size() != 0)
        {
            SD_prop_node.add("construction_ids", construction_ids);
        }

    }


} // add_structural_design()
#endif // SD_ANALYSIS_HPP

#ifdef BP_SIMULATION_HPP
void XML_Model::add_building_physics_design(Building_Physics::BP_Simulation& BP)
{
    // check if this SD_model has been initiated with the same conformal design

} // add_building_physics_design()
#endif // BP_SIMULATION_HPP


void XML_Model::init_MS_building(Spatial_Design::MS_Building& MS)
{
    /* Wish list: this initiates an MS design, we export a conformal
       building and now input an MS building hence conformation still needs to occur.
       Alternative: don't initiate an MS building but initiate a conformal building.
       */

    // initialise the MS building design to an empty design
    MS.clear_design();

    // for every space that is listed in the ptree
    using boost::property_tree::ptree;
    BOOST_FOREACH(ptree::value_type const&v, m_pt.get_child("gebouwmodel.ruimtes"))
    { // for each space

        std::vector<std::string> construction_ids;
        if (v.first == "ruimte")
        {
            construction_ids = v.second.get<std::vector<std::string> >("constructieids");
        }
        else
        {
            continue;
        }
        // get the minimum and maximum coordinates
        // store the coordinates of each construction
        std::map<std::string, std::vector<double> > construction_coords;
        BOOST_FOREACH(ptree::value_type const&w, m_pt.get_child("gebouwmodel.constructies"))
        { // for each construction
            if (w.first == "constructie")
            {
                construction_coords[w.second.get<std::string>("<xmlattr>.id")] = w.second.get<std::vector<double> >("geometrie");
            }
        }

        Vectors::Point min;
        min(0) = construction_coords.begin()->second[0];
        min(1) = construction_coords.begin()->second[1];
        min(2) = construction_coords.begin()->second[2];
        Vectors::Point max = min;

        for (auto w : construction_ids)
        { // for each construction belonging to the space w

            std::vector<double> temp_coords = construction_coords[w];

            for (unsigned int i = 0; i*3 < temp_coords.size(); i += 3)
            { // for each coordinate that is present
                for (unsigned int j = 0; j < 3; j++)
                { // for each dof of that coordinate
                    if (min(j) > temp_coords[i*3+j])
                    {
                        min(j) = temp_coords[i*3+j];
                    }

                    if (max(j) < temp_coords[i*3+j])
                    {
                        max(j) = temp_coords[i*3+j];
                    }
                }
            }
        }

        // use the coordinates to initialise a space
        max -= min;
        Spatial_Design::MS_Space temp_space;

        temp_space.ID = 1;//v.second.get<std::string>("<xmlattr>.id");
        temp_space.width = max(0);
        temp_space.depth = max(1);
        temp_space.height = max(2);
        temp_space.x = min(0);
        temp_space.y = min(1);
        temp_space.z = min(2);
        temp_space.surfaces_given = false;

        // add the space to the MS building design model
        MS.add_space(temp_space);
    }

} // init_MS_building()

void XML_Model::write_xml_file(std::string file_name)
{
    std::ofstream os(file_name.c_str());
    boost::property_tree::write_xml(os, m_pt, boost::property_tree::xml_writer_settings<std::string>(' ', 2));
    os.close();
} //write_xml()

void XML_Model::read_xml_file(std::string file_name)
{
    m_pt.clear();
    std::ifstream is(file_name.c_str());
    boost::property_tree::read_xml(is, m_pt);
    is.close();

} // read_xml()

} // namespace XML
} // namespace BSO

#endif // XML_MODEL_CPP
