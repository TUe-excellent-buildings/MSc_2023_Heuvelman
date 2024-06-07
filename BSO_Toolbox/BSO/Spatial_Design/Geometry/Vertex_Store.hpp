#ifndef BSO_VERTEX_STORE_HPP
#define BSO_VERTEX_STORE_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Vertex_Store
{
protected:
    std::vector<Vertex*> m_vertices;
    std::vector<Line*> m_lines;
    std::vector<Rectangle*> m_rectangles;
    std::vector<Cuboid*> m_cubes;
public:
    Vertex_Store();
    ~Vertex_Store();

    void write_vertices()
    {
        std::ofstream out("vertices.txt");
        for (unsigned int i = 0; i < m_vertices.size(); i++)
        {
            out << m_vertices[i]->get_coords()(0) << "," << m_vertices[i]->get_coords()(1) << "," << m_vertices[i]->get_coords()(2) << std::endl;
        }
    }

    Vertex* add_vertex(double x, double y, double z);
    Vertex* add_vertex(Vectors::Point p);
    Line* add_line(Vertex* one, Vertex* two);
    Rectangle* add_rectangle(Line* one, Line* two, Line* three, Line* four);
    Cuboid* add_cuboid(Rectangle* one, Rectangle* two, Rectangle* three, Rectangle* four, Rectangle* five, Rectangle* six);

    void delete_line(Line* l_d);
    void delete_rectangle(Rectangle* r_d);
    void delete_cuboid(Cuboid* c_d);
};


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_VERTEX_STORE_HPP
