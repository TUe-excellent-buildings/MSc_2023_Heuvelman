#ifndef BSO_VERTEX_HPP
#define BSO_VERTEX_HPP


namespace BSO { namespace Spatial_Design { namespace Geometry {

class Vertex
{
private:
    std::vector<Point*> m_points;
    Vectors::Point m_coords;

    bool m_structural;
    bool zoned = false;
public:
    Vertex(double x, double y, double z);
    Vertex(Vectors::Point p);
    Vertex();
    ~Vertex();
    bool operator == (const Vertex& rhs);

    bool is_structural();

    void add_point(Point* point_ptr);
    void make_structural();

    Vectors::Point get_coords();

    Point* get_last_point_ptr();

    void tag_zoned();
    void untag_zoned();
    bool get_zoned();
}; // Vertex

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_VERTEX_HPP
