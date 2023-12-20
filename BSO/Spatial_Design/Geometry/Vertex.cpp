#ifndef BSO_VERTEX_CPP
#define BSO_VERTEX_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry {

Vertex::Vertex(double x, double y, double z)
{
    m_structural = false;
    m_coords << x, y, z;
} // ctor

Vertex::Vertex(Vectors::Point p)
{
    m_structural = false;
    m_coords = p;
}

Vertex::Vertex()
{

} // ctor

Vertex::~Vertex()
{

} //dtor

bool Vertex::operator == (const Vertex& rhs)
{
    if (this == &rhs)
    {
        return true;
    }
    else if (Vectors::is_zero(this->m_coords - rhs.m_coords, 0.001))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Vertex::is_structural()
{
    return m_structural;
}

void Vertex::add_point(Point* point_ptr)
{
    bool ptr_found = false;
    for (unsigned int i = 0; i < m_points.size(); i++)
    {
        if (point_ptr == m_points[i])
        {
            ptr_found = true;
        }
    }
    if (!ptr_found)
    {
        m_points.push_back(point_ptr);
    }
}

void Vertex::make_structural()
{
    m_structural = true;
}

Vectors::Point Vertex::get_coords()
{
    return m_coords;
}


Point* Vertex::get_last_point_ptr()
{
    return m_points.back();
}

void Vertex::tag_zoned()
{
    zoned = true;
}

void Vertex::untag_zoned()
{
    zoned = false;
}

bool Vertex::get_zoned()
{
    return zoned;
}

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_VERTEX_CPP
