#ifndef BSO_EDGE_HPP
#define BSO_EDGE_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Edge
{
private:
    // members are:
    std::vector<Line*> m_lines; // this edge is represented by one or more lines (at least 1, the initial line, see ctor)
    std::vector<Vertex*> m_vertices; // the vertices that represent this edge
    Line m_encasing_line;
    Point* m_points[2]; // this edge has 2 points

    // member of:
    std::vector<Surface*> m_surface_ptrs; // the surfaces this edge belongs to (2 if cuboid)
    std::vector<Space*> m_space_ptrs; // the spaces this edge belongs to (1 in 3D-cases)



    std::string m_edge_type;
public:
    Edge(Line* initial_line, Point* p_1, Point* p_2);
    ~Edge();

    void add_surface(Surface* surface_ptr);
    void add_space(Space* space_ptr);

    void add_line(Line* line_ptr);
    void delete_line(Line* line_ptr);

    unsigned int get_space_count();
    Space* get_space_ptr(unsigned int n);
    unsigned int get_surface_count();
    Surface* get_surface_ptr(unsigned int n);

    void check_vertex(Vertex* vertex_ptr); // checks if a vertex interferes with an edge that this object represents, if so appropriate action is taken and "true" is returned afterwards
    Line& get_encasing_line();

    void assign_edge_type(std::string);
    std::string get_edge_type();
};


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_EDGE_HPP
