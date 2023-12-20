#ifndef BSO_SURFACE_HPP
#define BSO_SURFACE_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Surface
{
private:
    // members are:
    std::vector<Rectangle*> m_rectangles; // this surface is represented by one or more rectangles
    std::vector<Vertex*> m_vertices; // the vertices that represent this surface
    Rectangle m_encasing_rectangle;
    Edge* m_edges[4]; // this surface has 4 edges

    // member of:
    std::vector<Space*> m_space_ptrs; // the spaces that this surface belongs to (1 in 3D-cases)
    std::vector<std::pair<double, double> > m_facing; // facing of the surface's side that faces inwards to the spaces, vector follows indexing of m_space_ptrs
                                                    // facing is a pair of doubles: first: azimuth (wind direction) second: altitude (slope of the surface's normal)

    std::string m_surface_type;
public:
    Surface(Rectangle* initial_rectangle, Edge* e_1, Edge* e_2, Edge* e_3, Edge* e_4);
    ~Surface();

    void add_space(Space* space_ptr);

    void add_rectangle(Rectangle*& rectangle_ptr);
    void delete_rectangle(Rectangle* rectangle_ptr);

    unsigned int get_vertex_count();
    Vertex* get_vertex_ptr(unsigned int);
    unsigned int get_rectangle_count();
    Rectangle* get_rectangle_ptr(unsigned int);



    void calc_facing();

    unsigned int get_space_count();
    Space* get_space_ptr(unsigned int i);
    Edge* get_edge_ptr(int n);
    std::pair<double, double> get_facing(unsigned int n);

    void check_vertex(Vertex* vertex_ptr);
    Rectangle& get_encasing_rectangle();

    void assign_surface_type(std::string);
    std::string get_surface_type();
};

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_SURFACE_HPP
