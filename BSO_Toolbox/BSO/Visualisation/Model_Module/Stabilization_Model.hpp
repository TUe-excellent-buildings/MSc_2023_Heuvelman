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

        vertex calculateCoordAverage(const std::vector<Eigen::Vector3d>& coords);

        bool key_pressed(int key);

        // Method to draw a cone, used for the GUI JH (hinges)
        void drawCone(float radius, float height, int numSegments, float x, float y, float z) const {
            glPushMatrix();
            glTranslatef(x, y, z); // Translate to the specified position
            glColor3f(0.0, 0.0, 0.0);
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0.0f, height, 0.0f); // Apex of the cone
            for (int i = 0; i <= numSegments; ++i) {
                float theta = (2.0f * M_PI * i) / numSegments;
                float xx = radius * cos(theta);
                float zz = radius * sin(theta);
                glVertex3f(xx, 0.0f, zz); // Base vertices
            }
            glEnd();
            glPopMatrix();
        }

        // Method to draw a line between two points, used for the GUI JH (axes)
        void drawLine(float x, float y, float z, float x1, float y1, float z1) const {
            glLineWidth(2.5);
            glColor3f(1.0, 0.0, 0.0);
            glBegin(GL_LINES);
            glVertex3f(x, y, z);
            glVertex3f(x1, y1, z1);
            glEnd();
        }

        void drawRenderedTextBig(const char* text, float x, float y, float z) const {
            glRasterPos3f(x, y, z); // Position where to start the text
            while (*text) {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
                ++text;
            }
        }

    protected:

    private:
        std::list<polygon*> polygons;
        std::list<label*>   labels;

        polygon_props  colprops, trussprops, shellprops, colprops_t, trussprops_t, shellprops_t;
        std::vector<polygon_props> cluster_props;
        std::vector<line_props> cluster_lprops;

        line_props     lprops, lprops_t;
        label_props    lbprops;
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

                vertex v = calculateCoordAverage(coords);
                lbprops.textcolor.v[0] = 0.0f;
                lbprops.textcolor.v[1] = 0.0f;
                lbprops.textcolor.v[2] = 0.0f;
                lbprops.textcolor.v[3] = 1.0f;

                labels.push_back(create_label(&lbprops, std::to_string(i), v));

                // Leave this
                coords.clear();
                vis_coords.clear();
            }

            // for(unsigned int i = 0; i < SDA->get_points().size(); i++)
            // {
            //     Structural_Design::Components::Point* point = SDA->get_points()[i];
            //     // std::cout << "point: " << point->get_coords() << std::endl;

            //     vertex v(point->get_coords().x() + 0.1f, point->get_coords().z() + 0.1f, -point->get_coords().y() - 0.1f);
            //     lbprops.textcolor.v[0] = 0.0f;
            //     lbprops.textcolor.v[1] = 0.0f;
            //     lbprops.textcolor.v[2] = 0.0f;
            //     lbprops.textcolor.v[3] = 1.0f;

            //     labels.push_back(create_label(&lbprops, std::to_string(i), v));
            // }
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

        std::list<label*>::const_iterator lbit;
        for (lbit = labels.begin(); lbit != labels.end(); lbit++)
            (*lbit)->render();

        // Draw the hinges for the GUI JH
        drawCone(250.0f, 400.0f, 30, 0.0f, 0.0f, 0.0f);
        drawCone(250.0f, 400.0f, 30, 6000.0f, 0.0f, 0.0f);
        drawCone(250.0f, 400.0f, 30, 12000.0f, 0.0f, 0.0f);
        drawCone(250.0f, 400.0f, 30, 24000.0f, 0.0f, 0.0f);
        drawCone(250.0f, 400.0f, 30, 0.0f, 0.0f, -6000.0f);
        drawCone(250.0f, 400.0f, 30, 6000.0f, 0.0f, -6000.0f);
        drawCone(250.0f, 400.0f, 30, 24000.0f, 0.0f, -6000.0f);
        drawCone(250.0f, 400.0f, 30, 0.0f, 0.0f, -12000.0f);
        drawCone(250.0f, 400.0f, 30, 6000.0f, 0.0f, -12000.0f);
        drawCone(250.0f, 400.0f, 30, 12000.0f, 0.0f, -12000.0f);
        drawCone(250.0f, 400.0f, 30, 24000.0f, 0.0f, -12000.0f);
        drawCone(250.0f, 400.0f, 30, 0.0f, 0.0f, -24000.0f);
        drawCone(250.0f, 400.0f, 30, 6000.0f, 0.0f, -24000.0f);
        drawCone(250.0f, 400.0f, 30, 12000.0f, 0.0f, -24000.0f);
        drawCone(250.0f, 400.0f, 30, 24000.0f, 0.0f, -24000.0f);

        // Draw axes in for the GUI JH
        // x axis
        drawLine(2500.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        drawLine(2500.0f, 0.0f, 0.0f, 2200.0f, 0.0f, -300.0f);
        drawLine(2500.0f, 0.0f, 0.0f, 2200.0f, 0.0f, 300.0f);
        drawRenderedTextBig("X", 1500.0f, 0.0f, 600.0f); // Label X axis
        // y axis (z in the model)
        drawLine(0.0f, 2500.0f, 0.0f, 0.0f, 0.0f, 0.0f);
        drawLine(0.0f, 2500.0f, 0.0f, 0.0f, 2200.0f, -300.0f);
        drawLine(0.0f, 2500.0f, 0.0f, 0.0f, 2200.0f, 300.0f);
        drawRenderedTextBig("Z", 0.0f, 1500.0f, 400.0f); // Label Z axis
        // z axis (y in the model)
        drawLine(0.0f, 0.0f, -2500.0f, 0.0f, 0.0f, 0.0f);
        drawLine(0.0f, 0.0f, -2500.0f, -300.0f, 0.0f, -2200.0f);
        drawLine(0.0f, 0.0f, -2500.0f, 300.0f, 0.0f, -2200.0f);
        drawRenderedTextBig("Y", -600.0f, 0.0f, -1500.0f); // Label Y axis

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

    vertex Stabilization_Model::calculateCoordAverage(const std::vector<Eigen::Vector3d>& coords) {
        if (coords.empty()) {
            return vertex(0, 0, 0); // Return a default vertex if the input is empty
        }

        float sumX = 0.0f, sumY = 0.0f, sumZ = 0.0f;
        for (Eigen::Vector3d point : coords) {
            sumX += point(0);
            sumY += point(1);
            sumZ += point(2);
        }

        float avgX = sumX / coords.size();
        float avgY = sumY / coords.size();
        float avgZ = sumZ / coords.size();

        return vertex(avgX + 250.0f, avgZ + 250.0f, -avgY - 250.0f);
    }

} // namespace Visualisation
} // namespace BSO


#endif // STABILIZATION_MODEL_HPP
