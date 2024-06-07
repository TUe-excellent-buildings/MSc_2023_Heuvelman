#ifndef BP_SPACE_HPP
#define BP_SPACE_HPP

#include <BSO/Building_Physics/States/Dep_States/BP_Dep_State.hpp>
#include <BSO/Building_Physics/Construction/BP_Construction.hpp>
#include <BSO/Building_Physics/States/Indep_States/BP_Weather_Profile.hpp>

#include <algorithm>


namespace BSO {
namespace Building_Physics {

    /*
     * BP_SPace represents the temperature state of a space in the building physics
     * model. It can be connected to 0 or more adjacent states (on which the state's
     * temperature depends). A space is defined by a volume and is assigned some temperature
     * control settings
     */

    // Class definition:

    class BP_Space : public BP_Dep_State
    {
    private:
        double m_volume; // volume of the space
        double m_heat_cap, m_cool_cap;
        double m_heat_set_point, m_cool_sep_point;
		std::map<std::string, double> m_sum_heat_power, m_sum_cool_power;
        double m_ACH;
        std::string m_ID;
    public:
        BP_Space(BP_Simulation*, std::string ID, double volume, BP_Space_Settings space_settings, BP_Weather_Profile* outside_ptr); // initialize building space, with volume and initial temperature of that building space
        ~BP_Space();

        bool is_space();

        void update_sys(double t); // updates the A and B matrices of the state space system
        std::string get_ID(); // getter function for Space ID
        void show_vars();


        void set_power_count_zero();
		void add_Q_load(double dt, double Q);

        std::map<std::string, double> get_heating_energy();
        std::map<std::string, double> get_cooling_energy();

        double get_volume();
        double get_floor_area();

    }; // BP_Space





    // Implementation of member functions:

    BP_Space::BP_Space(BP_Simulation* system, std::string ID, double volume, BP_Space_Settings space_settings, BP_Weather_Profile* outside_ptr) : BP_Dep_State(system)
    {
        m_ID = ID;
        m_volume = volume;
        m_heat_cap = space_settings.m_heating_capacity;
        m_cool_cap = space_settings.m_cooling_capacity;
        m_heat_set_point = space_settings.m_heat_set_point;
        m_cool_sep_point = space_settings.m_cool_set_point;
        m_ACH = space_settings.m_ACH;
        m_capacitance = volume * 1.2 * 1000.0 * 3; // 1.2 [kg/m³] air weight, 1000.0 [J/(K kg)] air specific heat, remaining factor takes furniture etc. into account

        if (m_ACH != 0.0)
        {
            Adj_State outside_state;
            outside_state.m_state_ptr = outside_ptr;
            outside_state.m_resistance = 1/((1000.0) * (1.2 * (m_volume * m_ACH/3600))); // Rinfiltration = 1/(m'*c) where c = 1000 [J/(K kg)], m' = kg/sec = rho * V'; where rho = 1.2 [kg/m³], V' [m³/sec] = Volume*ACH/3600
            this->add_adj_state(outside_state);
        }
    } // ctor

    BP_Space::~BP_Space()
    {

    } // dtor

    bool BP_Space::is_space()
    {
        return true;
    } // is_space()

    void BP_Space::update_sys(double t) // updates the A and B matrices of the state space system
    {
		double current_T = m_system->m_SS_x(m_dep_index);
		double dQ = 0, current_Q = m_system->m_SS_B(m_dep_index,0);
		double dT_sys = m_system->m_SS_dT(m_dep_index);
		double dt = 3600.0 / (m_system->m_time_step_hour);


		if (current_T < m_heat_set_point)
		{ // heating must be added
			double Q_max = m_heat_cap * m_volume / m_capacitance;
			dQ = (m_heat_set_point - current_T - dT_sys) / dt;
			if ((current_Q + dQ) > Q_max) dQ = Q_max - current_Q;
		}
		else if (current_T > m_cool_sep_point)
		{ // cooling must be added
			double Q_min = -m_cool_cap * m_volume / m_capacitance;
			dQ = (m_cool_sep_point - current_T - dT_sys) / dt;
			if ((current_Q + dQ) < Q_min) dQ = Q_min - current_Q;
		}
		else
		{ // either cooling or heating must be decreased, or nothing happens
			if (current_Q > 0)
			{ // heating should be decreased
				dQ = (m_heat_set_point - current_T - dT_sys) / dt;
				if ((current_Q + dQ) < 0) dQ = -current_Q;
			}
			if (current_Q < 0)
			{ // cooling should be decreased
				dQ = (m_cool_sep_point - current_T - dT_sys) / dt;
				if ((current_Q + dQ) > 0) dQ = -current_Q;
			}
		}

		m_system->m_SS_B(m_dep_index,0) += dQ;
    } // update_sys()

    std::string BP_Space::get_ID() // getter function for Space ID
    {
        return m_ID;
    } // get_ID()

    void BP_Space::show_vars()
    {
        BP_Dep_State::show_vars();
        std::cout << "Volume: " << m_volume << std::endl;
        std::cout << "Heating Capacity: " << m_heat_cap << std::endl;
        std::cout << "Cooling Capacity: " << m_cool_cap << std::endl;
        std::cout << "Heating Set Point: " << m_heat_set_point << std::endl;
        std::cout << "Cooling Set Point: " << m_cool_sep_point << std::endl;
        std::cout << "Air Changes per Hour: " << m_ACH << std::endl;

    } // show_vars()

    void BP_Space::set_power_count_zero()
    {
		m_sum_heat_power.clear();
		m_sum_cool_power.clear();
		for (auto i : m_system->m_simulation_periods)
		{
			m_sum_heat_power[i.first] = 0;
			m_sum_cool_power[i.first] = 0;
		}
    } // set_power_count_zero()

	void BP_Space::add_Q_load(double dt, double Q)
	{
		double load = Q*dt / (3600*1000);
		if (load >= 0) m_sum_heat_power[m_system->m_current_sim_period] += load;
		else m_sum_cool_power[m_system->m_current_sim_period] -= load;
	} // add_Q_load()

    std::map<std::string, double> BP_Space::get_heating_energy()
    {
        return m_sum_heat_power;
    } // get_heating_energy()

    std::map<std::string, double> BP_Space::get_cooling_energy()
    {
        return m_sum_cool_power;
    } // get_cooling_energy()

    double BP_Space::get_volume()
    {
        return m_volume;
    } // get_volume()

    double BP_Space::get_floor_area()
    {
        double floor_area = 0;
        for (unsigned int i = 0; i < m_connections.size(); i++)
        {
            if (m_connections[i].m_state_ptr->is_floor())
            {
                floor_area += m_connections[i].m_state_ptr->get_area();
            }
        }
        floor_area /= 2.0; // divide by two because floor + ceiling area has been calculated
        return floor_area;
    } // get_floor_area()


} // namespace Building_Physics
} // namespace BSO

#endif // BP_SPACE_HPP
