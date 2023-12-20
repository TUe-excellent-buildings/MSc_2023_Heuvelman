#ifndef BSO_LINE_CPP
#define BSO_LINE_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Line::Line(Vertex* one, Vertex* two, Vertex_Store* store_ptr)
{
    m_store_ptr = store_ptr;
    m_deletion = false;
    m_structural = false;
    m_vertices[0] = one;
    m_vertices[1] = two;

    // calculate the center vertex
    Vectors::Point center;
    center.setZero();
    for (int i = 0; i < 2; i++)
    {
        center += m_vertices[i]->get_coords();
    }
    m_center_vertex = Vertex(center/2.0);
    //ctor
}

Line::Line()
{
    //ctor
}

Line::~Line()
{
    //dtor
}

bool Line::operator == (const Line& rhs)
{
    if (this == &rhs)
    {
        return true;
    }
    else if ((*m_vertices[0] == *rhs.m_vertices[0] && *m_vertices[1] == *rhs.m_vertices[1]) ||
        (*m_vertices[0] == *rhs.m_vertices[1] && *m_vertices[1] == *rhs.m_vertices[0]))
    {
        return true;
    }
    else
    {
        return false;
    }
}

double Line::get_length()
{
    return Vectors::length(m_vertices[1]->get_coords()-
                           m_vertices[0]->get_coords());
}

bool Line::check_vertex(Vertex* vertex_ptr)
{
    for (int i = 0; i < 2; i++)
    {
        if (*vertex_ptr == *m_vertices[i])
        {
            return false;
        }
    }

    Vectors::Vector vct_1 = m_vertices[1]->get_coords() - m_vertices[0]->get_coords();
    Vectors::Vector vct_2 = vertex_ptr->get_coords() - m_vertices[0]->get_coords();
    Vectors::Vector vct_3 = vertex_ptr->get_coords() - m_vertices[1]->get_coords();

    if (!Vectors::is_paral(vct_1, vct_2, 0.01)) // check if both vectors vct_1 and vct_2 are parallel, if so then the point lies either on the line or in its extent
    {
        return false;
    } // if the vectors are parallel, check if the point lies on the line

    double length1 = Vectors::length(vct_1);

    if ((Vectors::length(vct_2) < length1) &&
        (Vectors::length(vct_3) < length1))
    { // if the length of vectors 2 and 3 are both smaller than that of the line, then the point lies on the line
        return true;
    }
    else
    {
        return false;
    }
}

bool Line::check_line_intersect(Line* line_ptr, Vertex* intersect_ptr)
{
    Vectors::Vector v_1 = m_vertices[1]->get_coords() - m_vertices[0]->get_coords();
    Vectors::Vector v_2 = line_ptr->get_vertex_ptr(1)->get_coords() - line_ptr->get_vertex_ptr(0)->get_coords();
    Vectors::Vector v_3 = m_vertices[0]->get_coords() - line_ptr->get_vertex_ptr(0)->get_coords();

    if (Vectors::is_paral(v_1, v_2, 0.01) && Vectors::is_paral(v_1, v_3, 0.01)) // two lines are colinear
    { // there does exist an intersection point, but this will be found somewhere else in the conformation process
        return false;
    }
    else if (Vectors::is_paral(v_1, v_2, 0.01)) // two lines are parallel
    { // no intersection point exists
        return false;
    }
    else if (!Vectors::is_perp(v_3, Vectors::cross(v_1, v_2), 0.01)) // two lines are skew and thus not coplanar
    { // v_3 must be perpendicular to the line orhtogonal to both v_1 and v_2 if not no intersection point exists
        return false;
    }
/*
    Eigen::Matrix3d temp_matrix;

    // if none of the above are true, then there exists a point where vertices v_1 and v_2 intersect (given the points entered above)
    Vectors::Vector v_4 = Vectors::cross(v_1, v_2);

    temp_matrix.col(0) = v_3;
    temp_matrix.col(1) = v_1;
    temp_matrix.col(2) = v_4;

    double s = temp_matrix.determinant() / Vectors::magnitude(v_4);

    temp_matrix.col(1) = v_2;

    double t = -temp_matrix.determinant() / Vectors::magnitude(v_4);
*/
    Vectors::Vector v_4 = Vectors::cross(v_2, v_1);
    Vectors::Vector v_5 = Vectors::cross(v_3, v_1);

    double s = Vectors::length(v_5)/Vectors::length(v_4);

    if (Vectors::same_dir(v_4, Vectors::cross(v_3, v_2), 0.01))
    {
        s *= -1;
    }

    v_4 *= -1;
    v_5 = Vectors::cross(-v_3, v_2);

    double t = Vectors::length(v_5)/Vectors::length(-v_4);

    if (Vectors::same_dir(v_4, Vectors::cross(v_3, v_2), 0.01))
    {
        t *= -1;
    }

    *intersect_ptr = Vertex(m_vertices[0]->get_coords() + s * v_1);

    // now check if the point lies on both lines
    if ((s > 0.01 && s < 0.99) && (t > 0.01 && t < 0.99) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Line::check_deletion()
{
    return m_deletion;
}

bool Line::is_structural()
{
    return m_structural;
}

void Line::tag_deletion()
{
    m_deletion = true;
}

void Line::add_edge(Edge* edge_ptr)
{
    if (std::find(m_edges.begin(), m_edges.end(), edge_ptr) == m_edges.end())
    {
        m_edges.push_back(edge_ptr);
    }
}

void Line::check_associated_members(Vertex* v_check)
{
    for (unsigned int i = 0; i < this->get_edge_count(); i++)
    {
        this->get_edge_ptr(i)->check_vertex(v_check);
        for (unsigned int j = 0; j <  this->get_edge_ptr(i)->get_surface_count(); j++)
        {
            this->get_edge_ptr(i)->get_surface_ptr(j)->check_vertex(v_check);
        }

        for (unsigned int j = 0; j < this->get_edge_ptr(i)->get_space_count(); j++)
        {
            this->get_edge_ptr(i)->get_space_ptr(j)->check_vertex(v_check);
        }
    }
}

void Line::split(Vertex* v_split)
{
    this->tag_deletion();

    Line* l_1 = m_store_ptr->add_line(m_vertices[0], v_split);
    Line* l_2 = m_store_ptr->add_line(m_vertices[1], v_split);

    for (unsigned int i = 0; i < m_edges.size(); i++)
    {
        l_1->add_edge(m_edges[i]); // fill or update m_edges in l_1 with the edge pointers contained in this object
        l_2->add_edge(m_edges[i]); // fill or update m_edges in l_2 with the edge pointers contained in this object
        m_edges[i]->add_line(l_1); // update all edges that this object is referenced to with the new line pointed to by l_1
        m_edges[i]->add_line(l_2); // update all edges that this object is referenced to with the new line pointed to by l_2
        m_edges[i]->delete_line(this); // remove this line from all associated edges, as these have now been updated
    }

    // check the edges, surfaces and spaces associated to this object with the new vertices: v_split
    this->check_associated_members(v_split);

    m_edges.clear(); // clear the vector, this line object now does no longer belong to any edge
}

void Line::make_structural()
{
    m_structural = true;
    for (int i = 0; i < 2; i++)
    {
        m_vertices[i]->make_structural();
    }
}

Vertex_Store* Line::get_store_ptr()
{
    return m_store_ptr;
}

Vertex* Line::get_vertex_ptr(int n)
{
    return m_vertices[n];
}

Vertex* Line::get_center_vertex_ptr()
{
    return &m_center_vertex;
}

Vertex* Line::line_point_closest_to_vertex(Vertex* check_vertex)
{
    Vectors::Vector v_1 = m_vertices[1]->get_coords() - m_vertices[0]->get_coords(); // vector of this line
    Vectors::Vector v_3 = check_vertex->get_coords() - m_vertices[0]->get_coords(); // vector from a point on this line to vertex_ptr
    Vectors::Vector v_2 = Vectors::cross(v_1, v_3); // v_2 is perpendicular to both v_1 and v_3 (normal of the plane containing all points)
    v_2 = Vectors::cross(v_1, v_2); // v_2 is now coplanar with the plane containing all 3 vertices and is perpendicular to v_1

    // now the intersection point of v_1 and v_2 is searched
    double s = ((Vectors::length(Vectors::cross(v_2, v_3))) /
                (Vectors::length(Vectors::cross(v_2, v_1))));

    return m_store_ptr->add_vertex(m_vertices[0]->get_coords() + s * v_1);
}

unsigned int Line::get_edge_count()
{
    return m_edges.size();
}

Edge* Line::get_edge_ptr(unsigned int n)
{
    return m_edges[n];
}

Edge* Line::get_last_edge_ptr()
{
    return m_edges.back();
}

Point* Line::get_point_ptrs(int n)
{
    return m_vertices[n]->get_last_point_ptr();
}

void Line::associate_rectangle(Rectangle* r)
{
    auto it = std::find(m_rectangle_ptrs.begin(), m_rectangle_ptrs.end(), r);

    if (it == m_rectangle_ptrs.end())
    {
        m_rectangle_ptrs.push_back(r);
    }
} // associate_rectangle()

void Line::deassociate_rectangle(Rectangle* r)
{
    auto it = std::find(m_rectangle_ptrs.begin(), m_rectangle_ptrs.end(), r);

    if (it != m_rectangle_ptrs.end())
    {
        m_rectangle_ptrs.erase(it);
    }
    else
    {
        std::cerr << "Request for rectangle_ptr association with line is invalid, exiting now... (Line.cpp)" << std::endl;
        exit(1);
    }
} // deassociate_rectangle()

unsigned int Line::get_rectangle_count()
{
    return m_rectangle_ptrs.size();
} // get_rectangle_count

Rectangle* Line::get_rectangle_ptr(unsigned int i)
{
    return m_rectangle_ptrs[i];
} // get_rectangle_ptr()

void Line::tag_zoned()
{
    zoned = true;
}

void Line::untag_zoned()
{
    zoned = false;
}

bool Line::get_zoned()
{
    return zoned;
}

void Line::tag_constraint()
{
    constraint = true;
}

void Line::untag_constraint()
{
    constraint = false;
}

bool Line::get_constraint()
{
    return constraint;
}

void Line::add_thickness(unsigned int t)
{
    thickness = t;
}

unsigned int Line::get_thickness()
{
    return thickness;
}

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_LINE_CPP
