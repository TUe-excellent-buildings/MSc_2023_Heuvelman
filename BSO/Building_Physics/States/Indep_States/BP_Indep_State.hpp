#ifndef BP_INDEP_STATE_HPP
#define BP_INDEP_STATE_HPP

#include <BSO/Building_Physics/States/BP_State.hpp>

namespace BSO {
namespace Building_Physics {
/*
 * BP_Indep_State represents temperature states that are independant
 * of other temperature states in the building physics model like:
 * the weather or the ground profile.
 */

// Class definition

class BP_Indep_State : public BP_State
{
protected:
    unsigned int m_index;
public:
    BP_Indep_State(BP_Simulation* system);
    ~BP_Indep_State();

    virtual unsigned int get_index(); // for testing purposes

    virtual void update_sys(double t);
    virtual bool is_indep();
    virtual double get_temp();
    virtual void add_adj_state(Adj_State new_adj_state);
}; // BP_Indep_State

// Implementation of member functions:
BP_Indep_State::BP_Indep_State(BP_Simulation* system) : BP_State(system)
{
    m_index = m_system->m_indep_count;
    m_system->m_indep_count++;
} //ctor

BP_Indep_State::~BP_Indep_State()
{
    m_system->m_indep_count--;
} // dtor

unsigned int BP_Indep_State::get_index() // for testing purposes
{
    return m_index;
} // get_index()

double BP_Indep_State::get_temp()
{
    return m_system->m_SS_u(m_index);
} // get_temp()

void BP_Indep_State::add_adj_state(Adj_State new_adj_state)
{
    (void)new_adj_state; // this expression does nothing
    // function is empty as it will be called upon for independant states however, it should not add adjacent states to the independant class as they have no influence on the state.
} // add_adj_state()

void BP_Indep_State::update_sys(double t)
{ // empty here

} // update_sys()

bool BP_Indep_State::is_indep()
{
    return true;
} // is_indep()

} // Building_Physics
} // BSO

#endif // BP_INDEP_STATE_HPP
