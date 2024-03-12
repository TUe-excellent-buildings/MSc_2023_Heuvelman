#ifndef SD_ANALYSIS_CPP
#define SD_ANALYSIS_CPP

#include <iostream>
#include <vector>

#include <Read_SD_Settings.hpp>

namespace BSO { namespace Structural_Design {

    SD_Analysis::SD_Analysis(std::string file_name)
    {
        //read_SD_settings(file_name, this); // this is how this constructor should be called in the future
        m_fea_init = false;
        m_FEA = new FEA;
        m_spatial_design = nullptr;

        // set element cluster to 8 regular intervals 0, 0.125, ...
        unsigned int n_clusters = 8;
        m_element_clusters.resize(n_clusters);
        for (unsigned int i = 0; i < n_clusters; i++)
        { // set boundary values of each cluster 'i' at regular intervals of 1/8
            m_element_clusters[i] = (i+1)*(1.0/n_clusters);
        }

        std::ifstream input(file_name.c_str()); // initialize input stream from file: file_name

        if (!input.is_open())
        {
            std::cerr << "Error, could not find file: " << file_name << ". Exiting now..." << std::endl;
            exit(1);
        }

        std::string line;
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
        std::string type_ID; // holds information about what type of information is described by the line currently read

        std::map<unsigned long, Components::Point*>  temp_point_map;

        while (!input.eof())
        {
            getline(input,line); // get next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }

            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            type_ID = *token; // interpret first token as type ID
            boost::algorithm::trim(type_ID);

            if (type_ID == "K")
            {
                token++; // ID
                unsigned long ID = trim_and_cast_ulong(*token);
                token++; // x
                double x = trim_and_cast_double(*token);
                token++; // y
                double y = trim_and_cast_double(*token);
                token++; // z
                double z = trim_and_cast_double(*token);

                m_points.push_back(new Components::Point(x, y, z));
                m_all_points.push_back(m_points.back());
                temp_point_map[ID] = m_points.back();
            }
            else if (type_ID == "A")
            {
                unsigned long ID;
                token++; // node ID 1
                ID = trim_and_cast_uint(*token);
                Components::Point* p_1 = temp_point_map[ID];
                token++; // node ID 2
                ID = trim_and_cast_uint(*token);
                Components::Point* p_2 = temp_point_map[ID];
                token++; // node ID 3
                ID = trim_and_cast_uint(*token);
                Components::Point* p_3 = temp_point_map[ID];
                token++; // node ID 4
                ID = trim_and_cast_uint(*token);
                Components::Point* p_4 = temp_point_map[ID];

                double t = 150;
                double E = 30000;
                double v = 0.3;

                m_components.push_back(new Components::Flat_Shell(t, E, v, p_1, p_2, p_3, p_4));
                m_flat_shells.push_back(m_components.back());
            }
            else if (type_ID == "LOAD")
            {
                unsigned long ID;
                token++; // value
                double value = trim_and_cast_double(*token);
                token++; // switch for line load on surface or volume
                int line_load = trim_and_cast_uint(*token);
                token++; // switch for surface load on volume
                int surface_load = trim_and_cast_uint(*token);
                token++; // dir
                unsigned int dir = trim_and_cast_uint(*token);
                token++; // lc
                unsigned int lc = trim_and_cast_uint(*token);

                if (line_load == 1)
                {
                    token++; // point ID 1
                    ID = trim_and_cast_uint(*token);
                    Components::Point* p_1 = temp_point_map[ID];
                    token++; // point ID 2
                    ID = trim_and_cast_uint(*token);
                    Components::Point* p_2 = temp_point_map[ID];

                    m_components.back()->add_line_load(Components::Load(lc, dir-1, value), p_1, p_2);
                }
                else if (surface_load == 1)
                {
                    // empty
                }
                else
                {
                    m_components.back()->add_load(Components::Load(lc, dir-1, value));
                }
            }
            else if (type_ID == "CONSTRAINT")
            {
                unsigned long ID;
                token++; // switch for line constraint on surface or volume
                unsigned int line_constr = trim_and_cast_uint(*token);
                token++; // switch for surface constraint on volume
                unsigned int surface_constr = trim_and_cast_uint(*token);
                token++; // direction in which movement is constrained
                unsigned int dir = trim_and_cast_uint(*token);

                if (line_constr == 1)
                {
                    token++; // point ID 1
                    ID = trim_and_cast_uint(*token);
                    Components::Point* p_1 = temp_point_map[ID];
                    token++; // point ID 2
                    ID = trim_and_cast_uint(*token);
                    Components::Point* p_2 = temp_point_map[ID];

                    m_components.back()->add_line_constraint(Components::Constraint(dir-1), p_1, p_2);
                }
                else if (surface_constr == 1)
                {
                    // empty
                }
                else
                {
                    m_components.back()->add_constraint(Components::Constraint(dir-1));
                }
            }
            else if (type_ID == "C")
            {
                token++; // node ID
                unsigned long ID = trim_and_cast_ulong(*token);
                token++; // dir
                unsigned int dir = trim_and_cast_uint(*token);

                temp_point_map[ID]->update_constraints(Components::Constraint(dir-1));
            }
            else if (type_ID == "F")
            {
                token++; // node ID
                unsigned long ID = trim_and_cast_ulong(*token);
                token++; // value
                double value = trim_and_cast_double(*token);
                token++; // direction
                unsigned int dir = trim_and_cast_uint(*token);
                token++; // load case
                unsigned int lc = trim_and_cast_uint(*token);

                temp_point_map[ID]->update_loads(Components::Load(lc, dir-1, value));
            }
        }
        input.close();
        temp_point_map.clear();

    } // ctor

    SD_Analysis::SD_Analysis(Spatial_Design::MS_Conformal& CF)
    {
        std::cout<< "Initialising SD_Analysis..." << std::endl;
        m_fea_init = false;
        m_FEA = new FEA;
        m_spatial_design = &CF;
        std::cout<< "Done" << std::endl;

        // set element cluster to 8 regular intervals 0, 0.125, ...
        unsigned int n_clusters = 8;
        std::cout<< "Resizing element clusters..." << std::endl;
        m_element_clusters.resize(n_clusters);
        std::cout<< "Setting element clusters..." << std::endl;
        for (unsigned int i = 0; i < n_clusters; i++)
        { // set boundary values of each cluster 'i' at regular intervals of 1/8
            m_element_clusters[i] = (i+1)*(1.0/n_clusters);
        }
        std::cout<< "Done" << std::endl;

        std::cout<< "Requesting SD grammar..." << std::endl;
        if(CF.request_SD_grammar() == nullptr)
        {
            std::cerr << "Error, SD grammar not found. Exiting now..." << std::endl;
            exit(1);
        }
        CF.request_SD_grammar()(&CF, this); // Assuming this returns a function pointer or functor
        std::cout<< "Meshing..." << std::endl;
        mesh(m_mesh_division);
    } // ctor

    SD_Analysis::SD_Analysis()
    {
        m_fea_init = false;

        // set element cluster to 8 regular intervals 0, 0.125, ...
        unsigned int n_clusters = 8;
        m_element_clusters.resize(n_clusters);
        for (unsigned int i = 0; i < n_clusters; i++)
        { // set boundary values of each cluster 'i' at regular intervals of 1/8
            m_element_clusters[i] = (i+1)*(1.0/n_clusters);
        }

        m_FEA = nullptr;
    } // ctor

    SD_Analysis::~SD_Analysis()
    {
        for (unsigned int i = 0; i < m_all_points.size(); i++)
        {
            delete m_all_points[i];
        }
        m_all_points.clear();
        m_points.clear();

        for (unsigned int i = 0; i < m_components.size(); i++)
        {
            delete m_components[i];
        }

        for (SD_Analysis* ite : m_previous_designs)
        {
            delete ite;
        }

        m_previous_designs.clear();
        m_components.clear();
        m_trusses.clear();
        m_beams.clear();
        m_flat_shells.clear();

        if (m_FEA != nullptr)
            delete m_FEA;
    } // dtor

    void SD_Analysis::transfer_model(SD_Analysis& new_model)
    { // this function transfers the model in this instance to the returned (by reference) model
        new_model.m_points = m_points;
        new_model.m_all_points = m_all_points;
        new_model.m_components = m_components;
        new_model.m_trusses = m_trusses;
        new_model.m_beams = m_beams;
        new_model.m_flat_shells = m_flat_shells;
        new_model.m_abstract_loads = m_abstract_loads;
        new_model.m_truss_props = m_truss_props;
        new_model.m_beam_props = m_beam_props;
        new_model.m_flat_shell_props = m_flat_shell_props;
        new_model.m_mesh_division = m_mesh_division;
		new_model.m_FEA = m_FEA;
		new_model.m_FEA->clear_system();
        new_model.m_fea_init = false;
        new_model.m_spatial_design = m_spatial_design;
        new_model.m_element_clusters = m_element_clusters;
        new_model.m_building_results = m_building_results;
		new_model.clear_mesh();

        m_points.clear();
        m_all_points.clear();
        m_components.clear();
        m_trusses.clear();
        m_beams.clear();
        m_flat_shells.clear();

        m_FEA = new FEA;
        m_fea_init = false;
        m_building_results = SD_Building_Results();
    } // transfer_model()

	void SD_Analysis::remesh()
	{
		mesh(m_mesh_division);
	} // remesh()

	void SD_Analysis::mesh(unsigned int x)
	{ // mesh components
		mesh(x, true); // mesh all components (also the ghost components)
	} // mesh()

    void SD_Analysis::mesh(unsigned int x, bool ghost)
    { // mesh the components
        // clear any mesh that may exist already
		if (m_FEA == nullptr) m_FEA = new FEA;
        clear_mesh();
		m_FEA->clear_system();
		m_mesh_division = x;

        for (unsigned int i = 0; i < m_components.size(); i++)
        {
			if (!ghost && m_components[i]->is_ghost_component()) continue;
            if (!m_components[i]->get_mesh_switch())
            { // if the component should be meshed in one element
                m_components[i]->mesh(1, m_all_points);
            }
            else
            { // if it should be meshed
                m_components[i]->mesh(x, m_all_points);
            }
        }

        // give an ID to each point in the meshed structural design
        for (unsigned long i = 0; i < m_all_points.size(); i++)
        { // for each point in the meshed structural design
            m_all_points[i]->set_ID(i+1);
        }

        for (unsigned int i = 0; i < m_all_points.size(); i++)
        { // for each point in the meshed structural design
            // add nodes and their loads and constraints
			if (i < m_points.size())
			{ // if its not meshed, i.e. was initiated by the user
				m_FEA->add_node(m_all_points[i], true);
			}
			else
			{ // if the point was meshed
				m_FEA->add_node(m_all_points[i], false);
			}
        }

        for (unsigned int i = 0; i < m_components.size(); i++)
        { // for each component in the structural design
            // add their respective elements
			if (!ghost && m_components[i]->is_ghost_component()) continue;
            m_FEA->add_elements(m_components[i]);
        }
        m_FEA->generate_system();
		m_fea_init = true;
    } // mesh()

    void SD_Analysis::clear_mesh()
    {
        // clear mesh in components
        for (unsigned int i = 0; i < m_components.size(); i++)
        {
            m_components[i]->clear_mesh();
        }

        if (m_all_points.size() > m_points.size())
        {
            // delete meshed points
            for (unsigned int i = m_points.size(); i < m_all_points.size(); i++)
            {
                delete m_all_points[i];
            }
            m_all_points.erase((m_all_points.begin() + m_points.size()), m_all_points.end());
        }
        else if (m_all_points.size() < m_points.size())
        {
            // m_all_points has not been initialised yet, so initialise it
            m_all_points = m_points;
        } // else, they have the same size and are probably already copied from one and each other

        for (unsigned int i = m_points.size(); i < m_all_points.size(); i++)
        { // reset the loads and constraints on the input nodes
            m_all_points[i]->reset_settings();
        }

        m_fea_init = false; // m_fea is about to be cleared, so whenever initialise_fea() is called it should generate the system again

        m_FEA->clear_system(); // clears all containers in this system e.g. GSM, nodes, elements, load vectors etc.

    }

    void SD_Analysis::analyse()
    {
        if (!m_fea_init)
        { // if the FEA has not been initialised yet
            m_FEA->generate_system();
            std::cout << "System generated" << std::endl;
            m_fea_init = true;
        }
        std::cout << "Solving..." << std::endl;

        std::cout<< m_FEA->get_element_count() << std::endl;
        m_FEA->solve();
        std::cout << "Done solving" << std::endl;
    }

    void SD_Analysis::cluster_element_densities(unsigned int n)
    {
        // obtain the vector with all element pointers
        std::vector<double> densities;
        for (unsigned int i = 0; i < m_FEA->m_elements.size(); i++)
        {
            densities.push_back(m_FEA->m_elements[i]->get_density());
        }

        std::sort(densities.begin(), densities.end());
        m_element_clusters.clear();
        m_element_clusters.resize(n);
        std::vector<double> new_centroids(n), centroids(n);

        /*for (unsigned int i = 0; i < n; i++)
        {
            unsigned int l_index, u_index;
            l_index = (i/(double)n) * densities.size();
            u_index = ((i+1)/(double)n) * densities.size();

            double sum = 0;
            unsigned int count = 0;
            for (unsigned int j = l_index; j < u_index; j++)
            {
                sum += densities[j];
                count++;
            }
            new_centroids[i] = sum / count;
        }*/

        /*for (unsigned int i = 0; i < n; i++)
        {
            new_centroids[i] = (1/16.0)+(i/8.0);
        }*/

        unsigned int pos = densities.size()/n;

        new_centroids[0] = densities[0];
        new_centroids[1] = densities[pos];
        new_centroids[2] = densities[pos*2];
        new_centroids[3] = densities[pos*3];
        new_centroids[4] = densities[pos*4];
        new_centroids[5] = densities[pos*5];
        new_centroids[6] = densities[pos*6];
        new_centroids[7] = densities.back();

        int switcher = 0;
        while (switcher < 1 /*new_centroids == centroids */) //new_centroids != centroids)
        {
            switcher++;
            centroids = new_centroids;

            m_element_clusters[n-1] = densities.back();
            for (unsigned int i = 0; i < n-1; i++)
            {
               m_element_clusters[i] = (centroids[i] + centroids[i+1]) / 2.0;
            }

            double sum = 0;
            unsigned int count = 0;
            unsigned int index_track = 0;

            for (unsigned int i = 0; i < densities.size(); i ++)
            {
                if (densities[i] > m_element_clusters[index_track])
                {
                    new_centroids[index_track] = sum / count;
                    index_track++;
                    sum = count = 0;
                }
                else if (i == densities.size() - 1)
                {
                    sum += densities[i];
                    count++;
                    new_centroids[index_track] = sum / count;
                }
                else
                {
                    sum += densities[i];
                    count++;
                }
            }
        }
    } // cluster_element_densities()

	void SD_Analysis::scale_dimensions(double x)
	{
		clear_mesh();
		for (auto i : m_components)
		{
			i->scale_dimensions(x);
		}
		mesh(m_mesh_division);
	} // scale_dimensions()

	void SD_Analysis::reset_scale()
	{
		clear_mesh();
		for (auto i : m_components)
		{
			i->reset_scale();
		}
		mesh(m_mesh_division);
	} // reset_scale()

    unsigned int SD_Analysis::get_component_count()
    {
        return m_components.size();
    }

    std::vector<double> SD_Analysis::get_element_clusters()
    {
        return m_element_clusters;
    }

	std::map<Components::Point*, std::vector<unsigned int> > SD_Analysis::get_points_with_free_dofs()
	{
		return get_points_with_free_dofs(5);
	}

	std::map<Components::Point*, std::vector<unsigned int> > SD_Analysis::get_points_with_free_dofs(double x)
	{
		clear_mesh();
		//unsigned int original_division = m_mesh_division;
		mesh(1,false);

		std::map<Elements::Node*, std::vector<unsigned int> > nodes_with_free_dofs = m_FEA->get_nodes_with_free_dofs(x);
		std::map<Components::Point*, std::vector<unsigned int> > points_with_free_dofs;

		for (auto i : nodes_with_free_dofs)
        {
            bool i_found = false;
            for (auto j : m_points)
            {
                if (i.first->get_coord() == j->get_coords())
                {
                    points_with_free_dofs[j] = i.second;
                    i_found = true;
                }

            }
            if (!i_found) throw std::invalid_argument("Could not match node with point when looking for singulars. (SD_Analysis.cpp)");
        }

		clear_mesh();
		//mesh(original_division);
		return points_with_free_dofs;
	}
	
	std::map<Components::Point*, std::vector<unsigned int> > SD_Analysis::get_zoned_points_with_free_dofs(double x)
	{
		clear_mesh();
		//unsigned int original_division = m_mesh_division;
		mesh(1,false);

		std::map<Elements::Node*, std::vector<unsigned int> > nodes_with_free_dofs = m_FEA->get_nodes_with_free_dofs(x);
		std::map<Components::Point*, std::vector<unsigned int> > points_with_free_dofs;

		for (auto i : nodes_with_free_dofs)
        {
            bool i_found = false;
            for (auto j : m_points)
            {
                if (i.first->get_coord() == j->get_coords())
                {
                    points_with_free_dofs[j] = i.second;
                    i_found = true;
                }

            }
            if (!i_found) throw std::invalid_argument("Could not match node with point when looking for singulars. (SD_Analysis.cpp)");
        }
		for (unsigned int i = 0; i < m_spatial_design->get_vertex_count(); i++)
		{
			Spatial_Design::Geometry::Vertex* temp_vertex = m_spatial_design->get_vertex(i);
			if (temp_vertex->get_zoned() == false)
			{
				for (unsigned int j = 0; j < m_points.size(); j++)
				{
					Components::Point* temp_point = m_points[j];
					if (temp_point->get_coords() == temp_vertex->get_coords())
					{
						points_with_free_dofs.erase(temp_point);
					}
				}
			}
		}

		clear_mesh();
		//mesh(original_division);
		return points_with_free_dofs;
	}

    Components::Component* SD_Analysis::get_component_ptr(unsigned int n)
    {
        return m_components[n];
    }

    FEA* SD_Analysis::get_FEA_ptr()
    {
        return m_FEA;
    }

    std::vector<Flat_Shell_Props> SD_Analysis::get_flat_shell_props()
    {
        return m_flat_shell_props;
    } // get_flat_shell_props()

    std::vector<Beam_Props> SD_Analysis::get_beam_props()
    {
        return m_beam_props;
    } // get_beam_props()

    std::vector<Truss_Props> SD_Analysis::get_truss_props()
    {
        return m_truss_props;
    } // get_truss_props()

    std::vector<SD_Analysis*> SD_Analysis::get_previous_designs()
    {
        return m_previous_designs;
    } // get_previous_designs()

    SD_Building_Results& SD_Analysis::get_results()
    {
        m_building_results = SD_Building_Results(); // get rid of possible old results
        m_building_results.m_element_clusters = m_element_clusters;

        for (unsigned int i = 0; i < m_components.size(); i++)
        {
            m_building_results.add_component(m_components[i]);
        }
        m_building_results.obtain_results();
        return m_building_results;
    }

    Spatial_Design::MS_Conformal* SD_Analysis::get_spatial_design_ptr()
    {
        return m_spatial_design;
    }

    std::vector<Components::Point*> SD_Analysis::get_points()
	{
	    return m_points;
	}

    std::map<std::pair<Components::Point*, unsigned int>, double> SD_Analysis::get_points_singular_values()
    {
		clear_mesh();
		unsigned int original_division = m_mesh_division;
		mesh(1);

		std::map<std::pair<Elements::Node*, unsigned int>, double> nodes_singular_values = m_FEA->get_nodes_singular_values();
		std::map<std::pair<Components::Point*, unsigned int>, double> points_singular_values;

		for (auto i : nodes_singular_values)
        {
            bool i_found = false;
            for (auto j : m_points)
            {
                if (i.first.first->get_coord() == j->get_coords())
                {
                    std::pair<Components::Point*, unsigned int> temp_pair;
                    temp_pair = std::make_pair(j, i.first.second);
                    points_singular_values[temp_pair] = i.second;
                    i_found = true;
                }

            }
            if (!i_found) throw std::invalid_argument("Could not find node with point when looking for singulars");
        }
		clear_mesh();
		mesh(original_division);
		return points_singular_values;
	}
	
	Eigen::Vector6d SD_Analysis::get_displacements()
	{
		return m_FEA->get_node_displacements(m_points.back());
	}
	
	std::map<Elements::Node*, std::vector<unsigned int> > SD_Analysis::get_nodes_with_free_dofs(double x)
	{
		return m_FEA->get_nodes_with_free_dofs(x);
	}
	

} // namespace Structural_Design
} // namespace BSO



#endif // SD_ANALYSIS_CPP
