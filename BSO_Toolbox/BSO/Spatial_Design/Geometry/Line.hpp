#ifndef BSO_LINE_HPP
#define BSO_LINE_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Line
{
private:
    std::vector<Edge*> m_edges; // this line belongs to a number of edges
    std::vector<Rectangle*> m_rectangle_ptrs; // the rectangles that this line is part of
    Vertex_Store* m_store_ptr; // pointer towards the vector in which this object of the Line class is stored
    Vertex* m_vertices[2];
    Vertex m_center_vertex;
    bool m_deletion;
    bool m_structural;
    bool zoned = false;
    bool constraint = false;
    unsigned int thickness = 0;
public:
    Line(Vertex* one,Vertex* two, Vertex_Store* store_ptr);
    Line();
    ~Line();

    bool operator == (const Line& rhs);

    double get_length();

    bool check_vertex(Vertex* vertex_ptr);
    bool check_line_intersect(Line* line_ptr, Vertex* intersect_ptr);
    bool check_deletion();
    bool is_structural();
    void tag_deletion();

    void add_edge(Edge* edge_ptr);
    void check_associated_members(Vertex* v_check);
    void split(Vertex* vertex_ptr);
    void make_structural();
    Vertex_Store* get_store_ptr();

    Vertex* get_vertex_ptr(int n);
    Vertex* get_center_vertex_ptr();
    Vertex* line_point_closest_to_vertex(Vertex* v_1);

    unsigned int get_edge_count();
    Edge* get_edge_ptr(unsigned int n);
    Edge* get_last_edge_ptr();
    Point* get_point_ptrs(int n);

    void associate_rectangle(Rectangle* r);
    void deassociate_rectangle(Rectangle* r);
    unsigned int get_rectangle_count();
    Rectangle* get_rectangle_ptr(unsigned int i);

    void tag_zoned();
    void untag_zoned();
    bool get_zoned();

    void tag_constraint();
    void untag_constraint();
    bool get_constraint();

    void add_thickness(unsigned int t);
    unsigned int get_thickness();

};


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_LINE_HPP
