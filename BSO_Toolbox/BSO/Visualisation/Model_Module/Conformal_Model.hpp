#ifndef CONFORMAL_MODEL_HPP
#define CONFORMAL_MODEL_HPP

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Visualisation/Model_Module/Model.hpp>
#include <BSO/Visualisation/BSP_Module/BSP.hpp>
#include <cstdlib>

namespace BSO { namespace Visualisation
{

    class Conformal_Model : public model
    {
    public:
        Conformal_Model(Spatial_Design::MS_Conformal&, std::string);
        Conformal_Model(Spatial_Design::MS_Conformal&, std::string, unsigned int);
        ~Conformal_Model();
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
        std::vector<line_props> cluster_lprops;

    };





    // Implementation of member functions:

    // hh, below is the definition of the constructor of setofrooms_model as defined in setofrooms_model.h
    // hh, see datatypes.h for usage of pprops:
    // hh, rgba ambient,diffuse,specular, emission;
    // hh, float shininess;
    // hh, bool translucent, wosided;
    Conformal_Model::Conformal_Model(Spatial_Design::MS_Conformal& ms_conf, std::string type)
    {
        pprops_rectangle.ambient = rgba(0.5f, 0.5f, 0.05f, 0.3f);
        pprops_rectangle.diffuse = rgba(1.0f, 1.0f, 0.1f, 0.3f);
        pprops_rectangle.specular = rgba(1.0f, 1.0f, 0.2f, 0.3f);
        pprops_rectangle.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
        pprops_rectangle.shininess = 60.0;
        pprops_rectangle.translucent = true;
        pprops_rectangle.twosided = true;

        pprops_cuboid.ambient = rgba(0.1f, 0.5f, 0.1f, 0.3f);
        pprops_cuboid.diffuse = rgba(0.2f, 1.0f, 0.2f, 0.3f);
        pprops_cuboid.specular = rgba(0.2f, 1.0f, 0.2f, 0.3f);
        pprops_cuboid.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
        pprops_cuboid.shininess = 60.0;
        pprops_cuboid.translucent = true;
        pprops_cuboid.twosided = true;

        pprops_cuboid_overlap.ambient = rgba(1.0f, 0.1f, 0.1f, 1.0f);
        pprops_cuboid_overlap.diffuse = rgba(1.0f, 0.2f, 0.2f, 1.0f);
        pprops_cuboid_overlap.specular = rgba(1.0f, 0.2f, 0.2f, 1.0f);
        pprops_cuboid_overlap.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
        pprops_cuboid_overlap.shininess = 60.0;
        pprops_cuboid_overlap.translucent = true;
        pprops_cuboid_overlap.twosided = true;

        double offset = 200;

        if (type == "rectangles")
        {
            for (unsigned int i = 0; i < ms_conf.get_rectangle_count(); i++)
            {
                if (ms_conf.get_rectangle(i)->get_surface_count() !=0)
                {

                    Spatial_Design::Geometry::Line* temp_ptr_1 = ms_conf.get_rectangle(i)->get_line_ptr(0);
                    Spatial_Design::Geometry::Line* temp_ptr_2 = nullptr;
                    Vectors::Vector vct_1 = temp_ptr_1->get_vertex_ptr(1)->get_coords() -
                                            temp_ptr_1->get_vertex_ptr(0)->get_coords();

                    Vectors::Vector v[4]; // namespace: Visualisation

                    for (int j = 1; j < 4; j++)
                    {
                        temp_ptr_2 = ms_conf.get_rectangle(i)->get_line_ptr(j);
                        Vectors::Vector vct_2 = temp_ptr_2->get_vertex_ptr(1)->get_coords() -
                                                temp_ptr_2->get_vertex_ptr(0)->get_coords();

                        if (Vectors::is_paral(vct_1, vct_2, 0.01)) // if the two vectors are parallel
                        {
                            Vectors::Vector vct_3 = temp_ptr_2->get_vertex_ptr(0)->get_coords() -
                                                    temp_ptr_1->get_vertex_ptr(0)->get_coords();

                            if (Vectors::is_perp(vct_1, vct_3, 0.01)) // if the two vectors are perpendicular
                            {
                                v[1] = temp_ptr_1->get_vertex_ptr(0)->get_coords();

                                v[0] = temp_ptr_1->get_vertex_ptr(1)->get_coords();

                                v[2] = temp_ptr_2->get_vertex_ptr(0)->get_coords();

                                v[3] = temp_ptr_2->get_vertex_ptr(1)->get_coords();
                                break;
                            }
                            else
                            {
                                v[1] = temp_ptr_1->get_vertex_ptr(0)->get_coords();

                                v[0] = temp_ptr_1->get_vertex_ptr(1)->get_coords();

                                v[3] = temp_ptr_2->get_vertex_ptr(0)->get_coords();

                                v[2] = temp_ptr_2->get_vertex_ptr(1)->get_coords();
                                break;
                            }
                        }
                    }

                    Vectors::Vector diagonal_1 = v[2] - v[0];
                    Vectors::Vector diagonal_2 = v[3] - v[1];

                    diagonal_1 = Vectors::normalise(diagonal_1);
                    diagonal_2 = Vectors::normalise(diagonal_2);

                    double diagonal_offset = sqrt(2*offset*offset);

                    v[0] += diagonal_1*diagonal_offset;
                    v[1] += diagonal_2*diagonal_offset;
                    v[2] -= diagonal_1*diagonal_offset;
                    v[3] -= diagonal_2*diagonal_offset;

                    vertex v_vis[4];

                    for (int i = 0; i < 4; i++)
                    {
                        v_vis[i] = vect3d(v[i](0), v[i](2), -v[i](1));
                    }

                    create_area(polygons, &pprops_rectangle, &lprops, v_vis[0], v_vis[1], v_vis[2], v_vis[3]);
                }
            }
            for (unsigned int i = 0; i < ms_conf.get_space_count(); i++)
            {
                 std::string ID = std::to_string(ms_conf.get_space(i)->get_ID());
                 Vectors::Vector space_middle = ms_conf.get_space(i)->get_encasing_cuboid().get_center_vertex_ptr()->get_coords();

                 vertex center = vect3d (space_middle(0), space_middle(2), -space_middle(1));

                 labels.push_back(create_label(&lbprops, ID, center));
            }
        }
        else if (type == "cuboids")
        {
            vertex max, min;
            Vectors::Point temp_coords_1, temp_coords_2;


// temporarily added by Dennis for cuboid IDs; to undo delete section between these comments (166 - 181) and remove /* - */ below
            for (unsigned int i = 0; i < ms_conf.get_cuboid_count(); i++)
            {
                    temp_coords_1 = ms_conf.get_cuboid(i)->get_max_vertex()->get_coords();
                    temp_coords_2 = ms_conf.get_cuboid(i)->get_min_vertex()->get_coords();

                    max = vect3d(temp_coords_1(0)-offset, temp_coords_1(2)-offset, -temp_coords_2(1)-offset);
                    min = vect3d(temp_coords_2(0)+offset, temp_coords_2(2)+offset, -temp_coords_1(1)+offset);

                    add_cube(&pprops_cuboid, &lprops, min, max, polygons);

                    std::ostringstream out; out << ms_conf.get_cuboid(i)->get_ID(); std::string ID = out.str(); // cast the int value of ID as a string
                    labels.push_back(create_label(&lbprops, ID,
                                                  min + ((max-min)/2.0)));
            }
// temporarily added by Dennis for cuboid IDs; to undo delete section between these comments (166 - 181) and remove /* - */ below

/*
            for (unsigned int i = 0; i < ms_conf.get_cuboid_count(); i++)
            {
                if(ms_conf.get_cuboid(i)->get_space_count() == 1)
                {
                    temp_coords_1 = ms_conf.get_cuboid(i)->get_max_vertex()->get_coords();
                    temp_coords_2 = ms_conf.get_cuboid(i)->get_min_vertex()->get_coords();

                    max = vect3d(temp_coords_1(0)-offset, temp_coords_1(2)-offset, -temp_coords_2(1)-offset);
                    min = vect3d(temp_coords_2(0)+offset, temp_coords_2(2)+offset, -temp_coords_1(1)+offset);

                    add_cube(&pprops_cuboid, &lprops, min, max, polygons);

                    std::ostringstream out; out << ms_conf.get_cuboid(i)->get_space_ptr(0)->get_ID(); std::string ID = out.str(); // cast the int value of ID as a string
                    labels.push_back(create_label(&lbprops, ID,
                                     min + ((max-min)/2.0)));
                }
                else if (ms_conf.get_cuboid(i)->get_space_count() > 1)
                {
                    temp_coords_1 = ms_conf.get_cuboid(i)->get_max_vertex()->get_coords();
                    temp_coords_2 = ms_conf.get_cuboid(i)->get_min_vertex()->get_coords();

                    max = vect3d(temp_coords_1(0)-offset, temp_coords_1(2)-offset, -temp_coords_2(1)-offset);
                    min = vect3d(temp_coords_2(0)+offset, temp_coords_2(2)+offset, -temp_coords_1(1)+offset);

                    add_cube(&pprops_cuboid_overlap, &lprops, min, max, polygons);

                    std::string ID = "";
                    for (unsigned int j = 0; j < ms_conf.get_cuboid(i)->get_space_count(); j++)
                    {
                        if (j!=0)
                        {
                            ID += ",";
                        }
                        ID += std::to_string(ms_conf.get_cuboid(i)->get_space_ptr(j)->get_ID());
                    }
                    labels.push_back(create_label(&lbprops, ID, min + ((max-min)/2.0)));

                    std::cout << "!!! WARNING !!! Overlapping spaces!" << std::endl;
                }
                else
                {
                    std::cout << "Error, in visualisation of cuboid: no spaces assigned to cuboid" << std::endl;
                }
            } */
        }
        else
        {
            std::cout << "Error in visualisation of conformal set of spaces, exiting..." << std::endl;
            exit(1);
        }



        pbsp = new random_bsp(polygons);
    }

    Conformal_Model::~Conformal_Model()
    {
        delete pbsp;

        for (std::list<polygon*>::iterator pit = polygons.begin();
             pit != polygons.end(); pit++)
            delete *pit;

        for (std::list<label*>::iterator lbit = labels.begin();
             lbit != labels.end(); lbit++)
            delete *lbit;
    }

    const std::string Conformal_Model::get_description()
    {
        return std::string("//Conformal set of spaces in \"Movable and Sizable representation\"//     Space: Next Step");
    }

    void Conformal_Model::render(const camera &cam) const
    {
        glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);

        pbsp->render_btf(cam);

        std::list<label*>::const_iterator lbit;
        for (lbit = labels.begin(); lbit != labels.end(); lbit++)
            (*lbit)->render();

        glPopAttrib();
    }

    bool Conformal_Model::key_pressed(int key)
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


#endif // CONFORMAL_MODEL_HPP
