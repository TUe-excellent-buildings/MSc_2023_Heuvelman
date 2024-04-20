#ifndef SD_PROPS_VARS_HPP
#define SD_PROPS_VARS_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <vector>
#include <map>

#include <BSO/Vectors.hpp>
#include <BSO/Structural_Design/Components/Point_Comp.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>
#include <BSO/Structural_Design/Components/Truss_Comp.hpp>
#include <BSO/Structural_Design/Components/Beam_Comp.hpp>
#include <BSO/Structural_Design/Components/Flat_Shell_Comp.hpp>
#include <BSO/Structural_Design/Components/Load_Comp.hpp>
#include <BSO/Structural_Design/Components/Load.hpp>
#include <BSO/Structural_Design/Components/Constraint.hpp>
#include <BSO/Structural_Design/SD_Results.hpp>


namespace BSO { namespace Structural_Design {

    class SD_Analysis; // forward declaration

    struct Element_Properties
    {
        std::string m_ID;
    };

    struct Truss_Props : Element_Properties
    {
        double m_A;
        double m_E;
    };

    struct Beam_Props : Element_Properties
    {
        double m_b;
        double m_h;
        double m_E;
        double m_v;
    };

    struct Flat_Shell_Props : Element_Properties
    {
        double m_t;
        double m_E;
        double m_v;
    };

    struct Abstract_Load
    {
        double m_magnitude;
        double m_azimuth;
        double m_altitude;
        std::string m_type;
        BSO::Vectors::Vector m_direction;
        unsigned int m_lc;

        void calc_direction()
        {
            m_direction = Vectors::calc_direction(m_azimuth, m_altitude);
        }

    };

    struct SD_Analysis_Vars
    {
        std::vector<Components::Point*> m_points; // original points only (no meshed points) for clearing meshes
        std::vector<Components::Point*> m_all_points; // original points + meshed points
        std::vector<Components::Component*> m_components;
        std::vector<Components::Component*> m_trusses;
        std::vector<Components::Component*> m_beams;
        std::vector<Components::Component*> m_flat_shells;

        std::map<unsigned int, Abstract_Load> m_abstract_loads; // this will hold abstract loads like live loads, wind loads, etc.Loads are indexed by an ID-number (unsignd int)

        std::vector<Truss_Props> m_truss_props;
        std::vector<Beam_Props> m_beam_props;
        std::vector<Flat_Shell_Props> m_flat_shell_props;
        std::vector<Flat_Shell_Props> m_ghost_flat_shell_props;

        unsigned int m_mesh_division;

        virtual void mesh(unsigned int) = 0;
        virtual void analyse() = 0;
        virtual void transfer_model(SD_Analysis&) = 0;
        virtual SD_Building_Results& get_results() = 0;
		
        virtual std::map<Components::Point*, std::vector<unsigned int> > get_points_with_free_dofs(double x) = 0;
		virtual std::map<Components::Point*, std::vector<unsigned int> > get_zoned_points_with_free_dofs(double x) = 0;
		virtual std::vector<Components::Point*> get_points() = 0;
        virtual std::map<std::pair<Components::Point*, unsigned int>, double> get_points_singular_values() = 0;
		virtual void remesh() = 0;
		virtual unsigned int get_component_count() = 0;
        virtual Components::Component* get_component_ptr(unsigned int n) = 0;
        virtual std::vector<double> get_element_clusters() = 0;
		virtual Eigen::Vector6d get_displacements() = 0;
		virtual std::map<Elements::Node*, std::vector<unsigned int> > get_nodes_with_free_dofs(double) = 0;
        virtual unsigned int get_components_count() {
            return m_components.size();  // This method returns the count of components
        }

        Components::Component* getLastComponent() const {
            if (!m_components.empty()) {
                return m_components.back();
            }
            return nullptr;
        }

        void removeLastComponent() {
            if (!m_components.empty()) {
                m_components.pop_back();  // Remove the last component
            }
            else {
                std::cerr << "No components to remove." << std::endl;
            }
        }



        std::vector<SD_Analysis*> m_previous_designs;

        virtual ~SD_Analysis_Vars();
    };

    SD_Analysis_Vars::~SD_Analysis_Vars()
    {
        // nothing here, this is done in SD_Analysis
    }



} // namespace structural_Design
} // namespace BSO


#endif // SD_PROPS_VARS_HPP
