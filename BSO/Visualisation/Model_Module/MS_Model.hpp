#ifndef MS_MODEL_HPP
#define MS_MODEL_HPP


#include <BSO/Visualisation/Model_Module/Model.hpp>
#include <BSO/Visualisation/BSP_Module/BSP.hpp>
#include <BSO/Spatial_Design/Movable_Sizable.hpp>

#ifdef SD_ANALYSIS_HPP
#include <BSO/Structural_Design/SD_Analysis.hpp>
#endif // SD_ANALYSIS_HPP

#ifdef BP_SIMULATION_HPP
#include <BSO/Building_Physics/BP_Results.hpp>
#endif // BP_SIMULATION_HPP


namespace BSO { namespace Visualisation
{

    class MS_Model : public model
    {
    public:
        MS_Model(Spatial_Design::MS_Building&);
        MS_Model(Spatial_Design::MS_Building&, std::string type); // NEW

        #ifdef BP_SIMULATION_HPP
        MS_Model(Spatial_Design::MS_Building&, Building_Physics::BP_Building_Results&);
        #endif // BP_SIMULATION_HPP

        #ifdef SD_ANALYSIS_HPP
        MS_Model(Spatial_Design::MS_Building& , Structural_Design::SD_Building_Results&);
        #endif // SD_ANALYSIS_HPP

        ~MS_Model();
        void render(const camera &cam) const;
        const std::string get_description();

        // Method to draw a line between two points, used for the GUI JH
        void drawLine(float x, float y, float z, float x1, float y1, float z1) const {
            glLineWidth(2.0);
            glColor3f(1.0, 0.0, 0.0);
            glBegin(GL_LINES);
            glVertex3f(x, y, z);
            glVertex3f(x1, y1, z1);
            glEnd();
        }

    private:
        std::list<polygon*> polygons;
        std::list<label*>   labels;

        polygon_props  pprops;
        polygon_props  pprops_1, pprops_2, pprops_3, pprops_4, pprops_5;
        polygon_props  pprops_surface_type; // NEW

        line_props     lprops;
        label_props    lbprops;
        random_bsp     *pbsp;
    }; // MS_Model




    // Implementation of member functions:

    // hh, below is the definition of the constructor of setofrooms_model as defined in setofrooms_model.h
    // hh, see datatypes.h for usage of pprops:
    // hh, rgba ambient,diffuse,specular, emission;
    // hh, float shininess;
    // hh, bool translucent, wosided;

    MS_Model::MS_Model(Spatial_Design::MS_Building& MS) : MS_Model(MS, "spaces")
    {

    } // ctor

    MS_Model::MS_Model(Spatial_Design::MS_Building& MS, std::string type) //NEW
    {
        pprops.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
        pprops.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
        pprops.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
        pprops.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
        pprops.shininess = 60.0f;
        pprops.translucent = true;
        pprops.twosided = true;

        pprops_surface_type.ambient = rgba(1.0f, 1.0f, 0.04f, 0.4f); //NEW
        pprops_surface_type.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.4f);
        pprops_surface_type.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_surface_type.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_surface_type.shininess = 60.0;
        pprops_surface_type.translucent = true;
        pprops_surface_type.twosided = true;

        vertex max, min;
        vertex surface_center[6];

        double offset = 200;

        if (type == "spaces")
        {
            for(int i = 0; i < MS.obtain_space_count(); i++)
            {
                Spatial_Design::MS_Space temp = MS.obtain_space(i);

                min = vect3d(temp.x,temp.z,-(temp.y+temp.depth));
                max = vect3d(temp.x+temp.width,temp.z+temp.height,-temp.y);

                add_cube(&pprops, &lprops, min, max, polygons);
                std::string ID = std::to_string(temp.ID); // cast the int value of ID as a string
                labels.push_back(create_label(&lbprops, ID,
                                 min + ((max-min)/2.0)));
            }
        }

        else if (type == "surface_type") // NEW
        {
            for(int i = 0; i < MS.obtain_space_count(); i++)
            {
                Spatial_Design::MS_Space temp = MS.obtain_space(i);

                min = vect3d(temp.x+offset,temp.z+offset,-(temp.y+temp.depth-offset));
                max = vect3d(temp.x+temp.width-offset,temp.z+temp.height-offset,-(temp.y+offset));

                add_cube(&pprops_surface_type, &lprops, min, max, polygons);

                surface_center[0] = vect3d(temp.x+(temp.width/2),temp.z+(temp.height/2),-(temp.y+temp.depth-offset)); // north surface_center
                surface_center[1] = vect3d(temp.x+temp.width-offset,temp.z+(temp.height/2),-(temp.y+(temp.depth/2))); // east surface_center
                surface_center[2] = vect3d(temp.x+(temp.width/2),temp.z+(temp.height/2),-(temp.y+offset)); // south surface_center
                surface_center[3] = vect3d(temp.x+offset,temp.z+(temp.height/2),-(temp.y+(temp.depth/2))); // west surface_center
                surface_center[4] = vect3d(temp.x+(temp.width/2),temp.z+temp.height-offset,-(temp.y+(temp.depth/2))); // top surface_center
                surface_center[5] = vect3d(temp.x+(temp.width/2),temp.z+offset,-(temp.y+(temp.depth/2))); // bottom surface_center

                for (unsigned int j = 0; j < 6; j++)
                {
                    std::string surface_type = temp.surface_type[j]; // cast the int value of surface_type as a string
                    labels.push_back(create_label(&lbprops, surface_type, surface_center[j]));
                }
            }
        } // NEW
        else if (type == "space_type")
        {
            for(int i = 0; i < MS.obtain_space_count(); i++)
            {
                Spatial_Design::MS_Space temp = MS.obtain_space(i);

                min = vect3d(temp.x,temp.z,-(temp.y+temp.depth));
                max = vect3d(temp.x+temp.width,temp.z+temp.height,-temp.y);

                add_cube(&pprops_surface_type, &lprops, min, max, polygons);
                std::string type = temp.m_space_type; // cast the int value of ID as a string
                labels.push_back(create_label(&lbprops, type,
                                 min + ((max-min)/2.0)));
            }
        }
        else
        {
            std::cout << "Error in visualisation of MS set of spaces, exiting..." << std::endl;
            exit(1);
        }
        pbsp = new random_bsp(polygons);
    } //NEW

    #ifdef BP_SIMULATION_HPP
    MS_Model::MS_Model(Spatial_Design::MS_Building& MS, Building_Physics::BP_Building_Results& BP)
    {
        pprops_1.ambient = rgba(1.0f, 0.04f, 0.04f, 0.4f);
        pprops_1.diffuse = rgba(1.0f, 0.04f, 0.04f, 0.4f);
        pprops_1.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_1.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_1.shininess = 100.0;
        pprops_1.translucent = true;
        pprops_1.twosided = true;

        pprops_2.ambient = rgba(1.0f, 0.52f, 0.04f, 0.4f);
        pprops_2.diffuse = rgba(1.0f, 0.52f, 0.04f, 0.4f);
        pprops_2.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_2.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_2.shininess = 100.0;
        pprops_2.translucent = true;
        pprops_2.twosided = true;

        pprops_3.ambient = rgba(1.0f, 1.0f, 0.04f, 0.4f);
        pprops_3.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.4f);
        pprops_3.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_3.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_3.shininess = 100.0;
        pprops_3.translucent = true;
        pprops_3.twosided = true;

        pprops_4.ambient = rgba(0.52f, 1.0f, 0.04f, 0.4f);
        pprops_4.diffuse = rgba(0.52f, 1.0f, 0.04f, 0.4f);
        pprops_4.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_4.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_4.shininess = 100.0;
        pprops_4.translucent = true;
        pprops_4.twosided = true;

        pprops_5.ambient = rgba(0.26f, 0.52f, 0.0f, 0.4f);
        pprops_5.diffuse = rgba(0.26f, 0.52f, 0.0f, 0.4f);
        pprops_5.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_5.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_5.shininess = 100.0;
        pprops_5.translucent = true;
        pprops_5.twosided = true;


        vertex max, min;

        for(int i = 0; i < MS.obtain_space_count(); i++)
        {
            Spatial_Design::MS_Space temp = MS.obtain_space(i);

            min = vect3d(temp.x,temp.z,-(temp.y+temp.depth));
            max = vect3d(temp.x+temp.width,temp.z+temp.height,-temp.y);

            std::string ID = std::to_string(temp.ID); // cast the int value of ID as a string

            for(unsigned int j = 0; j < BP.m_space_results.size(); j++)
            {
                if (std::to_string(MS.obtain_space(i).ID) == BP.m_space_results[j].m_space_ID)
                {
                    if (BP.m_space_results[j].m_rel_performance <= 0.2)
                    {
                        add_cube(&pprops_1, &lprops, min, max, polygons);
                    }
                    else if (BP.m_space_results[j].m_rel_performance <= 0.4)
                    {
                        add_cube(&pprops_2, &lprops, min, max, polygons);
                    }
                    else if (BP.m_space_results[j].m_rel_performance <= 0.6)
                    {
                        add_cube(&pprops_3, &lprops, min, max, polygons);
                    }
                    else if (BP.m_space_results[j].m_rel_performance <= 0.8)
                    {
                        add_cube(&pprops_4, &lprops, min, max, polygons);
                    }
                    else if (BP.m_space_results[j].m_rel_performance <= 1.0)
                    {
                        add_cube(&pprops_5, &lprops, min, max, polygons);
                    }
                    break;
                }
            }

            labels.push_back(create_label(&lbprops, ID, min + ((max-min)/2.0)));
        }
        pbsp = new random_bsp(polygons);
    }
    #endif // BP_SIMULATION_HPP

    #ifdef SD_ANALYSIS_HPP
    MS_Model::MS_Model(Spatial_Design::MS_Building& MS, Structural_Design::SD_Building_Results& SD)
    {
        pprops_1.ambient = rgba(1.0f, 0.04f, 0.04f, 0.4f);
        pprops_1.diffuse = rgba(1.0f, 0.04f, 0.04f, 0.4f);
        pprops_1.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_1.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_1.shininess = 100.0;
        pprops_1.translucent = true;
        pprops_1.twosided = true;

        pprops_2.ambient = rgba(1.0f, 0.52f, 0.04f, 0.4f);
        pprops_2.diffuse = rgba(1.0f, 0.52f, 0.04f, 0.4f);
        pprops_2.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_2.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_2.shininess = 100.0;
        pprops_2.translucent = true;
        pprops_2.twosided = true;

        pprops_3.ambient = rgba(1.0f, 1.0f, 0.04f, 0.4f);
        pprops_3.diffuse = rgba(1.0f, 1.0f, 0.04f, 0.4f);
        pprops_3.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_3.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_3.shininess = 100.0;
        pprops_3.translucent = true;
        pprops_3.twosided = true;

        pprops_4.ambient = rgba(0.52f, 1.0f, 0.04f, 0.4f);
        pprops_4.diffuse = rgba(0.52f, 1.0f, 0.04f, 0.4f);
        pprops_4.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_4.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_4.shininess = 100.0;
        pprops_4.translucent = true;
        pprops_4.twosided = true;

        pprops_5.ambient = rgba(0.26f, 0.52f, 0.0f, 0.4f);
        pprops_5.diffuse = rgba(0.26f, 0.52f, 0.0f, 0.4f);
        pprops_5.specular = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_5.emission = rgba(0.04f, 0.04f, 0.04f, 1.0f);
        pprops_5.shininess = 100.0;
        pprops_5.translucent = true;
        pprops_5.twosided = true;


        vertex max, min;

        for(int i = 0; i < MS.obtain_space_count(); i++)
        {
            Spatial_Design::MS_Space temp = MS.obtain_space(i);

            min = vect3d(temp.x,temp.z,-(temp.y+temp.depth));
            max = vect3d(temp.x+temp.width,temp.z+temp.height,-temp.y);

            std::string ID = std::to_string(temp.ID); // cast the int value of ID as a string

            for(unsigned int j = 0; j < SD.m_spaces.size(); j++)
            {
                if (MS.obtain_space(i).ID == SD.m_spaces[j].m_ID)
                {
                    if (SD.m_spaces[j].m_rel_performance <= 0.2)
                    {
                        add_cube(&pprops_1, &lprops, min, max, polygons);
                    }
                    else if (SD.m_spaces[j].m_rel_performance <= 0.4)
                    {
                        add_cube(&pprops_2, &lprops, min, max, polygons);
                    }
                    else if (SD.m_spaces[j].m_rel_performance <= 0.6)
                    {
                        add_cube(&pprops_3, &lprops, min, max, polygons);
                    }
                    else if (SD.m_spaces[j].m_rel_performance <= 0.8)
                    {
                        add_cube(&pprops_4, &lprops, min, max, polygons);
                    }
                    else if (SD.m_spaces[j].m_rel_performance <= 1.0)
                    {
                        add_cube(&pprops_5, &lprops, min, max, polygons);
                    }
                }
            }

            labels.push_back(create_label(&lbprops, ID, min + ((max-min)/2.0)));
        }
        pbsp = new random_bsp(polygons);
    }
    #endif // SD_ANALYSIS_HPP

    MS_Model::~MS_Model()
    {
        delete pbsp;

        for (std::list<polygon*>::iterator pit = polygons.begin();
             pit != polygons.end(); pit++)
            delete *pit;

        for (std::list<label*>::iterator lbit = labels.begin();
             lbit != labels.end(); lbit++)
            delete *lbit;
    }

    const std::string MS_Model::get_description()
    {
        return std::string("\"Movable Sizable Building\"     Space: Next Step");
    }

    void MS_Model::render(const camera &cam) const
    {
        glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);

        pbsp->render_btf(cam);

        std::list<label*>::const_iterator lbit;
        for (lbit = labels.begin(); lbit != labels.end(); lbit++)
            (*lbit)->render();

        // Draw the lines in for the GUI JH, to create axis x,y,z
        // x axis
        drawLine(2000.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        drawLine(2000.0f, 0.0f, 0.0f, 1700.0f, 0.0f, -300.0f);
        drawLine(2000.0f, 0.0f, 0.0f, 1700.0f, 0.0f, 300.0f);
        // draw letter x
        drawLine(1300.0f, 0.0f, 500.0f, 700.0f, 0.0f, 1100.0f);
        drawLine(700.0f, 0.0f, 500.0f, 1300.0f, 0.0f, 1100.0f);
        // y axis (z in the model)
        drawLine(0.0f, 2000.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        drawLine(0.0f, 2000.0f, 0.0f, 0.0f, 1700.0f, -300.0f);
        drawLine(0.0f, 2000.0f, 0.0f, 0.0f, 1700.0f, 300.0f);
        // draw letter z
        drawLine(0.0f, 600.0f, 500.0f, 0.0f, 600.0f, 1000.0f);
        drawLine(0.0f, 1200.0f, 500.0f, 0.0f, 1200.0f, 1000.0f);
        drawLine(0.0f, 600.0f, 500.0f, 0.0f, 1200.0f, 1000.0f);
        // z axis (y in the model)
        drawLine(0.0f, 0.0f, -2000.0f, 0.0f, 0.0f, 0.0f);
        drawLine(0.0f, 0.0f, -2000.0f, -300.0f, 0.0f, -1700.0f);
        drawLine(0.0f, 0.0f, -2000.0f, 300.0f, 0.0f, -1700.0f);
        // draw letter y
        drawLine(-500.0f, 0.0f, -1500.0f, -1000.0f, 0.0f, -500.0f);
        drawLine(-1000.0f, 0.0f, -1500.0f, -750.0f, 0.0f, -1000.0f);


        glPopAttrib();
    }

} // namespace Visualisation
} // namespace BSO

#endif // MS_MODEL_HPP
