#ifndef BP_SIMULATION_VARS_HPP
#define BP_SIMULATION_VARS_HPP

namespace BSO { namespace Building_Physics {

/*
 * BP_Simulation stores all instances of the state classes and stores the
 * state space matrices. Its functions are used for simulation.
 */

// forward declaration of the state classes
class BP_State;
class BP_Dep_State;
class BP_Indep_State;
class BP_Wall;
class BP_Floor;
class BP_Window;
class BP_Space;
class BP_Ground_Profile;
class BP_Weather_Profile;
struct BP_Building_Results;
struct BP_Space_Settings;
struct BP_Material;
struct BP_Construction;
struct BP_Glazing;

// class definition

struct BP_Vis_Setting
{
    std::string m_ID;
    float m_r;
    float m_g;
    float m_b;
    float m_alpha;
};

// first variables that are declared (to support a variable constructor method with constant constructor arguments)
struct BP_Simulation_Vars
{
    std::vector<BP_State*> m_states; // collection of all temperature states in the system
    std::vector<BP_Dep_State*> m_dep_states; // collection of ptrs to dependant states in the system
    std::vector<BP_Indep_State*> m_indep_states; // collection of ptrs to independant states in the system
    std::vector<BP_Space*> m_space_ptrs; // vector of pointers to objects of the BP_Space class (pointers since adresses of objects should not change when vector size changes)
    std::vector<BP_Wall*> m_wall_ptrs; // vector of pointers to objects of the BP_Wall class (pointers since adresses of objects should not change when vector size changes)
    std::vector<BP_Window*> m_window_ptrs; // vector of pointers to objects of the BP_Window class (pointers since adresses of objects should not change when vector size changes)
    std::vector<BP_Floor*> m_floor_ptrs; // vector of pointers to objects of the BP_Wall class (pointers since adresses of objects should not change when vector size changes)

    std::vector<BP_Space_Settings> m_space_settings;
    std::vector<BP_Material> m_materials; // vector of objects of the BP_Material structure (structure is defined in BP_Construction.h)
    std::vector<BP_Construction> m_constructions; // vector of objects of the BP_Construction structure
    std::vector<BP_Glazing> m_glazings; // vector of objects of the BP_Glazing structure

    BP_Weather_Profile* m_weather_profile; // this profile simulates the outdoor temperature
    BP_Ground_Profile* m_ground_profile; // this profile simulates the ground temperature

	std::map<std::string, std::pair<boost::posix_time::ptime, boost::posix_time::ptime> > m_simulation_periods;
    boost::posix_time::ptime m_sim_begin, m_last_observer_moment;
	boost::posix_time::time_duration m_dt_observer;
	std::string m_current_sim_period;

    bool m_visualisation_model; // will be true when the constructor of the conformal model is used
    std::vector<BP_Vis_Setting> m_vis_settings;
    unsigned int m_warm_up_days; // the number of warm up days
    unsigned int m_time_step_hour; // the number of time steps per hour
    BP_Building_Results* m_building_results; // result structure to store simulation results in
}; // BP_Simulation_Vars

} // namespace Building Physics
} // namespace BSO

#endif // BP_SIMULATION_VARS_HPP
