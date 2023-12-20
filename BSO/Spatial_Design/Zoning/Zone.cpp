#ifndef ZONE_CPP
#define ZONE_CPP

namespace BSO { namespace Spatial_Design { namespace Zoning {

Zone::Zone(std::vector<Geometry::Cuboid*>& cuboids)
{
	//m_cuboids = cuboids;


} // ctor

Zone::~Zone()
{

} // dtor

void Zone::add_cuboid(Geometry::Cuboid* cuboid)
{
    m_cuboids.push_back(cuboid);
    std::sort(m_cuboids.begin(), m_cuboids.end());
} // add_cuboid()

void Zone::add_cuboid_ID(unsigned int n)
{
    m_cuboid_IDs.push_back(n);
    std::sort(m_cuboid_IDs.begin(), m_cuboid_IDs.end());
} // add_cuboid_ID()

bool Zone::check_double_cuboids(Geometry::Cuboid* cuboid)
{
    return (std::find(m_cuboids.begin(), m_cuboids.end(), cuboid) != m_cuboids.end());
} // check_double_cuboids()

std::vector<Geometry::Cuboid*> Zone::get_cuboids()
{
    return m_cuboids;
} // get_cuboids()

std::vector<unsigned int> Zone::get_cuboid_IDs()
{
    return m_cuboid_IDs;
} // get_cuboid_IDs()

void Zone::min_coords(Geometry::Cuboid* cuboid)
{
    x_min = cuboid->get_min_vertex()->get_coords()(0);
    y_min = cuboid->get_min_vertex()->get_coords()(1);
    z_min = cuboid->get_min_vertex()->get_coords()(2);
} // min_coords()

void Zone::max_coords(Geometry::Cuboid* cuboid)
{
    x_max = cuboid->get_max_vertex()->get_coords()(0);
    y_max = cuboid->get_max_vertex()->get_coords()(1);
    z_max = cuboid->get_max_vertex()->get_coords()(2);
} // max_coords()

void Zone::min_coords_space(Geometry::Space* space)
{
    x_min = space->get_encasing_cuboid().get_min_vertex()->get_coords()(0);
    y_min = space->get_encasing_cuboid().get_min_vertex()->get_coords()(1);
    z_min = space->get_encasing_cuboid().get_min_vertex()->get_coords()(2);
} // min_coords()

void Zone::max_coords_space(Geometry::Space* space)
{
    x_max = space->get_encasing_cuboid().get_max_vertex()->get_coords()(0);
    y_max = space->get_encasing_cuboid().get_max_vertex()->get_coords()(1);
    z_max = space->get_encasing_cuboid().get_max_vertex()->get_coords()(2);
} // max_coords()

int Zone::get_min_coords(unsigned int n)
{
    if (n == 0)
        return x_min;
    if (n == 1)
        return y_min;
    if (n == 2)
        return z_min;
    else
        return 0;
} // get_min_coords()

int Zone::get_max_coords(unsigned int n)
{
    if (n == 0)
        return x_max;
    if (n == 1)
        return y_max;
    if (n == 2)
        return z_max;
    else
        return 0;
} // get_max_coords()

void Zone::dimensions()
{
    width = x_max - x_min;
    depth = y_max - y_min;
    height = z_max - z_min;
} // dimensions()

unsigned int Zone::get_dimensions(unsigned int n)
{
    if (n == 0)
        return width;
    if (n == 1)
        return depth;
    if (n == 2)
        return height;
    else
        return 0;
} // get_dimensions()

void Zone::add_spaces()
{
   for (unsigned int j = 0; j < m_cuboids.size(); j++)
   {
        Geometry::Space* temp_space = m_cuboids[j]->get_space();
        temp_space->add_cuboid_IDs();
        if (std::find(m_spaces.begin(), m_spaces.end(), temp_space) == m_spaces.end())
            m_spaces.push_back(temp_space);
   }

} // add_spaces()

void Zone::check_spaces()
{
    whole_spaces = false;
    part_spaces = false;
    for (unsigned int i = 0; i < m_spaces.size(); i++)
    {
        std::vector<unsigned int> temp_cuboid_IDs = m_spaces[i]->get_cuboid_IDs();
        for (unsigned int j = 0; j < temp_cuboid_IDs.size(); j++)
        {
            if (std::binary_search(m_cuboid_IDs.begin(), m_cuboid_IDs.end(), temp_cuboid_IDs[j]) == true)
            {
                whole_spaces = true;
            }
            else
            {
                whole_spaces = false;
                break;
            }
        }
        if (whole_spaces == true)
            break;
    }

    for (unsigned int i = 0; i < m_spaces.size(); i++)
    {
        std::vector<unsigned int> temp_cuboid_IDs = m_spaces[i]->get_cuboid_IDs();
        for (unsigned int j = 0; j < temp_cuboid_IDs.size(); j++)
        {
            if (std::binary_search(m_cuboid_IDs.begin(), m_cuboid_IDs.end(), temp_cuboid_IDs[j]) == true)
            {
                part_spaces = false;
            }
            else
            {
                part_spaces = true;
                break;
            }
        }
        if (part_spaces == true)
            break;
    }
} // check_spaces()

bool Zone::get_whole_spaces()
{
    return whole_spaces;
} // get_whole_spaces()

bool Zone::get_part_spaces()
{
    return part_spaces;
} // get_part_spaces()

void Zone::add_type(unsigned int n)
{
    zone_type = n;
} // add_type()

unsigned int Zone::get_type()
{
    return zone_type;
} // get_type()

unsigned int Zone::get_cuboid_count()
{
    return m_cuboids.size();
} // get_cuboid_count()

Geometry::Cuboid* Zone::get_cuboid(unsigned int n)
{
    return m_cuboids[n];
} // get_cuboid()

void Zone::duplicate(Zone* zone)
{
    m_cuboids = zone->m_cuboids;
    m_cuboid_IDs = zone->m_cuboid_IDs;
    m_spaces = zone->m_spaces;
    x_min = zone->x_min;
    y_min = zone->y_min;
    z_min = zone->z_min;
    x_max = zone->x_max;
    y_max = zone->y_max;
    z_max = zone->z_max;
    width = zone->width;
    depth = zone->depth;
    height = zone->height;
    part_spaces = zone->part_spaces;
    whole_spaces = zone->whole_spaces;
} // duplicate()

void Zone::tag_for_type_change(unsigned int n)
{
    type_change = true;
    new_type = n;
} // tag_for_type_change()

void Zone::untag_for_type_change()
{
    type_change = false;
    new_type = 0;
} // untag_for_type_change()

bool Zone::check_type_change()
{
    return type_change;
} // check_type_change()

unsigned int Zone::get_type_change()
{
    return new_type;
} // get_type_change()

void Zone::tag_for_deletion()
{
    delete_zone = true;
} // tag_for_deletion()

void Zone::untag_for_deletion()
{
    delete_zone = false;
} // untag_for_deletion()

bool Zone::check_deletion()
{
    return delete_zone;
} // check_deletion()

void Zone::add_floor(int level)
{
    floor = level;
} // add_floor()

void Zone::add_floor_above(int level_above)
{
    floor_above = level_above;
} // add_floor_above()

int Zone::get_floor()
{
    return floor;
} // get_floor()

int Zone::get_floor_above()
{
    return floor_above;
} // get_floor_above()

void Zone::tag_floating_zone()
{
    floating_zone = true;
} // tag_floating_zone()

void Zone::untag_floating_zone()
{
    floating_zone = false;
} // untag_floating_zone()

bool Zone::check_floating_zone()
{
    return floating_zone;
} // check_floating_zone()

void Zone::combine_zones(Zone* zone)
{
    for (unsigned int i = 0; i < zone->m_cuboids.size(); i++)
    {
        Geometry::Cuboid* temp_cuboid = zone->m_cuboids[i];
        if (Zone::check_double_cuboids(temp_cuboid) == false)
        {
            Zone::add_cuboid(temp_cuboid);
            Zone::add_cuboid_ID(temp_cuboid->get_ID());
            if (temp_cuboid->get_max_vertex()->get_coords()(0) > x_max)
                x_max = temp_cuboid->get_max_vertex()->get_coords()(0);
            else if (temp_cuboid->get_min_vertex()->get_coords()(0) < x_min)
                x_min = temp_cuboid->get_min_vertex()->get_coords()(0);
            if (temp_cuboid->get_max_vertex()->get_coords()(1) > y_max)
                y_max = temp_cuboid->get_max_vertex()->get_coords()(1);
            else if (temp_cuboid->get_min_vertex()->get_coords()(1) < y_min)
                y_min = temp_cuboid->get_min_vertex()->get_coords()(1);
            if (temp_cuboid->get_max_vertex()->get_coords()(2) > z_max)
                z_max = temp_cuboid->get_max_vertex()->get_coords()(2);
            else if (temp_cuboid->get_min_vertex()->get_coords()(2) < z_min)
                z_min = temp_cuboid->get_min_vertex()->get_coords()(2);
        }
    }
} // combine_zones()

void Zone::add_ID(unsigned int n)
{
    ID = n;
} // add_ID()


unsigned int Zone::get_ID()
{
    return ID;
} // get_ID()

void Zone::tag_combined_zone()
{
    combined_zone = true;
} // tag_combined_zone()

bool Zone::check_combined_zone()
{
    return combined_zone;
} // check_combined_zone()

void Zone::add_rectangles()
{
    for (unsigned int i = 0; i < m_cuboids.size(); i++)
    {
        for (unsigned int j = 0; j < 6; j++) // 6 rectangles in each cuboid
        {
            int X = m_cuboids[i]->get_rectangle_ptr(j)->get_center_vertex_ptr()->get_coords()(0);
            int Y = m_cuboids[i]->get_rectangle_ptr(j)->get_center_vertex_ptr()->get_coords()(1);
            int Z = m_cuboids[i]->get_rectangle_ptr(j)->get_center_vertex_ptr()->get_coords()(2);
            if (x_min >= X || x_max <= X)
            {
                    m_rectangles.push_back(m_cuboids[i]->get_rectangle_ptr(j));
            }
            else if (y_min >= Y || y_max <= Y)
            {
                    m_rectangles.push_back(m_cuboids[i]->get_rectangle_ptr(j));
            }
            else if (z_min >= Z || z_max <= Z)
            {
                    m_rectangles.push_back(m_cuboids[i]->get_rectangle_ptr(j));
            }
        }
    }
} // add rectangles

void Zone::add_internal_floors()
{
    for (unsigned int i = 0; i < m_cuboids.size(); i++)
    {
        for (unsigned int j = 0; j < 6; j++) // 6 rectangles in each cuboid
        {
            if (m_cuboids[i]->get_rectangle_ptr(j)->get_horizontal() == true && m_cuboids[i]->get_rectangle_ptr(j)->get_zoned() == false
                && m_cuboids[i]->get_rectangle_ptr(j)->get_surface_count() == 2)
            {
                    m_rectangles.push_back(m_cuboids[i]->get_rectangle_ptr(j));
            }

        }
    }
}

unsigned int Zone::get_rectangle_count()
{
    return m_rectangles.size();
} // get rectangle count

Geometry::Rectangle* Zone::get_rectangle(unsigned int n)
{
    return m_rectangles[n];
} // get rectangle

void Zone::add_vertices()
{
    for (unsigned int i = 0; i < m_rectangles.size(); i++)
    {
        for (unsigned int j = 0; j < 4; j++) // 4 vertices in each rectangle
        {
            Geometry::Vertex* temp_vertex = m_rectangles[i]->get_vertex_ptr(j);
            if (std::find(m_vertices.begin(), m_vertices.end(), temp_vertex) == m_vertices.end())
            {
                m_vertices.push_back(temp_vertex);
            }
        }
    }
} // add vertices

std::vector<Geometry::Vertex*> Zone::get_vertices()
{
    return m_vertices;
} // get vertices()

unsigned int Zone::get_vertex_count()
{
    return m_vertices.size();
} // get vertex count

Geometry::Vertex* Zone::get_vertex(unsigned int n)
{
    return m_vertices[n];
} // get vertex

} // namespace Zoning
} // namespace Spatial_Design
} // namespace BSO

#endif //ZONE_CPP
