#ifndef BP_FLOOR_HPP
#define BP_FLOOR_HPP

#include <BSO/Building_Physics/States/Dep_States/BP_Dep_State.hpp>
#include <BSO/Building_Physics/Construction/BP_Construction.hpp>

#include <BSO/Visualisation/Model_Module/Model.hpp> // for polygon_props

#include <cstdlib>
#include <memory>

namespace BSO {
namespace Building_Physics {

    /*
     * BP_Floor represents the temperature state of a floor object in
     * a building physics model. It has two adjacent states: either a
     * space (BP_Dep) or a temperature profile (BP_Indep). A floor is
     * defined by a construction and a surface.
     */

    // Class definition:

    class BP_Floor : public BP_Dep_State
    {
    private:
        BP_Construction m_construction; // this is the construction of which the floor is made
        double m_area; // surface area of the floor
        std::string m_ID; // floor ID
        BP_State* m_side_1; // the temperature state at side one of the floor
        BP_State* m_side_2; // the temperature state at side two of the floor

        std::string m_vis_ID;
        BSO::Spatial_Design::Geometry::Rectangle* m_rectangle_ptr;
    public:
        BP_Floor(BP_Simulation* system, std::string floor_ID, const double area, BP_Construction construction, BP_State* side_1, BP_State* side_2);
        ~BP_Floor();

        std::string get_ID();
        bool is_floor();
        void update_sys(double t); // updates the A and B matrices of the state space system
        void show_vars();


        double get_thickness();
        double get_area();

        std::string get_vis_ID();
        void set_rect_ptr( BSO::Spatial_Design::Geometry::Rectangle*);
        BSO::Spatial_Design::Geometry::Rectangle* get_rectangle_ptr();
    }; // BP_Floor

    // Implementation of the member functions:

    BP_Floor::BP_Floor(BP_Simulation* system, std::string floor_ID, const double area, BP_Construction construction, BP_State* side_1, BP_State* side_2) : BP_Dep_State(system)
    {
        m_ID = floor_ID;
        m_area = area; // area in square meters
        m_construction = construction;
        m_capacitance = (construction.get_capacitance_per_area()) * (area); // capacitance in J/K
        m_vis_ID = construction.m_vis_ID;
        m_side_1 = side_1; // temperature state at side one of the floor
        m_side_2 = side_2; // temperature state at side two of the floor

        double trans_res_side_1, trans_res_side_2;
        if (m_side_1->is_space())
        {
            trans_res_side_1 = 0.13;
        }
        else if (m_side_1->is_weather_profile())
        {
            trans_res_side_1 = 0.04;
        }
        else if (m_side_1->is_ground_profile())
        {
            trans_res_side_1 = 0.0;
        }
        else
        {
            std::cout << "Something went wrong in assigning the transition resistances, exiting.. " << std::endl;
            exit(1);
        }

        if (m_side_2->is_space())
        {
            trans_res_side_2 = 0.13;
        }
        else if (m_side_2->is_weather_profile())
        {
            trans_res_side_2 = 0.04;
        }
        else if (m_side_2->is_ground_profile())
        {
            trans_res_side_2 = 0.0;
        }
        else
        {
            std::cout << "Something went wrong in assigning the transition resistances, exiting.. " << std::endl;
            exit(1);
        }

        // Create an Adjacent State structure for adding information to this and adjacent states (structure is declared in BP_State.h)
        Adj_State adj_state;

        // Add this state as adjacent state to adjacent states
        adj_state.m_state_ptr = this;
        adj_state.m_resistance = ((construction.get_resistance_to_side_1() + trans_res_side_1) / m_area);
        m_side_1->add_adj_state(adj_state);

        adj_state.m_resistance = ((construction.get_resistance_to_side_2() + trans_res_side_2) / m_area);
        m_side_2->add_adj_state(adj_state);

        // Add adjacent states to this state
        adj_state.m_state_ptr = m_side_1;
        adj_state.m_resistance = ((construction.get_resistance_to_side_1() + trans_res_side_1) / m_area);
        add_adj_state(adj_state);

        adj_state.m_state_ptr = m_side_2;

        adj_state.m_resistance = ((construction.get_resistance_to_side_2() + trans_res_side_2) / m_area);
        add_adj_state(adj_state);

    } // ctor

    BP_Floor::~BP_Floor()
    {

    } // dtor

    void BP_Floor::update_sys(double t) // updates the A and B matrices of the state space system
    {

    } // update_sys()

    void BP_Floor::show_vars()
    {
        BP_Dep_State::show_vars();
        std::cout << "Area: " << m_area << std::endl;
    } // show_vars()

    std::string BP_Floor::get_ID()
    {
        return m_ID;
    } // get_ID()

    bool BP_Floor::is_floor()
    {
        return true;
    } // is floor()

    double BP_Floor::get_thickness()
    {
        return m_construction.m_total_thickness;
    } // get_thickness

    double BP_Floor::get_area()
    {
        return m_area;
    } // get_area()

    void BP_Floor::set_rect_ptr(BSO::Spatial_Design::Geometry::Rectangle* rectangle_ptr)
    {
        m_rectangle_ptr = rectangle_ptr;
    } // set_rect_ptr()

    std::string BP_Floor::get_vis_ID()
    {
        return m_vis_ID;
    } // get_vis_ID

    BSO::Spatial_Design::Geometry::Rectangle* BP_Floor::get_rectangle_ptr()
    {
        return m_rectangle_ptr;
    } // get_rectangle_ptr()


} // namespace Building_Physics
} // namespace BSO

#endif // BP_FLOOR_HPP
