#ifndef BSO_POINT_CPP
#define BSO_POINT_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Point::Point(Vertex* initial_vertex)
{
    m_vertex = initial_vertex;
    //ctor
}

Point::~Point()
{
    //dtor
}

Vertex* Point::get_vertex_ptr()
{
    return m_vertex;
}

void Point::add_edge(Edge* edge_ptr)
{
    m_edge_ptrs.push_back(edge_ptr);
}

void Point::add_surface(Surface* surface_ptr)
{
    m_surface_ptrs.push_back(surface_ptr);
}

void Point::add_space(Space* space_ptr)
{
    m_space_ptrs.push_back(space_ptr);
    if (m_space_ptrs.size() > 1)
    {
        std::cout << "Error, point assigned to too many spaces (only 1 possible), exiting.." << std::endl;
        exit(1);
    }
}


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_POINT_CPP
