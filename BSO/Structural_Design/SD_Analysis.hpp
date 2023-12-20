#ifndef SD_ANALYSIS_HPP
#define SD_ANALYSIS_HPP

#ifdef VISUALISATION_HPP
#error Included "BSO/Visualisation/Visualisation.hpp" before "BSO/Structural_Design/SD_Analysis.hpp"
#endif

#ifdef HEURISTICS_HPP
#error Included "BSO/Heuristics.hpp" before "BSO/Structural_Design/SD_Analysis.hpp"
#endif

#ifdef PERFORMANCE_INDEXING_HPP
#error Included "BSO/Performance_indexing.hpp" before "BSO/Structural_Design/SD_Analysis.hpp"
#endif

#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/Analysis_Tools/SD_Props_Vars.hpp>
#include <BSO/Structural_Design/Analysis_Tools/FEA.hpp>

#include <vector>

namespace BSO { namespace Structural_Design {

    class SD_Analysis : SD_Analysis_Vars
    {
    private:
        bool m_fea_init; // switch to see if the finite element analysis has been initialised already
        FEA* m_FEA;
        Spatial_Design::MS_Conformal* m_spatial_design;

        std::vector<double> m_element_clusters;

        SD_Building_Results m_building_results;
    public:
        SD_Analysis(std::string file_name);
        SD_Analysis(Spatial_Design::MS_Conformal&);
        SD_Analysis();
        ~SD_Analysis();

        void transfer_model(SD_Analysis& new_model);

		void remesh();
        void mesh(unsigned int x);
		void mesh(unsigned int x, bool ghost);
        void clear_mesh();
        void analyse();
        void cluster_element_densities(unsigned int n);
		void scale_dimensions(double x);
		void reset_scale();

        unsigned int get_component_count();
        Components::Component* get_component_ptr(unsigned int n);
        std::vector<double> get_element_clusters();
		std::map<Components::Point*, std::vector<unsigned int> > get_points_with_free_dofs();
		std::map<Components::Point*, std::vector<unsigned int> > get_points_with_free_dofs(double x);
		std::map<Components::Point*, std::vector<unsigned int> > get_zoned_points_with_free_dofs(double x);

        FEA* get_FEA_ptr();

        std::vector<Flat_Shell_Props> get_flat_shell_props();
        std::vector<Beam_Props> get_beam_props();
        std::vector<Truss_Props> get_truss_props();
        std::vector<SD_Analysis*> get_previous_designs();

        SD_Building_Results& get_results();
        Spatial_Design::MS_Conformal* get_spatial_design_ptr();

        std::vector<Components::Point*> get_points();
        std::map<std::pair<Components::Point*, unsigned int>, double> get_points_singular_values();
		Eigen::Vector6d get_displacements();
		std::map<Elements::Node*, std::vector<unsigned int> > get_nodes_with_free_dofs(double);
    };

} // namespace Structural_Design
} // namespace BSO

#include <BSO/Structural_Design/SD_Analysis.cpp>

#endif // SD_ANALYSIS_HPP
