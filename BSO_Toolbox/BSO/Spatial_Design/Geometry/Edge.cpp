#ifndef BSO_EDGE_CPP
#define BSO_EDGE_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Edge::Edge(Line* initial_line, Point* p_1, Point* p_2)
{
    m_edge_type = ""; // default value, to check if it has been assigned a type (int value that is non zero)
    m_lines.push_back(initial_line);
    m_points[0] = p_1;
    m_points[1] = p_2;
    for (int i = 0; i < 2; i++)
    {
        m_vertices.push_back(initial_line->get_vertex_ptr(i));
    }
    m_encasing_line = Line(m_points[0]->get_vertex_ptr(),
                                 m_points[1]->get_vertex_ptr(),
                                 nullptr);
}

Edge::~Edge()
{
    //dtor
}

void Edge::add_surface(Surface* surface_ptr)
{
    m_surface_ptrs.push_back(surface_ptr);
    if (m_space_ptrs.size() > 2)
    {
        std::cout << "Error, edge assigned to too many surfaces (only 2 possible), exiting.." << std::endl;
        exit(1);
    }
}

void Edge::add_space(Space* space_ptr)
{
    m_space_ptrs.push_back(space_ptr);
    if (m_space_ptrs.size() > 1)
    {
        std::cout << "Error, edge assigned to too many spaces (only 1 possible), exiting.." << std::endl;
        exit(1);
    }
}

void Edge::add_line(Line* line_ptr)
{
    if (std::find(m_lines.begin(), m_lines.end(), line_ptr) == m_lines.end())
    {
        m_lines.push_back(line_ptr);
    }
}

void Edge::delete_line(Line* line_ptr)
{
    m_lines.erase( std::remove(m_lines.begin(), m_lines.end(), line_ptr), m_lines.end());
}

unsigned int Edge::get_space_count()
{
    return m_space_ptrs.size();
}

Space* Edge::get_space_ptr(unsigned int n)
{
    return m_space_ptrs[n]; // returns the last element of this vector, which cannot be larger than 1 element and must be larger than 0 elements
}

unsigned int Edge::get_surface_count()
{
    return m_surface_ptrs.size();
}

Surface* Edge::get_surface_ptr(unsigned int n)
{
    return m_surface_ptrs[n];
}

void Edge::check_vertex(Vertex* vertex_ptr) // checks if a vertex interferes with an edge that this object represents, if so appropriate action is taken
{
    if (!m_encasing_line.check_vertex(vertex_ptr)) // check if the vertex is located in or on the space's encasing cuboid
    {
        return;
    }
    if (std::find(m_vertices.begin(), m_vertices.end(), vertex_ptr) == m_vertices.end()) // if the vertex is not part of the edge yet, then add it to the edge and continue with the rest of the function
    {
        m_vertices.push_back(vertex_ptr);
    }
    else // if it already is added to the edge, then no further action is required and the function is ended
    {
        return;
    }

    for (unsigned int i = 0; i < m_lines.size(); i++) // find the lines in which or on which the vertex is located
    {
        if (m_lines[i]->check_vertex(vertex_ptr))
        {
            m_lines[i]->split(vertex_ptr); // split this line in 2 lines
        }
    }
}

Line& Edge::get_encasing_line()
{
    return m_encasing_line;
}

void Edge::assign_edge_type(std::string t)
{
    m_edge_type = t;
} // assign_edge_type()

std::string Edge::get_edge_type()
{
    return m_edge_type;
} // get_edge_type()


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_EDGE_CPP
