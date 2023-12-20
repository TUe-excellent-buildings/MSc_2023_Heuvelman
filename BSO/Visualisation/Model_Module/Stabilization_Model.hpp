#ifndef STABILIZATION_MODEL_HPP
#define STABILIZATION_MODEL_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>
#include <BSO/Visualisation/Model_Module/Model.hpp>
#include <BSO/Visualisation/BSP_Module/BSP.hpp>

#include <cstdlib>
#include <cmath>
#include <algorithm>

namespace BSO { namespace Visualisation
{

    class Stabilization_Model : public model
    {
    public:
        Stabilization_Model(Structural_Design::SD_Analysis_Vars*&, int vis_switch);
        Stabilization_Model(Structural_Design::SD_Analysis_Vars*&, int vis_switch, bool ghosty);
        ~Stabilization_Model();
        void render(const camera &cam) const;
        const std::string get_description();

        bool key_pressed(int key);

    protected:

    private:
        std::list<polygon*> polygons;

        polygon_props  colprops, trussprops, shellprops, colprops_t, trussprops_t, shellprops_t;
        std::vector<polygon_props> cluster_props;
        std::vector<line_props> cluster_lprops;

        line_props     lprops, lprops_t;
//        label_props    lbprops;
        random_bsp     *pbsp;
    };





    // Implementation of member functions:

    // hh, below is the definition of the constructor of setofrooms_model as defined in setofrooms_model.h
    // hh, see datatypes.h for usage of pprops:
    // hh, rgba ambient,diffuse,specular, emission;
    // hh, float shininess;
    // hh, bool translucent, wosided;


    Stabilization_Model::Stabilization_Model(Structural_Design::SD_Analysis_Vars*& SDA, int vis_switch) : Stabilization_Model(SDA, vis_switch, false)
    {
        // default ghosty setting
    }

    Stabilization_Model::Stabilization_Model(Structural_Design::SD_Analysis_Vars*& SDA, int vis_switch, bool ghosty)
    {
		lprops_t = rgba(0.0,0.0,0.0,0.1);

        colprops.ambient = rgba(1.0, 0.30, 0.30, 0.15);
        colprops.diffuse = rgba(1.0, 0.30, 0.30, 0.15);
        colprops.specular = rgba(0.1,0.1,0.1,1);
        colprops.shininess = 60;
        colprops.translucent = false;
        colprops.twosided = true;

		trussprops.ambient = rgba(0.30, 0.30, 1.0, 0.15);
        trussprops.diffuse = rgba(0.30, 0.30, 1.0, 0.15);
        trussprops.specular = rgba(0.1,0.1,0.1,1);
        trussprops.shininess = 60;
        trussprops.translucent = false;
        trussprops.twosided = true;

        shellprops.ambient = rgba(0.65, 0.65, 0.65, 0.15);
        shellprops.diffuse = rgba(0.65, 0.65, 0.65, 0.15);
        shellprops.specular = rgba(0.1,0.1,0.1,1);
        shellprops.shininess = 60;
        shellprops.translucent = false;
        shellprops.twosided = true;

		colprops_t = colprops;
		trussprops_t = trussprops;
		shellprops_t = shellprops;
		colprops_t.translucent = true;
		trussprops_t.translucent = true;
		shellprops_t.translucent = true;



        if (vis_switch == 1)
        { // vis unmeshed SD model
            for (unsigned int i = 0 ; i < SDA->get_component_count(); i++)
            {
                Structural_Design::Components::Component* component_ptr = SDA->get_component_ptr(i);
                if (!ghosty && component_ptr->is_ghost_component()) continue;
                std::vector<Eigen::Vector3d> coords;
                std::vector<vertex> vis_coords;

                if (component_ptr->is_flat_shell())
                { // if the component is a flat shell
                    coords = component_ptr->get_vis_points();

                    for (unsigned int j = 0; j < coords.size(); j++)
                    {
                        vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                    }

                    polygon_props* aprop_ptr = (component_ptr->get_vis_transparancy()&&ghosty) ? &shellprops_t : &shellprops;
                    line_props* lprop_ptr = (component_ptr->get_vis_transparancy()&&ghosty) ? &lprops_t : &lprops;
                    create_area(polygons, aprop_ptr, lprop_ptr,
                                vis_coords[0], vis_coords[1],
                                vis_coords[2], vis_coords[3]);
                }
                else
                { // else the component must be either a truss or beam
                    coords = component_ptr->get_vis_points();

                    double thickness = 0;
                    if (component_ptr->is_beam())
                        thickness = component_ptr->get_property(0);
                    else if (component_ptr->is_truss())
                        thickness = std::sqrt(component_ptr->get_property(0));
                    else
                        continue;
                    if (thickness == 0)
                        std::cerr << "warning visualising a beam or truss with a thickness of \"0\"" << std::endl;

                    for (unsigned int j = 0; j < coords.size(); j++)
                    {
                        vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                    }

                    polygon_props* cprop_ptr;
                    line_props* lprop_ptr;

					if (component_ptr->is_beam())
                    {
						cprop_ptr = (component_ptr->get_vis_transparancy()&&ghosty) ? &colprops_t : &colprops;
						lprop_ptr = (component_ptr->get_vis_transparancy()&&ghosty) ? &lprops_t : &lprops;
					}
                    else
					{
						cprop_ptr = (component_ptr->get_vis_transparancy()&&ghosty) ? &trussprops_t : &trussprops;
						lprop_ptr = (component_ptr->get_vis_transparancy()&&ghosty) ? &lprops_t : &lprops;
					}

                    create_column_Juan(polygons, cprop_ptr,  lprop_ptr,
                                       vis_coords[0], vis_coords[1], thickness);
                }
                coords.clear();
                vis_coords.clear();
            }
        }

        else
        {
            std::cout << "Unknown visualisation settings for structural design, exiting now..." << std::endl;
            exit(1);
        }

        pbsp = new random_bsp(polygons);
    }

    Stabilization_Model::~Stabilization_Model()
    {
        delete pbsp;

        for (std::list<polygon*>::iterator pit = polygons.begin();
             pit != polygons.end(); pit++)
            delete *pit;
/*
        for (std::list<label*>::iterator lbit = labels.begin();
             lbit != labels.end(); lbit++)
            delete *lbit;
*/
    }

    const std::string Stabilization_Model::get_description()
    {
        return std::string("//Structural Design        Space: Next Step");
    }

    void Stabilization_Model::render(const camera &cam) const
    {
        glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);

        pbsp->render_btf(cam);
/*
        std::list<label*>::const_iterator lbit;
        for (lbit = labels.begin(); lbit != labels.end(); lbit++)
            (*lbit)->render();
*/
        glPopAttrib();
    }

    bool Stabilization_Model::key_pressed(int key)
    {
        switch (key)
        {
        case 't':
        case 'T':
            //toggle geometry translucency
            colprops.translucent = !colprops.translucent;
            trussprops.translucent = !trussprops.translucent;
            shellprops.translucent = !shellprops.translucent;

			if (colprops.translucent)
			{
				colprops_t.ambient = rgba(1.0, 0.30, 0.30, 0.0);
				colprops_t.diffuse = rgba(1.0, 0.30, 0.30, 0.0);
				trussprops_t.ambient = rgba(0.30, 0.30, 1.0, 0.0);
				trussprops_t.diffuse = rgba(0.30, 0.30, 1.0, 0.0);
				shellprops_t.ambient = rgba(0.65, 0.65, 0.65, 0.0);
				shellprops_t.diffuse = rgba(0.65, 0.65, 0.65, 0.0);
				lprops_t = rgba(1.0,1.0,1.0,0.0);
			}
			else
			{
				colprops_t.ambient = rgba(1.0, 0.30, 0.30, 0.15);
				colprops_t.diffuse = rgba(1.0, 0.30, 0.30, 0.15);
				trussprops_t.ambient = rgba(0.30, 0.30, 1.0, 0.15);
				trussprops_t.diffuse = rgba(0.30, 0.30, 1.0, 0.15);
				shellprops_t.ambient = rgba(0.65, 0.65, 0.65, 0.15);
				shellprops_t.diffuse = rgba(0.65, 0.65, 0.65, 0.15);
				lprops_t = rgba(1.0,1.0,1.0,0.15);
			}


            for (unsigned int i= 0; i < cluster_props.size(); i++)
            {
                cluster_props[i].translucent = !cluster_props[i].translucent;
            }

            return true;
        }

	return false;
    }

} // namespace Visualisation
} // namespace BSO


#endif // STABILIZATION_MODEL_HPP
