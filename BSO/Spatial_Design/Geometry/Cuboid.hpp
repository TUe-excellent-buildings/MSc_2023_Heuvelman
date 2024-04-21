#ifndef BSO_CUBOID_HPP
#define BSO_CUBOID_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Cuboid
{
private:
    std::vector<Space*> m_spaces; // if .size() > 1 there are overlapping spaces.
    Vertex_Store* m_store_ptr;
    Rectangle* m_rectangles[6];
    Line* m_lines[12];
    Vertex* m_vertices[8];
    Vertex m_center_vertex;
    bool m_deletion;

    unsigned int ID; // added by Dennis
    std::vector<unsigned int> zone_IDs; // added by Dennis
public:
    Cuboid(Rectangle* one, Rectangle* two, Rectangle* three, Rectangle* four, Rectangle* five, Rectangle* six, Vertex_Store* store_ptr);
    Cuboid();
    ~Cuboid();

    bool operator == (const Cuboid& rhs);

    double get_volume();

    bool check_vertex(Vertex* vertex_ptr);
    bool check_deletion();
    void tag_deletion();

    void add_space(Space* space_ptr);
    void check_associated_members(Vertex* v_check);
    void split(Vertex* v_split);
    Vertex_Store* get_store_ptr();

    unsigned int get_space_count();
    Vertex* get_center_vertex_ptr();
    Vertex* get_vertex_ptr(int n);
    Vertex* get_max_vertex();
    Vertex* get_min_vertex();
    Line* get_line_ptr(int n);
    Rectangle* get_rectangle_ptr(int n);
    Space* get_last_space_ptr();
    Space* get_space_ptr (unsigned int n);
    Surface* get_surface_ptrs(int n);
    unsigned int get_edge_count();
    Edge* get_edge_ptrs(int n);
    Point* get_point_ptrs(int n);

    void add_ID(unsigned int n); // added by Dennis
    unsigned int get_ID(); // added by Dennis
    Space* get_space(); // added by Dennis
    void add_zone_ID(unsigned int);
    void remove_zone_ID(unsigned int zoneID); // added by Janneke
    unsigned int get_zone_ID(unsigned int);

    int get_last_zone() { return zone_IDs.back(); };
    const std::vector<unsigned int>& get_all_zones() const { return zone_IDs; }

    std::vector<Vertex*> get_vertices() { return std::vector<Vertex*>(m_vertices, m_vertices + 8); }
};


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_CUBOID_HPP
