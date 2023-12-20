#ifndef BP_DEP_STATE_HPP
#define BP_DEP_STATE_HPP

#include <BSO/Building_Physics/States/BP_State.hpp>

#include <cstdlib>
#include <vector>

namespace BSO {
namespace Building_Physics {

/*
 * BP_Dep_State represents dependant temperature states like wall-,
 * floor- and space temperatures. In this class, also the adjacent states
 * are stored (on which the temperature in the represented state depends).
 */

// Class Definition

class BP_Dep_State : public BP_State // dependant state, state is dependant of system.
{
protected:
    std::vector<Adj_State> m_connections; // this vector holds structures of adjacent states and the resistance to flux to these states
    double m_capacitance; // capacitance of this state
    unsigned int m_dep_index;
public:
    BP_Dep_State(BP_Simulation* system); //ctor
    virtual ~BP_Dep_State(); //dtor

    virtual unsigned int get_index(); // for testing purposes
    
    virtual void get_res(); // for testing purposes


    virtual void show_vars(); // for testing purposes
    virtual double get_temp();
	virtual double get_capacitance();

    virtual void update_sys(double t); // updates the state space system matrices
    virtual bool is_dep();
    virtual void init_sys(); // updates the invariant fluxes in the A and B matrices of the state space system
    virtual void add_adj_state(Adj_State new_adj_state); // adds a state (with respective resistance) to the vector m_capacitors
}; // BP_Dep_State

// Implementation of member functions

BP_Dep_State::BP_Dep_State(BP_Simulation* system) : BP_State(system)
{
    m_dep_index = m_system->m_dep_count;
    m_system->m_dep_count++;
} // ctor

BP_Dep_State::~BP_Dep_State()
{
    m_system->m_dep_count--;
} // dtor

unsigned int BP_Dep_State::get_index() // for testing purposes
{
    return m_dep_index;
} // get_index()

double BP_Dep_State::get_capacitance() // for testing purposes
{
    return m_capacitance;
} // get_capacitance()

void BP_Dep_State::show_vars() // for testing purposes
{
    if (this->is_space())
    {
        std::cout << std::endl << "!!! SPACE !!! ID: " << this->get_ID() << std::endl;
    }
    else if (this->is_wall())
    {
        std::cout << std::endl << "!!! WALL !!! ID: " << this->get_ID() << std::endl;
    }
    else if (this->is_floor())
    {
        std::cout << std::endl << "!!! FLOOR !!! ID: " << this->get_ID() << std::endl;
    }

    std::cout << "Capacitance: " << m_capacitance << std::endl;
    for (unsigned int i = 0; i < m_connections.size(); i++)
    {
        if (m_connections[i].m_state_ptr->is_wall())
        {
            std::cout << "Connection " << i+1 << ": Wall -> " << m_connections[i].m_state_ptr->get_ID() << std::endl;
            std::cout << "Resistance: " << m_connections[i].m_resistance << std::endl;
        }
        else if (m_connections[i].m_state_ptr->is_floor())
        {
            std::cout << "Connection " << i+1 << ": Floor -> " << m_connections[i].m_state_ptr->get_ID() << std::endl;
            std::cout << "Resistance: " << m_connections[i].m_resistance << std::endl;
        }
        else if (m_connections[i].m_state_ptr->is_space())
        {
            std::cout << "Connection " << i+1 << ": Space -> " << m_connections[i].m_state_ptr->get_ID() << std::endl;
            std::cout << "Resistance: " << m_connections[i].m_resistance << std::endl;
        }
        else if (m_connections[i].m_state_ptr->is_weather_profile())
        {
            std::cout << "Connection " << i+1 << ": Weather" << std::endl;
            std::cout << "Resistance: " << m_connections[i].m_resistance << std::endl;
        }
        else if (m_connections[i].m_state_ptr->is_ground_profile())
        {
            std::cout << "Connection " << i+1 << ": Ground" << std::endl;
            std::cout << "Resistance: " << m_connections[i].m_resistance << std::endl;
        }
    }
    std::cout << "Temperature: " << this->get_temp() << std::endl;
} // show_vars()

double BP_Dep_State::get_temp()
{
    return m_system->m_SS_x(m_dep_index);
} // get_temp()

void BP_Dep_State::get_res() // for testing purposes
{
    for (unsigned int i = 0; i < m_connections.size(); i++)
    {
        std::cout << m_connections[i].m_resistance << std::endl;
    }
} // get_res()

bool BP_Dep_State::is_dep()
{
    return true;
} // is_dep()

void BP_Dep_State::init_sys()
{ // initialise the state space matrices

    for (unsigned int i = 0; i < m_connections.size(); i++)
    { // for each connection to this dependant state, add teir influence

        if (m_connections[i].m_state_ptr->is_dep())
        {
            m_system->m_SS_A(this->get_index(), this->get_index()) += -1/(m_capacitance* m_connections[i].m_resistance);
            m_system->m_SS_A(this->get_index(), m_connections[i].m_state_ptr->get_index()) += 1/(m_capacitance* m_connections[i].m_resistance);
        }
        else if (m_connections[i].m_state_ptr->is_indep())
        {
            m_system->m_SS_A(this->get_index(), this->get_index()) += -1/(m_capacitance* m_connections[i].m_resistance);
            m_system->m_SS_B(this->get_index(), m_connections[i].m_state_ptr->get_index()) += 1/(m_capacitance* m_connections[i].m_resistance);
        }
        else
        {
            std::cout << "Error in assembling system (BP_Dep_State.hpp), exiting... " << std::endl;
            exit(1);
        }
    }
} // init_sys()

void BP_Dep_State::update_sys(double t)
{ // empty here

} // update_sys

void BP_Dep_State::add_adj_state(Adj_State new_adj_state)// adds a state (with respective resistance) to the vector m_capacitors
{
    m_connections.push_back(new_adj_state);
} // add_adj_state()

} // namespace Building_Physics
} // namespace BSO

#endif // BP_DEP_STATE_HPP
