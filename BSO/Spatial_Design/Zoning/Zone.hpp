#ifndef ZONE_HPP
#define ZONE_HPP

#include <vector>
#include <algorithm>
#include <iostream>

#include <BSO/Spatial_Design/Conformation.hpp>


namespace BSO { namespace Spatial_Design { namespace Zoning {

class Zone
{
private:
	BSO::Spatial_Design::MS_Conformal* m_CF;
	std::vector<Geometry::Cuboid*> m_cuboids;
	std::vector<unsigned int> m_cuboid_IDs;
	std::vector<Geometry::Space*> m_spaces;
	std::vector<Geometry::Vertex*> m_vertices;
	std::vector<Geometry::Rectangle*> m_rectangles;
	int x_min, y_min, z_min;
	int x_max, y_max, z_max;
	unsigned int width, depth, height;
	bool whole_spaces = false;
	bool part_spaces = false;
	unsigned int zone_type = 0;
	unsigned int new_type = 0;
	bool type_change = false;
	bool delete_zone = false;
	int floor;
	int floor_above;
	bool floating_zone = false;
	bool combined_zone = false;
	int ID;
public:
	Zone(std::vector<Geometry::Cuboid*>& cuboids);
	~Zone();

    void add_cuboid(Geometry::Cuboid* cuboid);
    void add_cuboid_ID(unsigned int);
    bool check_double_cuboids(Geometry::Cuboid* cuboid);
    std::vector<Geometry::Cuboid*> get_cuboids();
    std::vector<unsigned int> get_cuboid_IDs();
    void min_coords(Geometry::Cuboid*);
    void max_coords(Geometry::Cuboid*);
    void min_coords_space(Geometry::Space*);
    void max_coords_space(Geometry::Space*);
    int get_min_coords(unsigned int);
    int get_max_coords(unsigned int);
    void dimensions();
    unsigned int get_dimensions(unsigned int);
    void add_spaces();
    void check_spaces();
    bool get_whole_spaces();
    bool get_part_spaces();
    void add_type(unsigned int);
    unsigned int get_type();
    unsigned int get_cuboid_count();
    Geometry::Cuboid* get_cuboid(unsigned int);
    void duplicate(Zone*);
    void tag_for_type_change(unsigned int);
    void untag_for_type_change();
    bool check_type_change();
    unsigned int get_type_change();
    void tag_for_deletion();
    void untag_for_deletion();
    bool check_deletion();
    void add_floor(int);
    void add_floor_above(int);
    int get_floor();
    int get_floor_above();
    void tag_floating_zone();
    void untag_floating_zone();
    bool check_floating_zone();
    void combine_zones(Zone*);
    void add_ID(unsigned int);
    unsigned int get_ID();
    void tag_combined_zone();
    bool check_combined_zone();
    void add_rectangles();
    void add_internal_floors();
    unsigned int get_rectangle_count();
    Geometry::Rectangle* get_rectangle(unsigned int);
    void add_vertices();
    unsigned int get_vertex_count();
    Geometry::Vertex* get_vertex(unsigned int);
    std::vector <Geometry::Vertex*> get_vertices();
}; // Zone

} // namespace Zoning
} // namespace Spatial_Design
} // namespace BSO

#include <BSO/Spatial_Design/Zoning/Zone.cpp>

#endif //ZONE_HPP
