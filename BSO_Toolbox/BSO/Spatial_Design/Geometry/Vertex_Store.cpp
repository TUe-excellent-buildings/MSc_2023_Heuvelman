#ifndef BSO_VERTEX_STORE_CPP
#define BSO_VERTEX_STORE_CPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

Vertex_Store::Vertex_Store()
{
    //ctor
}

Vertex_Store::~Vertex_Store()
{

    for (unsigned int i = 0; i < m_vertices.size(); i++)
    {
        delete m_vertices[i];
        m_vertices.clear();
    }

    for (unsigned int i = 0; i < m_lines.size(); i++)
    {
        delete m_lines[i];
        m_lines.clear();
    }

    for (unsigned int i = 0; i < m_rectangles.size(); i++)
    {
        delete m_rectangles[i];
        m_rectangles.clear();
    }

    for (unsigned int i = 0; i < m_cubes.size(); i++)
    {
        delete m_cubes[i];
        m_rectangles.clear();
    }
    //dtor
}
unsigned int temp_v_count = 0;
Vertex* Vertex_Store::add_vertex(double x, double y, double z)
{
    Vertex* temp_ptr = new Vertex(x, y, z);
    bool ptr_found = false;
    for (unsigned int i = 0; i < m_vertices.size(); i++)
    {
        if (*temp_ptr == *(m_vertices[i]))
        {
            ptr_found = true;

            delete temp_ptr;
            return m_vertices[i];
        }
    }
    if (!ptr_found)
    {
        m_vertices.push_back(temp_ptr);
        return temp_ptr;
    }
    else
    {
        std::cerr << "Error in initializing vertex, exiting..." << std::endl;
        exit(1);
    }
}
unsigned int temp_va_count = 0;
Vertex* Vertex_Store::add_vertex(Vectors::Point p)
{
    Vertex* temp_ptr = new Vertex(p);
    bool ptr_found = false;
    for (unsigned int i = 0; i < m_vertices.size(); i++)
    {
        if (*temp_ptr == *(m_vertices[i]))
        {
            ptr_found = true;

            delete temp_ptr;
            return m_vertices[i];
        }
    }
    if (!ptr_found)
    {
        m_vertices.push_back(temp_ptr);
        return temp_ptr;
    }
    else
    {
        std::cerr << "Error in initializing vertex, exiting..." << std::endl;
        exit(1);
    }
}

Line* Vertex_Store::add_line(Vertex* one, Vertex* two)
{
    Line* temp_ptr = new Line(one, two, this);
    bool ptr_found = false;
    for (unsigned int i = 0; i < m_lines.size(); i++)
    {
        if (*temp_ptr == *(m_lines[i]))
        {
            ptr_found = true;

            delete temp_ptr;
            return m_lines[i];
        }
    }
    if (!ptr_found)
    {
        m_lines.push_back(temp_ptr);
        return temp_ptr;
    }
    else
    {
        std::cerr << "Error in initializing line, exiting..." << std::endl;
        exit(1);
    }
}

Rectangle* Vertex_Store::add_rectangle(Line* one, Line* two, Line* three, Line* four)
{
    Rectangle* temp_ptr = new Rectangle(one, two, three, four, this);
    bool ptr_found = false;
    for (unsigned int i = 0; i < m_rectangles.size(); i++)
    {
        if (*temp_ptr == *(m_rectangles[i]))
        {
            ptr_found = true;

            delete temp_ptr;
            return m_rectangles[i];
        }
    }
    if (!ptr_found)
    {
        m_rectangles.push_back(temp_ptr);
        return temp_ptr;
    }
    else
    {
        std::cerr << "Error in initializing rectangle, exiting..." << std::endl;
        exit(1);
    }
}

Cuboid* Vertex_Store::add_cuboid(Rectangle* one, Rectangle* two, Rectangle* three, Rectangle* four, Rectangle* five, Rectangle* six)
{
    Cuboid* temp_ptr = new Cuboid(one, two, three, four, five, six, this);
    bool ptr_found = false;
    for (unsigned int i = 0; i < m_cubes.size(); i++)
    {
        if (*temp_ptr == *(m_cubes[i]))
        {
            ptr_found = true;

            delete temp_ptr;
            return m_cubes[i];
        }
    }
    if (!ptr_found)
    {
        m_cubes.push_back(temp_ptr);
        return temp_ptr;
    }
    else
    {
        std::cerr << "Error in initializing cuboid, exiting..." << std::endl;
        exit(1);
    }
}

void Vertex_Store::delete_line(Line* l_d)
{
    m_lines.erase(std::remove(m_lines.begin(), m_lines.end(), l_d), m_lines.end()); // remove the element of the vector containing the address of this pointer
    delete l_d; // release the memory at the address pointed to by this pointer
}

void Vertex_Store::delete_rectangle(Rectangle* r_d)
{
    m_rectangles.erase( std::remove(m_rectangles.begin(), m_rectangles.end(), r_d), m_rectangles.end()); // remove the element of the vector containing the address of this pointer
    delete r_d; // release the memory at the address pointed to by this pointer
}

void Vertex_Store::delete_cuboid(Cuboid* c_d)
{
    m_cubes.erase(std::remove(m_cubes.begin(), m_cubes.end(), c_d), m_cubes.end()); // remove the element of the vector containing the address of this pointer
    delete c_d; // release the memory at the address pointed to by this pointer
}


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_VERTEX_STORE_CPP
