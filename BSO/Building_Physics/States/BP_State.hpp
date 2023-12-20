#ifndef BP_STATE_HPP
#define BP_STATE_HPP

#include <iostream>
#include <iomanip>
#include <cstdlib>

namespace BSO {
namespace Building_Physics {

class BP_Simulation;

/*
 * Adj_State stores a connection
 */

// Structure definition:
struct Adj_State;

/*
 * BP_State represents a temperature state in the building
 * physics model
 */

// Class definition

class BP_State
{
private:

protected:
    BP_Simulation* m_system; // the system that this state belongs to
public:
    BP_State(BP_Simulation* system);
    virtual ~BP_State();

    virtual void update_sys(double t) = 0;

    virtual bool is_dep();
    virtual bool is_indep();
    virtual bool is_space();
    virtual bool is_floor();
    virtual bool is_wall();
    virtual bool is_window();
    virtual bool is_weather_profile();
    virtual bool is_ground_profile();

    virtual double get_temp() = 0;

    virtual double get_area(); // to request area of floor or wall
    virtual std::string get_ID(); // to get ID of spaces, walls, etc

    virtual unsigned int get_index() = 0;
    virtual void add_adj_state(Adj_State new_adj_state) = 0; // required in Dep-State, but not Indep_state, however it should be possible to call this function on base class State (This fucntion will remain empty in class Indep_state)
}; // BP_State


// Implementation of the Adj_State structure:
struct Adj_State
{
    BP_State* m_state_ptr; // pointer to adjacent state
    double m_resistance; // flux resistance to that state
}; // Adj_State



//Implementation of the member functions of the BP_State class:

BP_State::BP_State(BP_Simulation* system)
{
    m_system = system;
} // ctor

BP_State::~BP_State()
{

} // dtor

bool BP_State::is_dep()
{
    return false;
} // is_dep()

bool BP_State::is_indep()
{
    return false;
} // is_indep()

bool BP_State::is_space()
{
    return false;
} // is_space()

bool BP_State::is_floor()
{
    return false;
} // is_floor()

bool BP_State::is_wall()
{
    return false;
} // is_wall()

bool BP_State::is_window()
{
    return false;
} // is_window()

bool BP_State::is_weather_profile()
{
    return false;
} // is_weather_profile()

bool BP_State::is_ground_profile()
{
    return false;
} // is_ground_profile()

double BP_State::get_area()
{
    std::cout << "Error, requested area from something other than wall or floor, exiting..." << std::endl;
    exit(1);
    return 0.0;
}

std::string BP_State::get_ID() // to get ID of spaces, walls, etc
{
    return "N/A";
}

} // namepsace Building_Physics
} // namespace BSO

#endif // BP_STATE_HPP
