// hh, compiler command: if MODEL_H is not defined, define it (as boolean?)
// hh, and carry out all further definitions
// hh, if MODEL_H already exists (and thus the header file has already been read)
// hh, do not read header file
// hh, this is merely a protection against multiple reads of the header file

#ifndef MODEL_HPP
#define MODEL_HPP

#ifndef M_PI
#define M_PI 3.141592653589793238463
#endif // M_PI

#include <BSO/Visualisation/Utility_Module/Datatypes.hpp>
#include <BSO/Visualisation/Utility_Module/Vertex_Store.hpp>
#include <BSO/Visualisation/Utility_Module/Aabbox.hpp>
#include <BSO/Visualisation/BSP_Module/BSP.hpp>
#include <BSO/Visualisation/Utility_Module/Objects.hpp>
#include <BSO/Visualisation/Utility_Module/Camera.hpp>
#include <BSO/Visualisation/Utility_Module/Vertex_Store.hpp>

#include <list>
#include <string>
#include <memory>



namespace BSO { namespace Visualisation
{
    //forward declaration
    // hh, although formally only difference is private/public variables, in c++ classes are most often used for variables and functions
    // hh, and structs for variables only
    class random_bsp;
    class camera;
    class polygon;
    class label;
    struct polygon_props;
    struct line_props;
    struct label_props;

    /*
       This is the base class off all models.
       It contains the geometry of a model, which is assumed to
       be static (no parts of it move independently). The base
       class has protected member functions that can be used by
       sub classes to add geometry, these functions use a vertex
       store to store all vertices and vertex normals.
     */
    class model
    {
        public:
    // hh, virtural functions exist in appearance but not in reality
    // hh, it should be defined as virtual in the class and it should be applied
    // hh, to objects of members descended from the same base class.

    // hh, destructor:
            virtual ~model() { }
    // hh, render function again virtual but now also constant, means that it will not modify the class's member data
    // hh, the cam in the function is given by reference, but is not allowed to be changed due to const
            virtual void render(const camera &cam) const = 0;
    // hh, inline means that the compiler will write the function code each time the function is called
            inline const aabboxd& get_bbox() const { return bbox; }

            virtual const std::string get_description() = 0;

            virtual bool key_pressed(int key) { (void)key; return false; }

    // hh, protected means accessable for base class functions (like private) and for derived classes, but not in general (like public)
        protected:
    // hh, vertex_store is defined in vertex_store.h
            vertex_store   store;
            aabboxd        bbox; //the bounding box of the model
                                 //the functions below that create
                                 //polygons, cubes, lines segments and
                                 //labels will update the bbox accordingly

            //used internally
            polygon *create_polygon(polygon_props *pprops,
                    line_props *lprops,
                        vertex vertarr[], //vertices
                        normal normarr[], //normals
                        int    indices[], //vertex indices
                        int    vcount);

            polygon *create_polygon(polygon_props *pprops,
                    line_props *lprops,
                        vertex vertarr[], //vertices
                        normal normarr[], //normals
                        int    indices[], //vertex indices
                        bool   edgeflags[],
                        int    vcount);

            void add_cube(polygon_props *pprops, line_props *lprops,
                          vect3d const &min, vect3d const &max,
                          std::list<polygon*> &plist);

            void create_area(std::list<polygon*> &polygons,
                             polygon_props *pp, line_props *lp,
                             vertex &v0, vertex &v1, vertex &v2, vertex &v3);

            void create_column(std::list<polygon*> &polygons,
                         polygon_props *pprops,
                                       vertex &v0, vertex &v1, double r = 0.1);


    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            void create_column_Lines(std::list<polygon*> &polygons,
                         polygon_props *pprops, line_props *lp,
                                       vertex &v0, vertex &v1, double r = 0.1);
    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            void create_column_Juan(std::list<polygon*> &polygons,
                         polygon_props *pprops, line_props *lp,
                                       vertex &v0, vertex &v1, double r = 0.1);
    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


            label *create_label(label_props *lbprops, const std::string &text,
                                const vertex &pos);
    }; // Model




    // Implementation of member functions

    // hh, polygon is a class used for storage, it represents a planar convex polygon
    // hh, polygon_props is structure, determines color polygon's surface
    // hh, line_props is structure, determines color and pattern of polygon outline
    // hh, vertex is a struct containing coordinates (double) for vertex, other name for vect3 as used in vect3.h
    // hh, normal is a struct containing coordinates (double) for normal vector, other name for vect3 as used in vect3.h

    polygon *model::create_polygon(polygon_props *pprops,
                        line_props *lprops,
                        vertex vertarr[], //vertices
                        normal normarr[], //normals
                        int    indices[], //vertex indices
                        int    vcount)     //number of vertices
    {
        //create a circularly linked list of edges
        //copy the vertices and normals
        // hh, an edge has a vertex, normal, bool flagedge, and pointer pnext to next edge
        edge *pedge,
                 *pbound;
        bbox.add(vertarr[indices[0]]);
        pedge = pbound = new edge(
                store.get_vertex(vertarr[indices[0]]),
                store.get_normal(normarr[indices[1]]));

        for (int i=2; i<2*vcount; i+=2) {
            bbox.add(vertarr[indices[i]]);
            edge *pnew = new edge(
                store.get_vertex(vertarr[indices[i]]),
                store.get_normal(normarr[indices[i+1]]));

            pedge->pnext = pnew;
            pedge = pnew;
        }

        //close the circle
        pedge->pnext = pbound;

        //create a new polygon
        return new polygon(pprops, lprops, pbound);
    }

    polygon *model::create_polygon(polygon_props *pprops,
                        line_props *lprops,
                        vertex vertarr[], //vertices
                        normal normarr[], //normals
                        int    indices[], //vertex indices
                        bool   edgeflags[],
                        int    vcount)     //number of vertices
    {
        //create a circularly linked list of edges
        //copy the vertices and normals
        edge *pedge,
                 *pbound;
        bbox.add(vertarr[indices[0]]);
        pedge = pbound = new edge(
                store.get_vertex(vertarr[indices[0]]),
                store.get_normal(normarr[indices[1]]),
                edgeflags[0]);

        for (int i=1; i<vcount; i++) {
            bbox.add(vertarr[indices[2*i]]);
            edge *pnew = new edge(
                store.get_vertex(vertarr[indices[2*i]]),
                store.get_normal(normarr[indices[2*i+1]]),
                edgeflags[i]);

            pedge->pnext = pnew;
            pedge = pnew;
        }

        //close the circle
        pedge->pnext = pbound;

        //create a new polygon
        return new polygon(pprops, lprops, pbound);
    }

    // hh, &min, min is also possible, but less memory use, *min needs -> operator is also possible

    void model::add_cube(polygon_props *pprops, line_props *lprops,
                         vect3d const &min, vect3d const &max, std::list<polygon*> &plist)
    {
        vertex verts[] = { vertex(min.x, max.y, min.z),
                           vertex(min.x, max.y, max.z),
                           vertex(max.x, max.y, max.z),
                           vertex(max.x, max.y, min.z),
                           vertex(min.x, min.y, min.z),
                           vertex(min.x, min.y, max.z),
                           vertex(max.x, min.y, max.z),
                           vertex(max.x, min.y, min.z) };
        normal norms[] = { normal(-1,  0,  0),  //left
                           normal( 1,  0,  0),  //right
                   normal( 0,  0, -1),  //back
                   normal( 0,  0,  1),  //front
                   normal( 0, -1,  0),  //down
                   normal( 0,  1,  0) };//up
        int indices[] = { 1,0, 0,0, 4,0, 5,0,  //left
                          3,1, 2,1, 6,1, 7,1,  //right
                          0,2, 3,2, 7,2, 4,2,  //back
                  2,3, 1,3, 5,3, 6,3,  //front
                  7,4, 6,4, 5,4, 4,4,  //down
                  0,5, 1,5, 2,5, 3,5 };//up

        plist.push_back(create_polygon(pprops, lprops, verts,
                                       norms, indices, 4));
        plist.push_back(create_polygon(pprops, lprops, verts,
                                       norms, indices+8, 4));
        plist.push_back(create_polygon(pprops, lprops, verts,
                                       norms, indices+16, 4));
        plist.push_back(create_polygon(pprops, lprops, verts,
                                       norms, indices+24, 4));
        plist.push_back(create_polygon(pprops, lprops, verts,
                                       norms, indices+32, 4));
        plist.push_back(create_polygon(pprops, lprops, verts,
                                       norms, indices+40, 4));
    }

    void model::create_area(std::list<polygon*> &polygons,
                     polygon_props *pp, line_props *lp,
                     vertex &v0, vertex &v1, vertex &v2, vertex &v3)
    // although cubes can only be orthogonal, below works for arbitrarily surfaces (but only 4 edges)
    {
        //create two one-sided areas with opposite normals
        //create a list of vertices
        int    indices[] = {0,0, 1,0, 2,0, 3,0, 3,1, 2,1, 1,1, 0,1};
        vertex v[] = {v0, v1, v2, v3};
        normal n[] = { cross(v2-v1,v0-v1).normalise(),
                       cross(v0-v1,v2-v1).normalise() };
        polygons.push_back(create_polygon(pp, lp,
                    v, n, indices, 4));
        polygons.push_back(create_polygon(pp, lp,
                    v, n, indices+8, 4));
    }

    void model::create_column(std::list<polygon*> &polygons, polygon_props *pprops,
                vertex &v0, vertex &v1, double r)
    {
        //create a column with 3 sides at position (0, 0, 0)
        //along the positive y axis and transform the
        //geometry to match the position and orientation
        //of the vector from v0 to v1
        //determine the length of the column
        vect3d v = v1-v0;
        double l = v.length();
        //determine the vector and angle of
        //rotation that are required to rotate the
        //+y axis in the direction of v
        //calculate the angle between v an +y
        vect3d y = vect3d(0.0, 1.0, 0.0);
        //normalise v
        v /= l;
        double ydotv = dot(y,v),theta;
        vect3d a; //axis of rotation

        //compute the angle
        if (ydotv == 1.0) {
            //the angle is 0
            theta = 0.0;
            a = vect3d(1.0, 0.0, 0.0);
        } else if (ydotv == -1.0) {
            theta = M_PI;
            a = vect3d(1.0, 0.0, 0.0);
        } else {
            //compute the angle
            theta = acos(ydotv);
            a = cross(y,v);
        }

        //create the transformation matrix
        mat4d trans = mat4d::trans(v0.x, v0.y, v0.z) *
                      mat4d::rotate(theta, a.x, a.y, a.z);

        double cos30 = 0.866025403784,
               sin30 = 0.5;

        vertex cverts[] = { vertex(cos30*r,  0.0, sin30*r),
                            vertex(0.0,      0.0, -r),
                            vertex(-cos30*r, 0.0, sin30*r),
                                vertex(cos30*r,    l, sin30*r),
                            vertex(0.0,        l, -r),
                            vertex(-cos30*r,   l, sin30*r) };
        normal cnorms[] = { normal(cos30,  0.0, sin30),
                            normal(0.0,    0.0, -1.0),
                            normal(-cos30, 0.0, sin30) };
        int    ind[] = { 0,0, 3,0, 5,2, 2,2,
                         1,1, 4,1, 3,0, 0,0,
                 1,1, 2,2, 5,2, 4,1 };

        //transform the vertices
        for (int i=0; i<6; i++)
            cverts[i] *= trans;

        //transform the normals and normalise them
        //normals are multiplied by the transpose of the inverse matrix
        mat4d invtrans = mat4d::rotate(theta, -a.x, -a.y, -a.z);
        for (int i=0; i<3; i++) {
            cnorms[i] = invtrans.transmultv(cnorms[i]).normalise();
        }

        //create the polygons
    //hh, columns do not have line properties
        polygons.push_back(create_polygon(pprops, NULL,
                    cverts, cnorms, ind+0, 4));
        polygons.push_back(create_polygon(pprops, NULL,
                    cverts, cnorms, ind+8, 4));
        polygons.push_back(create_polygon(pprops, NULL,
                    cverts, cnorms, ind+16, 4));
    }

    label *model::create_label(label_props *lbprops, const std::string &text,
                        const vertex &pos)
    {
        bbox.add(pos);
        return new label(lbprops, text, store.get_vertex(pos));
    }

    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    /// COLUMN WITH RENDERED EDGES
    void model::create_column_Lines(std::list<polygon*> &polygons,
                                   polygon_props *pprops, line_props *lp,
                vertex &v0, vertex &v1, double r)
    {
        //create a column with 3 sides at position (0, 0, 0)
        //along the positive y axis and transform the
        //geometry to match the position and orientation
        //of the vector from v0 to v1
        //determine the length of the column
        vect3d v = v1-v0;
        double l = v.length();
        //determine the vector and angle of
        //rotation that are required to rotate the
        //+y axis in the direction of v
        //calculate the angle between v an +y
        vect3d y = vect3d(0.0, 1.0, 0.0);
        //normalise v
        v /= l;
        double ydotv = dot(y,v),theta;
        vect3d a; //axis of rotation

        //compute the angle
        if (ydotv == 1.0) {
            //the angle is 0
            theta = 0.0;
            a = vect3d(1.0, 0.0, 0.0);
        } else if (ydotv == -1.0) {
            theta = M_PI;
            a = vect3d(1.0, 0.0, 0.0);
        } else {
            //compute the angle
            theta = acos(ydotv);
            a = cross(y,v);
        }

        //create the transformation matrix
        mat4d trans = mat4d::trans(v0.x, v0.y, v0.z) *
                      mat4d::rotate(theta, a.x, a.y, a.z);

        double cos30 = 0.866025403784,
               sin30 = 0.5;

        vertex cverts[] = { vertex(cos30*r,  0.0, sin30*r),
                            vertex(0.0,      0.0, -r),
                            vertex(-cos30*r, 0.0, sin30*r),
                                vertex(cos30*r,    l, sin30*r),
                            vertex(0.0,        l, -r),
                            vertex(-cos30*r,   l, sin30*r) };
        normal cnorms[] = { normal(cos30,  0.0, sin30),
                            normal(0.0,    0.0, -1.0),
                            normal(-cos30, 0.0, sin30) };
        int    ind[] = { 0,0, 3,0, 5,2, 2,2,
                         1,1, 4,1, 3,0, 0,0,
                 1,1, 2,2, 5,2, 4,1 };

        //transform the vertices
        for (int i=0; i<6; i++)
            cverts[i] *= trans;

        //transform the normals and normalise them
        //normals are multiplied by the transpose of the inverse matrix
        mat4d invtrans = mat4d::rotate(theta, -a.x, -a.y, -a.z);
        for (int i=0; i<3; i++) {
            cnorms[i] = invtrans.transmultv(cnorms[i]).normalise();
        }

        //create the polygons
    //hh, columns do not have line properties
        polygons.push_back(create_polygon(pprops, lp,
                    cverts, cnorms, ind+0, 4));
        polygons.push_back(create_polygon(pprops, lp,
                    cverts, cnorms, ind+8, 4));
        polygons.push_back(create_polygon(pprops, lp,
                    cverts, cnorms, ind+16, 4));
    }

    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    void model::create_column_Juan(std::list<polygon*> &polygons,
                                   polygon_props *pprops, line_props *lp,
                vertex &v0, vertex &v1, double r)
    {
        //create a column with 3 sides at position (0, 0, 0)
        //along the positive y axis and transform the
        //geometry to match the position and orientation
        //of the vector from v0 to v1
        //determine the length of the column
        vect3d v = v1-v0;
        double l = v.length();
        //determine the vector and angle of
        //rotation that are required to rotate the
        //+y axis in the direction of v
        //calculate the angle between v an +y
        vect3d y = vect3d(0.0, 1.0, 0.0);
        //normalise v
        v /= l;
        double ydotv = dot(y,v),theta;
        vect3d a; //axis of rotation

        //compute the angle
        if (ydotv == 1.0) {
            //the angle is 0
            theta = 0.0;
            a = vect3d(1.0, 0.0, 0.0);
        } else if (ydotv == -1.0) {
            theta = M_PI;
            a = vect3d(1.0, 0.0, 0.0);
        } else {
            //compute the angle
            theta = acos(ydotv);
            a = cross(y,v);
        }

        //create the transformation matrix
        mat4d trans = mat4d::trans(v0.x, v0.y, v0.z) *
                      mat4d::rotate(theta, a.x, a.y, a.z);

        double cos30 = 0.866025403784,
               sin30 = 0.5;

        vertex cverts[] = { vertex(cos30*r,  0.0, sin30*r),
                            vertex(0.0,      0.0, -r),
                            vertex(-cos30*r, 0.0, sin30*r),
                                vertex(cos30*r,    l, sin30*r),
                            vertex(0.0,        l, -r),
                            vertex(-cos30*r,   l, sin30*r) };
        normal cnorms[] = { normal(cos30,  0.0, sin30),
                            normal(0.0,    0.0, -1.0),
                            normal(-cos30, 0.0, sin30) };
        int    ind[] = { 0,0, 3,0, 5,2, 2,2,
                         1,1, 4,1, 3,0, 0,0,
                 1,1, 2,2, 5,2, 4,1 };

        //transform the vertices
        for (int i=0; i<6; i++)
            cverts[i] *= trans;

        //transform the normals and normalise them
        //normals are multiplied by the transpose of the inverse matrix
        mat4d invtrans = mat4d::rotate(theta, -a.x, -a.y, -a.z);
        for (int i=0; i<3; i++) {
            cnorms[i] = invtrans.transmultv(cnorms[i]).normalise();
        }

        //create the polygons
    //hh, columns do not have line properties
        polygons.push_back(create_polygon(pprops, lp,
                    cverts, cnorms, ind+0, 4));
        polygons.push_back(create_polygon(pprops, lp,
                    cverts, cnorms, ind+8, 4));
        polygons.push_back(create_polygon(pprops, lp,
                    cverts, cnorms, ind+16, 4));
    }

    ///!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!



} // namespace Visualisation
} // namespace BSO

#endif //MODEL_HPP
