#ifndef BSO_CUBOID_CPP
#define BSO_CUBOID_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Cuboid::Cuboid(Rectangle* one, Rectangle* two, Rectangle* three, Rectangle* four, Rectangle* five, Rectangle* six, Vertex_Store* store_ptr)
{
    m_store_ptr = store_ptr;
    m_deletion = false;

    std::fill_n(m_rectangles, 6, nullptr);
    std::fill_n(m_lines, 12, nullptr);
    std::fill_n(m_vertices, 8, nullptr);
    m_rectangles[0] = one;

    Rectangle* t_r[5];
    t_r[0] = two;
    t_r[1] = three;
    t_r[2] = four;
    t_r[3] = five;
    t_r[4] = six;

    Vectors::Vector vct_1 = *m_rectangles[0]->get_normal_ptr();
    Vectors::Vector vct_2 = m_rectangles[0]->get_line_ptr(0)->get_vertex_ptr(1)->get_coords() -
                            m_rectangles[0]->get_line_ptr(0)->get_vertex_ptr(0)->get_coords();

    Vectors::Vector vct_3;

    for (int i = 1; i < 4; i++) // find a line perpendicular to vct_2 and assign it to vct_3
    {
        vct_3 = m_rectangles[0]->get_line_ptr(i)->get_vertex_ptr(1)->get_coords() -
                m_rectangles[0]->get_line_ptr(i)->get_vertex_ptr(0)->get_coords();

        if(Vectors::is_perp(vct_3, vct_2, 0.01))
        {
            break;
        }
    }

    for (int i = 0; i < 5; i++) // determine the position of all the other rectangles
    {
        Vectors::Vector vct_4 = *t_r[i]->get_normal_ptr();
        if (Vectors::is_paral(vct_4, vct_1, 0.01))
        { // rectangle opposite to rectangle 0
            m_rectangles[5] = t_r[i];
        }
        else if (Vectors::is_perp(vct_4, vct_2, 0.01))
        { // two rectangles opposite to each other, adjacent to rectangle 0
            if (m_rectangles[1] == nullptr)
            {
                m_rectangles[1] = t_r[i];
            }
            else
            {
                m_rectangles[4] = t_r[i];
            }
        }
        else if (Vectors::is_perp(vct_4, vct_3, 0.01))
        { // two rectangles opposite to each other, adjacent to rectangle 0
            if (m_rectangles[2] == nullptr)
            {
                m_rectangles[2] = t_r[i];
            }
            else
            {
                m_rectangles[3] = t_r[i];
            }
        }
        else
        {
            std::cerr << "Error, rectangle not orthogonal during initialisation of a cuboid" << std::endl;
            exit(1);
        }
    }

    for (int i = 0; i < 6; i++)
    {
        if (m_rectangles[i] == nullptr)
        {
            std::cerr << "Too few rectangles were assigned to a cuboid during initialisation, exiting.." << std::endl;
            exit(1);
        }
    }

    // add lines to m_lines
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 12; k++)
            {
                if (m_lines[k] == nullptr)
                {
                    m_lines[k] = m_rectangles[i]->get_line_ptr(j); // it not yet in the array, add it and continue with the next vertex to be added
                    break; // it is already in the array, continue with the next vertex to be added
                }
                else if (*m_lines[k] == *m_rectangles[i]->get_line_ptr(j))
                {
                    break; // it is already in the array, continue with the next vertex to be added
                }
                else if (k == 11)
                {
                    std::cerr << "Attempted to add too many lines to a cuboid, exiting..." << std::endl;
                    exit(1);
                }
            }
        }
    }

    if (m_lines[11] == nullptr)
    {
        std::cerr << "Not enough lines have been added to a cuboid during initialisation, exiting..." << std::endl;
        exit(1); // if this occurs, then probably some lines are or are not coincident
    }

    // add vertices to m_vertices
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 8; k++)
            {
                if (m_vertices[k] == nullptr)
                {
                    m_vertices[k] = m_rectangles[i]->get_vertex_ptr(j); // it not yet in the array, add it and continue with the next vertex to be added
                    break;
                }
                else if (*m_vertices[k] == *m_rectangles[i]->get_vertex_ptr(j))
                {
                    break; // it is already in the array, continue with the next vertex to be added
                }
                else if (k == 7)
                {
                    std::cerr << "Attempted to add too many vertices to a cuboid, exiting..." << std::endl;
                    exit(1);
                }
            }
        }
    }
    if (m_vertices[7] == nullptr)
    {
        std::cerr << "Not enough vertices have been added to a cuboid during initialisation, exiting..." << std::endl;
        exit(1); // if this occurs, then probably some vertices are or are not coincident
    }

    // calculate the center vertex
    Vectors::Point center;
    center.setZero();
    for (int i = 0; i < 8; i++)
    {
        center += m_vertices[i]->get_coords();
    }

    m_center_vertex = Vertex(center / 8.0);
    //ctor
}

Cuboid::Cuboid()
{
    //ctor
}

Cuboid::~Cuboid()
{
    //dtor
}

bool Cuboid::operator == (const Cuboid& rhs)
{
    if (this == &rhs)
    {
        return true;
    }
    else
    {
        for (int i = 0; i < 6; i++)
        {
            if (*m_rectangles[0] == *rhs.m_rectangles[i])
            {
                for (int j = 0; j < 6; j++)
                {
                    if ((j != i) && (*m_rectangles[1] == *rhs.m_rectangles[j]))
                    {
                        for (int k = 0; k < 6; k++)
                        {
                            if ((k != j && k!= i) && (*m_rectangles[2] == *rhs.m_rectangles[k]))
                            {
                                return true; // if three surfaces of a cube are coincident then the entire cube is coincident
                            }
                        }
                    }
                }
            }
        }
        return false;
    }
}

double Cuboid::get_volume()
{
    Vectors::Vector vct_1 = m_lines[0]->get_vertex_ptr(1)->get_coords() -
                            m_lines[0]->get_vertex_ptr(0)->get_coords();
    Vectors::Vector vct_2;
    for (int i = 0; i < 6; i++)
    {
        vct_2 = *m_rectangles[i]->get_normal_ptr();
        if (Vectors::is_paral(vct_1, vct_2, 0.01))
        {
            return (m_rectangles[i]->get_area() * m_lines[0]->get_length());
        }
    }
    std::cerr << "Error in calculating Cuboid volume, exiting..." << std::endl;
    exit(1);
}

bool Cuboid::check_vertex(Vertex* vertex_ptr)
{
    for (int i = 0; i < 8; i++)
    {
        if (*vertex_ptr == *m_vertices[i])
        {
            return false;
        }
    }

	Eigen::MatrixXi lower, upper;
	lower.setZero(5,3);
	upper.setZero(5,3);

	for (auto i = 0; i < 5; i++)
	{
		Vectors::Vector check_v  = vertex_ptr->get_coords() - m_vertices[i]->get_coords();
		for (auto j = 0; j < 3; j++)
		{
			if (check_v(j) > 0) upper(i,j) = 1;
			if (check_v(j) < 0) lower(i,j) = 1;
		}
	}

	Eigen::VectorXi checks(6);
	for (auto i = 0; i < 3; i++)
	{
		checks(i)   = upper.col(i).prod();
		checks(i+3) = lower.col(i).prod();
	}

	return !checks.sum();
}

bool Cuboid::check_deletion()
{
    return m_deletion;
}

void Cuboid::tag_deletion()
{
    m_deletion = true;
}

void Cuboid::add_space(Space* space_ptr)
{
    if (std::find(m_spaces.begin(), m_spaces.end(), space_ptr) == m_spaces.end())
    {
        m_spaces.push_back(space_ptr);
    }
}

void Cuboid::check_associated_members(Vertex* v_check)
{
    for (unsigned int i = 0; i < this->get_space_count(); i++)
    {
        for (int j = 0; j < 12; j++)
        {
            this->get_space_ptr(i)->get_edge_ptr(j)->check_vertex(v_check);
        }
        for (int j = 0; j < 6; j++)
        {
            this->get_space_ptr(i)->get_surface_ptr(j)->check_vertex(v_check);
        }
        this->get_space_ptr(i)->check_vertex(v_check);
    }
}

void Cuboid::split(Vertex* v_split)
{
    this->tag_deletion();

    for (int i = 0; i < 6; i++) // if the vertex lies on one of the cuboids lines, then the cuboid is to be split into two new cuboids
    { // still need to tag rectangles for deletion
        if (m_rectangles[i]->check_vertex(v_split))
        {
            Rectangle* src_rec = m_rectangles[i];

            for (int j = 0; j < 4; j++)
            {
                if (src_rec->get_line_ptr(j)->check_vertex(v_split))
                {
                    Line* src_line = m_rectangles[i]->get_line_ptr(j);

                    Vertex* v[12];
                    std::fill_n(v, 12, nullptr);
                    Line* l[20];
                    Rectangle* r[11];
                    Cuboid* c[2];

                    v[0] = src_line->get_vertex_ptr(0);
                    v[2] = v_split;
                    v[4] = src_line->get_vertex_ptr(1);

                    Vectors::Vector vct_1 = v[4]->get_coords() - v[0]->get_coords();
                    Vectors::Vector vct_2;

                    for (int k = 0; k < 4; k++) // find the line opposite to temp_line_ptr
                    {
                        Line* temp_ptr = src_rec->get_line_ptr(k);
                        vct_2 = temp_ptr->get_vertex_ptr(1)->get_coords() -
                                temp_ptr->get_vertex_ptr(0)->get_coords(); // compute a vector of m_lines[k]
                        if ((j != k) && (Vectors::is_paral(vct_1, vct_2, 0.01))) // if a line with a different index than j is parallel to that indexed with j
                        {
                            v[3] = temp_ptr->line_point_closest_to_vertex(v_split);
                        }
                        else if (j != k)
                        {
                            if ((*v[0] == *temp_ptr->get_vertex_ptr(0)) ||
                                (*v[0] == *temp_ptr->get_vertex_ptr(1)))
                            {
                                if (*v[0] == *temp_ptr->get_vertex_ptr(0))
                                {
                                    v[1] = temp_ptr->get_vertex_ptr(1);
                                }
                                else
                                {
                                    v[1] = temp_ptr->get_vertex_ptr(0);
                                }
                            }
                            else
                            {
                                if (*v[4] == *temp_ptr->get_vertex_ptr(0))
                                {
                                    v[5] = temp_ptr->get_vertex_ptr(1);
                                }
                                else
                                {
                                    v[5] = temp_ptr->get_vertex_ptr(0);
                                }
                            }
                        }
                    }

                    if ((v[1] == nullptr) || (v[3] == nullptr) || (v[5] == nullptr))
                    {
                        std::cerr << "Error, not all vertices were initialized during the splitting of a cuboid(a), exiting..." << std::endl;
                        exit(1);
                    }


                    vct_2 = *src_rec->get_normal_ptr();
                    Vectors::Vector vct_3;
                    for (int k = 0; k < 6; k ++)
                    {
                        vct_3 = *m_rectangles[k]->get_normal_ptr();
                        if ((i != k) && (Vectors::is_paral(vct_2, vct_3, 0.01))) // if a plane with a different index than i is parallel to the plane with index i
                        {
                            v[6]  = m_rectangles[k]->surf_point_closest_vertex(v[0]);
                            v[7]  = m_rectangles[k]->surf_point_closest_vertex(v[1]);
                            v[8]  = m_rectangles[k]->surf_point_closest_vertex(v[2]);
                            v[9]  = m_rectangles[k]->surf_point_closest_vertex(v[3]);
                            v[10] = m_rectangles[k]->surf_point_closest_vertex(v[4]);
                            v[11] = m_rectangles[k]->surf_point_closest_vertex(v[5]);
                            break;
                        }
                        else if (k == 5)
                        {
                            std::cerr << "Error, could not find opposite plane while initialising cuboid, exiting..." << std::endl;
                            exit(1);
                        }
                    }

                    if ((v[6] == nullptr) || (v[7] == nullptr) || (v[8] == nullptr) || (v[9] == nullptr) || (v[10] == nullptr) || (v[11] == nullptr))
                    {
                        std::cerr << "Error, not all vertices were initialized during the splitting of a cuboid(b), exiting..." << std::endl;
                        exit(1);
                    }

                    for (int k = 0; k < 12; k++)
                    {
                        vct_3 = m_lines[k]->get_vertex_ptr(1)->get_coords() -
                                m_lines[k]->get_vertex_ptr(0)->get_coords();

                        if (Vectors::is_paral(vct_1, vct_3, 0.01))
                        {
                            m_lines[k]->tag_deletion(); // if the line is parallel to the source line, then it will be split and it needs to be tagged for deletion
                        }
                    }

                    for (int k = 0; k < 6; k++)
                    {
                        vct_3 = *m_rectangles[k]->get_normal_ptr();
                        if (!Vectors::is_paral(vct_1, vct_3, 0.01))
                        {
                            m_rectangles[k]->tag_deletion(); // if the rectangles normal is perpendicular to the source line, then it will be split and it needs to be tagged for deletion
                        }
                    }

                    l[0]  = m_store_ptr->add_line(v[0], v[1]);
                    l[1]  = m_store_ptr->add_line(v[0], v[2]);
                    l[2]  = m_store_ptr->add_line(v[1], v[3]);
                    l[3]  = m_store_ptr->add_line(v[2], v[3]);
                    l[4]  = m_store_ptr->add_line(v[2], v[4]);
                    l[5]  = m_store_ptr->add_line(v[3], v[5]);
                    l[6]  = m_store_ptr->add_line(v[4], v[5]);

                    l[7]  = m_store_ptr->add_line(v[6], v[7]);
                    l[8]  = m_store_ptr->add_line(v[6], v[8]);
                    l[9]  = m_store_ptr->add_line(v[7], v[9]);
                    l[10] = m_store_ptr->add_line(v[8], v[9]);
                    l[11] = m_store_ptr->add_line(v[8], v[10]);
                    l[12] = m_store_ptr->add_line(v[9], v[11]);
                    l[13] = m_store_ptr->add_line(v[10], v[11]);

                    l[14] = m_store_ptr->add_line(v[0], v[6]);
                    l[15] = m_store_ptr->add_line(v[1], v[7]);
                    l[16] = m_store_ptr->add_line(v[2], v[8]);
                    l[17] = m_store_ptr->add_line(v[3], v[9]);
                    l[18] = m_store_ptr->add_line(v[4], v[10]);
                    l[19] = m_store_ptr->add_line(v[5], v[11]);


                    r[0]  = m_store_ptr->add_rectangle(l[0],  l[1],  l[2],  l[3]);
                    r[1]  = m_store_ptr->add_rectangle(l[3],  l[4],  l[5],  l[6]);

                    r[2]  = m_store_ptr->add_rectangle(l[7],  l[8],  l[9],  l[10]);
                    r[3]  = m_store_ptr->add_rectangle(l[10], l[11], l[12], l[13]);

                    r[4]  = m_store_ptr->add_rectangle(l[0],  l[7],  l[14], l[15]);
                    r[5]  = m_store_ptr->add_rectangle(l[1],  l[8],  l[14], l[16]);
                    r[6]  = m_store_ptr->add_rectangle(l[2],  l[9],  l[15], l[17]);
                    r[7]  = m_store_ptr->add_rectangle(l[3],  l[10], l[16], l[17]);
                    r[8]  = m_store_ptr->add_rectangle(l[4],  l[11], l[16], l[18]);
                    r[9]  = m_store_ptr->add_rectangle(l[5],  l[12], l[17], l[19]);
                    r[10] = m_store_ptr->add_rectangle(l[6],  l[13], l[18], l[19]);


                    c[0] = m_store_ptr->add_cuboid(r[0], r[2], r[4], r[5], r[6], r[7]);
                    c[1] = m_store_ptr->add_cuboid(r[1], r[3], r[7], r[8], r[9], r[10]);

                    for (unsigned int k = 0; k < m_spaces.size(); k++)
                    {
                        for (int l = 0; l < 2; l++)
                        {
                            c[l]->add_space(m_spaces[k]); // fill or update m_spaces in c[l] with the space pointers contained in this object
                            m_spaces[k]->add_cuboid(c[l]); // update all spaces that this object is referenced to with the new cuboid pointed to by c[l]
                        }
                        m_spaces[k]->delete_cuboid(this); // remove this cuboid from all associated spaces, as these have now been updated
                    }

                    // check the edges, surfaces and spaces associated to this object with the new vertices: v_09, v_10, v_11 and v_split
                    this->check_associated_members(v_split);
                    for (int k = 0; k < 12; k++)
                    {
                        this->check_associated_members(v[k]);
                    }

                    m_spaces.clear();
                    return;
                }
            }
        }
    }
    for (int i = 0; i < 6; i++) // if the vertex lies on one of the cuboids rectangles, then the cuboid is to be split into four new cuboids
    {
        if (m_rectangles[i]->check_vertex(v_split))
        {
            Vertex* v[17];
            std::fill_n(v, 17, nullptr);
            Line* l[33];
            Rectangle* r[20];
            Cuboid* c[4];

            Line* src_line = m_rectangles[i]->get_line_ptr(0);

            v[0] = src_line->get_vertex_ptr(0);
            v[1] = src_line->get_vertex_ptr(1);
            v[9] = src_line->line_point_closest_to_vertex(v_split);

            Vectors::Vector vct_1 = v[1]->get_coords() - v[0]->get_coords();
            Vectors::Vector vct_2;
            for (int j = 1; j < 4; j++)
            {
                Line* temp_ptr = m_rectangles[i]->get_line_ptr(j);
                vct_2 = temp_ptr->get_vertex_ptr(1)->get_coords() -
                        temp_ptr->get_vertex_ptr(0)->get_coords();

                if (Vectors::is_paral(vct_1, vct_2, 0.01)) // if line j is parallel to line i
                {
                    v[10] = temp_ptr->line_point_closest_to_vertex(v_split);
                }
                else // else it is perpendicular
                {
                    if ((*v[0] == *temp_ptr->get_vertex_ptr(0)) || (*v[0] == *temp_ptr->get_vertex_ptr(1)))
                    {
                        v[11] = temp_ptr->line_point_closest_to_vertex(v_split);
                        if (*v[0] == *temp_ptr->get_vertex_ptr(0))
                        {
                            v[2] = temp_ptr->get_vertex_ptr(1);
                        }
                        else
                        {
                            v[2] = temp_ptr->get_vertex_ptr(0);
                        }
                    }
                    else
                    {
                        v[12] = temp_ptr->line_point_closest_to_vertex(v_split);
                        if (*v[1] == *temp_ptr->get_vertex_ptr(0))
                        {
                            v[3] = temp_ptr->get_vertex_ptr(1);
                        }
                        else
                        {
                            v[3] = temp_ptr->get_vertex_ptr(0);
                        }
                    }
                }
            }

            if ((v[0] == nullptr) || (v[1] == nullptr) || (v[2] == nullptr) || (v[3] == nullptr) ||
                (v[9] == nullptr) || (v[10] == nullptr) || (v[11] == nullptr)|| (v[12] == nullptr))
            {
                std::cerr << "Error, not all vertices were initialized during the splitting of a cuboid(c), exiting..." << std::endl;
                exit(1);
            }

            vct_2 = *m_rectangles[i]->get_normal_ptr();
            Vectors::Vector vct_3;
            for (int j = 0; j < 6; j ++)
            {
                vct_3 = *m_rectangles[j]->get_normal_ptr();
                if ((i != j) && (Vectors::is_paral(vct_2, vct_3, 0.01))) // if a plane with a different index than i is parallel to the plane with index i
                {
                    v[4]  = m_rectangles[j]->surf_point_closest_vertex(v[0]);
                    v[5]  = m_rectangles[j]->surf_point_closest_vertex(v[2]);
                    v[6]  = m_rectangles[j]->surf_point_closest_vertex(v[3]);
                    v[7]  = m_rectangles[j]->surf_point_closest_vertex(v[1]);
                    v[8]  = m_rectangles[j]->surf_point_closest_vertex(v_split);
                    v[13] = m_rectangles[j]->surf_point_closest_vertex(v[9]);
                    v[14] = m_rectangles[j]->surf_point_closest_vertex(v[10]);
                    v[15] = m_rectangles[j]->surf_point_closest_vertex(v[11]);
                    v[16] = m_rectangles[j]->surf_point_closest_vertex(v[12]);
                }
            }

            if ((v[4] == nullptr) || (v[5] == nullptr) || (v[6] == nullptr) || (v[7] == nullptr) ||
                (v[8] == nullptr) || (v[13] == nullptr) || (v[14] == nullptr)|| (v[15] == nullptr)|| (v[16] == nullptr))
            {
                std::cerr << "Error, not all vertices were initialized during the splitting of a cuboid(d), exiting..." << std::endl;
                exit(1);
            }

            for (int j = 0; j < 12; j++)
            {
                vct_3 = m_lines[j]->get_vertex_ptr(1)->get_coords() -
                        m_lines[j]->get_vertex_ptr(0)->get_coords();

                if (Vectors::is_perp(vct_2, vct_3, 0.01))
                {
                    m_lines[j]->tag_deletion(); // if the line is perpendicular to the normal of the rectangle on which v_split lies, then it will be split and it needs to be tagged for deletion
                }
            }

            for (int j = 0; j < 6; j++)
            {
                m_rectangles[j]->tag_deletion();
            }

            l[0]  = m_store_ptr->add_line(v[0],  v[4]);
            l[1]  = m_store_ptr->add_line(v[11], v[15]);
            l[2]  = m_store_ptr->add_line(v[2],  v[5]);
            l[3]  = m_store_ptr->add_line(v[9] , v[13]);
            l[4]  = m_store_ptr->add_line(v[8],  v_split);
            l[5]  = m_store_ptr->add_line(v[10], v[14]);
            l[6]  = m_store_ptr->add_line(v[1],  v[7]);
            l[7]  = m_store_ptr->add_line(v[12], v[16]);
            l[8]  = m_store_ptr->add_line(v[3],  v[6]);

            l[9]  = m_store_ptr->add_line(v[0],  v[11]);
            l[10] = m_store_ptr->add_line(v[11], v[2]);
            l[11] = m_store_ptr->add_line(v[0],  v[9]);
            l[12] = m_store_ptr->add_line(v[11], v_split);
            l[13] = m_store_ptr->add_line(v[2],  v[10]);
            l[14] = m_store_ptr->add_line(v[9] , v_split);
            l[15] = m_store_ptr->add_line(v[10], v_split);
            l[16] = m_store_ptr->add_line(v[9] , v[1]);
            l[17] = m_store_ptr->add_line(v[12], v_split);
            l[18] = m_store_ptr->add_line(v[10], v[3]);
            l[19] = m_store_ptr->add_line(v[1],  v[12]);
            l[20] = m_store_ptr->add_line(v[12], v[3]);

            l[21] = m_store_ptr->add_line(v[4],  v[15]);
            l[22] = m_store_ptr->add_line(v[15], v[5]);
            l[23] = m_store_ptr->add_line(v[4],  v[13]);
            l[24] = m_store_ptr->add_line(v[15], v[8]);
            l[25] = m_store_ptr->add_line(v[5],  v[14]);
            l[26] = m_store_ptr->add_line(v[13], v[8]);
            l[27] = m_store_ptr->add_line(v[8],  v[14]);
            l[28] = m_store_ptr->add_line(v[13], v[7]);
            l[29] = m_store_ptr->add_line(v[8],  v[16]);
            l[30] = m_store_ptr->add_line(v[14], v[6]);
            l[31] = m_store_ptr->add_line(v[7],  v[16]);
            l[32] = m_store_ptr->add_line(v[16], v[6]);


            r[0]  = m_store_ptr->add_rectangle(l[0],  l[1],  l[9] , l[21]);
            r[1]  = m_store_ptr->add_rectangle(l[1],  l[2],  l[10], l[22]);
            r[2]  = m_store_ptr->add_rectangle(l[0],  l[3],  l[11], l[23]);
            r[3]  = m_store_ptr->add_rectangle(l[1],  l[4],  l[12], l[24]);
            r[4]  = m_store_ptr->add_rectangle(l[2],  l[5],  l[13], l[25]);
            r[5]  = m_store_ptr->add_rectangle(l[3],  l[4],  l[14], l[26]);
            r[6]  = m_store_ptr->add_rectangle(l[4],  l[5],  l[15], l[27]);
            r[7]  = m_store_ptr->add_rectangle(l[3],  l[6],  l[16], l[28]);
            r[8]  = m_store_ptr->add_rectangle(l[4],  l[7],  l[17], l[29]);
            r[9]  = m_store_ptr->add_rectangle(l[5],  l[8],  l[18], l[30]);
            r[10] = m_store_ptr->add_rectangle(l[6],  l[7],  l[19], l[31]);
            r[11] = m_store_ptr->add_rectangle(l[7],  l[8],  l[20], l[32]);

            r[12] = m_store_ptr->add_rectangle(l[9] , l[11], l[12], l[14]);
            r[13] = m_store_ptr->add_rectangle(l[10], l[12], l[13], l[15]);
            r[14] = m_store_ptr->add_rectangle(l[14], l[16], l[17], l[19]);
            r[15] = m_store_ptr->add_rectangle(l[15], l[17], l[18], l[20]);

            r[16] = m_store_ptr->add_rectangle(l[21], l[23], l[24], l[26]);
            r[17] = m_store_ptr->add_rectangle(l[22], l[24], l[25], l[27]);
            r[18] = m_store_ptr->add_rectangle(l[26], l[28], l[29], l[31]);
            r[19] = m_store_ptr->add_rectangle(l[27], l[29], l[30], l[32]);


            c[0] = m_store_ptr->add_cuboid(r[0], r[2], r[3],  r[5],  r[12], r[16]);
            c[1] = m_store_ptr->add_cuboid(r[1], r[3], r[4],  r[6],  r[13], r[17]);
            c[2] = m_store_ptr->add_cuboid(r[5], r[7], r[8],  r[10], r[14], r[18]);
            c[3] = m_store_ptr->add_cuboid(r[6], r[8], r[9],  r[11], r[15], r[19]);

            for (unsigned int j = 0; j < m_spaces.size(); j++)
            {
                for (int k = 0; k < 4; k++)
                {
                    c[k]->add_space(m_spaces[j]); // fill or update m_spaces in c[k] with the space pointers contained in this object
                    m_spaces[j]->add_cuboid(c[k]); // update all spaces that this object is referenced to with the new cuboid pointed to by c[k]
                }
                m_spaces[j]->delete_cuboid(this); // remove this cuboid from all associated spaces, as these have now been updated
            }

            // check the edges, surfaces and spaces associated to this object with the new vertices: v_09, v_10, v_11 and v_split

            this->check_associated_members(v_split);
            for (int j = 0; j < 17; j++)
            {
                this->check_associated_members(v[j]);
            }

            m_spaces.clear();

            return;
        }
	}// if none of the above applies, then do one final check to see if the vertex is really in the cuboid

    Vertex* v[26];
    std::fill_n(v, 26, nullptr);
    Line* l[53];
    Rectangle* r[35];
    Cuboid* c[8];

    Rectangle* src_rec = m_rectangles[0];
    Line* src_line = src_rec->get_line_ptr(0);

    v[4] = src_rec->surf_point_closest_vertex(v_split);
    v[0] = src_line->get_vertex_ptr(0);
    v[3] = src_line->line_point_closest_to_vertex(v[4]);
    v[6] = src_line->get_vertex_ptr(1);


    Vectors::Vector vct_1 = v[6]->get_coords() - v[0]->get_coords();
    Vectors::Vector vct_2;
    for (int i = 1; i < 4; i++)
    {
        Line* temp_ptr = src_rec->get_line_ptr(i);
        vct_2 = temp_ptr->get_vertex_ptr(1)->get_coords() -
                temp_ptr->get_vertex_ptr(0)->get_coords();

        if (Vectors::is_paral(vct_1, vct_2, 0.01)) // if the lines are parallel
        {
            v[5] = m_lines[i]->line_point_closest_to_vertex(v[4]);
        }
        else
        {
            if ((*v[0] == *temp_ptr->get_vertex_ptr(0)) || (*v[0] == *temp_ptr->get_vertex_ptr(1)))
            {
                v[1] = temp_ptr->line_point_closest_to_vertex(v[4]);
                if (*v[0] == *temp_ptr->get_vertex_ptr(0))
                {
                    v[2] = temp_ptr->get_vertex_ptr(1);
                }
                else
                {
                    v[2] = temp_ptr->get_vertex_ptr(0);
                }
            }
            else
            {
                v[7] = temp_ptr->line_point_closest_to_vertex(v[4]);
                if (*v[6] == *temp_ptr->get_vertex_ptr(0))
                {
                    v[8] = temp_ptr->get_vertex_ptr(1);
                }
                else
                {
                    v[8] = temp_ptr->get_vertex_ptr(0);
                }
            }
        }
    }

    if ((v[0] == nullptr) || (v[1] == nullptr) || (v[2] == nullptr) || (v[3] == nullptr) ||
        (v[4] == nullptr) || (v[5] == nullptr) || (v[6] == nullptr)|| (v[7] == nullptr) || (v[8] == nullptr))
    {
        std::cerr << "Error, not all vertices were initialized during the splitting of a cuboid(e), exiting..." << std::endl;
        exit(1);
    }

    vct_2 = *src_rec->get_normal_ptr();
    Vectors::Vector vct_3;
    for (int i = 1; i < 6; i ++)
    {
        vct_3 = *m_rectangles[i]->get_normal_ptr();
        if (Vectors::is_paral(vct_2, vct_3, 0.01)) // if the plane is parallel to the source plane
        {
            v[17] = m_rectangles[i]->surf_point_closest_vertex(v[0]);
            v[18] = m_rectangles[i]->surf_point_closest_vertex(v[1]);
            v[19] = m_rectangles[i]->surf_point_closest_vertex(v[2]);
            v[20] = m_rectangles[i]->surf_point_closest_vertex(v[3]);
            v[21] = m_rectangles[i]->surf_point_closest_vertex(v[4]);
            v[22] = m_rectangles[i]->surf_point_closest_vertex(v[5]);
            v[23] = m_rectangles[i]->surf_point_closest_vertex(v[6]);
            v[24] = m_rectangles[i]->surf_point_closest_vertex(v[7]);
            v[25] = m_rectangles[i]->surf_point_closest_vertex(v[8]);
        }
    }

    if ((v[17] == nullptr) || (v[18] == nullptr) || (v[19] == nullptr) || (v[20] == nullptr) ||
        (v[21] == nullptr) || (v[22] == nullptr) || (v[23] == nullptr) || (v[24] == nullptr) || (v[25] == nullptr))
    {
        std::cerr << "Error, not all vertices were initialized during the splitting of a cuboid(e), exiting..." << std::endl;
        exit(1);
    }

    for (int i = 0; i < 12; i++)
    {
        m_lines[i]->tag_deletion();
    }

    for (int i = 0; i < 6; i++)
    {
        m_rectangles[i]->tag_deletion();
    }

    v[9]  = (m_store_ptr->add_line(v[0] , v[17]))->line_point_closest_to_vertex(v_split);
    v[10] = (m_store_ptr->add_line(v[1] , v[18]))->line_point_closest_to_vertex(v_split);
    v[11] = (m_store_ptr->add_line(v[2] , v[19]))->line_point_closest_to_vertex(v_split);
    v[12] = (m_store_ptr->add_line(v[3] , v[20]))->line_point_closest_to_vertex(v_split);

    v[13] = (m_store_ptr->add_line(v[5] , v[22]))->line_point_closest_to_vertex(v_split);
    v[14] = (m_store_ptr->add_line(v[6] , v[23]))->line_point_closest_to_vertex(v_split);
    v[15] = (m_store_ptr->add_line(v[7] , v[24]))->line_point_closest_to_vertex(v_split);
    v[16] = (m_store_ptr->add_line(v[8] , v[25]))->line_point_closest_to_vertex(v_split);


    l[0]  = m_store_ptr->add_line(v[0], v[1]);
    l[1]  = m_store_ptr->add_line(v[1], v[2]);
    l[2]  = m_store_ptr->add_line(v[0], v[3]);
    l[3]  = m_store_ptr->add_line(v[1], v[4]);
    l[4]  = m_store_ptr->add_line(v[2], v[5]);
    l[5]  = m_store_ptr->add_line(v[3], v[4]);
    l[6]  = m_store_ptr->add_line(v[4], v[5]);
    l[7]  = m_store_ptr->add_line(v[3], v[6]);
    l[8]  = m_store_ptr->add_line(v[4], v[7]);
    l[9]  = m_store_ptr->add_line(v[5], v[8]);
    l[10] = m_store_ptr->add_line(v[6], v[7]);
    l[11] = m_store_ptr->add_line(v[7], v[8]);

    l[12] = m_store_ptr->add_line(v[9], v[10]);
    l[13] = m_store_ptr->add_line(v[10], v[11]);
    l[14] = m_store_ptr->add_line(v[9], v[12]);
    l[15] = m_store_ptr->add_line(v[10], v_split);
    l[16] = m_store_ptr->add_line(v[11], v[13]);
    l[17] = m_store_ptr->add_line(v[12], v_split);
    l[18] = m_store_ptr->add_line(v_split, v[13]);
    l[19] = m_store_ptr->add_line(v[12], v[14]);
    l[20] = m_store_ptr->add_line(v_split, v[15]);
    l[21] = m_store_ptr->add_line(v[13], v[16]);
    l[22] = m_store_ptr->add_line(v[14], v[15]);
    l[23] = m_store_ptr->add_line(v[15], v[16]);

    l[24] = m_store_ptr->add_line(v[17], v[18]);
    l[25] = m_store_ptr->add_line(v[18], v[19]);
    l[26] = m_store_ptr->add_line(v[17], v[20]);
    l[27] = m_store_ptr->add_line(v[18], v[21]);
    l[28] = m_store_ptr->add_line(v[19], v[22]);
    l[29] = m_store_ptr->add_line(v[20], v[21]);
    l[30] = m_store_ptr->add_line(v[21], v[22]);
    l[31] = m_store_ptr->add_line(v[20], v[23]);
    l[32] = m_store_ptr->add_line(v[21], v[24]);
    l[33] = m_store_ptr->add_line(v[22], v[25]);
    l[34] = m_store_ptr->add_line(v[23], v[24]);
    l[35] = m_store_ptr->add_line(v[24], v[25]);

    l[36] = m_store_ptr->add_line(v[0], v[9]);
    l[37] = m_store_ptr->add_line(v[1], v[10]);
    l[38] = m_store_ptr->add_line(v[2], v[11]);
    l[39] = m_store_ptr->add_line(v[3], v[12]);
    l[40] = m_store_ptr->add_line(v[4], v_split);
    l[41] = m_store_ptr->add_line(v[5], v[13]);
    l[42] = m_store_ptr->add_line(v[6], v[14]);
    l[43] = m_store_ptr->add_line(v[7], v[15]);
    l[44] = m_store_ptr->add_line(v[8], v[16]);

    l[45] = m_store_ptr->add_line(v[9], v[17]);
    l[46] = m_store_ptr->add_line(v[10], v[18]);
    l[47] = m_store_ptr->add_line(v[11], v[19]);
    l[48] = m_store_ptr->add_line(v[12], v[20]);
    l[49] = m_store_ptr->add_line(v_split, v[21]);
    l[50] = m_store_ptr->add_line(v[13], v[22]);
    l[51] = m_store_ptr->add_line(v[14], v[23]);
    l[52] = m_store_ptr->add_line(v[15], v[24]);
    l[53] = m_store_ptr->add_line(v[16], v[25]);


    r[0]  = m_store_ptr->add_rectangle(l[0] , l[2] , l[3] , l[5] );
    r[1]  = m_store_ptr->add_rectangle(l[1] , l[3] , l[4] , l[6] );
    r[2]  = m_store_ptr->add_rectangle(l[5] , l[7] , l[8] , l[10]);
    r[3]  = m_store_ptr->add_rectangle(l[6] , l[8] , l[9] , l[11]);

    r[4]  = m_store_ptr->add_rectangle(l[12], l[14], l[15], l[17]);
    r[5]  = m_store_ptr->add_rectangle(l[13], l[15], l[16], l[18]);
    r[6]  = m_store_ptr->add_rectangle(l[17], l[19], l[20], l[22]);
    r[7]  = m_store_ptr->add_rectangle(l[18], l[20], l[21], l[23]);

    r[8]  = m_store_ptr->add_rectangle(l[24], l[26], l[27], l[29]);
    r[9]  = m_store_ptr->add_rectangle(l[25], l[27], l[28], l[30]);
    r[10] = m_store_ptr->add_rectangle(l[29], l[31], l[32], l[34]);
    r[11] = m_store_ptr->add_rectangle(l[30], l[32], l[33], l[35]);

    r[12] = m_store_ptr->add_rectangle(l[36], l[37], l[0] , l[12]);
    r[13] = m_store_ptr->add_rectangle(l[37], l[38], l[1] , l[13]);
    r[14] = m_store_ptr->add_rectangle(l[36], l[39], l[2] , l[14]);
    r[15] = m_store_ptr->add_rectangle(l[37], l[40], l[3] , l[15]);
    r[16] = m_store_ptr->add_rectangle(l[38], l[41], l[4] , l[16]);
    r[17] = m_store_ptr->add_rectangle(l[39], l[40], l[5] , l[17]);
    r[18] = m_store_ptr->add_rectangle(l[40], l[41], l[6] , l[18]);
    r[19] = m_store_ptr->add_rectangle(l[39], l[42], l[7] , l[19]);
    r[20] = m_store_ptr->add_rectangle(l[40], l[43], l[8] , l[20]);
    r[21] = m_store_ptr->add_rectangle(l[41], l[44], l[9] , l[21]);
    r[22] = m_store_ptr->add_rectangle(l[42], l[43], l[10], l[22]);
    r[23] = m_store_ptr->add_rectangle(l[43], l[44], l[11], l[23]);

    r[24] = m_store_ptr->add_rectangle(l[45], l[46], l[12], l[24]);
    r[25] = m_store_ptr->add_rectangle(l[46], l[47], l[13], l[25]);
    r[26] = m_store_ptr->add_rectangle(l[45], l[48], l[14], l[26]);
    r[27] = m_store_ptr->add_rectangle(l[46], l[49], l[15], l[27]);
    r[28] = m_store_ptr->add_rectangle(l[47], l[50], l[16], l[28]);
    r[29] = m_store_ptr->add_rectangle(l[48], l[49], l[17], l[29]);
    r[30] = m_store_ptr->add_rectangle(l[49], l[50], l[18], l[30]);
    r[31] = m_store_ptr->add_rectangle(l[48], l[51], l[19], l[31]);
    r[32] = m_store_ptr->add_rectangle(l[49], l[52], l[20], l[32]);
    r[33] = m_store_ptr->add_rectangle(l[50], l[53], l[21], l[33]);
    r[34] = m_store_ptr->add_rectangle(l[51], l[52], l[22], l[34]);
    r[35] = m_store_ptr->add_rectangle(l[52], l[53], l[23], l[35]);


    c[0] = m_store_ptr->add_cuboid(r[0] , r[4] , r[12], r[14], r[15], r[17]);
    c[1] = m_store_ptr->add_cuboid(r[1] , r[5] , r[13], r[15], r[16], r[18]);
    c[2] = m_store_ptr->add_cuboid(r[2] , r[6] , r[17], r[19], r[20], r[22]);
    c[3] = m_store_ptr->add_cuboid(r[3] , r[7] , r[18], r[20], r[21], r[23]);

    c[4] = m_store_ptr->add_cuboid(r[4] , r[8] , r[24], r[26], r[27], r[29]);
    c[5] = m_store_ptr->add_cuboid(r[5] , r[9] , r[25], r[27], r[28], r[30]);
    c[6] = m_store_ptr->add_cuboid(r[6] , r[10], r[29], r[31], r[32], r[34]);
    c[7] = m_store_ptr->add_cuboid(r[7] , r[11], r[30], r[32], r[33], r[35]);


    for (unsigned int i = 0; i < m_spaces.size(); i++)
    {
        for (int j = 0 ; j < 8; j++)
        {
            c[j]->add_space(m_spaces[i]); // fill or update m_spaces in c[j] with the space pointers contained in this object
            m_spaces[i]->add_cuboid(c[j]); // update all spaces that this object is referenced to with the new cuboid pointed to by c[j]
        }
        m_spaces[i]->delete_cuboid(this); // remove this cuboid from all associated spaces, as these have now been updated
    }

    // check the edges, surfaces and spaces associated to this object with the new vertices: v_09, v_10, v_11 and v_split
    this->check_associated_members(v_split);
    for (int i = 0; i < 26; i++) // index 1-7 are the original vertices
    {
        this->check_associated_members(v[i]);
    }

    m_spaces.clear();

    return;
}
Vertex_Store* Cuboid::get_store_ptr()
{
    return m_store_ptr;
}

unsigned int Cuboid::get_space_count()
{
    return m_spaces.size();
}

Vertex* Cuboid::get_center_vertex_ptr()
{
    return &m_center_vertex;
}

Vertex* Cuboid::get_vertex_ptr(int n)
{
    return m_vertices[n];
}

Vertex* Cuboid::get_max_vertex()
{
    Vectors::Point max = m_vertices[0]->get_coords();
    Vectors::Point check;
    int max_index = 0;
    for (int i = 1; i <8; i++)
    {
        check = m_vertices[i]->get_coords();
        if ((max(0) < check(0)) ||
            (max(1) < check(1)) ||
            (max(2) < check(2)))
        {
            max = check;
            max_index = i;
        }
    }
    return m_vertices[max_index];
}

Vertex* Cuboid::get_min_vertex()
{
    Vectors::Point min = m_vertices[0]->get_coords();
    Vectors::Point check;
    int min_index = 0;
    for (int i = 1; i <8; i++)
    {
        check = m_vertices[i]->get_coords();
        if ((min(0) > check(0)) ||
            (min(1) > check(1)) ||
            (min(2) > check(2)))
        {
            min = check;
            min_index = i;
        }
    }
    return m_vertices[min_index];
}

Line* Cuboid::get_line_ptr(int n)
{
    return m_lines[n];
}

Rectangle* Cuboid::get_rectangle_ptr(int n)
{
    return m_rectangles[n];
}

Space* Cuboid::get_last_space_ptr()
{
    return m_spaces.back();
}

Space* Cuboid::get_space_ptr(unsigned int n)
{
    return m_spaces[n];
}

Surface* Cuboid::get_surface_ptrs(int n)
{
    return m_rectangles[n]->get_last_surface_ptr();
}

Edge* Cuboid::get_edge_ptrs(int n)
{
    return m_lines[n]->get_last_edge_ptr();
}

Point* Cuboid::get_point_ptrs(int n)
{
    return m_vertices[n]->get_last_point_ptr();
}

// added by Dennis:
void Cuboid::add_ID(unsigned int n)
{
    ID = n + 1;
}

// added by Dennis:
unsigned int Cuboid::get_ID()
{
    return ID;
}

// added by Dennis:
Space* Cuboid::get_space()
{
    return m_spaces[0];
}

void Cuboid::add_zone_ID(unsigned int n)
{
    zone_IDs.push_back(n);
}

unsigned int Cuboid::get_zone_ID(unsigned int n)
{
    return zone_IDs[n];
}

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_CUBOID_CPP
