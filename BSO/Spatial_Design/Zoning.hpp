#ifndef ZONING_HPP
#define ZONING_HPP

#include <vector>
#include <algorithm>
#include <iostream>

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Spatial_Design/Zoning/Zone.hpp>
#include <BSO/Spatial_Design/Geometry/Geometry.hpp>
#include <Read_Zoning_Settings.hpp>

namespace BSO { namespace Spatial_Design { namespace Zoning {

class Zoned_Design
{
private:
	MS_Conformal* m_CF;
	Grammar::Zoning_Settings zoning_settings;
	std::vector<Zone*> m_zones;
	std::vector<Geometry::Cuboid*> m_cuboids;
	std::vector<Zone*> m_temp_zones;
	std::vector<Zone*> m_appendix_zones;
	std::vector<int> m_floors;
	std::vector<int> m_floor_coords;
	std::vector<Zoned_Design*> m_zoned;
	std::vector<Zoned_Design*> m_temp_zoned;
	std::vector<Geometry::Cuboid*> m_temp_cuboids;
	std::vector<Geometry::Space*> m_spaces;
	std::vector<Geometry::Vertex*> m_vertices;
	std::vector<Geometry::Rectangle*> m_rectangles;
	unsigned int base_type = 0;
	double total_compliance = 0;

    // switches
	unsigned int max_span;
	unsigned int min_span;
	bool whole_space_zones;
	bool delete_expanded_designs;
	bool zone_floors;
	bool adaptive_thickness;

	bool forced_whole_spaces = true; // set true when whole_space_zones == true
	bool whole_spaces_only = false;
	bool combine_appendix_zones = false; // non-cuboidal-shaped zones and no coords -> not suitable for SD!
	bool delete_non_combined_appendix_zones = false; // set false when combine_appendix_zones == false (or whole_spaces_only == true)
	bool zone_cores = true;
	bool always_use_cores = false;
	int ID;
	// switches

public:
	Zoned_Design(MS_Conformal* CF);
	~Zoned_Design();
	bool check_double_zones(Zone*);
	unsigned int get_double_zone(Zone*);
	void add_zone(Zone*, unsigned int);
	void prepare_zone(Zone*);
	void create_longest_zones();
	void create_cuboid_zones(Zone*);
	void get_floors();
	void add_floors();
	void delete_tagged_zones();
	void preserve_tagged_zones();
	void create_largest_cuboid_zones(Zone*);
	void check_floating_zones();
	void combine_floating_zones();
	bool check_cuboid_presence(Zone*);
	bool check_double_cuboids(Zone*);
	void add_cuboids(Zone*);
	std::vector<Zone*> get_zones();
	bool check_double_designs(Zoned_Design*);
	bool check_double_temp_designs(Zoned_Design*);
	void get_missing_cuboids(Zoned_Design*);
	bool check_double_appendix_zones(Zone*);
	unsigned int get_double_appendix_zone(Zone*);
	void add_appendix_zone(Zone*, unsigned int);
	void create_appendix_zones(unsigned int);
	void duplicate(Zoned_Design*);
	std::vector<Zone*> get_intersecting_zones(Zone*, unsigned int);
	std::vector<Zoned_Design*> get_designs();
	std::vector<Geometry::Cuboid*> get_cuboids();

	void add_rectangles();
	unsigned int get_rectangle_count();
	Geometry::Rectangle* get_rectangle(unsigned int);
	void add_vertices();
	unsigned int get_vertex_count();
	Geometry::Vertex* get_vertex(unsigned int);
	void reset_SD_model();
	void reset_zoned_SD_model();
	void prepare_unzoned_SD_model();
	void prepare_zoned_SD_model(unsigned int);
	void add_compliance(double, unsigned int);
	void add_unzoned_compliance(double);
	double get_unzoned_compliance();
	std::pair<double, unsigned int> get_min_compliance();
	double get_compliance(unsigned int);

	Zone* get_zone_by_ID(int ID);
	void make_zoning();
	Zoned_Design* make_zoning2(const std::vector<unsigned int>& zoneIDs);
	void prepare_cuboids();
	std::vector<int> get_zoned_floors();
	std::vector<int> get_floor_coords();

	void add_ID(unsigned int);
	unsigned int get_ID();

	bool remove_zone_by_ID(unsigned int zoneID);

	void add_zoned_design(Zoned_Design* ZD) { m_zoned.push_back(ZD); }
}; // Zoned_Design

} // namespace Zoning
} // namespace Spatial_Design
} // namespace BSO

#include <BSO/Spatial_Design/Zoning.cpp>

#endif //ZONING_HPP
