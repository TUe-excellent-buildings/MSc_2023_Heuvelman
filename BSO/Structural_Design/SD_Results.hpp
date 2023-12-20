#ifndef SD_RESULTS_HPP
#define SD_RESULTS_HPP

#include <BSO/Structural_Design/Elements/Element.hpp>

#include <vector>
#include <map>

namespace BSO { namespace Structural_Design
{
    /*
     * Structures to store results of an SD_analysis
     */


    // Structure definitions:
    struct SD_Element_Results
    {
        Elements::Element* m_element_ptr; // pointer to an element in the finite element analysis

        bool m_deletion; // tracks if an element is deleted (objective function in Juan's thesis)
        bool m_active_in_compliance;
        double m_total_compliance; // total compliance (sum of all load case entries in m_compliances)
        std::map<unsigned int, double> m_compliances; // compliance for each load case

        double m_density; // density of the element
        double m_volume; // volume of the element

        // structure functions:
        void delete_clusters(std::vector<double>& element_clusters, unsigned int n); // decide whether or not to tag this element for deletion
        SD_Element_Results(Elements::Element*); // can only be initialised with a pointer to the instance of the concerning element
        void obtain_results(); // obtains the results for this element
    }; // SD_Elements_Results

    struct SD_Component_Results
    {
        Components::Component* m_component_ptr;
        std::vector<SD_Element_Results> m_elements; // list of all the elements that are part of this component

        unsigned int m_deletion_count; // counts how many elements are deleted in this component
        double m_total_compliance; // total compliance (sum of all load case entries in m_compliances)
        double m_struct_volume; // the volume of this component
        std::map<unsigned int, double> m_compliances; // compliance for each load case

        double m_total_normal_compliance;
        double m_total_bending_compliance;
        double m_total_shear_compliance;
        double m_total_axial_compliance;
        bool m_decoupled_compliances_available;
        bool m_is_ghost;

        // structure functions:
        void add_element(Elements::Element*); // adds an element to this component
        void delete_clusters(std::vector<double>& element_clusters, unsigned int n); // tag elements within the first 'n' clusters for deletion
        void obtain_results(); // obtains the results of all elements that are a part of this component
    }; // SD_Component_Results

    struct SD_Space_Results
    {
        int m_ID;
        unsigned int m_deletion_count; // counts how many elements are deleted for this space
        double m_floor_area;
        double m_volume;
        double m_struct_volume;
        std::vector<SD_Component_Results*> m_components; // list of all the components that are part of this space

        double m_total_compliance; // total compliance (sum of all load case entries in m_compliances)
        double m_invert_compliance; // inverted total compliance (dependant on the maximum compliance in the design)
        std::map<unsigned int, double> m_compliances; // compliance for each load case

        double m_rel_performance; // relative performance index (0-1)

        // structure functions:
        void add_component(SD_Component_Results*); // adds a component to this space
        void get_deletion_count(); // gets the number of deleted elements thta belong to this space
        void obtain_results(); // obtains the results of all components that are a part of this space
    }; // SD_Space_Results

    struct SD_Building_Results
    {
        std::vector<SD_Component_Results*> m_components; // list of all the non ghost components that are a part of this building
        std::vector<SD_Component_Results*> m_ghost_components; // list of all the components that are a part of this building
        std::vector<SD_Space_Results> m_spaces; // list of all the spaces that are a part of this building
        std::vector<double> m_element_clusters; // lists all boundary values of the clusters of element densities

        double m_total_compliance; // total compliance (sum of all load case entries in m_compliances)
        double m_total_ghost_compliance; // total ghost compliance (sum of all load cases of ghost components)
        double m_struct_volume; // total structural volume of components
        double m_struct_ghost_volume; // structural volume of ghost components
        std::map<unsigned int, double> m_compliances; // compliances for each load case of non ghost components

        int* m_number_copies; // number of copies of this structure currently out there (required to track memory release, see destructor)

        // structure functions:
        void add_component(Components::Component*); // adds a component to this building
        void delete_clusters(unsigned int n); // tags all elements in the first 'n' clusters for deletion
        void obtain_results(); // obtains the result of all components that are part of this
        SD_Building_Results(); // to initialise copy counter
        SD_Building_Results(const SD_Building_Results& rhs); // copy constructor, to handle copy count and dynamic variables
        SD_Building_Results& operator = (const SD_Building_Results& rhs); // assignment operator, to handle copy count and dynamic variables
        ~SD_Building_Results(); // to delete all allocated instances of the SD_Component_Results
    }; // SD_results



    SD_Element_Results::SD_Element_Results(Elements::Element* ele_ptr)
    {
        m_element_ptr = ele_ptr;
    } // ctor

    void SD_Element_Results::obtain_results()
    {
        m_density = m_element_ptr->get_density(); // get the current density for this element
        m_volume = m_element_ptr->get_volume(); // get the volume of this element
        m_compliances = m_element_ptr->get_energies(); // get the compliances for each load case for this element
        m_total_compliance = 0; // initialisation
        m_active_in_compliance = m_element_ptr->is_active_in_compliance();

        typedef std::map<unsigned int, double>::iterator lc_ite;
        for (lc_ite ite = m_compliances.begin(); ite != m_compliances.end(); ite++)
        { // for each load case
			double temp_energy = ite->second *2; // to vanish the 1/2 term in the calculation of strain energy
            m_total_compliance += temp_energy; // add for each load case to the total amount
        }
    } // obtain_results() (SD_Element_Results)

    void SD_Element_Results::delete_clusters(std::vector<double>& element_clusters, unsigned int n)
    {
        m_deletion = m_density < element_clusters[n-1];
        if (m_deletion)
        {
            m_element_ptr->switch_visualisation();
        }
    }


    void SD_Component_Results::add_element(Elements::Element* ele_ptr)
    {
        m_elements.push_back(SD_Element_Results(ele_ptr));
    }

    void SD_Component_Results::delete_clusters(std::vector<double>& element_clusters, unsigned int n)
    {
        m_deletion_count = 0;
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            m_elements[i].delete_clusters(element_clusters, n);
            if (m_elements[i].m_deletion)
            {
                m_deletion_count++;
            }
        }
    }

    void SD_Component_Results::obtain_results()
    {
        m_total_compliance = 0; // initialisation
        m_total_bending_compliance = 0;
        m_total_normal_compliance = 0;
        m_total_shear_compliance = 0;
        m_total_axial_compliance = 0;
        m_struct_volume = 0;
        m_decoupled_compliances_available = m_component_ptr->is_flat_shell();
        m_compliances.clear();

        for (unsigned int i = 0; i < m_component_ptr->get_element_ptr_count(); i++)
        { // for each meshed element
            add_element(m_component_ptr->get_element_ptr(i)); // get the ith element
            if (!m_is_ghost && !m_component_ptr->get_element_ptr(i)->is_active_in_compliance())
                continue;

            m_elements[i].obtain_results(); // obtain the results for this element
            m_total_compliance += m_elements[i].m_total_compliance; // add the results of each element to the results of this component
            m_struct_volume += m_elements[i].m_volume;

            if (m_decoupled_compliances_available)
            {
                m_total_normal_compliance += 2*m_elements[i].m_element_ptr->get_normal_energy();
                m_total_bending_compliance += 2*m_elements[i].m_element_ptr->get_bending_energy();
                m_total_shear_compliance += 2*m_elements[i].m_element_ptr->get_shear_energy();
                m_total_axial_compliance += 2*m_elements[i].m_element_ptr->get_axial_energy();
            }

            typedef std::map<unsigned int, double>::iterator lc_ite;
            for (lc_ite ite = m_elements[i].m_compliances.begin(); ite != m_elements[i].m_compliances.end(); ite++)
            { // for the compliance in each load case of the current component
                if (m_compliances.find(ite->first) != m_compliances.end())
                { // if the key value (load case ID) is already in the map, then add it to that mapped variable
                    m_compliances[ite->first] += ite->second;
                }
                else
                { // if the key value (load case ID) is not yet in the map, then add the key value and initialise the variable that it is mapping
                    m_compliances[ite->first] = ite->second;
                }
            }

        }
    }





    void SD_Space_Results::add_component(SD_Component_Results* comp_result_ptr)
    {
        if (!comp_result_ptr->m_is_ghost)
            m_components.push_back(comp_result_ptr);
    }

    void SD_Space_Results::get_deletion_count()
    {
        m_deletion_count = 0;

        for (unsigned int i = 0; i < m_components.size(); i++)
        {
            m_deletion_count += m_components[i]->m_deletion_count;
        }
    }

    void SD_Space_Results::obtain_results()
    {
        m_total_compliance = 0;
        m_struct_volume = 0;
        m_compliances.clear();

        for (unsigned int i = 0; i < m_components.size(); i++)
        {
            m_total_compliance += m_components[i]->m_total_compliance; // add the result to this building (this assumes that the results for the components has been executed already. e.g. in the building result structure)
            m_struct_volume += m_components[i]->m_struct_volume;
            typedef std::map<unsigned int, double>::iterator lc_ite;
            for (lc_ite ite = m_components[i]->m_compliances.begin(); ite != m_components[i]->m_compliances.end(); ite++)
            { // for the compliance in each load case of the current component
                if (m_compliances.find(ite->first) != m_compliances.end())
                { // if the key value (load case ID) is already in the map, then add it to that mapped variable
                    m_compliances[ite->first] += ite->second;
                }
                else
                { // if the key value (load case ID) is not yet in the map, then add the key value and initialise the variable that it is mapping
                    m_compliances[ite->first] = ite->second;
                }
            }
        }
    } // obtain_results() (SD_Space_Results)




    void SD_Building_Results::obtain_results()
    {
        m_total_compliance = 0;
        m_total_ghost_compliance = 0;
        m_struct_volume = 0;
        m_struct_ghost_volume = 0;
        m_compliances.clear();

        for (unsigned int i = 0; i < m_components.size(); i++)
        { // for each component
            m_components[i]->obtain_results(); // obtain the results for the considered component
            m_total_compliance += m_components[i]->m_total_compliance; // add the result to this building
            m_struct_volume += m_components[i]->m_struct_volume;

            typedef std::map<unsigned int, double>::iterator lc_ite;
            for (lc_ite ite = m_components[i]->m_compliances.begin(); ite != m_components[i]->m_compliances.end(); ite++)
            { // for the compliance in each load case of the current component
                if (m_compliances.find(ite->first) != m_compliances.end())
                { // if the key value (load case ID) is already in the map, then add it to that mapped variable
                    m_compliances[ite->first] += ite->second;
                }
                else
                { // if the key value (load case ID) is not yet in the map, then add the key value and initialise the variable that it is mapping
                    m_compliances[ite->first] = ite->second;
                }
            }
        }

        for (auto ite : m_ghost_components)
        {
            ite->obtain_results(); // obtain the results, for the ghost component
            m_total_ghost_compliance += ite->m_total_compliance; // add the result to this building
            m_struct_ghost_volume += ite->m_struct_volume;
        }

        double max_space_compliance = 0;
        for (unsigned int i = 0; i < m_spaces.size(); i++)
        { // for each space, obtain the results from the components assigned to that space
            m_spaces[i].obtain_results();
            if (max_space_compliance < m_spaces[i].m_total_compliance)
            {
                max_space_compliance = m_spaces[i].m_total_compliance;
            }

        }

        for (unsigned int i = 0; i < m_spaces.size(); i++)
        { // for each space,
            m_spaces[i].m_invert_compliance = (1-(m_spaces[i].m_total_compliance/max_space_compliance))*max_space_compliance;
        }

    } // obtain_results() (SD_Building_Results)

    void SD_Building_Results::add_component(Components::Component* comp_ptr)
    {
        SD_Component_Results* comp_res_ptr;
        if (comp_ptr->is_ghost_component())
        {
            m_ghost_components.push_back(new SD_Component_Results);
            m_ghost_components.back()->m_component_ptr = comp_ptr;
			m_ghost_components.back()->m_is_ghost = true;
            comp_res_ptr = m_ghost_components.back();
        }
        else if (comp_ptr->is_beam() || comp_ptr->is_truss() || comp_ptr->is_flat_shell())
        {
            m_components.push_back(new SD_Component_Results);
            m_components.back()->m_component_ptr = comp_ptr;
			m_components.back()->m_is_ghost = false;
            comp_res_ptr = m_components.back();
        }
		else return;

        // look for spaces that this component is related to, but which have not yet been added to this building result structure
        for (unsigned int i = 0; i < comp_ptr->get_space_ptr_count(); i++)
        { // for each space in relation with the current component
            bool space_found = false;
            for (unsigned int j = 0; j < m_spaces.size(); j++)
            { // and for each space in relation with this building

                if (comp_ptr->get_space_ptr(i)->get_ID() == m_spaces[j].m_ID)
                { // if the space has already been added to this buildings result structure
                    // add the current component to the space
                    m_spaces[j].add_component(comp_res_ptr);
                    space_found = true;
                    break;
                }
            }

            if (!space_found)
            { // if the space has not yet been added to this buildings result structure
                // add a new instance of the space result structure to the building and initialise it
                m_spaces.push_back(SD_Space_Results());
                m_spaces.back().m_ID = comp_ptr->get_space_ptr(i)->get_ID();
                m_spaces.back().m_volume = comp_ptr->get_space_ptr(i)->get_encasing_cuboid().get_volume();
                m_spaces.back().m_floor_area = m_spaces.back().m_volume /
                                               (comp_ptr->get_space_ptr(i)->get_encasing_cuboid().get_max_vertex()->get_coords()(2) -
                                                comp_ptr->get_space_ptr(i)->get_encasing_cuboid().get_min_vertex()->get_coords()(2));
                m_spaces.back().m_rel_performance = -1;

                // add the current component to the space
                m_spaces.back().add_component(comp_res_ptr);
            }
        }
    } // add_component()

    void SD_Building_Results::delete_clusters(unsigned int n)
    {
        for (unsigned int i = 0; i < m_components.size(); i++)
        {
            m_components[i]->delete_clusters(m_element_clusters, n);
        }
        for (auto ite : m_ghost_components)
        {
            ite->delete_clusters(m_element_clusters, n);
        }
    } // delete_clusters()

    SD_Building_Results::SD_Building_Results()
    {
        m_number_copies = new int;
        *m_number_copies = 1;
    }

    SD_Building_Results::SD_Building_Results(const SD_Building_Results& rhs)
    {
        m_number_copies = rhs.m_number_copies;
        (*m_number_copies)++;

        m_components = rhs.m_components;
        m_ghost_components = rhs.m_ghost_components;
        m_struct_ghost_volume = rhs.m_struct_ghost_volume;
        m_spaces = rhs.m_spaces;
		m_struct_volume = rhs.m_struct_volume;

        m_total_compliance = rhs.m_total_compliance;
        m_total_ghost_compliance = rhs.m_total_ghost_compliance;
        m_compliances = rhs.m_compliances;

        m_element_clusters = rhs.m_element_clusters;
    }

    SD_Building_Results& SD_Building_Results::operator = (const SD_Building_Results& rhs)
    {
        if (this == &rhs)
            return *this;

         m_number_copies = rhs.m_number_copies;
        (*m_number_copies)++;

        m_components = rhs.m_components;
        m_ghost_components = rhs.m_ghost_components;
        m_struct_ghost_volume = rhs.m_struct_ghost_volume;
        m_spaces = rhs.m_spaces;

        m_total_compliance = rhs.m_total_compliance;
        m_total_ghost_compliance = rhs.m_total_ghost_compliance;
        m_compliances = rhs.m_compliances;
		m_struct_volume = rhs.m_struct_volume;

        m_element_clusters = rhs.m_element_clusters;

        return *this;
    }

    SD_Building_Results::~SD_Building_Results()
    {
        (*m_number_copies)--;
        if (*m_number_copies == 0)
        { // if this is the final copy with containing this allocated memory, then release the memory
            for (unsigned int i = 0; i < m_components.size(); i++)
            {
                delete m_components[i];
            }
            for (unsigned int i = 0; i < m_ghost_components.size(); i++)
            {
                delete m_ghost_components[i];
            }
            delete m_number_copies;
        }
//        m_components.clear();
    } // dtor()


} // namepsace Structural_Design
} // namespace BSO

#endif // SD_RESULTS_HPP
