#ifndef BSO_RECTANGLE_CPP
#define BSO_RECTANGLE_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Rectangle::Rectangle(Line* l_one, Line* l_two, Line* l_three, Line* l_four, Vertex_Store* store_ptr)
{/*
    std::cout << "making a rectangle: " << std::endl;
    std::cout << l_one->get_vertex_ptr(0)->get_coords().transpose() << std::endl;
    std::cout << l_one->get_vertex_ptr(1)->get_coords().transpose() << std::endl << std::endl;

    std::cout << l_two->get_vertex_ptr(0)->get_coords().transpose() << std::endl;
    std::cout << l_two->get_vertex_ptr(1)->get_coords().transpose() << std::endl << std::endl;

    std::cout << l_three->get_vertex_ptr(0)->get_coords().transpose() << std::endl;
    std::cout << l_three->get_vertex_ptr(1)->get_coords().transpose() << std::endl << std::endl;

    std::cout << l_four->get_vertex_ptr(0)->get_coords().transpose() << std::endl;
    std::cout << l_four->get_vertex_ptr(1)->get_coords().transpose() << std::endl << std::endl;
*/
    m_store_ptr = store_ptr;
    m_deletion = false;
    m_structural = false;
    m_lines_associated = false;

    std::fill_n(m_lines, 4, nullptr);
    m_lines[0] = l_one;

    Line* temp_container[3];
    temp_container[0] = l_two;
    temp_container[1] = l_three;
    temp_container[2] = l_four;

    // find the lines parallel and perpendicular to m_lines[0]
    Vectors::Vector v_1 = m_lines[0]->get_vertex_ptr(1)->get_coords() -
                          m_lines[0]->get_vertex_ptr(0)->get_coords();

    for (int i = 0; i < 3; i++)
    {
        Line* temp_ptr = temp_container[i];

        Vectors::Vector v_2 = temp_ptr->get_vertex_ptr(1)->get_coords() -
                              temp_ptr->get_vertex_ptr(0)->get_coords();

        if (Vectors::is_paral(v_1, v_2, 0.01))
        { // line opposite to the first line
            m_lines[2] = temp_ptr;
        }
        else if (Vectors::is_perp(v_1, v_2, 0.01))
        { // line perpendicular to first line
            if ((*m_lines[0]->get_vertex_ptr(0) == *temp_ptr->get_vertex_ptr(0)) ||
                (*m_lines[0]->get_vertex_ptr(0) == *temp_ptr->get_vertex_ptr(1)))
            {
                m_lines[1] = temp_ptr;
            }
            else if ((*m_lines[0]->get_vertex_ptr(1) == *temp_ptr->get_vertex_ptr(0)) ||
                     (*m_lines[0]->get_vertex_ptr(1) == *temp_ptr->get_vertex_ptr(1)))
            {
                m_lines[3] = temp_ptr;
            }
            else
            {
                std::cerr << "Error in initialization of rectangle(a): lines are not adjoint, exiting.." << std::endl;
                exit(1);
            }
        }
        else
        {
            std::cerr << "Error in initialization of rectangle: lines are not perpendicular(b), exiting.." << std::endl;
            exit(1);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (m_lines[i] == nullptr)
        {
            std::cerr << "Error, not all lines were initialised correctly (Rectangle.hpp), exiting.." << std::endl;
            exit(1);
        }
    }



    // all vertices should be all vertices on m_lines[0] and m_lines[2] (the two parallel vertices)
    m_vertices[0] = m_lines[0]->get_vertex_ptr(0);
    m_vertices[1] = m_lines[0]->get_vertex_ptr(1);
    m_vertices[2] = m_lines[2]->get_vertex_ptr(0);
    m_vertices[3] = m_lines[2]->get_vertex_ptr(1);

    { // check if they are ordered clock or counter clockwise, if not order them
        Vectors::Vector v_temp_1 = m_vertices[1]->get_coords() - m_vertices[0]->get_coords();
        Vectors::Vector v_temp_2 = m_vertices[2]->get_coords() - m_vertices[1]->get_coords();

        if (!(Vectors::is_perp(v_temp_1, v_temp_2, 0.001)))
        {
            std::swap(m_vertices[2], m_vertices[3]);
        }

    }


    // check if m_lines[3] and m_lines[1] have their vertices in common with m_lines[2]
    if ((*m_lines[2]->get_vertex_ptr(0) == *m_lines[1]->get_vertex_ptr(0)) ||
        (*m_lines[2]->get_vertex_ptr(0) == *m_lines[1]->get_vertex_ptr(1)))
    {
        if (!((*m_lines[2]->get_vertex_ptr(1) == *m_lines[3]->get_vertex_ptr(0)) ||
              (*m_lines[2]->get_vertex_ptr(1) == *m_lines[3]->get_vertex_ptr(1))))
        {
            std::cerr << "Error in initialization of rectangle(b): lines are not adjoint, exiting.." << std::endl;
            exit(1);
        }
    }
    else if ((*m_lines[2]->get_vertex_ptr(0) == *m_lines[3]->get_vertex_ptr(0)) ||
             (*m_lines[2]->get_vertex_ptr(0) == *m_lines[3]->get_vertex_ptr(1)))
    {
        if (!((*m_lines[2]->get_vertex_ptr(1) == *m_lines[1]->get_vertex_ptr(0)) ||
              (*m_lines[2]->get_vertex_ptr(1) == *m_lines[1]->get_vertex_ptr(1))))
        {
            std::cerr << "Error in initialization of rectangle(c): lines are not adjoint, exiting.." << std::endl;
            exit(1);
        }
    }
    else
    {
        std::cerr << "Error in initialization of rectangle(d): lines are not adjoint, exiting.." << std::endl;
        exit(1);
    }

    // compute the normal vector
    Vectors::Vector v_2 = m_lines[1]->get_vertex_ptr(1)->get_coords() -
                          m_lines[1]->get_vertex_ptr(0)->get_coords();

    m_normal_vector = Vectors::normalise(Vectors::cross(v_1, v_2));

    // calculate the center vertex
    Vectors::Point center;
    center.setZero();
    for (int i = 0; i < 4; i++)
    {
        center += m_vertices[i]->get_coords();
    }
    m_center_vertex = Vertex(center/4.0);
    //ctor
}

Rectangle::Rectangle()
{
    m_lines_associated = false;
    //ctor
}

Rectangle::~Rectangle()
{
    deassociate_lines();
    //dtor
}

bool Rectangle::operator == (const Rectangle& rhs)
{
    if (this == &rhs)
    {
        return true;
    }
    else
    {
        for (int i = 0; i < 4; i++)
        {
            if (m_lines[0] == rhs.m_lines[i])
            {
                for (int j = 0; j < 4; j++)
                {
                    if ((j != i) && (m_lines [1] == rhs.m_lines[j]))
                    {
                        return true; // if two lines of a rectangle surface are coincident, then the entire rectangle is coincident
                    }
                }
            }
        }
        return false;
    }
}

double Rectangle::get_area()
{
    Vectors::Vector v_1 = m_lines[0]->get_vertex_ptr(1)->get_coords() -
                          m_lines[0]->get_vertex_ptr(0)->get_coords();
    Vectors::Vector v_2 = m_vertices[2]->get_coords() - m_vertices[0]->get_coords();
    Vectors::Vector v_3;

    if (Vectors::is_perp(v_1, v_2, 0.01))
    {
        v_2 = m_vertices[3]->get_coords() - m_vertices[0]->get_coords();
        v_3 = m_vertices[2]->get_coords() - m_vertices[1]->get_coords();
    }
    else
    {
        v_3 = m_vertices[3]->get_coords() - m_vertices[1]->get_coords();
    }

    return Vectors::calc_area_quadri_lat(v_2, v_3);
}

bool Rectangle::check_vertex(Vertex* vertex_ptr)
{ // checks if a vertex A is coplanar and inside a rectangle BCDE
	Vectors::Point p_a = vertex_ptr->get_coords();
	Vectors::Point p_b = m_vertices[0]->get_coords();
	Vectors::Point p_c = m_vertices[1]->get_coords();
	Vectors::Point p_d = m_vertices[2]->get_coords();
	Vectors::Point p_e = m_vertices[3]->get_coords();

	if (!Vectors::coplanar(p_b, p_c, p_d, p_a, 0.001))
	{ // if the point is not on the same plane with 3 of the vertices of the rectangle, then it cannot be on the rectangle
		return false;
	}

    for (int i = 0; i < 4; i++) // check all vertices of the rectangle
    {
        if (*vertex_ptr == *m_vertices[i]) // if vertex_ptr is part of the rectangle already
        {
            return false; // then return false
        }
    }
	// check if the point is within any of the two triangles : BCE or DCE
	return (Vectors::inside_triangle(p_b, p_c, p_e, p_a, 0.001) || Vectors::inside_triangle(p_d, p_c, p_e, p_a, 0.001));
}

bool Rectangle::check_line_intersect(Line* line_ptr, Vertex* intersect_ptr)
{
    Vectors::Vector v_1 = line_ptr->get_vertex_ptr(1)->get_coords() -
                          line_ptr->get_vertex_ptr(0)->get_coords(); // directional vector of the line from vertex A to vertex B

    Vectors::Vector v_2 = m_vertices[0]->get_coords() -
                          line_ptr->get_vertex_ptr(0)->get_coords(); // directional vector from vertex A to vertex C (which is a point defining the plane, together with the normal vector of the rectangle)

    if (Vectors::is_perp(v_1, m_normal_vector, 0.01)) // check if the line is orthogonal to the plane normal, if so, the line is parallel to the plane or coplanar
    {
        return false;
    }
    else // if not, then the line or its extension intersects the plane at a point
    {
        // determine the scalar for the vector of the line where the line intersects the plane i.e.: Intersection point = A + s.v_1
        double s = (Vectors::dot(v_2, m_normal_vector)/Vectors::dot(v_1, m_normal_vector));

        // use the Vertex pointer to pass the intersection point which is calculated with the scalar 's'
        *intersect_ptr = Vertex(line_ptr->get_vertex_ptr(0)->get_coords() + (s * v_1));

        // check if that vertex lies within this rectangle and lies on the line that is being checked
        if (this->check_vertex(intersect_ptr) && (s < 1)) // if the point lies within the rectangle and on the line, then the line intersects the rectangle
        {
            return true;
        }
        else // if not the intersection point of the line and the plane of the rectangle lies outside the rectangle or outside the line (i.e. s < 0 or s > 1
        {
            return false;
        }
    }
}

bool Rectangle::check_deletion()
{
    return m_deletion;
}

bool Rectangle::is_structural()
{
    return m_structural;
}

void Rectangle::tag_deletion()
{
    m_deletion = true;
}

void Rectangle::add_surface(Surface* surface_ptr)
{
    if (std::find(m_surfaces.begin(), m_surfaces.end(), surface_ptr) == m_surfaces.end())
    {
        m_surfaces.push_back(surface_ptr);
    }
}

void Rectangle::check_associated_members(Vertex* v_check)
{
    for (unsigned int i = 0; i < this->get_surface_count(); i++)
    {
        for (int j = 0; j < 4; j++)
        {
            this->get_surface_ptr(i)->get_edge_ptr(j)->check_vertex(v_check);
        }
        this->get_surface_ptr(i)->check_vertex(v_check);

        for (unsigned int j = 0; j < this->get_surface_ptr(i)->get_space_count(); j++)
        {
            this->get_surface_ptr(i)->get_space_ptr(j)->check_vertex(v_check);
        }
    }
}

void Rectangle::split(Vertex* v_split)
{
    this->tag_deletion();

    for (int i = 0; i < 4; i++) // if the vertex lies on one of the rectangle's lines, then the rectangle is to be split into two new rectangles
    {
        if (m_lines[i]->check_vertex(v_split))
        { // if the vertex lies on the line, then split the line
            m_lines[i]->tag_deletion(); // tag the line for deletion
            Vertex* v_1 = nullptr, * v_2 = nullptr, * v_3 = nullptr, * v_4 = nullptr, * v_5 = nullptr; // initialise these to nullptrs to suppress a warning about the use of uninitialized variables
            Line* l_1 = nullptr, * l_2 = nullptr, * l_3, * l_4, * l_5, * l_6, * l_7;
            Rectangle * r_1, * r_2;

            v_1 = m_lines[i]->get_vertex_ptr(0);
            v_2 = m_lines[i]->get_vertex_ptr(1);

            // find v_1 and v_2
            for (int j = 0; j < 4; j++) // compare properties of m_line[0] with those of other lines of the rectangle
            {
                if (i != j)
                {
                    if ((*m_lines[j]->get_vertex_ptr(0) == *v_1) || (*m_lines[j]->get_vertex_ptr(1) == *v_1))
                    { // line j is connected to line i in a vertex
                        l_1 = m_lines[j];
                        if (*m_lines[j]->get_vertex_ptr(0) == *v_1)
                        {
                            v_3 = m_lines[j]->get_vertex_ptr(1);
                        }
                        else
                        {
                            v_3 = m_lines[j]->get_vertex_ptr(0);
                        }
                    }
                    else if ((*m_lines[j]->get_vertex_ptr(0) == *v_2) || (*m_lines[j]->get_vertex_ptr(1) == *v_2))
                    { // line j is connected to line i in a vertex
                        l_2 = m_lines[j];
                        if (*m_lines[j]->get_vertex_ptr(0) == *v_2)
                        {
                            v_4 = m_lines[j]->get_vertex_ptr(1);
                        }
                        else
                        {
                            v_4 = m_lines[j]->get_vertex_ptr(0);
                        }
                    }
                    else
                    { // line j is the opposite line to line i, and harbours a new point (v_5)
                        m_lines[j]->tag_deletion();
                        v_5 = m_lines[j]->line_point_closest_to_vertex(v_split);
                    }
                }
            }

            if ((v_1 == nullptr) || (v_2 == nullptr) || (v_3 == nullptr) || (v_4 == nullptr) || (v_5 == nullptr))
            {
                std::cerr << "Error, not all vertices were initialized during the splitting of a rectangle(a), exiting..." << std::endl;
                exit(1);
            }

            if ((l_1 == nullptr) || (l_2 == nullptr))
            {
                std::cerr << "Error, not all lines were initialized during the splitting of a rectangle (a), exiting..." << std::endl;
                exit(1);
            }

            l_3 = m_store_ptr->add_line(v_5, v_split);
            l_4 = m_store_ptr->add_line(v_3, v_5);
            l_5 = m_store_ptr->add_line(v_4, v_5);
            l_6 = m_store_ptr->add_line(v_1, v_split);
            l_7 = m_store_ptr->add_line(v_2, v_split);

            r_1 = m_store_ptr->add_rectangle(l_1, l_3, l_4, l_6);
            r_2 = m_store_ptr->add_rectangle(l_2, l_3, l_5, l_7);

            for (unsigned int i = 0; i < m_surfaces.size(); i++)
            {
                r_1->add_surface(m_surfaces[i]); // fill or update m_surfaces in r_1 with the surface pointers contained in this object
                r_2->add_surface(m_surfaces[i]); // fill or update m_surfaces in r_2 with the surface pointers contained in this object
                m_surfaces[i]->add_rectangle(r_1); // update all surfaces that this object is referenced to with the new rectangle pointed to by r_1
                m_surfaces[i]->add_rectangle(r_2); // update all surfaces that this object is referenced to with the new rectangle pointed to by r_2
                m_surfaces[i]->delete_rectangle(this); // remove this rectangle from all associated surfaces, as these have now been updated
            }

            // check the edges, surfaces and spaces associated to this object with the new vertices: v_5 and v_split
            this->check_associated_members(v_5);
            this->check_associated_members(v_split);

            m_surfaces.clear(); // clear the vector, this rectangle object now does no longer belong to any surface
            return;
        }
    }
    // if none of the above applies, then the vertex lies on the rectangle, and the rectangle is to be split into four new rectangles:
    for (int i = 0; i < 4; i++) // tag all lines for deletion
    {
        m_lines[i]->tag_deletion();
    }

    // initialize the vertices, lines and rectangles required to define the four new rectangles
    Vertex* v_1 = nullptr, * v_2 = nullptr, * v_3 = nullptr, * v_4 = nullptr,
          * v_5 = nullptr, * v_6 = nullptr, * v_7 = nullptr, * v_8 = nullptr; // initialise these to nullptrs to suppress a warning about the use of uninitialized variables
            Line* l_1, * l_2, * l_3, * l_4, * l_5, * l_6, * l_7, * l_8, * l_9, * l_10, * l_11, * l_12;
            Rectangle * r_1, * r_2, * r_3, * r_4;

    v_1 = m_lines[0]->get_vertex_ptr(0);
    v_2 = m_lines[0]->get_vertex_ptr(1);
    v_5 = m_lines[0]->line_point_closest_to_vertex(v_split);


    for (int i = 1; i < 4; i++) // compare properties of m_line[0] with those of other lines of the rectangle
    {
        if ((*m_lines[i]->get_vertex_ptr(0) == *v_1) || (*m_lines[i]->get_vertex_ptr(1) == *v_1))
        { // line j is connected to line i
            v_7 = m_lines[i]->line_point_closest_to_vertex(v_split);
            if (*m_lines[i]->get_vertex_ptr(0) == *v_1)
            {
                v_3 = m_lines[i]->get_vertex_ptr(1);
            }
            else
            {
                v_3 = m_lines[i]->get_vertex_ptr(0);
            }
        }
        else if ((*m_lines[i]->get_vertex_ptr(0) == *v_2) || (*m_lines[i]->get_vertex_ptr(1) == *v_2))
        { // line j is connected to line i
            v_8 = m_lines[i]->line_point_closest_to_vertex(v_split);
            if (*m_lines[i]->get_vertex_ptr(0) == *v_2)
            {
                v_4 = m_lines[i]->get_vertex_ptr(1);
            }
            else
            {
                v_4 = m_lines[i]->get_vertex_ptr(0);
            }
        }
        else
        { // line j is opposite to line i
            v_6 = m_lines[i]->line_point_closest_to_vertex(v_split);
        }
    }

    if ((v_1 == nullptr) || (v_2 == nullptr) || (v_3 == nullptr) || (v_4 == nullptr) ||
        (v_5 == nullptr) || (v_6 == nullptr) || (v_7 == nullptr) || (v_8 == nullptr))
    {
        std::cerr << "Error, not all vertices were initialized during the splitting of a rectangle (b), exiting..." << std::endl;
        exit(1);
    }

    l_1  = m_store_ptr->add_line(v_1, v_7);
    l_2  = m_store_ptr->add_line(v_7, v_3);
    l_3  = m_store_ptr->add_line(v_1, v_5);
    l_4  = m_store_ptr->add_line(v_7, v_split);
    l_5  = m_store_ptr->add_line(v_3, v_6);
    l_6  = m_store_ptr->add_line(v_5, v_split);
    l_7  = m_store_ptr->add_line(v_6, v_split);
    l_8  = m_store_ptr->add_line(v_5, v_2);
    l_9  = m_store_ptr->add_line(v_8, v_split);
    l_10 = m_store_ptr->add_line(v_6, v_4);
    l_11 = m_store_ptr->add_line(v_2, v_8);
    l_12 = m_store_ptr->add_line(v_8, v_4);

    r_1 = m_store_ptr->add_rectangle(l_1,  l_3,  l_4,  l_6 );
    r_2 = m_store_ptr->add_rectangle(l_2,  l_4,  l_5,  l_7 );
    r_3 = m_store_ptr->add_rectangle(l_6,  l_8,  l_9,  l_11);
    r_4 = m_store_ptr->add_rectangle(l_7,  l_9,  l_10, l_12);

    for (unsigned int i = 0; i < m_surfaces.size(); i++)
    {
        r_1->add_surface(m_surfaces[i]); // fill or update m_surfaces in r_1 with the surface pointers contained in this object
        r_2->add_surface(m_surfaces[i]); // fill or update m_surfaces in r_2 with the surface pointers contained in this object
        r_3->add_surface(m_surfaces[i]); // fill or update m_surfaces in r_3 with the surface pointers contained in this object
        r_4->add_surface(m_surfaces[i]); // fill or update m_surfaces in r_4 with the surface pointers contained in this object
        m_surfaces[i]->add_rectangle(r_1); // update all surfaces that this object is referenced to with the new rectangle pointed to by r_1
        m_surfaces[i]->add_rectangle(r_2); // update all surfaces that this object is referenced to with the new rectangle pointed to by r_2
        m_surfaces[i]->add_rectangle(r_3); // update all surfaces that this object is referenced to with the new rectangle pointed to by r_3
        m_surfaces[i]->add_rectangle(r_4); // update all surfaces that this object is referenced to with the new rectangle pointed to by r_4
        m_surfaces[i]->delete_rectangle(this); // remove this rectangle from the associated surface, as it has now been updated
    }

    // check the edges, surfaces and spaces associated to this object with the new vertices: v_5 and v_split
    this->check_associated_members(v_5);
    this->check_associated_members(v_6);
    this->check_associated_members(v_7);
    this->check_associated_members(v_8);
    this->check_associated_members(v_split);

    m_surfaces.clear(); // clear the vector, this rectangle object now does no longer belong to any surface
    return;

}

void Rectangle::make_structural()
{
    m_structural = true;
    for (int i = 0; i < 4; i++)
    {
        m_lines[i]->make_structural();
    }
}

void Rectangle::make_not_structural()
{
    m_structural = false;
}

void Rectangle::associate_lines()
{
    m_lines_associated = true;
    for (unsigned int i = 0; i < 4; i++)
    {
        m_lines[i]->associate_rectangle(this);
    }
} // associate_lines()

void Rectangle::deassociate_lines()
{
    if(m_lines_associated)
    for (unsigned int i = 0; i < 4; i++)
    {
        m_lines[i]->deassociate_rectangle(this);
    }
    m_lines_associated = false;
} // deassociate_lines()

Vertex_Store* Rectangle::get_store_ptr()
{
    return m_store_ptr;
}

Vectors::Vector* Rectangle::get_normal_ptr()
{
    return &m_normal_vector;
}

Vertex* Rectangle::get_vertex_ptr(int n)
{
    return m_vertices[n];
}

Vertex* Rectangle::get_max_vertex()
{
    Vectors::Point max = m_vertices[0]->get_coords();
    Vectors::Point check;
    int index = 0;
    for (int i = 1; i < 4; i++)
    {
        check = m_vertices[i]->get_coords();
        if ((max(0) < check(0)) ||
            (max(1) < check(1)) ||
            (max(2) < check(2)))
        { // only works for orhtogonal cases
            max = check;
            index = i;
        }
    }
    return m_vertices[index];
}

Vertex* Rectangle::get_min_vertex()
{
    Vectors::Point min = m_vertices[0]->get_coords();
    Vectors::Point check;
    int index = 0;
    for (int i = 1; i < 4; i++)
    {
        check = m_vertices[i]->get_coords();
        if ((min(0) > check(0)) ||
            (min(1) > check(1)) ||
            (min(2) > check(2)))
        { // only works for orhtogonal cases
            min = check;
            index = i;
        }
    }
    return m_vertices[index];
}

Line* Rectangle::get_line_ptr(int n)
{
    return m_lines[n];
}

Vertex* Rectangle::get_center_vertex_ptr()
{
    return &m_center_vertex;
}

Vertex* Rectangle::surf_point_closest_vertex(Vertex* vertex_ptr)
{
    Vectors::Vector v_1 = m_vertices[0]->get_coords() - vertex_ptr->get_coords(); // directional vector of the line from a point on the rectangle to the vertex
    Vectors::Vector v_2; // will be vector pointing toward direction of face on which the vertex is located

    int index_max_abs; // will store the index in which the normal has the largest increment (abs value)
    m_normal_vector.cwiseAbs().maxCoeff(&index_max_abs); // find the index

    if ((v_1(index_max_abs) >= 0) ^ (m_normal_vector(index_max_abs) < 0))
    { // if both vectors have the same sign in the direction in which the normal has the biggest increment

        v_2 = -1 * m_normal_vector; // v_2 is facing opposite to the normal
    }
    else
    {
        v_2 = m_normal_vector; // v_2 is normal
    }

    // determine the scalar for the vector of the line where the line intersects the plane i.e.: Intersection point = A + s.v_1
    double s = (Vectors::dot(v_1, v_2)) / (Vectors::dot(v_2, v_2));

    //compute the point with scalar s and add it to the vertex store
    return m_store_ptr->add_vertex(vertex_ptr->get_coords() + s * v_2);
}

Surface* Rectangle::get_last_surface_ptr() // dangerous?
{
    return m_surfaces.back();
}

unsigned int Rectangle::get_surface_count()
{
    return m_surfaces.size();
}

Surface* Rectangle::get_surface_ptr(unsigned int n)
{
    return m_surfaces[n];
}

Edge* Rectangle::get_edge_ptr(int n)
{
    return m_lines[n]->get_last_edge_ptr();
}

Point* Rectangle::get_point_ptrs(int n)
{
    return m_vertices[n]->get_last_point_ptr();
}

void Rectangle::tag_zoned()
{
    zoned = true;
}

void Rectangle::untag_zoned()
{
    zoned = false;
}

bool Rectangle::get_zoned()
{
    return zoned;
}

void Rectangle::tag_horizontal()
{
    horizontal = true;
}

void Rectangle::untag_horizontal()
{
    horizontal = false;
}

bool Rectangle::get_horizontal()
{
    return horizontal;
}

void Rectangle::add_thickness(unsigned int t)
{
    thickness = t;
}

unsigned int Rectangle::get_thickness()
{
    return thickness;
}

void Rectangle::add_loading(double l)
{
    loading = l;
}

double Rectangle::get_loading()
{
    return loading;
}

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_RECTANGLE_CPP
