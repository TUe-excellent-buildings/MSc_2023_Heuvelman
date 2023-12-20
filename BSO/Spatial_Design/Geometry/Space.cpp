#ifndef BSO_SPACE_CPP
#define BSO_SPACE_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Space::Space(int room_ID, Cuboid* initial_cuboid, Surface* s_1, Surface* s_2, Surface* s_3, Surface* s_4, Surface* s_5, Surface* s_6)
{
    m_space_type = ""; // default value, to check if a type has been assigned (a type is described by a non zero int)
    m_space_ID = room_ID;
    m_cuboids.push_back(initial_cuboid);
    std::fill_n(m_edges, 12, nullptr);
    std::fill_n(m_points, 8, nullptr);

    m_surfaces[0] = s_1;
    m_surfaces[1] = s_2;
    m_surfaces[2] = s_3;
    m_surfaces[3] = s_4;
    m_surfaces[4] = s_5;
    m_surfaces[5] = s_6;

    for (int i = 0; i < 8; i++)
    {
        m_vertices.push_back(initial_cuboid->get_vertex_ptr(i));
    }

    m_encasing_cuboid = Cuboid(&m_surfaces[0]->get_encasing_rectangle(),
                                     &m_surfaces[1]->get_encasing_rectangle(),
                                     &m_surfaces[2]->get_encasing_rectangle(),
                                     &m_surfaces[3]->get_encasing_rectangle(),
                                     &m_surfaces[4]->get_encasing_rectangle(),
                                     &m_surfaces[5]->get_encasing_rectangle(),
                                    nullptr);

    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 12; k++)
            {
                if (m_surfaces[i]->get_edge_ptr(j) == m_edges[k])
                {
                    break; // it is already in the array, break
                }
                else if (m_edges[k] == nullptr)
                {
                    m_edges[k] = m_surfaces[i]->get_edge_ptr(j);
                }
                else if (k == 12)
                {
                    std::cerr << "Too many edges encountered while initializing a space, exiting..." << std::endl;
                    exit(1);
                }
            }
        }
    }

    if (m_edges[11] == nullptr)
    {
        std::cerr << "Too few edges found while initializing a space, exiting..." << std::endl;
        exit(1);
    }
    //ctor
}

Space::~Space()
{
    //dtor
}

int Space::get_ID()
{
    return m_space_ID;
}

void Space::add_cuboid(Cuboid* cuboid_ptr)
{
    if (std::find(m_cuboids.begin(),m_cuboids.end(), cuboid_ptr) == m_cuboids.end())
    {
        m_cuboids.push_back(cuboid_ptr);
    }
}

void Space::delete_cuboid(Cuboid* cuboid_ptr)
{
    m_cuboids.erase( std::remove(m_cuboids.begin(), m_cuboids.end(), cuboid_ptr), m_cuboids.end());
}

Edge* Space::get_edge_ptr(int n)
{
    return m_edges[n];
}

Surface* Space::get_surface_ptr(int n)
{
    return m_surfaces[n];
}

void Space::check_vertex(Vertex* vertex_ptr)
{
    if (!m_encasing_cuboid.check_vertex(vertex_ptr)) // the vertex is not located in or on the cuboid
    {
        return;
    }

    if (std::find(m_vertices.begin(), m_vertices.end(), vertex_ptr) == m_vertices.end()) // if the vertex is not part of the space yet, then add it to the space
    {
        m_vertices.push_back(vertex_ptr);
    }
    else // if it already is added to the space, then no further action is required and the function is ended
    {
        return;
    }

    for (unsigned int i = 0; i < m_cuboids.size(); i++) // find the cubes in which or on which the vertex is located
    {
        if (m_cuboids[i]->check_vertex(vertex_ptr))
        {
           m_cuboids[i]->split(vertex_ptr); // split this cuboid in 2,4 or 8 new rectangles depending on where the vertex intersects
        }
    }
}

Cuboid& Space::get_encasing_cuboid()
{
    return m_encasing_cuboid;
}

void Space::assign_space_type(std::string t)
{
    m_space_type = t;
} // assign_space_type()

std::string Space::get_space_type()
{
    return m_space_type;
} // get_space_type

// added by Dennis
void Space::add_cuboid_IDs()
{
    for (unsigned int i = 0; i < m_cuboids.size(); i++)
    {
        if (std::binary_search(m_cuboid_IDs.begin(), m_cuboid_IDs.end(), m_cuboids[i]->get_ID()) == false)
        m_cuboid_IDs.push_back(m_cuboids[i]->get_ID());
        std::sort(m_cuboid_IDs.begin(), m_cuboid_IDs.end());
    }
} // add_cuboid_IDs()

// added by Dennis
std::vector<unsigned int> Space::get_cuboid_IDs()
{
    return m_cuboid_IDs;
} // get_cuboid_IDs()

// added by Dennis
std::vector<Cuboid*> Space::get_cuboids()
{
    return m_cuboids;
} // get_cuboids()

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_SPACE_CPP
