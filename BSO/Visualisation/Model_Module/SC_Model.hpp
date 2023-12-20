#ifndef SUPERCUBE_MODEL_HPP
#define SUPERCUBE_MODEL_HPP

#include <BSO/Spatial_Design/Supercube.hpp>
#include <BSO/Visualisation/Model_Module/Model.hpp>
#include <BSO/Visualisation/BSP_Module/BSP.hpp>

namespace BSO { namespace Visualisation
{

    class SC_Model : public model
    {
    public:
        SC_Model(Spatial_Design::SC_Building&);
        ~SC_Model();
        void render(const camera &cam) const;
        const std::string get_description();

    protected:

    private:
        std::list<polygon*> polygons;
        std::list<label*>   labels;

        polygon_props  pprops_empty_cell;
        polygon_props  pprops_active_cell;

        line_props     lprops;
        label_props    lbprops;
        random_bsp     *pbsp;
    };





    // Implementation of member functions:

    // hh, below is the definition of the constructor of setofrooms_model as defined in setofrooms_model.h
    // hh, see datatypes.h for usage of pprops:
    // hh, rgba ambient,diffuse,specular, emission;
    // hh, float shininess;
    // hh, bool translucent, wosided;
    SC_Model::SC_Model(Spatial_Design::SC_Building& S)
    {
        pprops_empty_cell.ambient = rgba(0.1f, 0.1f, 0.1f, 0.2f);
        pprops_empty_cell.diffuse = rgba(0.1f, 0.1f, 0.1f, 0.03f);
        pprops_empty_cell.specular = rgba(0.1f, 0.1f, 0.1f, 0.0f);
        pprops_empty_cell.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
        pprops_empty_cell.shininess = 60.0;
        pprops_empty_cell.translucent = true;
        pprops_empty_cell.twosided = true;

        pprops_active_cell.ambient = rgba(0.5f, 0.5f, 0.05f, 0.3f);
        pprops_active_cell.diffuse = rgba(1.0f, 1.0f, 0.1f, 0.3f);
        pprops_active_cell.specular = rgba(1.0f, 1.0f, 0.2f, 0.3f);
        pprops_active_cell.emission = rgba(0.0f, 0.0f, 0.0f, 0.0f);
        pprops_active_cell.shininess = 60.0;
        pprops_active_cell.translucent = true;
        pprops_active_cell.twosided = true;


        std::vector<int> active_tracker;
        for (unsigned int i = 0; i < S.w_size()*S.d_size()*S.h_size(); i++) // initialize the active tracker vector's size and values to all zeros
            { active_tracker.push_back(0); }

        vertex min, max;

        std::vector<double> x_values; x_values.push_back(0.0);
        std::vector<double> y_values; y_values.push_back(0.0);
        std::vector<double> z_values; z_values.push_back(0.0);

        for (unsigned int w_index = 0; w_index < S.w_size(); w_index++)
            { x_values.push_back(x_values[w_index] + S.request_w(w_index)); }
        for (unsigned int d_index = 0; d_index < S.d_size(); d_index++)
            { y_values.push_back(y_values[d_index] + S.request_d(d_index)); }
        for (unsigned int h_index = 0; h_index < S.h_size(); h_index++)
            { z_values.push_back(z_values[h_index] + S.request_h(h_index)); }

        for (unsigned int room_index = 0; room_index < S.b_size(); room_index++)
        {
            for (unsigned int cell_index = 1; cell_index < S.b_row_size(room_index);cell_index++) // first index (=0) contains room ID
            {
                if (S.request_b(room_index, cell_index) == 1)
                {
                    // pushback active pprops polygon
                    min = vect3d(x_values[S.get_w_index(cell_index)], z_values[S.get_h_index(cell_index)], -y_values[S.get_d_index(cell_index)+1]);
                    max = vect3d(x_values[S.get_w_index(cell_index)+1], z_values[S.get_h_index(cell_index)+1], -y_values[S.get_d_index(cell_index)]);

                    add_cube(&pprops_active_cell, &lprops, min, max, polygons);
                    std::ostringstream out; out << S.request_b(room_index, 0); std::string ID = out.str(); // cast the int value of ID as a string
                    labels.push_back(create_label(&lbprops, ID, min + ((max-min)/2.0)));

                    active_tracker[cell_index-1] = 1;
                }
                else if (S.request_b(room_index, cell_index) != 0)
                {
                    std::cout << "b_values vector contains values other than 1 and 0.";
                    exit(1);
                }
            }
        }

        for (unsigned int cell_index = 1; cell_index <= active_tracker.size(); cell_index++)
        {
            if (active_tracker[cell_index-1] == 0)
            {
                min = vect3d(x_values[S.get_w_index(cell_index)], z_values[S.get_h_index(cell_index)], -y_values[S.get_d_index(cell_index)+1]);
                max = vect3d(x_values[S.get_w_index(cell_index)+1], z_values[S.get_h_index(cell_index)+1], -y_values[S.get_d_index(cell_index)]);

                add_cube(&pprops_empty_cell, &lprops, min, max, polygons);

                //labels.push_back(create_label(&lbprops, ID, min + ((max-min)/2.0)));
            }
        }
        pbsp = new random_bsp(polygons);
    }

    SC_Model::~SC_Model()
    {
        delete pbsp;

        for (std::list<polygon*>::iterator pit = polygons.begin();
             pit != polygons.end(); pit++)
            delete *pit;

        for (std::list<label*>::iterator lbit = labels.begin();
             lbit != labels.end(); lbit++)
            delete *lbit;
    }

    const std::string SC_Model::get_description()
    {
        return std::string("//Set of spaces in \"Supercube representation\"//     Space: Next Step");
    }

    void SC_Model::render(const camera &cam) const
    {
        glPushAttrib(GL_ENABLE_BIT);

        glDisable(GL_DEPTH_TEST);

        pbsp->render_btf(cam);

        std::list<label*>::const_iterator lbit;
        for (lbit = labels.begin(); lbit != labels.end(); lbit++)
            (*lbit)->render();

        glPopAttrib();
    }

} // namespace Visualisation
} // namespace BSO


#endif // SUPERCUBE_MODEL_HPP
