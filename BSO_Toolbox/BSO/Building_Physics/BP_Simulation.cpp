#ifndef BP_SIMULATION_CPP
#define BP_SIMULATION_CPP

#include <utility>

#include <boost/date_time/posix_time/posix_time.hpp> // for simulation time
#include <boost/bind.hpp> // used in odeint stepper
#include <boost/tokenizer.hpp> // to tokenize strings
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/numeric/odeint.hpp> // to solve ODE's


#include <BSO/Building_Physics/Construction/BP_Construction.hpp>
#include <BSO/Building_Physics/BP_Results.hpp>
#include <BSO/Building_Physics/States/Dep_States/BP_Space.hpp>
#include <BSO/Building_Physics/States/Dep_States/BP_Wall.hpp>
#include <BSO/Building_Physics/States/Dep_States/BP_window.hpp>
#include <BSO/Building_Physics/States/Dep_States/BP_Floor.hpp>
#include <BSO/Building_Physics/States/Indep_States/BP_Weather_Profile.hpp>
#include <BSO/Building_Physics/States/Indep_States/BP_Ground_profile.hpp>

#include <BSO/Trim_And_Cast.hpp>
#include <Read_BP_Settings.hpp>


namespace BSO { namespace Building_Physics {

// implementation of the class member functions
namespace odeint = boost::numeric::odeint; // shorten name space declaration a bit
//typedef odeint::runge_kutta_fehlberg78<BP_Vector_Type, double, BP_Vector_Type, double, odeint::vector_space_algebra> explicit_stepper; // this defines what solver is used and what type of variable is used for the states
typedef odeint::runge_kutta_dopri5<BP_Vector_Type, double, BP_Vector_Type, double, odeint::vector_space_algebra> explicit_stepper; // this defines what solver is used and what type of variable is used for the states
//typedef odeint::euler<ublas::vector<double>, double, ublas::vector<double>, double, odeint::vector_space_algebra> explicit_stepper; // this defines what solver is used and what type of variable is used for the states

typedef odeint::implicit_euler<double> implicit_stepper;


BP_Simulation::BP_Simulation(std::string file_name)
{// still needs a catch for the case where the warm up time is longer than the simulation time
	m_indep_count = 1; // must be 1, first index (index = 0) belongs to heating/cooling
    m_dep_count = 0; // must be 0, no indices reserved yet
	read_BP_settings(file_name, this);
	m_spatial_design = nullptr;
	m_visualisation_model = false;

	initialize();
} // ctor()

BP_Simulation::BP_Simulation(Spatial_Design::MS_Conformal& CF)
{
	m_indep_count = 1; // must be 1, first index (index = 0) belongs to heating/cooling
    m_dep_count = 0; // must be 0, no indices reserved yet
	CF.request_BP_grammar()(&CF, this);
    m_spatial_design = &CF;
	m_visualisation_model = true;

	initialize();
} // ctor

void BP_Simulation::initialize()
{
	m_solver_type = solver_type::CONTROLLED_EXPLICIT;
	m_output = output::NONE;

	m_building_results = new BP_Building_Results;

	// initialize the state vectors and matrices to their correct sizes and some initial values
    m_SS_u = BP_Vector_Type(m_indep_count, 1.0); // seed state vector u with initial values (must be initiated to 1!)
    m_SS_x = BP_Vector_Type(m_dep_count, (m_space_settings[0].m_heat_set_point + m_space_settings[0].m_cool_set_point) /2.0); // seed state vector x with initial values
	m_SS_dT = BP_Vector_Type(m_dep_count, 0.0);
    m_SS_A = BP_Matrix_Type(m_dep_count, m_dep_count, 0.0); // seed state matrix A with initial values (must be zero!)
    m_SS_B = BP_Matrix_Type(m_dep_count, m_indep_count, 0.0); // seed state matrix B with initial values (must be zero!)
} // initialize()

BP_Simulation::~BP_Simulation()
{
    delete m_building_results;

    for (unsigned int i = 0; i < m_states.size(); i++)
    {
        delete m_states[i];
    }
    m_states.clear();
    m_indep_states.clear();
    m_dep_states.clear();
    m_wall_ptrs.clear();
    m_window_ptrs.clear();
    m_floor_ptrs.clear();
    m_space_ptrs.clear();

} // dtor

void BP_Simulation::ODE_function(const BP_Vector_Type &x, BP_Vector_Type &dxdt, const double& t)
{
	for (auto i : m_indep_states)
	{
        i->update_sys(t);
	}

    dxdt = prod(m_SS_A, x) + prod(m_SS_B ,m_SS_u); // x is passed as a function argument as it must be varied by the ODE solver
	if (t < 0) dxdt *= -1; // for warm up period with negative time steps
} // ODE_function()

void BP_Simulation::ODE_Jacobi(const BP_Vector_Type &x, BP_Matrix_Type &J, const double& t)
{
	J = m_SS_A;
	if (t < 0) J *= -1; // for warm up period with negative time steps
}

void BP_Simulation::Observer_function(const BP_Vector_Type &x, double t)
{
	bool stream_open = m_observer_stream.is_open();
	ODE_function(x, m_SS_dT, t);
	m_SS_dT *= 3600.0/m_time_step_hour; // get an estimate for heating during the next time step
	
	for(auto i : m_space_ptrs) // update heating
	{
		i->update_sys(t);
	}
	
	boost::posix_time::ptime current_time = (m_sim_begin + boost::posix_time::seconds(t));
	boost::posix_time::time_duration abs_dt = m_last_observer_moment - current_time;
	if (abs_dt.is_negative())abs_dt *= -1; // to make abs_dt an absolute time duration
    if (stream_open) m_observer_stream << current_time;

    // send external temperature to the observer file
    if (stream_open) m_observer_stream << "," << m_weather_profile->get_temp();

    // send space temperatures to the observer file
    for (auto i : m_space_ptrs)
    {
        unsigned int space_index = i->get_index();
        double Q = m_SS_B(space_index,0) * i->get_capacitance();
		i->add_Q_load((double)abs_dt.total_seconds(), Q);
		if (stream_open)
		{
			m_observer_stream << "," << m_SS_x(space_index) // space temperature
							  << "," << ((Q >= 0) ? Q : 0) // heating power
							  << "," << ((Q < 0) ? -Q : 0); // cooling power
		}
	}

    // send wall temperatures to the observer file
	if (stream_open)
	{
		// send wall temperatures to the observer file
		for (auto i : m_wall_ptrs)
			m_observer_stream << "," << i->get_temp();

		// send floor temperatures to the observer file
		for (auto i : m_floor_ptrs)
			m_observer_stream << "," << i->get_temp();

		// send window temperatures to the observer filer
		for (auto i : m_window_ptrs)
			m_observer_stream << "," << i->get_temp();
		m_observer_stream << std::endl;
	}

	m_last_observer_moment = current_time;
}

void BP_Simulation::init_observer_file(std::string file_name)
{
	if (m_output != output::SIM_RESULTS) return; // if output of the results is turned off, then skip all this
    m_observer_stream.open(file_name.c_str());
    // write the header
    m_observer_stream << "Time,Te";
    for (auto i : m_space_ptrs) // set heading for the spaces
    {
        m_observer_stream << ",T_space_" + i->get_ID()   // temperature
                          << ",Qh_space_" + i->get_ID() // active heating power
                          << ",Qc_space_" + i->get_ID();  // active cooling power
    }
    for (auto i : m_wall_ptrs) // set heading for the walls
        m_observer_stream << ",T_wall_" + i->get_ID();
    for (auto i : m_floor_ptrs) // set heading for the floors
        m_observer_stream << ",T_floor_" + i->get_ID();
    for (auto i : m_window_ptrs) // set heading for the windows
        m_observer_stream << ",T_window_" + i->get_ID();
    m_observer_stream << std::endl;
} // init_observer_file()

void BP_Simulation::end_observer_file()
{
    if (m_observer_stream.is_open()) m_observer_stream.close(); // only close it when it is actually open
} // end_observer_file()

void BP_Simulation::sim_period()
{
	m_building_results->reset(); // clear the structure (in case a new simulation is being run)
	double time_step = 3600.0/(m_time_step_hour);
	boost::posix_time::time_duration duration;

	for (auto sim_period : m_simulation_periods)
    {
		std::pair<boost::posix_time::ptime, boost::posix_time::ptime> period = sim_period.second; // the begin (first of pair) and the end (second of pair) of the simulation period
		m_current_sim_period = sim_period.first; // the ID of the simulation period
		

		// warm up period
		m_sim_begin = period.first + boost::gregorian::days(m_warm_up_days); // this is the begin of this simulation period
		duration = period.first - m_sim_begin;
		m_last_observer_moment = m_sim_begin;

		for (auto i : m_dep_states)
		{ // for each dependant state
			i->init_sys(); // initialise the state space matrices
		}
		for (auto i : m_states)
		{
			i->update_sys(0.0); // update the system according to the initial values (i.e. add heating/cooling loads and ground/weather profile to the system)
		}

		if (m_solver_type == solver_type::CONTROLLED_EXPLICIT)
		{
		// solve the ODE from begin to end
		odeint::integrate_const(odeint::make_controlled(1e-6,1e-6,explicit_stepper()) // which stepper is used
							   ,boost::bind(&BP_Simulation::ODE_function, this, _1, _2, _3)
							   ,m_SS_x // all dependent states
							   ,0.0 // the current time
							   ,(double)duration.total_seconds() // the time after the simulation
							   ,-time_step
							   ,boost::bind(&BP_Simulation::Observer_function, this, _1, _2)
							   ); // duration of the time step
		}
		else if (m_solver_type == solver_type::IMPLICIT)
		{
			// solve the ODE from begin to end
			odeint::integrate_const(implicit_stepper() // which stepper is used
								   ,std::make_pair(boost::bind(&BP_Simulation::ODE_function, this, _1, _2, _3),
												   boost::bind(&BP_Simulation::ODE_Jacobi,   this, _1, _2, _3))
								   ,m_SS_x // all dependent states
								   ,0.0 // the current time
								   ,(double)duration.total_seconds() // the time after the simulation
								   ,-time_step
								   ,boost::bind(&BP_Simulation::Observer_function, this, _1, _2)
								   ); // duration of the time step
		}
		else if (m_solver_type == solver_type::UNCONTROLLED_EXPLICIT)
		{
			// solve the ODE from begin to end
		odeint::integrate_const(explicit_stepper() // which stepper is used
							   ,boost::bind(&BP_Simulation::ODE_function, this, _1, _2, _3)
							   ,m_SS_x // all dependent states
							   ,0.0 // the current time
							   ,(double)duration.total_seconds() // the time after the simulation
							   ,-time_step
							   ,boost::bind(&BP_Simulation::Observer_function, this, _1, _2)
							   ); // duration of the time step
		}
		else
		{
			std::cerr << ", exiting now... (BP_Simulation.cpp)" << std::endl;
			exit(1);
		}

		// simulation period
		m_sim_begin = period.first; // this is the begin of this simulation period
		duration = period.second - m_sim_begin;
		m_last_observer_moment = m_sim_begin;
		init_observer_file("BP_sim_from_"+boost::posix_time::to_iso_string(period.first)+"_until_"+boost::posix_time::to_iso_string(period.second)+".txt");

		for (unsigned int i = 0; i < m_space_ptrs.size(); i++)
		{ // for each space
			m_space_ptrs[i]->set_power_count_zero(); // reset the power counters for the current simulation period
		}

		if (m_solver_type == solver_type::CONTROLLED_EXPLICIT)
		{
			// solve the ODE from begin to end
			odeint::integrate_const( odeint::make_controlled(1e-6,1e-6,explicit_stepper()) // which stepper is used
								   ,boost::bind(&BP_Simulation::ODE_function, this, _1, _2, _3)
								   ,m_SS_x // all dependent states
								   ,0.0 // the current time
								   ,(double)duration.total_seconds() // the time after the simulation
								   ,time_step
								   ,boost::bind(&BP_Simulation::Observer_function, this, _1, _2)
								   ); // duration of the time step
		}
		else if (m_solver_type == solver_type::IMPLICIT)
		{
			// solve the ODE from begin to end
			odeint::integrate_const(implicit_stepper() // which stepper is used
								   ,std::make_pair(boost::bind(&BP_Simulation::ODE_function, this, _1, _2, _3),
												   boost::bind(&BP_Simulation::ODE_Jacobi,   this, _1, _2, _3))
								   ,m_SS_x // all dependent states
								   ,0.0 // the current time
								   ,(double)duration.total_seconds() // the time after the simulation
								   ,time_step
								   ,boost::bind(&BP_Simulation::Observer_function, this, _1, _2)
								   ); // duration of the time step
		}
		else if (m_solver_type == solver_type::UNCONTROLLED_EXPLICIT)
		{
			// solve the ODE from begin to end
		odeint::integrate_const(explicit_stepper() // which stepper is used
							   ,boost::bind(&BP_Simulation::ODE_function, this, _1, _2, _3)
							   ,m_SS_x // all dependent states
							   ,0.0 // the current time
							   ,(double)duration.total_seconds() // the time after the simulation
							   ,time_step
							   ,boost::bind(&BP_Simulation::Observer_function, this, _1, _2)
							   ); // duration of the time step
		}
		else
		{
			std::cerr << ", exiting now... (BP_Simulation.cpp)" << std::endl;
			exit(1);
		}

		end_observer_file();
	} // end for every simulation period

    // add results to the results structure
    for (unsigned int i = 0; i < m_space_ptrs.size(); i++)
    { // for each space in the system
        m_building_results->add_space(m_space_ptrs[i]); // add the results to the data structure
    }

} // sim_period()

void BP_Simulation::test_values() // for testing purposes
{
    for (unsigned int i = 0; i < m_dep_states.size(); i++)
    {
        m_dep_states[i]->show_vars();
    }
}

unsigned int BP_Simulation::get_wall_count()
{
    return m_wall_ptrs.size();
} // get_wall_count()

unsigned int BP_Simulation::get_floor_count()
{
    return m_floor_ptrs.size();
} // get_floor_count()

unsigned int BP_Simulation::get_window_count()
{
    return m_window_ptrs.size();
} // get_window_count()

BP_Wall* BP_Simulation::get_wall_ptr(unsigned int i)
{
    return m_wall_ptrs[i];
} // get_wall_ptr()

BP_Floor* BP_Simulation::get_floor_ptr(unsigned int i)
{
    return m_floor_ptrs[i];
} // get_floor_ptr()

BP_Window* BP_Simulation::get_window_ptr(unsigned int i)
{
    return m_window_ptrs[i];
} // get_window_ptr()

bool BP_Simulation::visualisation_possible()
{
    return m_visualisation_model;
} // visualisation_possible()

std::vector<BP_Vis_Setting> BP_Simulation::get_vis_settings()
{
    return m_vis_settings;
} // get_vis_settings();

BP_Building_Results BP_Simulation::get_results()
{
    return *m_building_results;
} // get_results()

Spatial_Design::MS_Conformal* BP_Simulation::get_spatial_design_ptr()
{
    return m_spatial_design;
}

} // namespace Building_Physics
} // namespace BSO

#endif // BP_SIMULATION_CPP
