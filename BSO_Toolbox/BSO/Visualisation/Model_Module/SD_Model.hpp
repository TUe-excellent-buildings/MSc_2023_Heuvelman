#ifndef SD_MODEL_HPP
#define SD_MODEL_HPP

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

    class SD_Model : public model
    {
    public:
        SD_Model(Structural_Design::SD_Analysis&, int vis_switch);
        SD_Model(Structural_Design::SD_Analysis&, int vis_switch, bool ghosty);
        ~SD_Model();
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


    SD_Model::SD_Model(Structural_Design::SD_Analysis& SDA, int vis_switch) : SD_Model(SDA, vis_switch, false)
    {
        // default ghosty setting
    }

    SD_Model::SD_Model(Structural_Design::SD_Analysis& SDA, int vis_switch, bool ghosty)
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



        if (vis_switch == 2)
        { // vis meshed SD model
            Structural_Design::FEA* FEA_ptr = SDA.get_FEA_ptr();

            for (unsigned int i = 0; i < FEA_ptr->get_element_count(); i ++)
            {
                Structural_Design::Elements::Element* element_ptr = FEA_ptr->get_element_ptr(i);
                if (!ghosty && !element_ptr->is_active_in_compliance()) continue;
                if (element_ptr->visualise())
                {
                    std::vector<Eigen::Vector3d> coords;
                    std::vector<vertex> vis_coords;

                    if (element_ptr->is_flat_shell())
                    { // grey
                        coords = element_ptr->get_vis_coords();

                        for (unsigned int j = 0; j < coords.size(); j++)
                        {
                            vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                        }

                        polygon_props* aprop_ptr = (element_ptr->get_vis_transparancy()&&ghosty) ? &shellprops_t : &shellprops;
                        line_props* lprop_ptr = (element_ptr->get_vis_transparancy()&&ghosty) ? &lprops_t : &lprops;
                        create_area(polygons, aprop_ptr, lprop_ptr,
                                    vis_coords[0], vis_coords[1],
                                    vis_coords[2], vis_coords[3]);
                    }
                    else
                    {
                        coords = element_ptr->get_vis_coords();

                        double thickness = 0;
                        if (element_ptr->is_beam())
                            thickness = element_ptr->get_property(1);
                        else
                            thickness = std::sqrt(element_ptr->get_property(1));
                        if (thickness == 0)
                            std::cerr << "warning visualising a beam or truss with a thickness of \"0\"" << std::endl;

                        for (unsigned int j = 0; j < coords.size(); j++)
                        {
                            vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                        }

						polygon_props* cprop_ptr;
						line_props* lprop_ptr;

						if (element_ptr->is_beam())
                        {
							cprop_ptr = (element_ptr->get_vis_transparancy()&&ghosty) ? &colprops_t : &colprops;
							lprop_ptr = (element_ptr->get_vis_transparancy()&&ghosty) ? &lprops_t : &lprops;
						}
                        else
						{
							cprop_ptr = (element_ptr->get_vis_transparancy()&&ghosty) ? &trussprops_t : &trussprops;
							lprop_ptr = (element_ptr->get_vis_transparancy()&&ghosty) ? &lprops_t : &lprops;

						}

                        create_column_Juan(polygons, cprop_ptr,  lprop_ptr,
                                           vis_coords[0], vis_coords[1], thickness);
                    }
                    coords.clear();
                    vis_coords.clear();
                }
            }
        }
        else if (vis_switch == 1)
        { // vis unmeshed SD model
            for (unsigned int i = 0 ; i < SDA.get_component_count(); i++)
            {
                Structural_Design::Components::Component* component_ptr = SDA.get_component_ptr(i);
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
        else if (vis_switch == 3)
        { // vis optimised densities
            Structural_Design::FEA* FEA_ptr = SDA.get_FEA_ptr();

            std::vector<double> clusters = SDA.get_element_clusters();

            for (unsigned int i = 0; i < clusters.size(); i++)
            {
                // initialise variables that handle the gradient of the colormap (just try it :) )
                double beta = 10;
                double eta = 0.62;

                // color gradient: on scale of 0-1 the colorband obtained by given beta and eta
                double color_gradient = i/((double)(clusters.size()-1.0));

                // compute the color values for this color gradient
                double red = (tanh(beta * eta) + tanh(beta * (color_gradient - eta))) /
                             (tanh(beta * eta) + tanh(beta * (1 - eta)));
                double green = pow((0.5 + 0.5*cos(2*PI*color_gradient - PI)), (eta/3));
                double blue = 1 - ((tanh(beta * (1 - eta)) + tanh(beta * (color_gradient - (1 - eta)))) /
                                   (tanh(beta * (1 - eta)) + tanh(beta * eta)));
                double alpha;

                if (i < 2)
                {
                    alpha = 0.01;
                }
                else
                {
                    alpha = 0.9;
                }

                // assign the color values to the graphic properties structure
                polygon_props temp_props;
                temp_props.ambient = rgba(red,green,blue,alpha);
                temp_props.diffuse = rgba(red,green,blue,alpha);
                temp_props.specular = rgba(0,0,0,alpha);
                temp_props.shininess = 0.1;
                temp_props.translucent = true;
                temp_props.twosided = true;

                line_props temp_lprops;
                temp_lprops.color = rgba(0.1,0.1,0.1,alpha);

                // add the graphic properties to this clusters index
                cluster_props.push_back(temp_props);
                cluster_lprops.push_back(temp_lprops);
            }

            for (unsigned int i = 0; i < FEA_ptr->get_element_count(); i ++)
            {
                std::vector<Eigen::Vector3d> coords;
                std::vector<vertex> vis_coords;

                Structural_Design::Elements::Element* element_ptr = FEA_ptr->get_element_ptr(i);

                if (!element_ptr->is_active_in_compliance())
                    continue;

                if (element_ptr->is_flat_shell())
                {
                    coords = element_ptr->get_vis_coords();

                    for (unsigned int j = 0; j < coords.size(); j++)
                    {
                        vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                    }

                    double density = element_ptr->get_density();
                    unsigned int cluster_index = 0;

                    for (unsigned int j = 0; j < clusters.size(); j++)
                    {
                        if (density >= clusters[j])
                        {
                            cluster_index = j;
                        }
                        else
                        {
                            break;
                        }
                    }

                    create_area(polygons, &cluster_props[cluster_index], &cluster_lprops[cluster_index],
                                vis_coords[0], vis_coords[1],
                                vis_coords[2], vis_coords[3]);

                }
                else
                {
                    coords = element_ptr->get_vis_coords();

                    for (unsigned int j = 0; j < coords.size(); j++)
                    {
                        vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                    }

                    double thickness = 0;
                    if (element_ptr->is_beam())
                        thickness = element_ptr->get_property(1);
                    else
                        thickness = std::sqrt(element_ptr->get_property(1));
                    if (thickness == 0)
                        std::cerr << "warning visualising a beam or truss with a thickness of \"0\"" << std::endl;


                    double density = element_ptr->get_density();
                    unsigned int cluster_index = 0;

                    for (unsigned int j = 0; j < clusters.size(); j++)
                    {
                        if (density >= clusters[j])
                        {
                            cluster_index = j;
                        }
                        else
                        {
                            break;
                        }
                    }

                    create_column_Juan(polygons, &cluster_props[cluster_index],  &lprops,
                                       vis_coords[0], vis_coords[1], thickness);
                }
                coords.clear();
                vis_coords.clear();
            }
        }
        else if (vis_switch == 4)
        { // shows strain energies

            Structural_Design::FEA* FEA_ptr = SDA.get_FEA_ptr();
            double element_energy = FEA_ptr->get_element_ptr(0)->get_energy();
            double element_volume = FEA_ptr->get_element_ptr(0)->get_volume();
            double max = element_energy/element_volume;
            double min = element_energy/element_volume;

            for (unsigned int i = 0; i < FEA_ptr->get_element_count(); i++)
            {
                Structural_Design::Elements::Element* element_ptr = FEA_ptr->get_element_ptr(i);
                if (!element_ptr->is_active_in_compliance()) continue;
                element_energy = element_ptr->get_energy();
                element_volume = element_ptr->get_volume();
                double energy_per_vol = element_energy / element_volume;


                if (max < energy_per_vol)
                {
                    max = energy_per_vol;
                }
                else if (min > energy_per_vol)
                {
                    min = energy_per_vol;
                }
            }

            std::vector<double> clusters(8);
            clusters[0] = min;
            clusters[7] = max;

            for (unsigned int i = 1; i < 8; i++)
            {
                clusters[i] = min + (max-min)*(i/8.0);
            }

            for (unsigned int i = 0; i < clusters.size(); i++)
            {
                // initialise variables that handle the gradient of the colormap (just try it :) )
                double beta = 10;
                double eta = 0.62;

                // color gradient: on scale of 0-1 the colorband obtained by given beta and eta
                double color_gradient = i/((double)(clusters.size()-1.0));

                // compute the color values for this color gradient
                double red = (tanh(beta * eta) + tanh(beta * (color_gradient - eta))) /
                             (tanh(beta * eta) + tanh(beta * (1 - eta)));
                double green = pow((0.5 + 0.5*cos(2*PI*color_gradient - PI)), (eta/3));
                double blue = 1 - ((tanh(beta * (1 - eta)) + tanh(beta * (color_gradient - (1 - eta)))) /
                                   (tanh(beta * (1 - eta)) + tanh(beta * eta)));
                double alpha;

                //if (i < 2)
                {
                    alpha = 0.1;
                }/*
                else
                {
                    alpha = 0.9;
                }*/

                // assign the color values to the graphic properties structure
                polygon_props temp_props;
                temp_props.ambient = rgba(red,green,blue,alpha);
                temp_props.diffuse = rgba(red,green,blue,alpha);
                temp_props.specular = rgba(0,0,0,alpha);
                temp_props.shininess = 0.1;
                temp_props.translucent = false;
                temp_props.twosided = true;

                line_props temp_lprops;
                temp_lprops.color = rgba(0.1,0.1,0.1,alpha);

                // add the graphic properties to this clusters index
                cluster_props.push_back(temp_props);
                cluster_lprops.push_back(temp_lprops);
            }

            for (unsigned int i = 0; i < FEA_ptr->get_element_count(); i ++)
            {
                std::vector<Eigen::Vector3d> coords;
                std::vector<vertex> vis_coords;

                Structural_Design::Elements::Element* element_ptr = FEA_ptr->get_element_ptr(i);
                if (!element_ptr->is_active_in_compliance()) continue;

                if (element_ptr->is_flat_shell())
                {
                    coords = element_ptr->get_vis_coords();

                    for (unsigned int j = 0; j < coords.size(); j++)
                    {
                        vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                    }

                    element_energy = element_ptr->get_energy();
                    element_volume = element_ptr->get_volume();
                    unsigned int cluster_index = 0;

                    for (unsigned int j = 0; j < clusters.size(); j++)
                    {
                        if (element_energy/element_volume >= clusters[j])
                        {
                            cluster_index = j;
                        }
                        else
                        {
                            break;
                        }
                    }

                    create_area(polygons, &cluster_props[cluster_index], &cluster_lprops[cluster_index],
                                vis_coords[0], vis_coords[1],
                                vis_coords[2], vis_coords[3]);

                }
                else
                {
                    coords = element_ptr->get_vis_coords();

                    for (unsigned int j = 0; j < coords.size(); j++)
                    {
                        vis_coords.push_back(vertex(coords[j](0),coords[j](2),-coords[j](1)));
                    }

                    double thickness = 0;
                    if (element_ptr->is_beam())
                        thickness = element_ptr->get_property(1);
                    else
                        thickness = std::sqrt(element_ptr->get_property(1));
                    if (thickness == 0)
                        std::cerr << "warning visualising a beam or truss with a thickness of \"0\"" << std::endl;

                    element_energy = element_ptr->get_energy();
                    element_volume = element_ptr->get_volume();
                    unsigned int cluster_index = 0;

                    for (unsigned int j = 0; j < clusters.size(); j++)
                    {
                        if (element_energy/element_volume >= clusters[j])
                        {
                            cluster_index = j;
                        }
                        else
                        {
                            break;
                        }
                    }

                    create_column_Juan(polygons, &cluster_props[cluster_index],  &cluster_lprops[cluster_index],
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

    SD_Model::~SD_Model()
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

    const std::string SD_Model::get_description()
    {
        return std::string("//Structural Design        Space: Next Step");
    }

    void SD_Model::render(const camera &cam) const
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

    bool SD_Model::key_pressed(int key)
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


#endif // SD_MODEL_HPP
