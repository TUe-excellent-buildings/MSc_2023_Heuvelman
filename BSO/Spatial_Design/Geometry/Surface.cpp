#ifndef BSO_SURFACE_CPP
#define BSO_SURFACE_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Surface::Surface(Rectangle* initial_rectangle, Edge* e_1, Edge* e_2, Edge* e_3, Edge* e_4)
{
    m_surface_type = ""; // default value, to check if a type has been assigned (a type is described by a non zero int)
    m_rectangles.push_back(initial_rectangle);
    m_edges[0] = e_1;
    m_edges[1] = e_2;
    m_edges[2] = e_3;
    m_edges[3] = e_4;
    for (int i = 0; i < 4; i++)
    {
        m_vertices.push_back(initial_rectangle->get_vertex_ptr(i));
    }
    m_encasing_rectangle = Rectangle(&m_edges[0]->get_encasing_line(),
                                           &m_edges[1]->get_encasing_line(),
                                           &m_edges[2]->get_encasing_line(),
                                           &m_edges[3]->get_encasing_line(),
                                           nullptr);
    calc_facing();
    //ctor
}

Surface::~Surface()
{
    //dtor
}

void Surface::add_space(Space* space_ptr)
{
    m_space_ptrs.push_back(space_ptr);
    if (m_space_ptrs.size() > 1)
    {
        std::cerr << "Error, surface assigned to too many spaces (only 1 possible), exiting.." << std::endl;
        exit(1);
    }
}

void Surface::add_rectangle(Rectangle*& rectangle_ptr)
{
    if (std::find(m_rectangles.begin(),m_rectangles.end(), rectangle_ptr) == m_rectangles.end())
    {
        m_rectangles.push_back(rectangle_ptr);
    }
}

void Surface::delete_rectangle(Rectangle* rectangle_ptr)
{
    m_rectangles.erase( std::remove(m_rectangles.begin(), m_rectangles.end(), rectangle_ptr), m_rectangles.end());
}

unsigned int Surface::get_vertex_count()
{
    return m_vertices.size();
}
Vertex* Surface::get_vertex_ptr(unsigned int n)
{
    return m_vertices[n];
}

unsigned int Surface::get_rectangle_count()
{
    return m_rectangles.size();
}

Rectangle* Surface::get_rectangle_ptr(unsigned int n)
{
    return m_rectangles[n];
}

void Surface::calc_facing()
{
    // get normal vector
    BSO::Vectors::Vector v_norm = *m_encasing_rectangle.get_normal_ptr();

    m_facing.resize(m_space_ptrs.size());

    for (unsigned int i = 0; i < m_space_ptrs.size(); i++)
    {
        // get center vertex
        BSO::Vectors::Point p_center = m_space_ptrs[i]->get_encasing_cuboid().get_center_vertex_ptr()->get_coords();

        // get a vertex of the surface
        BSO::Vectors::Point p_surf = m_vertices[0]->get_coords();

        // compute vector from surface point to space center vertex
        BSO::Vectors::Vector v_1 = p_center-p_surf;

        // find index of largest abs coefficient in the normal vector
        unsigned int j;
        v_norm.cwiseAbs().maxCoeff(&j);

        // see if both vectors are pointing in the same direction and then determine the azimuth and altitude of the normal
        std::pair<double, double> temp_facing;
        if ((v_norm(j) >= 0) ^ (v_1(j) < 0))
        {
            temp_facing.first = BSO::Vectors::azimuth(v_norm);
            temp_facing.second = BSO::Vectors::altitude(v_norm);
        }
        else
        {
            temp_facing.first = BSO::Vectors::azimuth(-1 * v_norm);
            temp_facing.second = BSO::Vectors::altitude(-1 * v_norm);
        }
        m_facing[i] = temp_facing;
    }

} // calc_facing()

unsigned int Surface::get_space_count()
{
    return m_space_ptrs.size();
}

Space* Surface::get_space_ptr(unsigned int n)
{
    return m_space_ptrs[n]; // returns the last element of this vector, which cannot be larger than 1 element and must be larger than 0 elements
}

Edge* Surface::get_edge_ptr(int n)
{
    return m_edges[n];
}

std::pair<double, double> Surface::get_facing(unsigned int n)
{
    calc_facing();
    return m_facing[n];
}

void Surface::check_vertex(Vertex* vertex_ptr) // checks if a vertex lies on this surface, if so then split the rectangle on which it lies it into four new rectangles
{
    if (!m_encasing_rectangle.check_vertex(vertex_ptr)) // the vertex is not located in or on the cuboid
    {
        return;
    }

    if (std::find(m_vertices.begin(), m_vertices.end(), vertex_ptr) == m_vertices.end()) // if the vertex is not part of the surface yet, then add it to the space and continue with the rest of the function
    {
        m_vertices.push_back(vertex_ptr);
    }
    else // if it already is added to the surface, then no further action is required and the function is ended
    {
        return;
    }

    for (unsigned int i = 0; i < m_rectangles.size(); i++) // find the rectangles in which or on which the vertex is located
    {
        if (m_rectangles[i]->check_vertex(vertex_ptr)) // if the point is located on the rectangle, then split it
        {
            m_rectangles[i]->split(vertex_ptr); // split this rectangle in 2 or 4 new rectangles depending on where the vertex intersects
        }
    }
}

Rectangle& Surface::get_encasing_rectangle()
{
    return m_encasing_rectangle;
} // get_encasing_rectangle()

void Surface::assign_surface_type(std::string t)
{
    m_surface_type = t;
} // assign_surface_type()

std::string Surface::get_surface_type()
{
    return m_surface_type;
} // get_surface_type()


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_SURFACE_CPP
