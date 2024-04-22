#ifndef ZONING_MODEL_HPP
#define ZONING_MODEL_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI
#include <vector>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Visualisation/Model_Module/Model.hpp>
#include <BSO/Visualisation/BSP_Module/BSP.hpp>
#include <BSO/Spatial_Design/Zoning.hpp>
#include <BSO/Spatial_Design/Zoning/Zone.hpp>

#include <cstdlib>

namespace BSO { namespace Visualisation
{

    class Zoning_Model : public model
    {
    public:
        Zoning_Model(Spatial_Design::MS_Conformal&, std::string, unsigned int, std::vector<int>);
        Zoning_Model(Spatial_Design::MS_Conformal& ms_conf, unsigned int i, int zone_ID_chosen);
        ~Zoning_Model();
        void render(const camera &cam) const;
        const std::string get_description();

        bool key_pressed(int key);

    protected:

    private:
        std::list<polygon*> polygons;
        std::list<label*>   labels;

        polygon_props  pprops_rectangle;
        polygon_props  pprops_cuboid;
        polygon_props  pprops_cuboid_overlap;

        line_props     lprops;
        label_props    lbprops;
        random_bsp     *pbsp;

        std::vector<polygon_props> cluster_props;
        std::vector<polygon_props> cluster_props2;
        std::vector<line_props> cluster_lprops;

        unsigned int design_ID = 1;
        unsigned int zones_ID = 1;
        bool is_specific_zone = false;
    };

    // Implementation of member functions:

    // hh, below is the definition of the constructor of setofrooms_model as defined in setofrooms_model.h
    // hh, see datatypes.h for usage of pprops:
    // hh, rgba ambient,diffuse,specular, emission;
    // hh, float shininess;
    // hh, bool translucent, wosided;

    Zoning_Model::Zoning_Model(Spatial_Design::MS_Conformal& ms_conf, std::string type, unsigned int i, std::vector<int> ZoneIDs2)
    {
        if (type == "zones")
        {
            design_ID += i;
            is_specific_zone = false;
            double offset = 200;


            vertex max, min;
            Vectors::Point temp_coords_1, temp_coords_2;

            
            // store zone ID's for current design (i) in vector
            std::vector<unsigned int> zone_IDs;
            //std::cout << "zoneIDs passed to the visualization model:" << zoneIDs.size() << std::endl;
            
            for (unsigned int k = 0; k < ms_conf.get_cuboid_count(); k++)
            {
                unsigned int zone_ID = ms_conf.get_cuboid(k)->get_zone_ID(i);
                //std::cout << "Cuboid " << k << ": Zone ID = " << zone_ID << std::endl;
                if (std::find(zone_IDs.begin(), zone_IDs.end(), zone_ID) == zone_IDs.end())
                {
                    zone_IDs.push_back(zone_ID);
                }
            }
            std::sort(zone_IDs.begin(), zone_IDs.end());
            
            for (unsigned int k = 0; k < ms_conf.get_cuboid_count(); k++)
            {
                unsigned int zone_ID = ms_conf.get_cuboid(k)->get_all_zones().back();
                //std::cout << "Cuboid " << k << ": Zone ID = " << zone_ID << std::endl;
            }
            
            /*
            // to show zoned design with all latest created zones
            std::vector<unsigned int> zone_IDs;
            for (unsigned int k = 0; k < ms_conf.get_cuboid_count(); k++)
            {
                unsigned int zone_ID = ms_conf.get_cuboid(k)->get_all_zones().back();
                std::cout << "Cuboid " << k << ": Zone ID = " << zone_ID << std::endl;
                if (std::find(zone_IDs.begin(), zone_IDs.end(), zone_ID) == zone_IDs.end())
                {
                    zone_IDs.push_back(zone_ID);
                }
            }
            std::sort(zone_IDs.begin(), zone_IDs.end());
            */

            /*
            for (unsigned int k = 0; k < ms_conf.get_cuboid_count(); k++)
            {
                auto all_zone_IDs = ms_conf.get_cuboid(k)->get_all_zones();  // Retrieve all zone IDs for the cuboid at index k
                std::cout << "Cuboid " << k << " is associated with zone IDs: ";
                for (auto zone_ID : all_zone_IDs) {  // Iterate over each zone ID
                    std::cout << zone_ID << " ";
                }
                std::cout << std::endl;  // Print newline for better readability between cuboid entries
            }
            */

            for (unsigned int k = 0; k < zone_IDs.size(); k++)
            {
                // initialise variables that handle the gradient of the colormap (just try it :) )
                double beta = 10;
                double eta = 0.62;

                // color gradient: on scale of 0-1 the colorband obtained by given beta and eta
                double color_gradient = k/((double)(zone_IDs.size()-1.0));

                // compute the color values for this color gradient
                double red = (tanh(beta * eta) + tanh(beta * (color_gradient - eta))) /
                (tanh(beta * eta) + tanh(beta * (1 - eta)));
                double green = pow((0.5 + 0.5*cos(2*PI*color_gradient - PI)), (eta/3));
                double blue = 1 - ((tanh(beta * (1 - eta)) + tanh(beta * (color_gradient - (1 - eta)))) /
                                   (tanh(beta * (1 - eta)) + tanh(beta * eta)));
                double alpha = 0.25;


                // assign the color values to the graphic properties structure
                polygon_props temp_props;
                temp_props.ambient = rgba(red,green,blue,alpha);
                temp_props.diffuse = rgba(red,green,blue,alpha);
                temp_props.specular = rgba(0,0,0,alpha);
                temp_props.shininess = 0.1;
                temp_props.translucent = true;
                temp_props.twosided = false;

                line_props temp_lprops;
                temp_lprops.color = rgba(0.1,0.1,0.1,alpha);

                // add the graphic properties to this clusters index
                cluster_props.push_back(temp_props);
                cluster_lprops.push_back(temp_lprops);
            }

            for (unsigned int k = 0; k < ms_conf.get_cuboid_count(); k++)
            {
                unsigned int zone_ID = ms_conf.get_cuboid(k)->get_zone_ID(i);
                ptrdiff_t pos = std::find(zone_IDs.begin(), zone_IDs.end(), zone_ID) - zone_IDs.begin();

                /*
                // to show zoned design with all latest created zones
                unsigned int zone_ID = ms_conf.get_cuboid(k)->get_all_zones().back();  // Retrieve all zone IDs for the cuboid at index k
                ptrdiff_t pos = std::find(zone_IDs.begin(), zone_IDs.end(), zone_ID) - zone_IDs.begin();
                */

                temp_coords_1 = ms_conf.get_cuboid(k)->get_max_vertex()->get_coords();
                temp_coords_2 = ms_conf.get_cuboid(k)->get_min_vertex()->get_coords();

                max = vect3d(temp_coords_1(0)-offset, temp_coords_1(2)-offset, -temp_coords_2(1)-offset);
                min = vect3d(temp_coords_2(0)+offset, temp_coords_2(2)+offset, -temp_coords_1(1)+offset);
                add_cube(&cluster_props[pos], &cluster_lprops[pos], min, max, polygons);

                std::ostringstream out; out << zone_ID; std::string ID = out.str(); // cast the int value of ID as a string
                labels.push_back(create_label(&lbprops, ID, min + ((max-min)/2.0)));

            }
        }
        else
        {
            std::cout << "Error in visualisation of conformal set of spaces, exiting..." << std::endl;
            exit(1);
        }



        pbsp = new random_bsp(polygons);

    }

    Zoning_Model::Zoning_Model(Spatial_Design::MS_Conformal& ms_conf, unsigned int i, int zone_ID_chosen)
    {
        zones_ID += i-6;
        is_specific_zone = true;
        double offset = 200;


        vertex max, min;
        Vectors::Point temp_coords_1, temp_coords_2;

        // store zone ID's for current design (i) in vector
        std::vector<unsigned int> zone_IDs;
        zone_IDs.push_back(zone_ID_chosen);


        for (unsigned int k = 0; k < 2; k++)
        {
            // initialise variables that handle the gradient of the colormap (just try it :) )
            double beta = 10;
            double eta = 0.62;

            // color gradient: on scale of 0-1 the colorband obtained by given beta and eta
            double color_gradient = k/((double)(zone_IDs.size()-1.0));

            // compute the color values for this color gradient
            double red = (tanh(beta * eta) + tanh(beta * (color_gradient - eta))) /
            (tanh(beta * eta) + tanh(beta * (1 - eta)));
            double green = pow((0.5 + 0.5*cos(2*PI*color_gradient - PI)), (eta/3));
            double blue = 1 - ((tanh(beta * (1 - eta)) + tanh(beta * (color_gradient - (1 - eta)))) /
                                (tanh(beta * (1 - eta)) + tanh(beta * eta)));
            double alpha = 0.1;
            double alpha2 = 1.0;

            // assign the color values to the graphic properties structure
            polygon_props temp_props;
            temp_props.ambient = rgba(red,green,blue,alpha);
            temp_props.diffuse = rgba(red,green,blue,alpha);
            temp_props.specular = rgba(0,0,0,alpha);
            temp_props.shininess = 0.1;
            temp_props.translucent = true;
            temp_props.twosided = false;

            line_props temp_lprops;
            temp_lprops.color = rgba(0.1,0.1,0.1,alpha);

            // add the graphic properties to this clusters index
            cluster_props.push_back(temp_props);
            cluster_lprops.push_back(temp_lprops);

            polygon_props temp_props2;
            temp_props2.ambient = rgba(red,green,blue,alpha2);
            temp_props2.diffuse = rgba(red,green,blue,alpha2);
            temp_props2.specular = rgba(0,0,0,alpha2);
            temp_props2.shininess = 0.1;
            temp_props2.translucent = true;
            temp_props2.twosided = false;

            cluster_props2.push_back(temp_props2);
        }

        unsigned int highest_zone_ID = 0;
        labels.clear();
        for (unsigned int k = 0; k < ms_conf.get_cuboid_count(); k++)
        {
            //unsigned int zone_ID = ms_conf.get_cuboid(k)->get_last_zone();
            //unsigned int zone_ID = ms_conf.get_cuboid(k)->get_all_zones();
            auto all_zone_ID = ms_conf.get_cuboid(k)->get_all_zones();  // This now returns a vector of all zone IDs
            for (auto zone_ID : all_zone_ID) {  // Iterate over each zone ID
                //std::cout << "zone_ID used for visual: " << zone_ID << std::endl;

                if (zone_ID > highest_zone_ID) {
                    highest_zone_ID = zone_ID;  // Update if the current zone_ID is greater
                }
                //std::cout << "Highest zone_ID encountered: " << highest_zone_ID << std::endl;

                temp_coords_1 = ms_conf.get_cuboid(k)->get_max_vertex()->get_coords();
                temp_coords_2 = ms_conf.get_cuboid(k)->get_min_vertex()->get_coords();

                max = vect3d(temp_coords_1(0) - offset, temp_coords_1(2) - offset, -temp_coords_2(1) - offset);
                min = vect3d(temp_coords_2(0) + offset, temp_coords_2(2) + offset, -temp_coords_1(1) + offset);

                /*
                //if(zone_ID != highest_zone_ID)
                //if (zone_ID != >get_zones().size() + 1)
                if (zone_ID != 10 + zones_ID)
                {
                    add_cube(&cluster_props[0], &cluster_lprops[0], min, max, polygons);
                } else {
                    add_cube(&cluster_props[1], &cluster_lprops[1], min, max, polygons);
                }
                */
                //std::cout << "zones_ID " << zones_ID << std::endl;
                if (zone_ID == 10 + zones_ID)
                {
                    add_cube(&cluster_props2[1], &cluster_lprops[0], min, max, polygons);
                    std::ostringstream out; out << zone_ID; std::string ID = out.str(); // cast the int value of ID as a string
                    labels.push_back(create_label(&lbprops, ID, min + ((max - min) / 2.0)));
                }
                //if (zone_ID <= 10)
                if (zone_ID == 2 | zone_ID == 6 | zone_ID == 7 | zone_ID == 10)
                {
                    add_cube(&cluster_props[0], &cluster_lprops[1], min, max, polygons);
                    std::ostringstream out; out << zone_ID; std::string ID = out.str(); // cast the int value of ID as a string
                    //labels.push_back(create_label(&lbprops, ID, min + ((max - min) / 2.0)));
                }
            }

        }
        pbsp = new random_bsp(polygons);

    }
    Zoning_Model::~Zoning_Model()
    {
        delete pbsp;

        for (std::list<polygon*>::iterator pit = polygons.begin();
             pit != polygons.end(); pit++)
            delete *pit;

        for (std::list<label*>::iterator lbit = labels.begin();
             lbit != labels.end(); lbit++)
            delete *lbit;
    }

    const std::string Zoning_Model::get_description()
    {
        
        if (is_specific_zone) {
            return std::string("Zone " + std::to_string(zones_ID));
        }
        else {
            return std::string("Zoned Design " + std::to_string(design_ID));
        }
    }

    void Zoning_Model::render(const camera &cam) const
    {
        glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);

        pbsp->render_btf(cam);

        std::list<label*>::const_iterator lbit;
        for (lbit = labels.begin(); lbit != labels.end(); lbit++)
            (*lbit)->render();

        glPopAttrib();
    }

    bool Zoning_Model::key_pressed(int key)
    {
        switch (key)
        {
        case 't':
        case 'T':
            //toggle geometry translucency
            pprops_rectangle.translucent = !pprops_rectangle.translucent;
            pprops_cuboid.translucent = !pprops_cuboid.translucent;

            return true;
        }

	return false;
    }

} // namespace Visualisation
} // namespace BSO


#endif // ZONING_MODEL_HPP
