#ifndef BSO_POINT_HPP
#define BSO_POINT_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Point
{
private:
    // members are:
    Vertex* m_vertex; // this point is represented by one vertex

    // member of:
    std::vector<Edge*> m_edge_ptrs; // the edges this point belongs to (3 if cuboid)
    std::vector<Surface*> m_surface_ptrs; // the surfaces this point belongs to (3 if cuboid)
    std::vector<Space*> m_space_ptrs; // the spaces this point belongs to (1 in 3D-cases)
public:
    Point(Vertex* initial_vertex);
    ~Point();

    Vertex* get_vertex_ptr();

    void add_edge(Edge* edge_ptr);
    void add_surface(Surface* surface_ptr);
    void add_space(Space* space_ptr);
};

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_POINT_HPP
