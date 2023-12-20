#ifndef BP_RESULTS_HPP
#define BP_RESULTS_HPP

#include <BSO/Building_Physics/States/Dep_States/BP_Space.hpp>



namespace BSO { namespace Building_Physics
{
    /*
     * Structures to store results of a BP_simulation
     */

    // Structure definitions:
    struct BP_Space_Results
    {
        std::string m_space_ID;
        double m_floor_area;
        double m_space_volume;

        std::map<std::string, double> m_heating_energy;
        std::map<std::string, double> m_cooling_energy;
        double m_total_heating_energy, m_total_cooling_energy, m_total_energy;

        double m_rel_performance;
    };

    struct BP_Building_Results
    {
        double m_total_heating_energy;
        double m_total_cooling_energy;
        double m_total_energy;

        std::vector<BP_Space_Results> m_space_results;

        BP_Building_Results();
        void reset();
        void add_space(BP_Space* space_ptr);
    }; // BP_results

    BP_Building_Results::BP_Building_Results()
    {
        m_total_heating_energy = 0;
        m_total_cooling_energy = 0;
        m_total_energy = 0;
        m_space_results.clear();
    } // ctor

    void BP_Building_Results::reset()
    {
        m_total_heating_energy = 0;
        m_total_cooling_energy = 0;
        m_total_energy = 0;
        m_space_results.clear();
    } // reset()

    void BP_Building_Results::add_space(BP_Space* space_ptr)
    {
        BP_Space_Results temp;
        temp.m_space_ID = space_ptr->get_ID();
        temp.m_space_volume = space_ptr->get_volume();
        temp.m_floor_area = space_ptr->get_floor_area();
        temp.m_heating_energy = space_ptr->get_heating_energy();
        temp.m_cooling_energy = space_ptr->get_cooling_energy();
        temp.m_total_heating_energy = 0;
        temp.m_total_cooling_energy = 0;

        for (auto i : temp.m_heating_energy)
        {
            temp.m_total_heating_energy += i.second;
        }
        for (auto i : temp.m_cooling_energy)
        {
            temp.m_total_cooling_energy += i.second;
        }
        temp.m_total_energy = temp.m_total_heating_energy + temp.m_total_cooling_energy;
        temp.m_rel_performance = -1.0; // to be initiated after all spaces have been added if value remains -1.0 then it has not been initiated

        m_space_results.push_back(temp);
        m_total_heating_energy += temp.m_total_heating_energy;
        m_total_cooling_energy += temp.m_total_cooling_energy;
        m_total_energy += temp.m_total_energy;

    } // add_space()


} // namepsace Building_Physics
} // namespace BSO

#endif // BP_RESULTS_HPP
