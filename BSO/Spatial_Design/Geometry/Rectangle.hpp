#ifndef BSO_RECTANGLE_HPP
#define BSO_RECTANGLE_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Rectangle
{
private:
    std::vector<Surface*> m_surfaces;
    Vertex_Store* m_store_ptr; // pointer towards the vector in which this object of the Rectangle class is stored
    Line* m_lines[4];
    Vertex* m_vertices[4];
    Vectors::Vector m_normal_vector;
    Vertex m_center_vertex;
    bool m_deletion;
    bool m_structural;
    bool m_lines_associated;
    bool zoned = false;
    bool horizontal = false;
    unsigned int thickness = 0;
    double loading = 0;
public:
    Rectangle(Line* one, Line* two, Line* three, Line* four, Vertex_Store* store_ptr);
    Rectangle();
    ~Rectangle();

    bool operator == (const Rectangle& rhs);

    double get_area();

    bool check_vertex(Vertex* vertex_ptr);
    bool check_line_intersect(Line* line_ptr, Vertex* intersect_ptr);
    bool check_deletion();
    bool is_structural();
    void tag_deletion();

    void add_surface(Surface* surface_ptr);
    void check_associated_members(Vertex* v_check);
    void split(Vertex* v_split);
    void make_structural();
	void make_not_structural();
    void associate_lines();
    void deassociate_lines();
    Vertex_Store* get_store_ptr();

    Line* get_line_ptr(int n);
    Vectors::Vector* get_normal_ptr();
    Vertex* get_vertex_ptr(int n);
    Vertex* get_max_vertex();
    Vertex* get_min_vertex();
    Vertex* get_center_vertex_ptr();
    Vertex* surf_point_closest_vertex(Vertex* vertex_ptr);

    unsigned int get_surface_count();
    Surface* get_surface_ptr(unsigned int n);
    Surface* get_last_surface_ptr();
    Edge* get_edge_ptr(int n);
    Point* get_point_ptrs(int n);

    void tag_zoned();
    void untag_zoned();
    bool get_zoned();

    void tag_horizontal();
    void untag_horizontal();
    bool get_horizontal();

    void add_thickness(unsigned int t);
    unsigned int get_thickness();

    void add_loading(double l);
    double get_loading();
};

} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_RECTANGLE_HPP
