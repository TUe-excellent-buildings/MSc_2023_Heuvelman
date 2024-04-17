#ifndef ZONING_CPP
#define ZONING_CPP


namespace BSO { namespace Spatial_Design { namespace Zoning {

Zoned_Design::Zoned_Design(MS_Conformal* CF)
{
	m_CF = CF;
	zoning_settings = Grammar::read_zoning_settings("JH_Zoning_Assignment_GUI/Settings/Zoning_Settings.txt"); // read the zoning settings file
	max_span = zoning_settings.max_span;
	min_span = zoning_settings.min_span;
	whole_space_zones = zoning_settings.whole_space_zones;
	delete_expanded_designs = zoning_settings.delete_expanded_designs;
	zone_floors = zoning_settings.zone_floors;
	adaptive_thickness = zoning_settings.adaptive_thickness;
} // ctor

Zoned_Design::~Zoned_Design()
{

} // dtor

bool Zoned_Design::check_double_zones(Zone* zone)
{
    bool found = false;
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        if (zone->get_cuboid_IDs() == m_zones[i]->get_cuboid_IDs() && m_zones[i]->get_type() != 0)
        {
            found = true;
            break;
        }
    }
    return found;
} // check_double_zones()

unsigned int Zoned_Design::get_double_zone(Zone* zone)
{
    unsigned int found = 0;
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        if (zone->get_cuboid_IDs() == m_zones[i]->get_cuboid_IDs())
        {
            found = i;
            break;
        }
    }
    return found;
} // get_double_zone()

void Zoned_Design::add_zone(Zone* zone, unsigned int n)
{
    zone->dimensions();
    zone->add_spaces();
    zone->check_spaces();
    zone->add_type(n);
    m_zones.push_back(zone);
} // add_zone()

void Zoned_Design::add_ID(unsigned int n)
{
    ID = n;
} // add_ID()


unsigned int Zoned_Design::get_ID()
{
    return ID;
} // get_ID()


void Zoned_Design::prepare_zone(Zone* zone)
{
    zone->dimensions();
    zone->add_spaces();
    zone->check_spaces();
} // prepare_zone()

void Zoned_Design::delete_tagged_zones()
{
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        if (m_zones[i]->check_deletion() == false)
        {
            m_temp_zones.push_back(m_zones[i]);
        }
    }
    m_zones.clear();
    m_zones = m_temp_zones;
    m_temp_zones.clear();
} // delete_tagged_zones()

void Zoned_Design::preserve_tagged_zones()
{
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        m_zones[i]->untag_for_deletion();
    }
} // preserve_tagged_zones()

void Zoned_Design::create_longest_zones()
{
    unsigned int cuboid_count = m_CF->get_cuboid_count();

    // create longest zones in x-direction for every floor (type 1)
    for (unsigned int i = 0; i < cuboid_count; i++)
    {
        Geometry::Cuboid* temp_cuboid = m_CF->get_cuboid(i);
        int x_min_temp = temp_cuboid->get_min_vertex()->get_coords()(0);
        int y_min_temp = temp_cuboid->get_min_vertex()->get_coords()(1);
        int z_min_temp = temp_cuboid->get_min_vertex()->get_coords()(2);
        int x_max_temp = temp_cuboid->get_max_vertex()->get_coords()(0);
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->min_coords(temp_cuboid);
        temp_zone->max_coords(temp_cuboid);
        temp_zone->add_cuboid(temp_cuboid);
        temp_zone->add_cuboid_ID(temp_cuboid->get_ID());

        // switch: forced whole spaces 1/3
        if (forced_whole_spaces == true)
        {
            Zoned_Design::prepare_zone(temp_zone);
            if (temp_zone->get_part_spaces() == false)
            {
                Zone* whole_space_zone = new Zone(m_cuboids);
                whole_space_zone->duplicate(temp_zone);
                m_temp_zones.push_back(whole_space_zone);
            }
        }
        if (whole_spaces_only == true)
        {
            if (Zoned_Design::check_double_zones(temp_zone) == false)
            {
                Zone* whole_space_zone = new Zone(m_cuboids);
                whole_space_zone->duplicate(temp_zone);
                Zoned_Design::add_zone(whole_space_zone, 1);
            }
        }

        bool end_loop = false;
        while (end_loop == false) // search connected cuboids in x-direction for temp_cuboid of temp_zone
        {
            unsigned int new_cuboid_count = 0;
            for (unsigned int j = 0; j < cuboid_count; j++)
            {
                if (j != i) // exclude temp_cuboid from search
                {
                    Geometry::Cuboid* new_cuboid = m_CF->get_cuboid(j);
                    int y_min_new = new_cuboid->get_min_vertex()->get_coords()(1);
                    int z_min_new = new_cuboid->get_min_vertex()->get_coords()(2);

                    int x_min_new = new_cuboid->get_min_vertex()->get_coords()(0);
                    int x_max_new = new_cuboid->get_max_vertex()->get_coords()(0);

                    if (y_min_new == y_min_temp && z_min_new == z_min_temp &&
                        x_min_new == x_max_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->max_coords(new_cuboid);
                        x_max_temp = x_max_new;

                        // switch: forced whole spaces 2/3
                        if (forced_whole_spaces == true)
                        {
                            Zoned_Design::prepare_zone(temp_zone);
                            if (temp_zone->get_part_spaces() == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                m_temp_zones.push_back(whole_space_zone);
                            }
                        }
                        if (whole_spaces_only == true)
                        {
                            if (Zoned_Design::check_double_zones(temp_zone) == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                Zoned_Design::add_zone(whole_space_zone, 1);
                            }
                        }
                    }

                    if (y_min_new == y_min_temp && z_min_new == z_min_temp &&
                        x_max_new == x_min_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->min_coords(new_cuboid);
                        x_min_temp = x_min_new;

                        // switch: forced whole spaces 2/3
                        if (forced_whole_spaces == true)
                        {
                            Zoned_Design::prepare_zone(temp_zone);
                            if (temp_zone->get_part_spaces() == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                m_temp_zones.push_back(whole_space_zone);
                            }
                        }
                        if (whole_spaces_only == true)
                        {
                            if (Zoned_Design::check_double_zones(temp_zone) == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                Zoned_Design::add_zone(whole_space_zone, 1);

                            }
                        }
                    }
                } // if (j != i)
            } // for (unsigned int j = 0; j < cuboid_count; j++)
            if (new_cuboid_count == 0) // no new connected cuboids found
            {
                if (m_zones.size() > 0) // exclude first zone from check for double zones
                {
                    if (Zoned_Design::check_double_zones(temp_zone) == false)
                    {
                        Zoned_Design::add_zone(temp_zone, 1);
                    }
                }
                else
                {
                    Zoned_Design::add_zone(temp_zone, 1);
                }

                // switch: forced whole spaces 3/3
                if (forced_whole_spaces == true)
                {
                    Zoned_Design::prepare_zone(temp_zone);
                    if (temp_zone->get_part_spaces() == true && m_temp_zones.size() > 0)
                    {
                        if (Zoned_Design::check_double_zones(m_temp_zones.back()) == false)
                        {
                            Zoned_Design::add_zone(m_temp_zones.back(), 21);
                        }
                    }
                    m_temp_zones.clear();
                }
                end_loop = true;
            } // if (new_cuboid_count == 0)
        } // while (x-direction)
    } // for (x-direction)

    // create longest zones in y-direction for every floor (type 2)
    for (unsigned int i = 0; i < cuboid_count; i++)
    {
        Geometry::Cuboid* temp_cuboid = m_CF->get_cuboid(i);
        int x_min_temp = temp_cuboid->get_min_vertex()->get_coords()(0);
        int y_min_temp = temp_cuboid->get_min_vertex()->get_coords()(1);
        int z_min_temp = temp_cuboid->get_min_vertex()->get_coords()(2);
        int y_max_temp = temp_cuboid->get_max_vertex()->get_coords()(1);
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->min_coords(temp_cuboid);
        temp_zone->max_coords(temp_cuboid);
        temp_zone->add_cuboid(temp_cuboid);
        temp_zone->add_cuboid_ID(temp_cuboid->get_ID());

        // switch: forced whole spaces 1/3
        if (forced_whole_spaces == true)
        {
            Zoned_Design::prepare_zone(temp_zone);
            if (temp_zone->get_part_spaces() == false)
            {
                Zone* whole_space_zone = new Zone(m_cuboids);
                whole_space_zone->duplicate(temp_zone);
                m_temp_zones.push_back(whole_space_zone);
            }
        }
        if (whole_spaces_only == true)
        {
            if (Zoned_Design::check_double_zones(temp_zone) == false)
            {
                Zone* whole_space_zone = new Zone(m_cuboids);
                whole_space_zone->duplicate(temp_zone);
                Zoned_Design::add_zone(whole_space_zone, 2);

            }
        }

        bool end_loop = false;
        while (end_loop == false) // search connected cuboids in y-direction for temp_cuboid of temp_zone
        {
            unsigned int new_cuboid_count = 0;
            for (unsigned int j = 0; j < cuboid_count; j++)
            {

                if (j != i) // exclude temp_cuboid from search
                {
                    Geometry::Cuboid* new_cuboid = m_CF->get_cuboid(j);
                    int x_min_new = new_cuboid->get_min_vertex()->get_coords()(0);
                    int z_min_new = new_cuboid->get_min_vertex()->get_coords()(2);

                    int y_min_new = new_cuboid->get_min_vertex()->get_coords()(1);
                    int y_max_new = new_cuboid->get_max_vertex()->get_coords()(1);

                    if (x_min_new == x_min_temp && z_min_new == z_min_temp &&
                        y_min_new == y_max_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->max_coords(new_cuboid);
                        y_max_temp = y_max_new;

                        // switch: forced whole spaces 2/3
                        if (forced_whole_spaces == true)
                        {
                            Zoned_Design::prepare_zone(temp_zone);
                            if (temp_zone->get_part_spaces() == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                m_temp_zones.push_back(whole_space_zone);
                            }
                        }
                        if (whole_spaces_only == true)
                        {
                            if (Zoned_Design::check_double_zones(temp_zone) == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                Zoned_Design::add_zone(whole_space_zone, 2);
                            }
                        }
                    }

                    if (x_min_new == x_min_temp && z_min_new == z_min_temp &&
                        y_max_new == y_min_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->min_coords(new_cuboid);
                        y_min_temp = y_min_new;

                        // switch: forced whole spaces 2/3
                        if (forced_whole_spaces == true)
                        {
                            Zoned_Design::prepare_zone(temp_zone);
                            if (temp_zone->get_part_spaces() == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                m_temp_zones.push_back(whole_space_zone);
                            }
                        }
                        if (whole_spaces_only == true)
                        {
                            if (Zoned_Design::check_double_zones(temp_zone) == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                Zoned_Design::add_zone(whole_space_zone, 2);
                            }
                        }
                    }
                } // if (j != i)
            } // for (unsigned int j = 0; j < cuboid_count; j++)
            if (new_cuboid_count == 0) // no new connected cuboids found
            {
                if (Zoned_Design::check_double_zones(temp_zone) == false)
                {
                    Zoned_Design::add_zone(temp_zone, 2);
                }

                // switch: forced whole spaces 3/3
                if (forced_whole_spaces == true)
                {
                    Zoned_Design::prepare_zone(temp_zone);
                    if (temp_zone->get_part_spaces() == true && m_temp_zones.size() > 0)
                    {
                        if (Zoned_Design::check_double_zones(m_temp_zones.back()) == false)
                        {
                            Zoned_Design::add_zone(m_temp_zones.back(), 22);
                        }
                    }
                    m_temp_zones.clear();
                }
                end_loop = true;
            } // if (new_cuboid_count == 0)
        } // while (y-direction)
    } // for (y-direction)

    // create longest zones in z-direction (type 0)
    for (unsigned int i = 0; i < cuboid_count; i++)
    {
        Geometry::Cuboid* temp_cuboid = m_CF->get_cuboid(i);
        int x_min_temp = temp_cuboid->get_min_vertex()->get_coords()(0);
        int y_min_temp = temp_cuboid->get_min_vertex()->get_coords()(1);
        int z_min_temp = temp_cuboid->get_min_vertex()->get_coords()(2);
        int z_max_temp = temp_cuboid->get_max_vertex()->get_coords()(2);
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->min_coords(temp_cuboid);
        temp_zone->max_coords(temp_cuboid);
        temp_zone->add_cuboid(temp_cuboid);
        temp_zone->add_cuboid_ID(temp_cuboid->get_ID());

        bool end_loop = false;
        while (end_loop == false) // search connected cuboids in z-direction for temp_cuboid of temp_zone
        {
            unsigned int new_cuboid_count = 0;
            for (unsigned int j = 0; j < cuboid_count; j++)
            {

                if (j != i) // exclude temp_cuboid from search
                {
                    Geometry::Cuboid* new_cuboid = m_CF->get_cuboid(j);
                    int x_min_new = new_cuboid->get_min_vertex()->get_coords()(0);
                    int y_min_new = new_cuboid->get_min_vertex()->get_coords()(1);
                    int z_min_new = new_cuboid->get_min_vertex()->get_coords()(2);
                    int z_max_new = new_cuboid->get_max_vertex()->get_coords()(2);

                    if (x_min_new == x_min_temp && y_min_new == y_min_temp &&
                        z_min_new == z_max_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->max_coords(new_cuboid);
                        z_max_temp = z_max_new;
                    }

                    if (x_min_new == x_min_temp && y_min_new == y_min_temp &&
                        z_max_new == z_min_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->min_coords(new_cuboid);
                        z_min_temp = z_min_new;
                    }
                } // if (j != i)
            } // for (unsigned int j = 0; j < cuboid_count; j++)
            if (new_cuboid_count == 0) // no new connected cuboids found
            {
                if (Zoned_Design::check_double_zones(temp_zone) == false)
                    {
                        Zoned_Design::add_zone(temp_zone, 0);
                    }
                end_loop = true;
            } // if (new_cuboid_count == 0)
        } // while (z-direction)
    } // for (z-direction)
} // create_longest_zones()

void Zoned_Design::create_cuboid_zones(Zone* zone)
{
    std::vector<Zone*> m_net_zones;
    std::vector<Zone*> m_swb_zones;
    char check_side;
    unsigned int xy;
    unsigned int yx;
    unsigned int zone_expansion_count = 0;
    size_t zones = m_zones.size();
    if (zone->get_type() == 1 || zone->get_type() == 21)
    {
        check_side = 'n';
        xy = 0;
        yx = 1;
    }
    if (zone->get_type() == 2 || zone->get_type() == 22)
    {
        check_side = 'e';
        xy = 1;
        yx = 0;
    }
    /*if (zone->get_cuboid_count() == 1)
    {
        zone->tag_for_type_change(3);
    }
    else*/
    {
        bool end_loop = false;
        unsigned int loop_count = 0;
        while (end_loop == false)
        {
            bool max_span_reached = false;

            for (size_t j = 0; j < zones; j++)
            {
                if (zone != m_zones[j] && m_zones[j]->get_type() == zone->get_type() &&
                    m_zones[j]->get_min_coords(xy) <= zone->get_min_coords(xy) &&
                    m_zones[j]->get_max_coords(xy) >= zone->get_max_coords(xy) &&
                    m_zones[j]->get_min_coords(2) == zone->get_min_coords(2)
                   )
                {
                    if (m_zones[j]->get_min_coords(yx) >= zone->get_max_coords(yx))
                    {
                        m_net_zones.push_back(m_zones[j]);
                    }
                    if (m_zones[j]->get_max_coords(yx) <= zone->get_min_coords(yx))
                    {
                        m_swb_zones.push_back(m_zones[j]);
                    }
                }
            }
            if (zone->get_part_spaces() == false && zone->get_dimensions(yx) >= min_span)
            {
                Zone* whole_space_zone = new Zone(m_cuboids);
                whole_space_zone->duplicate(zone);
                m_temp_zones.push_back(whole_space_zone);
            }
            Zone* temp_zone = new Zone(m_cuboids);
            temp_zone->duplicate(zone);
            while (check_side == 'n' || check_side == 's' || check_side == 'e' || check_side == 'w' ||
                   check_side == 'N' || check_side == 'S' || check_side == 'E' || check_side == 'W'
                  )
            {
                zone_expansion_count = 0;
                if (check_side == 'n' || check_side == 'e' || check_side == 'N' || check_side == 'E' )
                {
                    for (unsigned int j = 0; j < m_net_zones.size(); j++)
                    {
                        if (m_net_zones[j]->get_min_coords(yx) == temp_zone->get_max_coords(yx))
                        {
                            Zoned_Design::prepare_zone(temp_zone);
                            if (temp_zone->get_dimensions(xy) <= max_span ||
                                (temp_zone->get_dimensions(yx) + m_net_zones[j]->get_dimensions(yx)) <= max_span
                               )
                            {
                                for (unsigned int k = 0; k < m_net_zones[j]->get_cuboid_count(); k++)
                                {
                                    if (m_net_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) >= temp_zone->get_min_coords(xy) &&
                                        m_net_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) <= temp_zone->get_max_coords(xy)
                                       )
                                    {
                                        temp_zone->add_cuboid(m_net_zones[j]->get_cuboid(k));
                                        temp_zone->add_cuboid_ID(m_net_zones[j]->get_cuboid(k)->get_ID());
                                        if (m_net_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) == temp_zone->get_max_coords(xy))
                                        {
                                            temp_zone->max_coords(m_net_zones[j]->get_cuboid(k));
                                        }
                                    }
                                }
                                zone_expansion_count++;
                                if (whole_spaces_only == true)
                                {
                                    if (Zoned_Design::check_double_zones(temp_zone) == false)
                                    {
                                        Zone* whole_space_zone = new Zone(m_cuboids);
                                        whole_space_zone->duplicate(temp_zone);
                                        Zoned_Design::add_zone(whole_space_zone, 3);
                                        zones++;
                                    }
                                }
                                Zoned_Design::prepare_zone(temp_zone);
                                if (temp_zone->get_part_spaces() == false && temp_zone->get_dimensions(yx) >= min_span)
                                {
                                    Zone* whole_space_zone = new Zone(m_cuboids);
                                    whole_space_zone->duplicate(temp_zone);
                                    m_temp_zones.push_back(whole_space_zone);
                                }
                                break;
                            }
                            else if (temp_zone->get_dimensions(xy) >= max_span &&
                                     (temp_zone->get_dimensions(yx) + m_net_zones[j]->get_dimensions(yx)) >= max_span
                                    )
                            {
                                max_span_reached = true;
                                break;
                            }
                        }
                    }
                    if (check_side == 'n')
                        check_side = 's';
                    else if (check_side == 'e')
                        check_side = 'w';
                    else if (check_side == 'N')
                        check_side = 'N';
                    else if (check_side == 'E')
                        check_side = 'E';
                }
                if (check_side == 's' || check_side == 'w' || check_side == 'S' || check_side == 'W' )
                {
                    for (unsigned int j = 0; j < m_swb_zones.size(); j++)
                    {
                        if (m_swb_zones[j]->get_max_coords(yx) == temp_zone->get_min_coords(yx))
                        {
                            Zoned_Design::prepare_zone(temp_zone);
                            if (temp_zone->get_dimensions(xy) <= max_span ||
                                (temp_zone->get_dimensions(yx) + m_swb_zones[j]->get_dimensions(yx)) <= max_span
                               )
                            {
                                for (unsigned int k = 0; k < m_swb_zones[j]->get_cuboid_count(); k++)
                                {
                                    if (m_swb_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) >= temp_zone->get_min_coords(xy) &&
                                        m_swb_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) <= temp_zone->get_max_coords(xy)
                                       )
                                    {
                                        temp_zone->add_cuboid(m_swb_zones[j]->get_cuboid(k));
                                        temp_zone->add_cuboid_ID(m_swb_zones[j]->get_cuboid(k)->get_ID());
                                        if (m_swb_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) == temp_zone->get_min_coords(xy))
                                        {
                                            temp_zone->min_coords(m_swb_zones[j]->get_cuboid(k));
                                        }
                                    }
                                }
                                zone_expansion_count++;
                                if (whole_spaces_only == true)
                                {
                                    if (Zoned_Design::check_double_zones(temp_zone) == false)
                                    {
                                        Zone* whole_space_zone = new Zone(m_cuboids);
                                        whole_space_zone->duplicate(temp_zone);
                                        Zoned_Design::add_zone(whole_space_zone, 3);
                                        zones++;
                                    }
                                }
                                Zoned_Design::prepare_zone(temp_zone);
                                if (temp_zone->get_part_spaces() == false && temp_zone->get_dimensions(yx) >= min_span)
                                {
                                    Zone* whole_space_zone = new Zone(m_cuboids);
                                    whole_space_zone->duplicate(temp_zone);
                                    m_temp_zones.push_back(whole_space_zone);
                                }
                                break;
                            }
                            else if (temp_zone->get_dimensions(xy) >= max_span &&
                                     (temp_zone->get_dimensions(yx) + m_swb_zones[j]->get_dimensions(yx)) >= max_span
                                    )
                            {
                                max_span_reached = true;
                                break;
                            }
                        }
                    }
                    if (check_side == 's')
                        check_side = 'n';
                    else if (check_side == 'w')
                        check_side = 'e';
                    else if (check_side == 'S')
                        check_side = 'S';
                    else if (check_side == 'W')
                        check_side = 'W';
                }
                if (zone_expansion_count == 0)
                {
                    temp_zone->add_type(zone->get_type());

                    // switch: forced whole spaces 1/2
                    if (forced_whole_spaces == true)
                    {
                        if (temp_zone->get_part_spaces() == true && m_temp_zones.size() > 0)
                        {
                            if (Zoned_Design::check_double_zones(m_temp_zones.back()) == false)
                            {
                                for (unsigned int j = 0; j < Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3)).size() ; j++)
                                {
                                    if (Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3))[j]->get_cuboid_count() <
                                        m_temp_zones.back()->get_cuboid_count())
                                        {
                                            Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3))[j]->tag_for_deletion();
                                        }
                                    else if (Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3))[j]->get_cuboid_count() >
                                        m_temp_zones.back()->get_cuboid_count())
                                        {
                                            m_temp_zones.back()->tag_for_deletion();
                                        }
                                }
                                    Zoned_Design::add_zone(m_temp_zones.back(), (temp_zone->get_type() + 3));
                                    zones++;
                            }
                            else if (Zoned_Design::check_double_zones(m_temp_zones.back()) == true &&
                                    (m_zones[Zoned_Design::get_double_zone(m_temp_zones.back())]->get_type() == 1 ||
                                    m_zones[Zoned_Design::get_double_zone(m_temp_zones.back())]->get_type() == 2))
                            {
                                    for (unsigned int j = 0; j < Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3)).size() ; j++)
                                    {
                                        if (Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3))[j]->get_cuboid_count() <
                                            m_temp_zones.back()->get_cuboid_count())
                                            {
                                                Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3))[j]->tag_for_deletion();
                                            }
                                        else if (Zoned_Design::get_intersecting_zones(m_temp_zones.back(), (temp_zone->get_type() + 3))[j]->get_cuboid_count() >
                                            m_temp_zones.back()->get_cuboid_count())
                                            {
                                                m_temp_zones.back()->tag_for_deletion();
                                            }
                                    }
                                        Zoned_Design::add_zone(m_temp_zones.back(), (temp_zone->get_type() + 3));
                                        zones++;
                            }
                        }
                    } // switch: forced whole spaces 1/2
                    if (temp_zone->get_type() == 21 || temp_zone->get_type() == 22)
                    {
                        if (Zoned_Design::check_double_zones(temp_zone) == false && temp_zone->get_part_spaces() == false)
                        {
                            Zoned_Design::add_zone(temp_zone, 4);
                            //zones++;
                        }
                        else if (Zoned_Design::check_double_zones(temp_zone) == true && m_zones[Zoned_Design::get_double_zone(temp_zone)]->get_type() == temp_zone->get_type())
                        {
                            m_zones[Zoned_Design::get_double_zone(temp_zone)]->tag_for_type_change(4);
                        }
                    }
                    else if (Zoned_Design::check_double_zones(temp_zone) == false)
                    {
                        Zoned_Design::add_zone(temp_zone, 3);
                        zones++;
                    }
                    else if (m_zones[Zoned_Design::get_double_zone(temp_zone)]->get_type() != 3 &&
                            (check_side == 'n' || check_side == 'e') && loop_count == 0)
                    {
                        m_zones[Zoned_Design::get_double_zone(temp_zone)]->tag_for_type_change(3);
                    }

                    m_temp_zones.clear();
                    m_net_zones.clear();
                    m_swb_zones.clear();
                    if (check_side == 'n' || check_side == 's')
                        check_side = 'x';
                    if (check_side == 'e' || check_side == 'w')
                        check_side = 'y';
                    if (check_side == 'N')
                        check_side = 'X';
                    if (check_side == 'E')
                        check_side = 'Y';
                    if (check_side == 'S' || check_side == 'W')
                        check_side = 'Z';
                }

            }
            loop_count++;
            if (max_span_reached == false && (check_side == 'x' || check_side == 'y'))
                end_loop = true;
            if (max_span_reached == true && check_side == 'x' && loop_count == 1)
                check_side = 's';
            if (max_span_reached == true && check_side == 'y' && loop_count == 1)
                check_side = 'w';
            if (max_span_reached == true && check_side == 'x' && loop_count == 2)
                check_side = 'N';
            if (max_span_reached == true && check_side == 'y' && loop_count == 2)
                check_side = 'E';
            if (check_side == 'X')
                check_side = 'S';
            if (check_side == 'Y')
                check_side = 'W';
            if (check_side == 'Z')
                end_loop = true;
        }
    }
} // create_cuboid_zones()

void Zoned_Design::get_floors()
{
    std::vector<int> below_ground;
    std::vector<int> above_ground;
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        int z_min = m_zones[i]->get_min_coords(2);
        int z_max = m_zones[i]->get_max_coords(2);
        if (z_min < 0 && std::binary_search(below_ground.begin(), below_ground.end(), z_min) == false)
        {
            below_ground.push_back(z_min);
            std::sort(below_ground.begin(), below_ground.end());
        }
        if (z_max < 0 && std::binary_search(below_ground.begin(), below_ground.end(), z_max) == false)
        {
            below_ground.push_back(z_max);
            std::sort(below_ground.begin(), below_ground.end());
        }
        if (z_min >= 0 && std::binary_search(above_ground.begin(), above_ground.end(), z_min) == false)
        {
            above_ground.push_back(z_min);
            std::sort(above_ground.begin(), above_ground.end());
        }
        if (z_max >= 0 && std::binary_search(above_ground.begin(), above_ground.end(), z_max) == false)
        {
            above_ground.push_back(z_max);
            std::sort(above_ground.begin(), above_ground.end());
        }
    }
    for (unsigned int i = 1; i <= below_ground.size(); i++)
    {
        m_floors.push_back(-i);
        m_floor_coords.push_back(below_ground[i-1]);
    }
    for (unsigned int i = 0; i < above_ground.size(); i++)
    {
        m_floors.push_back(i);
        m_floor_coords.push_back(above_ground[i]);
    }
    std::sort(m_floors.begin(), m_floors.end());
    std::sort(m_floor_coords.begin(), m_floor_coords.end());
} // get_floors()

void Zoned_Design::add_floors()
{
    for (unsigned int i = 0; i < m_floor_coords.size(); i++)
    {
        for (unsigned int j = 0; j < m_zones.size(); j++)
        {
            if (m_zones[j]->get_min_coords(2) == m_floor_coords[i])
            {
                m_zones[j]->add_floor(m_floors[i]);
            }
            else if (m_zones[j]->get_max_coords(2) == m_floor_coords[i])
            {
                m_zones[j]->add_floor_above(m_floors[i]);
            }
        }
    }
} // add_floors()

void Zoned_Design::create_largest_cuboid_zones(Zone* zone)
{
    Zone* temp_zone = new Zone(m_cuboids);
    temp_zone->duplicate(zone);
    int x_min = temp_zone->get_min_coords(0);
    int y_min = temp_zone->get_min_coords(1);
    int x_max = temp_zone->get_max_coords(0);
    int y_max = temp_zone->get_max_coords(1);
    unsigned int cuboid_count = m_CF->get_cuboid_count();
    unsigned int zone_cuboid_count = zone->get_cuboid_count();
    bool bottom_floor = false;
    bool top_floor = false;
    char check_side = 'x';

    if (zone->get_floor() == m_floors.front())
        bottom_floor = true;

    else if (zone->get_floor() == m_floors.back())
        top_floor = true;

    if(bottom_floor == true && top_floor == false)
        check_side = 't';

    else if(bottom_floor == false && top_floor == true)
        check_side = 'b';

    else if(bottom_floor == false && top_floor == false)
        check_side = 'T';

    unsigned int zone_expansion_count = 0;
    unsigned int loop_count = 0;
    while (check_side == 't' || check_side == 'b' || check_side == 'T' || check_side == 'B')
    {
			std::cout << "loop_count: " << loop_count << " - zone_expansion_count: " << zone_expansion_count << " - check_side: " << check_side << std::endl;
        int z_min = temp_zone->get_min_coords(2);
        int z_max = temp_zone->get_max_coords(2);
        for (unsigned int i = 0; i < cuboid_count; i++)
        {
            Geometry::Cuboid* temp_cuboid = m_CF->get_cuboid(i);
            if (temp_cuboid->get_min_vertex()->get_coords()(0) >= x_min &&
                temp_cuboid->get_min_vertex()->get_coords()(1) >= y_min &&
                temp_cuboid->get_max_vertex()->get_coords()(0) <= x_max &&
                temp_cuboid->get_max_vertex()->get_coords()(1) <= y_max
               )
            {
                if ((check_side == 't' || check_side == 'T') && temp_cuboid->get_min_vertex()->get_coords()(2) == z_max)
                    m_cuboids.push_back(temp_cuboid);

                else if ((check_side == 'b' || check_side == 'B') && temp_cuboid->get_max_vertex()->get_coords()(2) == z_min)
                    m_cuboids.push_back(temp_cuboid);
            }
        }
        std::sort(m_cuboids.begin(), m_cuboids.end());
        loop_count++;
				std::cout << "m_couboids.size(): " << m_cuboids.size() << "  - zone_cuboid_count: " << zone_cuboid_count << std::endl;
        if (m_cuboids.size() == zone_cuboid_count)
        {
            zone_expansion_count++;
            zone->tag_for_deletion();
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_cuboids() == m_cuboids)
                {
                    if (zone->get_type() == 5 && m_zones[i]->get_type() == 3 && m_zones[i]->get_part_spaces() == false)
                    {
                        zone->tag_for_type_change(3);
                    }
                    m_zones[i]->tag_for_deletion();
                    break;
                }
            }
            for (unsigned int i = 0; i < m_cuboids.size(); i++)
            {
                temp_zone->add_cuboid(m_cuboids[i]);
                temp_zone->add_cuboid_ID(m_cuboids[i]->get_ID());
                if ((check_side == 't' || check_side == 'T') &&
                    m_cuboids[i]->get_max_vertex()->get_coords()(0) == x_max &&
                    m_cuboids[i]->get_max_vertex()->get_coords()(1) == y_max
                   )
                   {
                        temp_zone->max_coords(m_cuboids[i]);
                   }

                else if ((check_side == 'b' || check_side == 'B') &&
                         m_cuboids[i]->get_min_vertex()->get_coords()(0) == x_min &&
                         m_cuboids[i]->get_min_vertex()->get_coords()(1) == y_min
                        )
                   {
                        temp_zone->min_coords(m_cuboids[i]);
                   }
            }
            if (whole_spaces_only == true)
            {
                if (Zoned_Design::check_double_zones(temp_zone) == false)
                {
                   Zone* whole_space_zone = new Zone(m_cuboids);
                   whole_space_zone->duplicate(temp_zone);
                   Zoned_Design::add_zone(whole_space_zone, 8);
                }
            }

        }

        m_cuboids.clear();
        if((check_side == 't' || check_side == 'b') && loop_count > zone_expansion_count)
        {
            if (zone_expansion_count == 0)
            {
                if (zone->get_type() == 3)
                    zone->add_type(6);
                else if (zone->get_type() == 5)
                    zone->add_type(7);
                check_side = 'x';
            }

            else if (zone_expansion_count > 0 && zone->get_type() != 5)
            {
                Zoned_Design::delete_tagged_zones();
                Zoned_Design::add_zone(temp_zone, 6);
                check_side = 'x';
            }
            else if (zone_expansion_count > 0 && zone->get_type() == 5)
            {
                Zoned_Design::prepare_zone(temp_zone);
                if (temp_zone->get_part_spaces() == false)
                {
                    if (zone->check_type_change() == false)
                    {
                        Zoned_Design::add_zone(temp_zone, 7);
                    }
                    else if (zone->check_type_change() == true)
                    {
                        Zoned_Design::add_zone(temp_zone, 6);
                    }
                    Zoned_Design::delete_tagged_zones();
                    check_side = 'x';
                }
                else
                {
                    Zoned_Design::preserve_tagged_zones();
                    zone->untag_for_type_change();
                    zone->add_type(7);
                    check_side = 'x';
                }
            }
        }
        else if (loop_count > zone_expansion_count&& check_side == 'B')
        {
            if (zone_expansion_count == 0)
            {
                if (zone->get_type() == 3)
                    zone->add_type(6);
                else if (zone->get_type() == 5)
                    zone->add_type(7);
                check_side = 'x';
            }

            else if (zone_expansion_count > 0 && zone->get_type() != 5)
            {
                Zoned_Design::delete_tagged_zones();
                Zoned_Design::add_zone(temp_zone, 6);
                check_side = 'x';
            }
            else if (zone_expansion_count > 0 && zone->get_type() == 5)
            {
                Zoned_Design::prepare_zone(temp_zone);
                if (temp_zone->get_part_spaces() == false)
                {
                    if (zone->check_type_change() == false)
                    {
                        Zoned_Design::add_zone(temp_zone, 7);
                    }
                    else if (zone->check_type_change() == true)
                    {
                        Zoned_Design::add_zone(temp_zone, 6);
                    }
                    Zoned_Design::delete_tagged_zones();
                    check_side = 'x';
                }
                else
                {
                    Zoned_Design::preserve_tagged_zones();
                    zone->untag_for_type_change();
                    zone->add_type(7);
                    check_side = 'x';
                }
            }
        }
        else if (loop_count > zone_expansion_count && check_side == 'T')
        {
            check_side = 'B';
            if (zone_expansion_count == 0)
            {
                zone_expansion_count = 0;
                loop_count = 0;
            }
            else
            {
                zone_expansion_count = loop_count;
            }
        }
    }
} // create_largest_cuboid_zones()

void Zoned_Design::check_floating_zones()
{
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        if (m_zones[i]->get_floor() > 0)
            m_zones[i]->tag_floating_zone();
    }
} // check_floating_zones()

void Zoned_Design::combine_floating_zones()
{
    unsigned int init_size = m_zones.size();
    for (unsigned int i = 0; i < init_size; i++)
    {
        m_temp_zones.clear();
        if (m_zones[i]->check_floating_zone() == true)
        {
            int x_min_i = m_zones[i]->get_min_coords(0);
            int y_min_i = m_zones[i]->get_min_coords(1);
            int x_max_i = m_zones[i]->get_max_coords(0);
            int y_max_i = m_zones[i]->get_max_coords(1);
            unsigned int f_i = m_zones[i]->get_floor();
            unsigned int f_a_i = m_zones[i]->get_floor_above();

            for (unsigned int j = (i + 1); j < m_zones.size(); j++)
            {
                if (m_zones[j]->check_floating_zone() == true)
                {
                    unsigned int f_j = m_zones[j]->get_floor();
                    unsigned int f_a_j = m_zones[j]->get_floor_above();

                    if ( (f_j <= f_a_i && f_a_i <= f_a_j) || (f_j <= f_i && f_i <= f_a_j) )
                    {
                        int x_min_j = m_zones[j]->get_min_coords(0);
                        int y_min_j = m_zones[j]->get_min_coords(1);
                        int x_max_j = m_zones[j]->get_max_coords(0);
                        int y_max_j = m_zones[j]->get_max_coords(1);

                        if (
                            (x_min_i < x_max_j && x_max_j <= x_max_i && y_min_i < y_max_j && y_max_j <= y_max_i) ||
                            (x_min_i <= x_min_j && x_min_j < x_max_i && y_min_i < y_max_j && y_max_j <= y_max_i) ||
                            (x_min_i <= x_min_j && x_min_j < x_max_i && y_min_i <= y_min_j && y_min_j < y_max_i) ||
                            (x_min_i < x_max_j && x_max_j <= x_max_i && y_min_i <= y_min_j && y_min_j < y_max_i)
                           )
                            m_temp_zones.push_back(m_zones[j]);
                    }
                }
            } // m_temp_zones now contains every floating zone connected to/intersecting with m_zones[i]
            for (unsigned int j = 0; j < m_temp_zones.size(); j++)
            {
                Zone* temp_zone = new Zone(m_cuboids);
                temp_zone->duplicate(m_zones[i]);
                temp_zone->combine_zones(m_temp_zones[j]);
                temp_zone->tag_combined_zone();
                if (Zoned_Design::check_double_zones(temp_zone) == false)
                    Zoned_Design::add_zone(temp_zone, 8);
                for (unsigned int k = j + 1; k < m_temp_zones.size(); k++)
                {
                    Zone* temp_zone_k = new Zone(m_cuboids);
                    temp_zone_k->duplicate(temp_zone);
                    temp_zone_k->combine_zones(m_temp_zones[k]);
                    if (Zoned_Design::check_double_zones(temp_zone_k) == false)
                        Zoned_Design::add_zone(temp_zone_k, 8);
                }
            }
        }
    }
    Zoned_Design::add_floors();
    Zoned_Design::check_floating_zones();
    m_temp_zones.clear();
} // combine_floating_zones()

bool Zoned_Design::check_cuboid_presence(Zone* zone) // checks if all cuboids of a zone occur in another [WHOLE-SPACE] zone
{
    bool found = false;
    std::vector<Geometry::Cuboid*> cuboids = zone->get_cuboids();
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        if (m_zones[i]->get_type() != 0 && m_zones[i]->get_part_spaces() == false && m_zones[i] != zone)
        {
            found = true;
            for (unsigned int j = 0; j < cuboids.size(); j++)
            {
                if (m_zones[i]->check_double_cuboids(cuboids[j]) == false)
                {
                    found = false;
                    break;
                }
            }
            if (found == true)
            {
                break;
            }
        }
    }
    return found;
} // check_cuboid_presence()

bool Zoned_Design::check_double_cuboids(Zone* zone)
{
    bool found = false;
    std::vector<Geometry::Cuboid*> cuboids = zone->get_cuboids();
    for (unsigned int i = 0; i < cuboids.size(); i++)
    {
        if (std::find(m_cuboids.begin(), m_cuboids.end(), cuboids[i]) != m_cuboids.end())
        {
            found = true;
            break;
        }
    }
    return found;
} // check_double_cuboids()

void Zoned_Design::add_cuboids(Zone* zone)
{
    std::vector<Geometry::Cuboid*> cuboids = zone->get_cuboids();
    for (unsigned int i = 0; i < cuboids.size(); i++)
    {
        m_cuboids.push_back(cuboids[i]);
    }
} // add_cuboids()

std::vector<Zone*> Zoned_Design::get_zones()
{
    return m_zones;
} // get_zones()

bool Zoned_Design::check_double_designs(Zoned_Design* zoned)
{
    bool found = false;
    for (unsigned int i = 0; i < m_zoned.size(); i++)
    {
        if (zoned->get_zones() == m_zoned[i]->get_zones())
        {
            found = true;
            break;
        }
    }
    return found;
} // check_double_designs()

bool Zoned_Design::check_double_temp_designs(Zoned_Design* zoned)
{
    bool found = false;
    for (unsigned int i = 0; i < m_temp_zoned.size(); i++)
    {
        if (zoned->get_zones() == m_temp_zoned[i]->get_zones())
        {
            found = true;
            break;
        }
    }
    return found;
} // check_double_temp_designs()

void Zoned_Design::get_missing_cuboids(Zoned_Design* zoned)
{
    m_temp_cuboids.clear();
    for (unsigned int i = 0; i < m_cuboids.size(); i++)
    {
        if (std::find(zoned->m_cuboids.begin(), zoned->m_cuboids.end(), m_cuboids[i]) == zoned->m_cuboids.end())
            m_temp_cuboids.push_back(m_cuboids[i]);
    }
    std::sort(m_temp_cuboids.begin(), m_temp_cuboids.end());
} // get_missing_cuboids()

bool Zoned_Design::check_double_appendix_zones(Zone* zone)
{
    bool found = false;
    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (zone->get_cuboid_IDs() == m_appendix_zones[i]->get_cuboid_IDs())
        {
            found = true;
            break;
        }
    }
    return found;
} // check_double_appendix_zones()

unsigned int Zoned_Design::get_double_appendix_zone(Zone* zone)
{
    unsigned int found = 0;
    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (zone->get_cuboid_IDs() == m_appendix_zones[i]->get_cuboid_IDs())
        {
            found = i;
            break;
        }
    }
    return found;
} // get_double_appendix_zone()

void Zoned_Design::add_appendix_zone(Zone* zone, unsigned int n)
{
    zone->dimensions();
    zone->add_spaces();
    zone->check_spaces();
    zone->add_type(n);
    m_appendix_zones.push_back(zone);
} // add_appendix_zone()

void Zoned_Design::create_appendix_zones(unsigned int base_type)
{
    for (unsigned int i = 0; i < m_temp_cuboids.size(); i++) // x-direction
    {
        Geometry::Cuboid* temp_cuboid = m_temp_cuboids[i];
        int x_min_temp = temp_cuboid->get_min_vertex()->get_coords()(0);
        int y_min_temp = temp_cuboid->get_min_vertex()->get_coords()(1);
        int z_min_temp = temp_cuboid->get_min_vertex()->get_coords()(2);
        int x_max_temp = temp_cuboid->get_max_vertex()->get_coords()(0);
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->min_coords(temp_cuboid);
        temp_zone->max_coords(temp_cuboid);
        temp_zone->add_cuboid(temp_cuboid);
        temp_zone->add_cuboid_ID(temp_cuboid->get_ID());


        bool end_loop = false;
        while (end_loop == false) // search connected cuboids in x-direction for temp_cuboid of temp_zone
        {
            unsigned int new_cuboid_count = 0;
            for (unsigned int j = 0; j < m_temp_cuboids.size(); j++)
            {
                if (j != i) // exclude temp_cuboid from search
                {
                    Geometry::Cuboid* new_cuboid = m_temp_cuboids[j];
                    int y_min_new = new_cuboid->get_min_vertex()->get_coords()(1);
                    int z_min_new = new_cuboid->get_min_vertex()->get_coords()(2);

                    int x_min_new = new_cuboid->get_min_vertex()->get_coords()(0);
                    int x_max_new = new_cuboid->get_max_vertex()->get_coords()(0);

                    if (y_min_new == y_min_temp && z_min_new == z_min_temp &&
                        x_min_new == x_max_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->max_coords(new_cuboid);
                        x_max_temp = x_max_new;
                    }

                    if (y_min_new == y_min_temp && z_min_new == z_min_temp &&
                        x_max_new == x_min_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->min_coords(new_cuboid);
                        x_min_temp = x_min_new;
                    }
                } // if (j != i)
            } // for (unsigned int j = 0; j < cuboid_count; j++)
            if (new_cuboid_count == 0) // no new connected cuboids found
            {
                if (Zoned_Design::check_double_appendix_zones(temp_zone) == false)
                {
                    Zoned_Design::add_appendix_zone(temp_zone, 5);
                }
                end_loop = true;
            } // if (new_cuboid_count == 0)
        } // while (x-direction)
    } // for (x-direction)

    for (unsigned int i = 0; i < m_temp_cuboids.size(); i++) // y-direction
    {
        Geometry::Cuboid* temp_cuboid = m_temp_cuboids[i];
        int x_min_temp = temp_cuboid->get_min_vertex()->get_coords()(0);
        int y_min_temp = temp_cuboid->get_min_vertex()->get_coords()(1);
        int z_min_temp = temp_cuboid->get_min_vertex()->get_coords()(2);
        int y_max_temp = temp_cuboid->get_max_vertex()->get_coords()(1);
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->min_coords(temp_cuboid);
        temp_zone->max_coords(temp_cuboid);
        temp_zone->add_cuboid(temp_cuboid);
        temp_zone->add_cuboid_ID(temp_cuboid->get_ID());


        bool end_loop = false;
        while (end_loop == false) // search connected cuboids in y-direction for temp_cuboid of temp_zone
        {
            unsigned int new_cuboid_count = 0;
            for (unsigned int j = 0; j < m_temp_cuboids.size(); j++)
            {
                if (j != i) // exclude temp_cuboid from search
                {
                    Geometry::Cuboid* new_cuboid = m_temp_cuboids[j];
                    int x_min_new = new_cuboid->get_min_vertex()->get_coords()(0);
                    int z_min_new = new_cuboid->get_min_vertex()->get_coords()(2);

                    int y_min_new = new_cuboid->get_min_vertex()->get_coords()(1);
                    int y_max_new = new_cuboid->get_max_vertex()->get_coords()(1);

                    if (x_min_new == x_min_temp && z_min_new == z_min_temp &&
                        y_min_new == y_max_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->max_coords(new_cuboid);
                        y_max_temp = y_max_new;
                    }

                    if (x_min_new == x_min_temp && z_min_new == z_min_temp &&
                        y_max_new == y_min_temp && temp_zone->check_double_cuboids(new_cuboid) == false)
                    {
                        temp_zone->add_cuboid(new_cuboid);
                        temp_zone->add_cuboid_ID(new_cuboid->get_ID());
                        new_cuboid_count++;
                        temp_zone->min_coords(new_cuboid);
                        y_min_temp = y_min_new;
                    }
                } // if (j != i)
            } // for (unsigned int j = 0; j < cuboid_count; j++)
            if (new_cuboid_count == 0) // no new connected cuboids found
            {
                if (Zoned_Design::check_double_appendix_zones(temp_zone) == false)
                {
                    Zoned_Design::add_appendix_zone(temp_zone, 6);
                }
                end_loop = true;
            } // if (new_cuboid_count == 0)
        } // while (y-direction)
    } // for (y-direction)

    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (m_appendix_zones[i]->get_dimensions(0) <= max_span || m_appendix_zones[i]->get_dimensions(1) <= max_span)
        {
            m_temp_zones.push_back(m_appendix_zones[i]);
        }
    }
    m_appendix_zones.clear();
    m_appendix_zones = m_temp_zones;
    m_temp_zones.clear();

    unsigned int init_size = m_appendix_zones.size();
    for (unsigned int i = 0; i < init_size; i++)
    {
        std::vector<Zone*> m_net_zones;
        std::vector<Zone*> m_swb_zones;
        char check_side = 'x';
        unsigned int xy = 0;
        unsigned int yx = 0;
        if (m_appendix_zones[i]->get_type() == 5)
        {
            check_side = 'n';
            xy = 0;
            yx = 1;
        }
        else if (m_appendix_zones[i]->get_type() == 6)
        {
            check_side = 'e';
            xy = 1;
            yx = 0;
        }
        unsigned int zone_expansion_count = 0;

        for (unsigned int j = 0; j < init_size; j++)
        {
            if (i != j && m_appendix_zones[j]->get_type() != 7 && m_appendix_zones[j]->get_type() != 10 &&
                m_appendix_zones[j]->get_min_coords(xy) <= m_appendix_zones[i]->get_min_coords(xy) &&
                m_appendix_zones[j]->get_max_coords(xy) >= m_appendix_zones[i]->get_max_coords(xy) &&
                m_appendix_zones[j]->get_min_coords(2) == m_appendix_zones[i]->get_min_coords(2)
               )
            {
                if (m_appendix_zones[j]->get_min_coords(yx) >= m_appendix_zones[i]->get_max_coords(yx))
                {
                    m_net_zones.push_back(m_appendix_zones[j]);
                }
                if (m_appendix_zones[j]->get_max_coords(yx) <= m_appendix_zones[i]->get_min_coords(yx))
                {
                    m_swb_zones.push_back(m_appendix_zones[j]);
                }
            }
        }
        if (m_appendix_zones[i]->get_part_spaces() == false && m_appendix_zones[i]->get_dimensions(yx) >= min_span)
        {
            Zone* whole_space_zone = new Zone(m_cuboids);
            whole_space_zone->duplicate(m_appendix_zones[i]);
            m_temp_zones.push_back(whole_space_zone);
        }
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->duplicate(m_appendix_zones[i]);
        while (check_side == 'n' || check_side == 's' || check_side == 'e' || check_side == 'w')
        {
            zone_expansion_count = 0;
            if (check_side == 'n' || check_side == 'e')
            {
                for (unsigned int j = 0; j < m_net_zones.size(); j++)
                {
                    if (m_net_zones[j]->get_min_coords(yx) == temp_zone->get_max_coords(yx))
                    {
                        if (temp_zone->get_dimensions(xy) <= max_span ||
                            (temp_zone->get_dimensions(yx) + m_net_zones[j]->get_dimensions(yx)) <= max_span
                           )
                        {
                            for (unsigned int k = 0; k < m_net_zones[j]->get_cuboid_count(); k++)
                            {
                                if (m_net_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) >= temp_zone->get_min_coords(xy) &&
                                    m_net_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) <= temp_zone->get_max_coords(xy)
                                   )
                                {
                                    temp_zone->add_cuboid(m_net_zones[j]->get_cuboid(k));
                                    temp_zone->add_cuboid_ID(m_net_zones[j]->get_cuboid(k)->get_ID());
                                    if (m_net_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) == temp_zone->get_max_coords(xy))
                                    {
                                        temp_zone->max_coords(m_net_zones[j]->get_cuboid(k));
                                    }
                                }
                            }
                            Zoned_Design::prepare_zone(temp_zone);
                            zone_expansion_count++;
                            if (temp_zone->get_part_spaces() == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                m_temp_zones.push_back(whole_space_zone);
                            }
                            break;
                        }
                    }
                }
                if (check_side == 'n')
                    check_side = 's';
                else if (check_side == 'e')
                    check_side = 'w';
            }
            if (check_side == 's' || check_side == 'w')
            {
                for (unsigned int j = 0; j < m_swb_zones.size(); j++)
                {
                    if (m_swb_zones[j]->get_max_coords(yx) == temp_zone->get_min_coords(yx))
                    {
                        if (temp_zone->get_dimensions(xy) <= max_span ||
                            (temp_zone->get_dimensions(yx) + m_swb_zones[j]->get_dimensions(yx)) <= max_span
                           )
                        {
                            for (unsigned int k = 0; k < m_swb_zones[j]->get_cuboid_count(); k++)
                            {
                                if (m_swb_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) >= temp_zone->get_min_coords(xy) &&
                                    m_swb_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) <= temp_zone->get_max_coords(xy)
                                   )
                                {
                                    temp_zone->add_cuboid(m_swb_zones[j]->get_cuboid(k));
                                    temp_zone->add_cuboid_ID(m_swb_zones[j]->get_cuboid(k)->get_ID());
                                    if (m_swb_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) == temp_zone->get_min_coords(xy))
                                    {
                                        temp_zone->min_coords(m_swb_zones[j]->get_cuboid(k));
                                    }
                                }
                            }
                            Zoned_Design::prepare_zone(temp_zone);
                            zone_expansion_count++;
                            if (temp_zone->get_part_spaces() == false)
                            {
                                Zone* whole_space_zone = new Zone(m_cuboids);
                                whole_space_zone->duplicate(temp_zone);
                                m_temp_zones.push_back(whole_space_zone);
                            }
                            break;
                        }
                    }
                }
                if (check_side == 's')
                    check_side = 'n';
                else if (check_side == 'w')
                    check_side = 'e';
            }
            if (zone_expansion_count == 0)
            {
                // switch: forced whole spaces 2/2
                if (forced_whole_spaces == true && base_type == 4)
                {
                    if (temp_zone->get_part_spaces() == true && m_temp_zones.size() > 0)
                    {
                        if (Zoned_Design::check_double_appendix_zones(m_temp_zones.back()) == false)
                        {
                            Zoned_Design::add_appendix_zone(m_temp_zones.back(), 10);
                        }
                        else if (m_appendix_zones[Zoned_Design::get_double_appendix_zone(m_temp_zones.back())]->get_type() != 7)
                        {
                            m_appendix_zones[Zoned_Design::get_double_appendix_zone(m_temp_zones.back())]->tag_for_type_change(10);
                        }
                    }
                } // switch: forced whole spaces 2/2

                if (Zoned_Design::check_double_appendix_zones(temp_zone) == false)
                {
                    Zoned_Design::add_appendix_zone(temp_zone, 7);
                }
                else if (m_appendix_zones[Zoned_Design::get_double_appendix_zone(temp_zone)]->get_type() != 7 &&
                        m_appendix_zones[Zoned_Design::get_double_appendix_zone(temp_zone)]->get_type() != 10)
                {
                    m_appendix_zones[Zoned_Design::get_double_appendix_zone(temp_zone)]->tag_for_type_change(7);
                }
                m_net_zones.clear();
                m_swb_zones.clear();
                m_temp_zones.clear();
                check_side = 'x';
            }
        }
    }
    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (m_appendix_zones[i]->check_type_change() == true)
        {
            m_appendix_zones[i]->add_type(m_appendix_zones[i]->get_type_change());
            m_appendix_zones[i]->untag_for_type_change();
        }
    }
    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (m_appendix_zones[i]->get_type() == 7 || m_appendix_zones[i]->get_type() == 10)
        {
            m_temp_zones.push_back(m_appendix_zones[i]);
        }
    }
    m_appendix_zones.clear();
    m_appendix_zones = m_temp_zones;
    m_temp_zones.clear();

    init_size = m_appendix_zones.size();
    for (unsigned int i = 0; i < init_size; i++)
    {
        std::vector<Zone*> m_net_zones;
        std::vector<Zone*> m_swb_zones;
        char check_side = 't';
        unsigned int xy = 0;
        unsigned int yx = 1;
        unsigned int zone_expansion_count = 0;

        for (unsigned int j = 0; j < init_size; j++)
        {
            if (i != j && (m_appendix_zones[j]->get_type() == 7 || m_appendix_zones[j]->get_type() == m_appendix_zones[i]->get_type())&&
                m_appendix_zones[j]->get_min_coords(xy) == m_appendix_zones[i]->get_min_coords(xy) &&
                m_appendix_zones[j]->get_min_coords(yx) == m_appendix_zones[i]->get_min_coords(yx) &&
                m_appendix_zones[j]->get_max_coords(yx) == m_appendix_zones[i]->get_max_coords(yx) &&
                m_appendix_zones[j]->get_max_coords(xy) == m_appendix_zones[i]->get_max_coords(xy) &&
                m_appendix_zones[j]->get_min_coords(2) != m_appendix_zones[i]->get_min_coords(2)
               )
            {
                if (m_appendix_zones[j]->get_min_coords(2) >= m_appendix_zones[i]->get_max_coords(2))
                {
                    m_net_zones.push_back(m_appendix_zones[j]);
                }
                else if (m_appendix_zones[j]->get_max_coords(2) <= m_appendix_zones[i]->get_min_coords(2))
                {
                    m_swb_zones.push_back(m_appendix_zones[j]);
                }
            }
        }
        if (m_appendix_zones[i]->get_part_spaces() == false)
        {
            Zone* whole_space_zone = new Zone(m_cuboids);
            whole_space_zone->duplicate(m_appendix_zones[i]);
            m_temp_zones.push_back(whole_space_zone);
        }
        Zone* temp_zone = new Zone(m_cuboids);
        temp_zone->duplicate(m_appendix_zones[i]);
        while (check_side == 't' || check_side == 'b')
        {
            zone_expansion_count = 0;
            if (check_side == 't')
            {
                for (unsigned int j = 0; j < m_net_zones.size(); j++)
                {
                    if (m_net_zones[j]->get_min_coords(2) == temp_zone->get_max_coords(2))
                    {
                        for (unsigned int k = 0; k < m_net_zones[j]->get_cuboid_count(); k++)
                        {
                            temp_zone->add_cuboid(m_net_zones[j]->get_cuboid(k));
                            temp_zone->add_cuboid_ID(m_net_zones[j]->get_cuboid(k)->get_ID());
                            if (m_net_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(xy) == temp_zone->get_max_coords(xy) &&
                                m_net_zones[j]->get_cuboid(k)->get_max_vertex()->get_coords()(yx) == temp_zone->get_max_coords(yx)
                               )
                            {
                                temp_zone->max_coords(m_net_zones[j]->get_cuboid(k));
                            }
                        }
                        Zoned_Design::prepare_zone(temp_zone);
                        zone_expansion_count++;
                        if (temp_zone->get_part_spaces() == false)
                        {
                            Zone* whole_space_zone = new Zone(m_cuboids);
                            whole_space_zone->duplicate(temp_zone);
                            m_temp_zones.push_back(whole_space_zone);
                        }
                        break;
                    }
                }
                check_side = 'b';
            }
            if (check_side == 'b')
            {
                for (unsigned int j = 0; j < m_swb_zones.size(); j++)
                {
                    if (m_swb_zones[j]->get_max_coords(2) == temp_zone->get_min_coords(2))
                    {
                        for (unsigned int k = 0; k < m_swb_zones[j]->get_cuboid_count(); k++)
                        {
                            temp_zone->add_cuboid(m_swb_zones[j]->get_cuboid(k));
                            temp_zone->add_cuboid_ID(m_swb_zones[j]->get_cuboid(k)->get_ID());
                            if (m_swb_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(xy) == temp_zone->get_min_coords(xy) &&
                                m_swb_zones[j]->get_cuboid(k)->get_min_vertex()->get_coords()(yx) == temp_zone->get_min_coords(yx)
                               )
                            {
                                temp_zone->min_coords(m_swb_zones[j]->get_cuboid(k));
                            }
                        }
                        Zoned_Design::prepare_zone(temp_zone);
                        zone_expansion_count++;
                        if (temp_zone->get_part_spaces() == false)
                        {
                            Zone* whole_space_zone = new Zone(m_cuboids);
                            whole_space_zone->duplicate(temp_zone);
                            m_temp_zones.push_back(whole_space_zone);
                        }
                        break;
                    }
                }
                check_side = 't';
            }
            if (zone_expansion_count == 0)
            {
                // switch: forced whole spaces 2/2
                if (forced_whole_spaces == true && base_type == 4)
                {
                    if (temp_zone->get_part_spaces() == true && m_temp_zones.size() > 0)
                    {
                        if (Zoned_Design::check_double_appendix_zones(m_temp_zones.back()) == false)
                        {
                            Zoned_Design::add_appendix_zone(m_temp_zones.back(), 11);
                        }
                        else if (m_appendix_zones[Zoned_Design::get_double_appendix_zone(m_temp_zones.back())]->get_type() != 8)
                        {
                            m_appendix_zones[Zoned_Design::get_double_appendix_zone(m_temp_zones.back())]->tag_for_type_change(11);
                        }
                    }
                } // switch: forced whole spaces 2/2

                if (Zoned_Design::check_double_appendix_zones(temp_zone) == false)
                {
                    Zoned_Design::add_appendix_zone(temp_zone, 8);
                }
                else if (m_appendix_zones[Zoned_Design::get_double_appendix_zone(temp_zone)]->get_type() != 8 &&
                        m_appendix_zones[Zoned_Design::get_double_appendix_zone(temp_zone)]->get_type() != 11)
                {
                    m_appendix_zones[Zoned_Design::get_double_appendix_zone(temp_zone)]->tag_for_type_change(8);
                }
                m_net_zones.clear();
                m_swb_zones.clear();
                m_temp_zones.clear();
                check_side = 'x';
            }
        }
    }
    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (m_appendix_zones[i]->check_type_change() == true)
        {
            m_appendix_zones[i]->add_type(m_appendix_zones[i]->get_type_change());
            m_appendix_zones[i]->untag_for_type_change();
        }
    }
    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (m_appendix_zones[i]->get_type() == 8 || m_appendix_zones[i]->get_type() == 11)
        {
            m_temp_zones.push_back(m_appendix_zones[i]);
        }
    }
    m_appendix_zones.clear();
    m_appendix_zones = m_temp_zones;
    m_temp_zones.clear();

    for (unsigned int i = 0; i < m_floor_coords.size(); i++)
    {
        for (unsigned int j = 0; j < m_appendix_zones.size(); j++)
        {
            if (m_appendix_zones[j]->get_min_coords(2) == m_floor_coords[i])
            {
                m_appendix_zones[j]->add_floor(m_floors[i]);
            }
            else if (m_appendix_zones[j]->get_max_coords(2) == m_floor_coords[i])
            {
                m_appendix_zones[j]->add_floor_above(m_floors[i]);
            }
        }
    }

    // switch: combine appendix zones
    if (combine_appendix_zones == true)
    {
        for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
        {
            for (unsigned int j = 0; j < m_appendix_zones.size(); j++)
            {
                if (j != i)
                {
                    for (unsigned int k = 0; k < m_appendix_zones[i]->get_cuboids().size(); k++)
                    {
                        if (m_appendix_zones[j]->check_double_cuboids(m_appendix_zones[i]->get_cuboids()[k]) == true)
                        {
                            m_appendix_zones[i]->tag_for_deletion();
                            m_appendix_zones[j]->tag_for_deletion();
                            m_temp_zones.push_back(m_appendix_zones[j]);
                            break;
                        }
                    }
                }
            }
            for (unsigned int j = 0; j < m_appendix_zones.size(); j++)
            if (m_appendix_zones[j]->check_deletion() == false && m_temp_zones.size() > 0)
            {
                for (unsigned int k = 0; k < m_temp_zones[0]->get_cuboids().size(); k++)
                {
                    if (m_appendix_zones[j]->check_double_cuboids(m_temp_zones[0]->get_cuboids()[k]) == true)
                    {
                        m_appendix_zones[j]->tag_for_deletion();
                        m_temp_zones.push_back(m_appendix_zones[j]);
                        break;
                    }
                }
            }
            Zone* temp_zone = new Zone(m_cuboids);
            temp_zone->duplicate(m_appendix_zones[i]);
            for (unsigned int j = 0; j < m_temp_zones.size(); j++)
            {
                temp_zone->combine_zones(m_temp_zones[j]);
            }
            for (unsigned int i = 0; i < m_floor_coords.size(); i++)
            {
                if (temp_zone->get_min_coords(2) == m_floor_coords[i])
                {
                    temp_zone->add_floor(m_floors[i]);
                }
                else if (temp_zone->get_max_coords(2) == m_floor_coords[i])
                {
                    temp_zone->add_floor_above(m_floors[i]);
                }
            }
            if (Zoned_Design::check_double_appendix_zones(temp_zone) == false)
            {
                temp_zone->untag_for_deletion();
                Zoned_Design::add_appendix_zone(temp_zone, 9);
            }
            m_temp_zones.clear();
        }

        // switch: delete non-combined appendix zones
        if (delete_non_combined_appendix_zones == true)
        {
            for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
            {
                if ((m_appendix_zones[i]->check_deletion() == false) || (m_appendix_zones[i]->get_type() == 9))
                    m_temp_zones.push_back(m_appendix_zones[i]);
                //if (m_appendix_zones[i]->check_deletion() == true && m_appendix_zones[i]->get_part_spaces() == false)
                    //m_temp_zones.push_back(m_appendix_zones[i]);
            }
            m_appendix_zones.clear();
            m_appendix_zones = m_temp_zones;
            m_temp_zones.clear();
        } // switch: delete non-combined appendix zones
    } // switch: combine appendix zones

    for (unsigned int i = 0; i < m_appendix_zones.size(); i++)
    {
        if (m_appendix_zones[i]->get_type() == 11)
            m_appendix_zones[i]->add_type(8);
        else if (m_appendix_zones[i]->get_whole_spaces() == true && m_appendix_zones[i]->get_part_spaces() == false)
            m_appendix_zones[i]->add_type(5);
        else if (m_appendix_zones[i]->get_whole_spaces() == true && m_appendix_zones[i]->get_part_spaces() == true)
            m_appendix_zones[i]->add_type(6);
        else if (m_appendix_zones[i]->get_whole_spaces() == false && m_appendix_zones[i]->get_part_spaces() == true)
            m_appendix_zones[i]->add_type(7);
    }
} // create_appendix_zones()

void Zoned_Design::duplicate(Zoned_Design* zoned)
{
    m_cuboids = zoned->m_cuboids;
    m_zones = zoned->m_zones;
    base_type = zoned->base_type;
} // duplicate()

std::vector<Zone*> Zoned_Design::get_intersecting_zones(Zone* zone, unsigned int n)
{
    std::vector<Zone*> intersecting_zones;
    intersecting_zones.clear();
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        for (unsigned int j = 0; j < m_zones[i]->get_cuboid_count(); j++)
        {
            if (zone != m_zones[i] && m_zones[i]->get_type() == n && zone->check_double_cuboids(m_zones[i]->get_cuboid(j)) == true)
            {
                intersecting_zones.push_back(m_zones[i]);
                break;
            }
        }
    }
    std::sort(intersecting_zones.begin(), intersecting_zones.end());
    return intersecting_zones;
} // get_intersecting_zones()

std::vector<Zoned_Design*> Zoned_Design::get_designs()
{
    return m_zoned;
} // get_designs()

std::vector<Geometry::Cuboid*> Zoned_Design::get_cuboids()
{
    return m_cuboids;
}

void Zoned_Design::add_rectangles()
{
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        for (unsigned int j = 0; j < m_zones[i]->get_rectangle_count(); j++)
        {
            if (std::find(m_rectangles.begin(), m_rectangles.end(), m_zones[i]->get_rectangle(j)) == m_rectangles.end())
            {
                m_rectangles.push_back(m_zones[i]->get_rectangle(j));
            }
        }
    }
}

unsigned int Zoned_Design::get_rectangle_count()
{
    return m_rectangles.size();
} // get rectangle count

Geometry::Rectangle* Zoned_Design::get_rectangle(unsigned int n)
{
	return m_rectangles[n];
}

void Zoned_Design::add_vertices()
{
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        for (unsigned int j = 0; j < m_zones[i]->get_vertex_count(); j++)
        {
            if (std::find(m_vertices.begin(), m_vertices.end(), m_zones[i]->get_vertex(j)) == m_vertices.end())
            {
                m_vertices.push_back(m_zones[i]->get_vertex(j));
            }
        }
    }
}

unsigned int Zoned_Design::get_vertex_count()
{
    return m_vertices.size();
} // get vertex count


void Zoned_Design::make_zoning()
{
    unsigned int cuboid_count = m_CF->get_cuboid_count();
        // add cuboid ID's and check maximum span
        for (unsigned int i = 0; i < cuboid_count; i++)
        {
            Geometry::Cuboid* temp_cuboid = m_CF->get_cuboid(i);
            temp_cuboid->add_ID(i);
            unsigned int x_span = temp_cuboid->get_max_vertex()->get_coords()(0) - temp_cuboid->get_min_vertex()->get_coords()(0);
            unsigned int y_span = temp_cuboid->get_max_vertex()->get_coords()(1) - temp_cuboid->get_min_vertex()->get_coords()(1);
            if (x_span > max_span && y_span > max_span)
            {
                if (x_span <= y_span)
                {
                    max_span = x_span;
                }
                else if (y_span < x_span)
                {
                    max_span = y_span;
                }
                std::cout << std::endl << "WARNING: Cuboid " << i + 1 << " exceeds maximum span, which is now increased to " << max_span << " mm" << std::endl;
            }
        }

        // check maximum span for spaces
        for (unsigned int i = 0; i < m_CF->get_space_count(); i++)
        {
            Geometry::Cuboid encasing_cuboid = m_CF->get_space(i)->get_encasing_cuboid();
            unsigned int x_span = encasing_cuboid.get_max_vertex()->get_coords()(0) - encasing_cuboid.get_min_vertex()->get_coords()(0);
            unsigned int y_span = encasing_cuboid.get_max_vertex()->get_coords()(1) - encasing_cuboid.get_min_vertex()->get_coords()(1);
            if (x_span > max_span && y_span > max_span)
            {
                if (x_span <= y_span)
                {
                    max_span = x_span;
                }
                else if (y_span < x_span)
                {
                    max_span = y_span;
                }
                std::cout << std::endl << "WARNING: Space " << i + 1 << " exceeds maximum span, which is now increased to " << max_span << " mm" << std::endl;
            }
        }

        // create longest zones at every floor for x- and y- direction (type 1 and 2)
        Zoned_Design::create_longest_zones();

        for (unsigned int i = 0; i < m_zones.size(); i++)
        {
            if (m_zones[i]->get_dimensions(0) <= max_span || m_zones[i]->get_dimensions(1) <= max_span)
            {
                m_temp_zones.push_back(m_zones[i]);
            }
        }
        m_zones.clear();
        m_zones = m_temp_zones;
        m_temp_zones.clear();

        // create largest cuboid-shaped zones at every floor (type 3 and 5)
        size_t zones = m_zones.size();
        for (size_t i = 0; i < zones; i++)
        {
            if (m_zones[i]->get_type() == 1 || m_zones[i]->get_type() == 2)
            {
                Zoned_Design::create_cuboid_zones(m_zones[i]);
            }
        }

        for (unsigned int i = 0; i < m_zones.size(); i++)
        {
            if (m_zones[i]->get_type() == 21 || m_zones[i]->get_type() == 22)
            {
                if (Zoned_Design::check_cuboid_presence(m_zones[i]) == false)
                {
                    Zoned_Design::create_cuboid_zones(m_zones[i]);
                }
            }
        }

        if (whole_spaces_only == true)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 1 || m_zones[i]->get_type() == 2)
                {
                    m_zones[i]->add_type(3);
                }
            }
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 3)
                {
                    m_temp_zones.push_back(m_zones[i]);
                }
            }
            m_zones.clear();
            m_zones = m_temp_zones;
            m_temp_zones.clear();

        }
        // change types and delete remaining type-1/-2 zones
        for (unsigned int i = 0; i < m_zones.size(); i++)
        {
            if (m_zones[i]->check_type_change() == true)
            {
                m_zones[i]->add_type(m_zones[i]->get_type_change());
                m_zones[i]->untag_for_type_change();
            }
            if (m_zones[i]->get_type() == 4)
            {
                m_zones[i]->add_type(5);
            }

            if ((m_zones[i]->get_type() == 3 || m_zones[i]->get_type() == 5 || m_zones[i]->get_type() == 0) &&
                m_zones[i]->check_deletion() == false)
            {
                m_temp_zones.push_back(m_zones[i]);
            }
        }
        m_zones.clear();
        m_zones = m_temp_zones;
        m_temp_zones.clear();

        // create largest cuboid-shaped zones over multiple floors
        if (whole_spaces_only == true)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 3)
                {
                    Zone* whole_space_zone = new Zone(m_cuboids);
                    whole_space_zone->duplicate(m_zones[i]);
                    Zoned_Design::add_zone(whole_space_zone, 8);

                }
            }
        }

        Zoned_Design::get_floors();
        Zoned_Design::add_floors();

        bool end_loop = false;
        while (end_loop == false)
        {
            bool all_zones_checked = true;
            for (unsigned int i = 0; i < m_floors.size(); i++)
            {
                for (unsigned int j = 0; j < m_zones.size(); j++)
                {
                    if (m_zones[j]->get_type() == 5)
                    {
                        if (m_zones[j]->get_floor() == m_floors[i])
                        Zoned_Design::create_largest_cuboid_zones(m_zones[j]);
                    }
                }
            }
            for (unsigned int i = 0; i < m_floors.size(); i++)
            {
                for (unsigned int j = 0; j < m_zones.size(); j++)
                {
                    if (m_zones[j]->get_type() == 3)
                    {
                        if (m_zones[j]->get_floor() == m_floors[i])
                        Zoned_Design::create_largest_cuboid_zones(m_zones[j]);
                    }
                }
            }
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 3 || m_zones[i]->get_type() == 5)
                {
                    all_zones_checked = false;
                    break;
                }
            }
            if (all_zones_checked == true)
            {
                Zoned_Design::add_floors();
                Zoned_Design::check_floating_zones();
                end_loop = true;
            }

        }

				if (whole_spaces_only == true)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 8 && m_zones[i]->get_whole_spaces() == true && m_zones[i]->get_part_spaces() == false)
                {
                    m_temp_zones.push_back(m_zones[i]);
                }
            }
            m_zones.clear();
            m_zones = m_temp_zones;
            m_temp_zones.clear();
        }

        for (unsigned int i = 0; i < m_zones.size(); i++)
        {
            if (m_zones[i]->get_type() == 7 && Zoned_Design::check_cuboid_presence(m_zones[i]) == true)
            {
                m_zones[i]->tag_for_deletion();
            }
        }
        m_temp_zones.clear();
        for (unsigned int i = 0; i < m_zones.size(); i++)
        {
            if (m_zones[i]->check_deletion() == false)
            {
                m_temp_zones.push_back(m_zones[i]);
            }
        }
        m_zones.clear();
        m_zones = m_temp_zones;
        m_temp_zones.clear();

        // create large vertical zones
        if (whole_spaces_only == false)
        {
            unsigned int init_size = m_zones.size();
            for (unsigned int i = 0; i < init_size; i++)
            {
                m_temp_zones.clear();
                if (m_zones[i]->check_floating_zone() == true && m_zones[i]->get_type() != 0)
                {
                    int x_min_i = m_zones[i]->get_min_coords(0);
                    int y_min_i = m_zones[i]->get_min_coords(1);
                    int x_max_i = m_zones[i]->get_max_coords(0);
                    int y_max_i = m_zones[i]->get_max_coords(1);
                    for (unsigned int j = 0; j < init_size; j++)
                    {
                        if (m_zones[j]->get_type() == 0 && m_zones[j]->get_floor_above() == m_zones[i]->get_floor_above() &&
                                        m_zones[j]->get_floor() < m_zones[i]->get_floor())
                        {
                            int x_min_j = m_zones[j]->get_min_coords(0);
                            int y_min_j = m_zones[j]->get_min_coords(1);
                            int x_max_j = m_zones[j]->get_max_coords(0);
                            int y_max_j = m_zones[j]->get_max_coords(1);
                            if (x_min_j >= x_min_i && y_min_j >= y_min_i && x_max_j <= x_max_i && y_max_j <= y_max_i)
                            {
                                m_temp_zones.push_back(m_zones[j]);
                            }
                        }
                    }
                    for (unsigned int j = 0; j < m_temp_zones.size(); j++)
                    {
                        Zone* temp_zone = new Zone(m_cuboids);
                        temp_zone->duplicate(m_temp_zones[j]);
                        for (unsigned int k = 0; k < m_temp_zones.size(); k++)
                        {
                            if (k != j && Zoned_Design::get_intersecting_zones(m_temp_zones[j], 6) == Zoned_Design::get_intersecting_zones(m_temp_zones[k], 6))
                            {
                                temp_zone->combine_zones(m_temp_zones[k]);
                            }
                        }
                        if (Zoned_Design::check_double_zones(temp_zone) == false /*&& Zoned_Design::check_cuboid_presence(temp_zone) == false*/)
                        {
                            temp_zone->untag_floating_zone();
                            Zoned_Design::add_zone(temp_zone, 10);
                        }
                    }
                }
            }
        }
        m_temp_zones.clear();
        for (unsigned int i = 0; i < m_zones.size(); i++)
        {
            if (m_zones[i]->get_type() != 0)
            {
                m_temp_zones.push_back(m_zones[i]);
            }
        }
        m_zones.clear();
        m_zones = m_temp_zones;
        m_temp_zones.clear();

        // combine connected/intersecting floating zones with eachother
        //Zoned_Design::combine_floating_zones();

        // switch: zone cores
        if (zone_cores == true && whole_spaces_only == false)
        {
            for (unsigned int i = 0; i < m_CF->get_space_count(); i++)
            {
                m_spaces.push_back(m_CF->get_space(i));
            }
            for (unsigned int i = 0; i < m_spaces.size(); i++)
            {
                Geometry::Cuboid* cuboid = m_spaces[i]->get_cuboids()[0];
                for (unsigned int j = 1; j < m_spaces[i]->get_cuboids().size(); j++)
                {
                    Geometry::Cuboid* temp_cuboid = m_spaces[i]->get_cuboids()[j];
                    if (temp_cuboid->get_min_vertex()->get_coords()(2) != cuboid->get_min_vertex()->get_coords()(2))
                    {
                        Zone* temp_zone = new Zone(m_cuboids);
                        for (unsigned int k = 0; k < m_spaces[i]->get_cuboids().size(); k++)
                        {
                            temp_zone->add_cuboid(m_spaces[i]->get_cuboids()[k]);
                            temp_zone->add_cuboid_ID(m_spaces[i]->get_cuboids()[k]->get_ID());
                        }
                        temp_zone->min_coords_space(m_spaces[i]);
                        temp_zone->max_coords_space(m_spaces[i]);

                        if (Zoned_Design::check_double_zones(temp_zone) == false)
                        {
                            Zoned_Design::add_zone(temp_zone, 11);
                        }
                        else if (Zoned_Design::check_double_zones(temp_zone) == true)
                        {
                            m_zones[Zoned_Design::get_double_zone(temp_zone)]->add_type(11);
                        }
                        break;
                    }
                }
            }
        }

		Zoned_Design::add_floors();
    //re-assign zone types for combining zones
    /*
    Type 1: only whole spaces
    Type 2: whole spaces and partial spaces
    Type 3: only partial spaces
    Type 4: only whole spaces (restricted zone size)
    Type 5: appendix zone, only whole spaces
    Type 6: appendix zone, whole spaces and partial spaces
    Type 7: appendix zone, only partial spaces
    Type 8: appendix zone, only whole spaces (restricted zone size)
    Type 10: core
    */
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        if (zone_cores == true && always_use_cores == true)
        {
            if (m_zones[i]->get_type() == 11)
                m_zones[i]->add_type(10);
            else if (m_zones[i]->get_whole_spaces() == true && m_zones[i]->get_part_spaces() == false && m_zones[i]->get_type() != 7)
                m_zones[i]->add_type(1);
            else if (m_zones[i]->get_whole_spaces() == true && m_zones[i]->get_part_spaces() == true && m_zones[i]->get_type() != 7)
                m_zones[i]->add_type(2);
            else if (m_zones[i]->get_whole_spaces() == false && m_zones[i]->get_part_spaces() == true && m_zones[i]->get_type() != 7)
                m_zones[i]->add_type(3);
            else if (m_zones[i]->get_type() == 7)
                m_zones[i]->add_type(4);
        }
        else
        {
            if (m_zones[i]->get_whole_spaces() == true && m_zones[i]->get_part_spaces() == false && m_zones[i]->get_type() != 7)
                m_zones[i]->add_type(1);
            else if (m_zones[i]->get_whole_spaces() == true && m_zones[i]->get_part_spaces() == true && m_zones[i]->get_type() != 7)
                m_zones[i]->add_type(2);
            else if (m_zones[i]->get_whole_spaces() == false && m_zones[i]->get_part_spaces() == true && m_zones[i]->get_type() != 7)
                m_zones[i]->add_type(3);
            else if (m_zones[i]->get_type() == 7)
                m_zones[i]->add_type(4);
        }

    }

    // combine zones into zoned designs
    m_cuboids.clear();
    for (unsigned int i = 0; i < cuboid_count; i++)
    {
        m_cuboids.push_back(m_CF->get_cuboid(i));
    }

    size_t designs = 0;
    size_t temp_designs = 0;
    unsigned int expansion = 0;
    // switch: whole and partial spaces
    if (whole_spaces_only == false && whole_space_zones == false)
    {
        // Base zones:
        //switch: always use cores
        if (zone_cores == true && always_use_cores == true)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 10)
                {
                    Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                    temp_zoned->m_zones.push_back(m_zones[i]);
                    temp_zoned->add_cuboids(m_zones[i]);
                    temp_zoned->base_type = 1;
                    m_zoned.push_back(temp_zoned);
                }
            }

            designs = m_zoned.size();
            if (designs > 0)
            {
                for (size_t i = 0; i < designs; i++)
                {
                    expansion = 0;
                    for (unsigned int j = 0; j < m_zones.size(); j++)
                    {

                        Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                        temp_zoned->duplicate(m_zoned[i]);
                        if (m_zones[j]->get_type() == 10 && temp_zoned->check_double_cuboids(m_zones[j]) == false)
                        {
                            expansion++;
                            temp_zoned->m_zones.push_back(m_zones[j]);
                            temp_zoned->add_cuboids(m_zones[j]);
                            std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                            std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                            if (Zoned_Design::check_double_designs(temp_zoned) == false)
                            {
                                m_zoned.push_back(temp_zoned);
                                designs++;
                            }
                        }
                    }
                    if (expansion > 0)
                    {
                        m_zoned.erase(m_zoned.begin() + i);
                        designs--;
                        i--;
                    }
                }

                for (unsigned int i = 0; i < m_zones.size(); i++)
                {
                    if (m_zones[i]->get_type() != 10)
                    {
                        for (unsigned int j = 0; j < m_zoned.size(); j++)
                        {
                            Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                            temp_zoned->duplicate(m_zoned[j]);
                            if (temp_zoned->check_double_cuboids(m_zones[i]) == false)
                            {
                                temp_zoned->m_zones.push_back(m_zones[i]);
                                temp_zoned->add_cuboids(m_zones[i]);
                                temp_zoned->base_type = m_zones[i]->get_type();
                                m_zoned.push_back(temp_zoned);
                            }
                        }
                    }
                }
            } // if designs > 0
        } // always use cores

        if (designs == 0)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                temp_zoned->m_zones.push_back(m_zones[i]);
                temp_zoned->add_cuboids(m_zones[i]);
                temp_zoned->base_type = m_zones[i]->get_type();
                m_zoned.push_back(temp_zoned);
            }
        }

        designs = m_zoned.size();
        for (size_t i = 0; i < designs; i++)
        {
            expansion = 0;
            for (unsigned int j = 0; j < m_zones.size(); j++)
            {

                Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                temp_zoned->duplicate(m_zoned[i]);
                if (temp_zoned->check_double_cuboids(m_zones[j]) == false &&
                    temp_zoned->base_type != 4 &&
                    (m_zones[j]->get_type() == 1 || m_zones[j]->get_type() == 2 || m_zones[j]->get_type() == 3))
                {
                    if (delete_expanded_designs == true)
                    {
                        expansion++;
                    }
                    temp_zoned->m_zones.push_back(m_zones[j]);
                    temp_zoned->add_cuboids(m_zones[j]);
                    std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                    std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                    if (Zoned_Design::check_double_designs(temp_zoned) == false)
                    {
                        m_zoned.push_back(temp_zoned);
                        designs++;
                    }
                }
                else if (temp_zoned->check_double_cuboids(m_zones[j]) == false &&
                    temp_zoned->base_type == 4 &&
                    (m_zones[j]->get_type() == 1 || m_zones[j]->get_type() == 4))
                {
                    //if (delete_expanded_designs == true)
                    {
                        expansion++;
                    }
                    temp_zoned->m_zones.push_back(m_zones[j]);
                    temp_zoned->add_cuboids(m_zones[j]);
                    std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                    std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                    if (Zoned_Design::check_double_designs(temp_zoned) == false)
                    {
                        m_zoned.push_back(temp_zoned);
                        designs++;
                    }
                }
            }
            if (expansion > 0)
            {
                m_zoned.erase(m_zoned.begin() + i);
                designs--;
                i--;
            }
        }

        // create appendix zones:
        designs = m_zoned.size();
        for (size_t i = 0; i < designs; i++)
        {
            if (m_zoned[i]->m_cuboids.size() < m_cuboids.size())
            {
                unsigned int last_appendix = m_zones.size();
                Zoned_Design::get_missing_cuboids(m_zoned[i]);
                Zoned_Design::create_appendix_zones(m_zoned[i]->base_type);
                for (unsigned int j = 0; j < m_appendix_zones.size(); j++)
                {
                    if (m_zoned[i]->base_type != 4 && m_appendix_zones[j]->get_type() != 8)
                    {
                        if (Zoned_Design::check_double_zones(m_appendix_zones[j]) == false)
                        {
                            Zoned_Design::add_zone(m_appendix_zones[j], m_appendix_zones[j]->get_type());
                        }
                        else if (Zoned_Design::check_double_zones(m_appendix_zones[j]) == true)
                        {
                            m_zones.push_back(m_zones[Zoned_Design::get_double_zone(m_appendix_zones[j])]);
                            m_zones.erase(m_zones.begin() + Zoned_Design::get_double_zone(m_appendix_zones[j]));
                            last_appendix--;
                        }
                    }
                    else if (m_zoned[i]->base_type == 4)
                    {
                        if (Zoned_Design::check_double_zones(m_appendix_zones[j]) == false)
                        {
                            Zoned_Design::add_zone(m_appendix_zones[j], m_appendix_zones[j]->get_type());
                        }
                        else if (Zoned_Design::check_double_zones(m_appendix_zones[j]) == true)
                        {
                            m_zones.push_back(m_zones[Zoned_Design::get_double_zone(m_appendix_zones[j])]);
                            m_zones.erase(m_zones.begin() + Zoned_Design::get_double_zone(m_appendix_zones[j]));
                            last_appendix--;
                        }

                    }
                }
                m_appendix_zones.clear();
                m_temp_zoned.push_back(m_zoned[i]);
                m_zoned.erase(m_zoned.begin() + i);
                i--;
                designs--;
                temp_designs = m_temp_zoned.size();
                for (size_t j = 0; j < temp_designs; j++)
                {
                    expansion = 0;
                    for (unsigned int k = last_appendix; k < m_zones.size(); k++)
                    {

                        Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                        temp_zoned->duplicate(m_temp_zoned[j]);
                        if (temp_zoned->check_double_cuboids(m_zones[k]) == false &&
                            temp_zoned->base_type != 4) //&& m_zones[k]->get_type() != 8)
                        {
                            expansion++;
                            temp_zoned->m_zones.push_back(m_zones[k]);
                            temp_zoned->add_cuboids(m_zones[k]);
                            std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                            std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                            if (Zoned_Design::check_double_temp_designs(temp_zoned) == false)
                            {
                                m_temp_zoned.push_back(temp_zoned);
                                temp_designs++;
                            }
                        }
                        if (temp_zoned->check_double_cuboids(m_zones[k]) == false &&
                            temp_zoned->base_type == 4 &&
                            (m_zones[k]->get_type() == 5 || m_zones[k]->get_type() == 1 ||
                            m_zones[k]->get_type() == 4 || m_zones[k]->get_type() == 8))
                        {
                            expansion++;
                            temp_zoned->m_zones.push_back(m_zones[k]);
                            temp_zoned->add_cuboids(m_zones[k]);
                            std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                            std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                            if (Zoned_Design::check_double_temp_designs(temp_zoned) == false)
                            {
                                m_temp_zoned.push_back(temp_zoned);
                                temp_designs++;
                            }
                        }
                    }
                    if (expansion > 0)
                    {
                        m_temp_zoned.erase(m_temp_zoned.begin() + j);
                        temp_designs--;
                        j--;
                    }
                }
                for (unsigned int j = 0; j < m_temp_zoned.size(); j++)
                {
                    Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                    temp_zoned->duplicate(m_temp_zoned[j]);
                    if (Zoned_Design::check_double_designs(temp_zoned) == false)
                    {
                    m_zoned.push_back(temp_zoned);
                    designs++;
                    }
                }
                m_temp_zoned.clear();

            } // if zoned.cuboids < cuboids
        } // create appendix zones
    } // switch: whole and partial spaces

    // switch: whole spaces only
    if (whole_spaces_only == true || whole_space_zones == true)
    {
        // Base zones:
        //switch: always use cores
        if (zone_cores == true && always_use_cores == true)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 10)
                {
                    Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                    temp_zoned->m_zones.push_back(m_zones[i]);
                    temp_zoned->add_cuboids(m_zones[i]);
                    temp_zoned->base_type = 1;
                    m_zoned.push_back(temp_zoned);
                }
            }

            designs = m_zoned.size();
            if (designs > 0)
            {
                for (size_t i = 0; i < designs; i++)
                {
                    expansion = 0;
                    for (unsigned int j = 0; j < m_zones.size(); j++)
                    {

                        Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                        temp_zoned->duplicate(m_zoned[i]);
                        if (m_zones[j]->get_type() == 10 && temp_zoned->check_double_cuboids(m_zones[j]) == false)
                        {
                            expansion++;
                            temp_zoned->m_zones.push_back(m_zones[j]);
                            temp_zoned->add_cuboids(m_zones[j]);
                            std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                            std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                            if (Zoned_Design::check_double_designs(temp_zoned) == false)
                            {
                                m_zoned.push_back(temp_zoned);
                                designs++;
                            }
                        }
                    }
                    if (expansion > 0)
                    {
                        m_zoned.erase(m_zoned.begin() + i);
                        designs--;
                        i--;
                    }
                }

                for (unsigned int i = 0; i < m_zones.size(); i++)
                {
                    if (m_zones[i]->get_type() == 1 || m_zones[i]->get_type() == 4)
                    {
                        for (unsigned int j = 0; j < m_zoned.size(); j++)
                        {
                            Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                            temp_zoned->duplicate(m_zoned[j]);
                            if (temp_zoned->check_double_cuboids(m_zones[i]) == false)
                            {
                                temp_zoned->m_zones.push_back(m_zones[i]);
                                temp_zoned->add_cuboids(m_zones[i]);
                                temp_zoned->base_type = m_zones[i]->get_type();
                                m_zoned.push_back(temp_zoned);
                            }
                        }
                    }
                }
            } // if designs > 0
        } // always use cores

        if (designs == 0)
        {
            for (unsigned int i = 0; i < m_zones.size(); i++)
            {
                if (m_zones[i]->get_type() == 1 || m_zones[i]->get_type() == 4)
                {
                    Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                    temp_zoned->m_zones.push_back(m_zones[i]);
                    temp_zoned->add_cuboids(m_zones[i]);
                    temp_zoned->base_type = m_zones[i]->get_type();
                    m_zoned.push_back(temp_zoned);
                }
            }
        }

        designs = m_zoned.size();
        for (size_t i = 0; i < designs; i++)
        {
            expansion = 0;
            for (unsigned int j = 0; j < m_zones.size(); j++)
            {

                Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                temp_zoned->duplicate(m_zoned[i]);
                if (temp_zoned->check_double_cuboids(m_zones[j]) == false &&
                    (m_zones[j]->get_type() == 1 || m_zones[j]->get_type() == 4))
                {
                    //if (delete_expanded_designs == true && m_zoned[i]->base_type != 4)
                    {
                        expansion++;
                    }
                    temp_zoned->m_zones.push_back(m_zones[j]);
                    temp_zoned->add_cuboids(m_zones[j]);
                    std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                    std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                    if (Zoned_Design::check_double_designs(temp_zoned) == false)
                    {
                        m_zoned.push_back(temp_zoned);
                        designs++;
                    }
                }
            }
            if (expansion > 0)
            {
                m_zoned.erase(m_zoned.begin() + i);
                designs--;
                i--;
            }
        }

        // create appendix zones:
        designs = m_zoned.size();
        for (size_t i = 0; i < designs; i++)
        {
            if (m_zoned[i]->m_cuboids.size() < m_cuboids.size())
            {
                unsigned int last_appendix = m_zones.size();
                Zoned_Design::get_missing_cuboids(m_zoned[i]);
                Zoned_Design::create_appendix_zones(m_zoned[i]->base_type);
                for (unsigned int j = 0; j < m_appendix_zones.size(); j++)
                {
                    if (Zoned_Design::check_double_zones(m_appendix_zones[j]) == false)
                    {
                        Zoned_Design::add_zone(m_appendix_zones[j], m_appendix_zones[j]->get_type());
                    }
                    else if (Zoned_Design::check_double_zones(m_appendix_zones[j]) == true)
                    {
                        m_zones.push_back(m_zones[Zoned_Design::get_double_zone(m_appendix_zones[j])]);
                        m_zones.erase(m_zones.begin() + Zoned_Design::get_double_zone(m_appendix_zones[j]));
                        last_appendix--;
                    }
                }
                m_appendix_zones.clear();
                m_temp_zoned.push_back(m_zoned[i]);
                m_zoned.erase(m_zoned.begin() + i);
                i--;
                designs--;
                temp_designs = m_temp_zoned.size();
                for (size_t j = 0; j < temp_designs; j++)
                {
                    expansion = 0;
                    for (unsigned int k = last_appendix; k < m_zones.size(); k++)
                    {

                        Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                        temp_zoned->duplicate(m_temp_zoned[j]);
                        if (temp_zoned->check_double_cuboids(m_zones[k]) == false &&
                            (m_zones[k]->get_type() == 5 || m_zones[k]->get_type() == 1 ||
                            m_zones[k]->get_type() == 4|| m_zones[k]->get_type() == 8))
                        {
                            expansion++;
                            temp_zoned->m_zones.push_back(m_zones[k]);
                            temp_zoned->add_cuboids(m_zones[k]);
                            std::sort(temp_zoned->m_zones.begin(), temp_zoned->m_zones.end());
                            std::sort(temp_zoned->m_cuboids.begin(), temp_zoned->m_cuboids.end());
                            if (Zoned_Design::check_double_temp_designs(temp_zoned) == false)
                            {
                                m_temp_zoned.push_back(temp_zoned);
                                temp_designs++;
                            }
                        }
                    }
                    if (expansion > 0)
                    {
                        m_temp_zoned.erase(m_temp_zoned.begin() + j);
                        temp_designs--;
                        j--;
                    }
                }
                for (unsigned int j = 0; j < m_temp_zoned.size(); j++)
                {
                    Zoned_Design* temp_zoned = new Zoned_Design(m_CF);
                    temp_zoned->duplicate(m_temp_zoned[j]);
                    if (Zoned_Design::check_double_designs(temp_zoned) == false)
                    {
                    m_zoned.push_back(temp_zoned);
                    designs++;
                    }
                }
                m_temp_zoned.clear();

            } // if zoned.cuboids < cuboids
        } // create appendix zones
    } // switch: whole spaces only

    // delete unused zones
    zones = m_zones.size();
    for (size_t i = 0; i < zones; i++)
    {
        bool found = false;
        // uncomment below for appendix zones only
        //if (m_zones[i]->get_type() == 5 || m_zones[i]->get_type() == 6 || m_zones[i]->get_type() == 7 || m_zones[i]->get_type() == 8)
        {
            for (unsigned int j = 0; j < m_zoned.size(); j++)
            {
                if (std::find(m_zoned[j]->m_zones.begin(), m_zoned[j]->m_zones.end(), m_zones[i]) != m_zoned[j]->m_zones.end())
                {
                    found = true;
                    break;
                }
            }
            if (found == false)
            {
                m_zones.erase(m_zones.begin() + i);
                zones--;
                i--;
            }
        }
    }

    // assign ID to zones
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        m_zones[i]->add_ID(i + 1);
    }

    // add zone ID to every cuboid for every zoned design (visualisation purpose)
    for (unsigned int i = 0; i < cuboid_count; i++)
    {
        for (unsigned int j = 0; j < m_zoned.size(); j++)
        {
            for (unsigned int k = 0; k < m_zoned[j]->m_zones.size(); k++)
            {
                if (m_zoned[j]->m_zones[k]->check_double_cuboids(m_CF->get_cuboid(i)) == true)
                {
                    m_CF->get_cuboid(i)->add_zone_ID(m_zoned[j]->m_zones[k]->get_ID());
                    break;
                }
            }
        }
    }

    // add rectangles and vertices to zones and zoned designs (for SD-grammar)
    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        m_zones[i]->add_rectangles();
        m_zones[i]->add_vertices();
    }
    for (unsigned int i = 0; i < m_zoned.size(); i++)
    {
        m_zoned[i]->add_rectangles();
        m_zoned[i]->add_vertices();
    }
    // output for control

    for (unsigned int i = 0; i < m_zones.size(); i++)
    {
        std::cout << std::endl << "Zone " << m_zones[i]->get_ID() << ": cuboids(";

        for (unsigned int j = 0; j < (m_zones[i]->get_cuboid_IDs()).size(); j++)
        {
            if (j < ((m_zones[i]->get_cuboid_IDs()).size() - 1))
                std::cout << (m_zones[i]->get_cuboid_IDs())[j] << ", ";
            else
                std::cout << (m_zones[i]->get_cuboid_IDs())[j];
        }

        std::cout << ")" << std::endl << "type: " << m_zones[i]->get_type() << " w/p: "
        << m_zones[i]->get_whole_spaces() << ", " << m_zones[i]->get_part_spaces()
        << " Floor: " << m_zones[i]->get_floor() << ", Floor above: " << m_zones[i]->get_floor_above() << std::endl
        << "Min coords: ("
        << m_zones[i]->get_min_coords(0) << ", " << m_zones[i]->get_min_coords(1) << ", "<< m_zones[i]->get_min_coords(2) << ")" << std::endl
        << "Max coords: ("
        << m_zones[i]->get_max_coords(0) << ", " << m_zones[i]->get_max_coords(1) << ", "<< m_zones[i]->get_max_coords(2) << ")" << std::endl
        << "X-dim: " << m_zones[i]->get_dimensions(0) << " Y-dim: " << m_zones[i]->get_dimensions(1) << std::endl
		<< "Floating: " << m_zones[i]->check_floating_zone() << std::endl;
    }

    for (unsigned int i = 0; i < m_zoned.size(); i++)
    {
        std::cout << std::endl << "Design " << i + 1 << ": zones(";

        for (unsigned int j = 0; j < m_zoned[i]->m_zones.size(); j++)
        {
            if (j < (m_zoned[i]->m_zones.size() - 1))
                std::cout << m_zoned[i]->m_zones[j]->get_ID() << ", ";
            else
                std::cout << m_zoned[i]->m_zones[j]->get_ID();
        }
        std::cout << ")" << std::endl;
    }

} // make_zoning()

void Zoned_Design::prepare_cuboids()
{
    unsigned int cuboid_count = m_CF->get_cuboid_count();
    // add cuboid ID's and check maximum span
    for (unsigned int i = 0; i < cuboid_count; i++)
    {
        Geometry::Cuboid* temp_cuboid = m_CF->get_cuboid(i);
        temp_cuboid->add_ID(i);
    }
}

void Zoned_Design::reset_SD_model()
{
    for (unsigned int i = 0; i < m_CF->get_vertex_count(); i++)
    {
        m_CF->get_vertex(i)->untag_zoned();
    }
    for (unsigned int i = 0; i < m_CF->get_rectangle_count(); i++)
    {
        m_CF->get_rectangle(i)->untag_zoned();
		m_CF->get_rectangle(i)->make_not_structural();
        m_CF->get_rectangle(i)->add_thickness(0);
        m_CF->get_rectangle(i)->add_loading(0);
        if (m_CF->get_rectangle(i)->get_min_vertex()->get_coords()(2) == m_CF->get_rectangle(i)->get_max_vertex()->get_coords()(2))
        {
            m_CF->get_rectangle(i)->tag_horizontal();
        }
    }
    for (unsigned int i = 0; i < m_CF->get_line_count(); i++)
    {
        m_CF->get_line(i)->untag_zoned();
        m_CF->get_line(i)->untag_constraint();
        m_CF->get_line(i)->add_thickness(0);
    }
}

void Zoned_Design::reset_zoned_SD_model()
{
    for (unsigned int i = 0; i < m_CF->get_vertex_count(); i++)
    {
        m_CF->get_vertex(i)->untag_zoned();
    }
    for (unsigned int i = 0; i < m_CF->get_rectangle_count(); i++)
    {
        m_CF->get_rectangle(i)->untag_zoned();
        m_CF->get_rectangle(i)->add_thickness(0);
        m_CF->get_rectangle(i)->add_loading(0);
        if (m_CF->get_rectangle(i)->get_min_vertex()->get_coords()(2) == m_CF->get_rectangle(i)->get_max_vertex()->get_coords()(2))
        {
            m_CF->get_rectangle(i)->tag_horizontal();
        }
    }
    for (unsigned int i = 0; i < m_CF->get_line_count(); i++)
    {
        m_CF->get_line(i)->untag_zoned();
        m_CF->get_line(i)->untag_constraint();
        m_CF->get_line(i)->add_thickness(0);
    }
}

void Zoned_Design::prepare_unzoned_SD_model()
{
    for (unsigned int i = 0; i < m_CF->get_vertex_count(); i++)
    {
        m_CF->get_vertex(i)->tag_zoned();
    }
    for (unsigned int i = 0; i < m_CF->get_rectangle_count(); i++)
    {
        if (m_CF->get_rectangle(i)->get_surface_count() != 0)
        {
            m_CF->get_rectangle(i)->tag_zoned();
            if (m_CF->get_rectangle(i)->get_min_vertex()->get_coords()(2) < m_CF->get_rectangle(i)->get_max_vertex()->get_coords()(2)) // vertical rectangle
            {
                for (unsigned int j = 0; j < 4; j++) // 4 lines in vertical rectangle; possibly relevant for foundation constraints
                {
                    if (m_CF->get_rectangle(i)->get_line_ptr(j)->get_center_vertex_ptr()->get_coords()(2) < 0.001)
                    {
                        m_CF->get_rectangle(i)->get_line_ptr(j)->tag_constraint();
                    }
                }
            }
        }
        if (adaptive_thickness == true)
        {
            unsigned int t;
            double l;
            if (m_CF->get_rectangle(i)->get_horizontal() == true && m_CF->get_rectangle(i)->get_zoned() == true)
            {
                if (m_CF->get_rectangle(i)->get_surface_count() == 1)
                {
                    Geometry::Rectangle encasing_rectangle = m_CF->get_rectangle(i)->get_surface_ptr(0)->get_encasing_rectangle();
                    unsigned int x_span = encasing_rectangle.get_max_vertex()->get_coords()(0) - encasing_rectangle.get_min_vertex()->get_coords()(0);
                    unsigned int y_span = encasing_rectangle.get_max_vertex()->get_coords()(1) - encasing_rectangle.get_min_vertex()->get_coords()(1);
                    if (x_span <= y_span)
                        t = 0.05 * x_span;
                    else
                        t = 0.05 * y_span;
                    l = 0.000025 * t;
                    m_CF->get_rectangle(i)->add_thickness(t);
                    m_CF->get_rectangle(i)->add_loading(l);
                }
                else if (m_CF->get_rectangle(i)->get_surface_count() == 2)
                {
                    Geometry::Rectangle encasing_rectangle_1 = m_CF->get_rectangle(i)->get_surface_ptr(0)->get_encasing_rectangle();
                    unsigned int x_span_1 = encasing_rectangle_1.get_max_vertex()->get_coords()(0) - encasing_rectangle_1.get_min_vertex()->get_coords()(0);
                    unsigned int y_span_1 = encasing_rectangle_1.get_max_vertex()->get_coords()(1) - encasing_rectangle_1.get_min_vertex()->get_coords()(1);
                    unsigned int span_1;
                    if (x_span_1 <= y_span_1)
                        span_1 = x_span_1;
                    else
                        span_1 = y_span_1;

                    Geometry::Rectangle encasing_rectangle_2 = m_CF->get_rectangle(i)->get_surface_ptr(0)->get_encasing_rectangle();
                    unsigned int x_span_2 = encasing_rectangle_2.get_max_vertex()->get_coords()(0) - encasing_rectangle_2.get_min_vertex()->get_coords()(0);
                    unsigned int y_span_2 = encasing_rectangle_2.get_max_vertex()->get_coords()(1) - encasing_rectangle_2.get_min_vertex()->get_coords()(1);
                    unsigned int span_2;
                    if (x_span_2 <= y_span_2)
                        span_2 = x_span_2;
                    else
                        span_2 = y_span_2;

                    if (span_1 >= span_2)
                        t = 0.05 * span_1;
                    else
                        t = 0.05 * span_2;
                    l = 0.000025 * t;
                    m_CF->get_rectangle(i)->add_thickness(t);
                    m_CF->get_rectangle(i)->add_loading(l);
                }
            }
        }
    }
}

void Zoned_Design::prepare_zoned_SD_model(unsigned int n)
{
    m_zoned[n]->m_floors = m_floors;
	m_zoned[n]->m_floor_coords = m_floor_coords;
	for (unsigned int i = 0; i < m_zoned[n]->m_vertices.size(); i++)
    {
        m_zoned[n]->m_vertices[i]->tag_zoned();
    }
    for (unsigned int i = 0; i < m_zoned[n]->m_rectangles.size(); i++)
    {
        m_zoned[n]->m_rectangles[i]->tag_zoned();
        for (unsigned int j = 0; j < 4; j++) // 4 lines in rectangle
        {
             if (m_zoned[n]->m_rectangles[i]->get_min_vertex()->get_coords()(2) <
                m_zoned[n]->m_rectangles[i]->get_line_ptr(j)->get_center_vertex_ptr()->get_coords()(2))
                m_zoned[n]->m_rectangles[i]->get_line_ptr(j)->tag_zoned();
        }
        if (m_zoned[n]->m_rectangles[i]->get_min_vertex()->get_coords()(2) < m_zoned[n]->m_rectangles[i]->get_max_vertex()->get_coords()(2)) // vertical rectangle
        {
            for (unsigned int j = 0; j < 4; j++) // 4 lines in vertical rectangle; possibly relevant for foundation constraints
            {
                if (m_zoned[n]->m_rectangles[i]->get_line_ptr(j)->get_center_vertex_ptr()->get_coords()(2) < 0.001)
                {
                    m_zoned[n]->m_rectangles[i]->get_line_ptr(j)->tag_constraint();
                }
            }
        }
    }
    if (zone_floors == true)
    {
        for (unsigned int i = 0; i < m_zoned[n]->m_zones.size(); i++)
        {
            m_zoned[n]->m_zones[i]->add_internal_floors();
			m_zoned[n]->m_zones[i]->add_vertices();
        }

        for (unsigned int i = 0; i < m_CF->get_rectangle_count(); i++)
        {
            if (m_CF->get_rectangle(i)->get_zoned() == false && m_CF->get_rectangle(i)->get_horizontal() == true && m_CF->get_rectangle(i)->get_surface_count() == 2)
            {
                m_CF->get_rectangle(i)->tag_zoned();
                for (unsigned int j = 0; j < 4; j++)
                {
                    m_CF->get_rectangle(i)->get_line_ptr(j)->tag_zoned();
                }
                for (unsigned int j = 0; j < 4; j++)
                {
                    m_CF->get_rectangle(i)->get_vertex_ptr(j)->tag_zoned();
                }
            }
        }
    }
    if (adaptive_thickness == true)
    {
        for (unsigned int i = 0; i < m_zoned[n]->m_zones.size(); i++)
        {
            for (unsigned int j = 0; j < m_zoned[n]->m_zones[i]->get_rectangle_count(); j++)
            {
                unsigned int t;
                double l;
                if (m_zoned[n]->m_zones[i]->get_rectangle(j)->get_horizontal() == true)
                {
                    unsigned int x_span = m_zoned[n]->m_zones[i]->get_dimensions(0);
                    unsigned int y_span = m_zoned[n]->m_zones[i]->get_dimensions(1);
                    if (x_span <= y_span)
                        t = 0.05 * x_span;
                    else
                        t = 0.05 * y_span;
                    l = 0.000025 * t;

                    if (m_zoned[n]->m_zones[i]->get_rectangle(j)->get_thickness() < t)
                    {
                        m_zoned[n]->m_zones[i]->get_rectangle(j)->add_thickness(t);
                        m_zoned[n]->m_zones[i]->get_rectangle(j)->add_loading(l);
                    }
                    for (int k = 0; k < 4; k++)
                    {
                        m_zoned[n]->m_zones[i]->get_rectangle(j)->get_line_ptr(k)->add_thickness(t);
                    }
                }
            }
        }
    }
}

void Zoned_Design::add_compliance(double compliance, unsigned int n)
{
    m_zoned[n]->total_compliance = compliance;
}

void Zoned_Design::add_unzoned_compliance(double compliance)
{
    total_compliance = compliance;
}

double Zoned_Design::get_unzoned_compliance()
{
    return total_compliance;
}

std::pair<double, unsigned int> Zoned_Design::get_min_compliance()
{
    std::vector<double> m_compliances;
    for (unsigned int i = 0; i < m_zoned.size(); i++)
    {
        m_compliances.push_back(m_zoned[i]->total_compliance);
    }
    std::sort(m_compliances.begin(), m_compliances.end());

    unsigned int n = 0;
    for (unsigned int i = 0; i < m_zoned.size(); i++)
    {
        n = i;
        if (m_zoned[i]->total_compliance == m_compliances[0])
            break;
    }

    std::pair<double, unsigned int> min_compliance = std::make_pair(m_compliances[0], n);

    return min_compliance;
}

double Zoned_Design::get_compliance(unsigned int n)
{
    if (n < m_zoned.size())
        return m_zoned[n]->total_compliance;
    else
        return n;
}

std::vector<int> Zoned_Design::get_zoned_floors()
{
	return m_floors;
}

std::vector<int> Zoned_Design::get_floor_coords()
{
	return m_floor_coords;
}


} // namespace Zoning
} // namespace Spatial_Design
} // namespace BSO


#endif //ZONING_CPP
