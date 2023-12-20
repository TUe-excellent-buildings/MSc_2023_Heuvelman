#ifndef BP_WINDOW_HPP
#define BP_WINDOW_HPP

#include <BSO/Building_Physics/States/Dep_States/BP_Dep_State.hpp>
#include <BSO/Building_Physics/Construction/BP_Construction.hpp>

#include <BSO/Visualisation/Model_Module/Model.hpp> // for polygon_props

#include <cstdlib>
#include <memory>


namespace BSO {
namespace Building_Physics {
    /*
     * BP_Window represents the temperature state of a window in the building
     * physics model. It is connected to two adjacent states, either a space (BP_Dep)
     * or a temperature profile (BP_Indep). A window is defined by an area and a
     * U-value (heat resistance)
     */

     // Class definition:


    class BP_Window : public BP_Dep_State
    {
    private:
        double m_area; // surface area of the wall
        double m_U_value; // heat resistance of the window
        std::string m_ID;
        BP_State* m_side_1; // the temperature state at side one of the wall
        BP_State* m_side_2; // the temperature state at side two of the wall

        std::string m_vis_ID;
        BSO::Spatial_Design::Geometry::Rectangle* m_rectangle_ptr;
    public:
        BP_Window(BP_Simulation* system, std::string window_ID, const double area, BP_Glazing glazing, BP_State* side_1, BP_State* side_2); // ctor
        ~BP_Window(); // dtor

        std::string get_ID();
        bool is_window();
        void update_sys(double t); // updates the A and B matrices of the state space system
        void show_vars();


        std::string get_vis_ID();
        void set_rect_ptr(BSO::Spatial_Design::Geometry::Rectangle*);
        BSO::Spatial_Design::Geometry::Rectangle* get_rectangle_ptr();
    }; // BP_Window

    // Implementation of member functions:


    BP_Window::BP_Window(BP_Simulation* system, std::string window_ID, const double area, BP_Glazing glazing, BP_State* side_1, BP_State* side_2) : BP_Dep_State(system)
    {
        m_ID = window_ID;
        m_U_value = glazing.m_U_value;
        m_vis_ID = glazing.m_vis_ID;
        m_area = area; // area in square meters
        m_capacitance = glazing.m_capacitance_per_area * area; // capacitance in J/K
        m_vis_ID = glazing.m_vis_ID;
        m_side_1 = side_1; // temperature state at side one of the wall
        m_side_2 = side_2; // temperature state at side two of the wall

        // Create an Adjacent State structure for adding information to this and adjacent states (structure is declared in BP_State.h)
        Adj_State adj_state;

        // Add this state as adjacent state to adjacent states
        adj_state.m_state_ptr = this;
        adj_state.m_resistance = (1/(2*m_U_value*m_area));
        m_side_1->add_adj_state(adj_state);

        adj_state.m_resistance = (1/(2*m_U_value*m_area));
        m_side_2->add_adj_state(adj_state);

        // Add adjacent states to this state
        adj_state.m_state_ptr = m_side_1;
        adj_state.m_resistance = (1/(2*m_U_value*m_area));
        add_adj_state(adj_state); // add the adjacent states as adjacent states to this state

        adj_state.m_state_ptr = m_side_2;
        adj_state.m_resistance = (1/(2*m_U_value*m_area));
        add_adj_state(adj_state); // add the adjacent states as adjacent states to this state
    } // ctor

    BP_Window::~BP_Window()
    {

    } // dtor

    void BP_Window::update_sys(double t) // updates the A and B matrices of the state space system
    {

    } // update_sys()

    void BP_Window::show_vars()
    {
        BP_Dep_State::show_vars();
        std::cout << "Area: " << m_area << std::endl;
    } // show_vars()


    std::string BP_Window::get_ID()
    {
        return m_ID;
    } // get_ID()

    bool BP_Window::is_window()
    {
        return true;
    } // is_window()

    void BP_Window::set_rect_ptr(BSO::Spatial_Design::Geometry::Rectangle* rectangle_ptr)
    {
        m_rectangle_ptr = rectangle_ptr;
    } // set_rect_ptr()

    std::string BP_Window::get_vis_ID()
    {
        return m_vis_ID;
    } // get_vis_ID

    BSO::Spatial_Design::Geometry::Rectangle* BP_Window::get_rectangle_ptr()
    {
        return m_rectangle_ptr;
    } // get_rectangle_ptr()



} // namespace Building_Physics
} // namespace BSO

#endif // BP_WINDOW_HPP
