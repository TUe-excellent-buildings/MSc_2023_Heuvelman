#ifndef BSO_SPACE_HPP
#define BSO_SPACE_HPP

namespace BSO { namespace Spatial_Design { namespace Geometry
{

class Space
{
private:
    // members are:
    std::vector<Cuboid*> m_cuboids; // this space is represented by one or more cuboid
    std::vector<Vertex*> m_vertices;
    std::vector<unsigned int> m_cuboid_IDs;
    Cuboid m_encasing_cuboid;
    Surface* m_surfaces[6]; // this space has 6 surfaces
    Edge* m_edges[12]; // this space has 12 edges
    Point* m_points[8]; // this space has 8 points



    std::string m_space_type;
    int m_space_ID;
public:
    Space(int room_ID, Cuboid* initial_cuboid, Surface* s_1, Surface* s_2, Surface* s_3, Surface* s_4, Surface* s_5, Surface* s_6);
    ~Space();

    int get_ID();
    void add_cuboid(Cuboid* cuboid_ptr);
    void delete_cuboid(Cuboid* cuboid_ptr);

    Edge* get_edge_ptr(int n);
    Surface* get_surface_ptr(int n);

    void check_vertex(Vertex* vertex_ptr);
    Cuboid& get_encasing_cuboid();

    void assign_space_type(std::string);
    std::string get_space_type();

    void add_cuboid_IDs(); // added by Dennis
    std::vector<unsigned int> get_cuboid_IDs(); // added by Dennis
    std::vector<Cuboid*> get_cuboids(); // added by Dennis
};


} // Geometry
} // Spatial_Design
} // BSO

#endif // BSO_SPACE_HPP
