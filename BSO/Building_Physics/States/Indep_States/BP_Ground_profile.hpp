#ifndef BP_GROUND_PROFILE_HPP
#define BP_GROUND_PROFILE_HPP

#include <BSO/Building_Physics/States/Indep_States/BP_Indep_State.hpp>

namespace BSO {
namespace Building_Physics {

/*
 * BP_Ground_profile stores a constant temperature to
 * represent the temperature of the ground
 */

// Class definition

class BP_Ground_Profile : public BP_Indep_State
{
private:
    double m_temperature;
public:
    BP_Ground_Profile(BP_Simulation* system, double temperature);
    ~BP_Ground_Profile();

    bool is_ground_profile();

    void update_sys(double t);
    double get_temp();
}; // BP_Ground_profile

// Implementation of member functions:

BP_Ground_Profile::BP_Ground_Profile(BP_Simulation* system, double temperature) : BP_Indep_State(system)
{
    m_temperature = temperature;
} // ctor

BP_Ground_Profile::~BP_Ground_Profile()
{

} // dtor

bool BP_Ground_Profile::is_ground_profile()
{
    return true;
} // is_ground_profile()

void BP_Ground_Profile::update_sys(double t)
{ // empty here
    m_system->m_SS_u(m_index) = m_temperature;
} // update_sys

double BP_Ground_Profile::get_temp()
{
    return m_temperature;
} // get_temp()

} // namespace Buildign_Physics
} // namespace BSO

#endif // BP_GROUND_PROFILE_HPP
