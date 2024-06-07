#ifndef BP_SIMULATION_HPP
#define BP_SIMULATION_HPP

#ifdef VISUALISATION_HPP
#error Included "BSO/Visualisation/Visualisation.hpp" before "BSO/Building_Physics/BP_Simulation.hpp"
#endif

#ifdef HEURISTICS_HPP
#error Included "BSO/Heuristics.hpp" before "BSO/Building_Physics/BP_Simulation.hpp"
#endif

#ifdef PERFORMANCE_INDEXING_HPP
#error Included "BSO/Performance_indexing.hpp" before "BSO/Building_Physics/BP_Simulation.hpp"
#endif

#ifndef BOOST_UBLAS_NDEBUG
#define BOOST_UBLAS_NDEBUG // to improve ublas algebra speed
#endif // BOOST_UBLAS_NDEBUG

// include all the building physics related objects
#include <BSO/Spatial_Design/Conformation.hpp>
#include <boost/numeric/ublas/matrix.hpp> // for state space matrices
#include <boost/numeric/ublas/vector.hpp> // for state space matrices
#include <boost/date_time/posix_time/posix_time.hpp> // for simulation time

#include <BSO/Building_Physics/BP_Simulation_Vars.hpp>

namespace ublas = boost::numeric::ublas;

typedef ublas::matrix<double> BP_Matrix_Type;
typedef ublas::vector<double> BP_Vector_Type;

namespace BSO {
namespace Building_Physics {

enum class solver_type{CONTROLLED_EXPLICIT, UNCONTROLLED_EXPLICIT, IMPLICIT, ARG_COUNT};
enum class output{NONE, SIM_RESULTS, ARG_COUNT};

class BP_Simulation : public BP_Simulation_Vars
{
private:
    // also look at BP_Simulation_Vars
	BP_Vector_Type m_SS_x, m_SS_u, m_SS_dT;
	BP_Matrix_Type m_SS_A, m_SS_B;

	std::string m_observer_file;
	std::ofstream m_observer_stream;

	void initialize();

    void ODE_function(const BP_Vector_Type &x, BP_Vector_Type &dxdt, const double& t);
	void ODE_Jacobi(const BP_Vector_Type &x, BP_Matrix_Type &J, const double& t);
	void Observer_function(const BP_Vector_Type &x, double t);
	void init_observer_file(std::string);
	void end_observer_file();

    friend BP_State;
    friend BP_Dep_State;
    friend BP_Indep_State;
    friend BP_Wall;
    friend BP_Floor;
    friend BP_Window;
    friend BP_Space;
    friend BP_Ground_Profile;
    friend BP_Weather_Profile;

    unsigned int m_indep_count;
    unsigned int m_dep_count;

    Spatial_Design::MS_Conformal* m_spatial_design;

public:
    BP_Simulation(std::string file_name);
    BP_Simulation(Spatial_Design::MS_Conformal&);
    ~BP_Simulation();

    void test_values(); // for testing purposes

    void sim_period();

    unsigned int get_wall_count();
    unsigned int get_floor_count();
    unsigned int get_window_count();

    BP_Wall* get_wall_ptr(unsigned int);
    BP_Floor* get_floor_ptr(unsigned int);
    BP_Window* get_window_ptr(unsigned int);

    bool visualisation_possible();
    std::vector<BP_Vis_Setting> get_vis_settings();

	solver_type m_solver_type;
	output m_output;

    BP_Building_Results get_results();
    Spatial_Design::MS_Conformal* get_spatial_design_ptr();
}; // BP_Simulation
} // namespace Building Physics
} // namespace BSO

#include <BSO/Building_Physics/BP_Simulation.cpp>

#endif // BP_SIMULATION_HPP
