#ifndef FLAT_SHELL_COMP_HPP
#define FLAT_SHELL_COMP_HPP

#include <BSO/Structural_Design/Components/Point_Comp.hpp>

#include <iostream>
#include <cstdlib>


namespace BSO { namespace Structural_Design {  namespace Components {

    class Flat_Shell : public Quadri_Lateral
    {
    private:
        double m_t; // shell thickness [mm]
		double m_unscaled_t;
        double m_E; // youngs modulus [N/mm²]
        double m_v; // poisson ratio [-]

    public:
        Flat_Shell(double t, double E, double v, Point* p1, Point* p2, Point* p3, Point* p4);
        Flat_Shell(double t, double E, double v, Spatial_Design::Geometry::Rectangle* rec_ptr,
                   std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map);
        ~Flat_Shell();

        double get_property(int n);
		void scale_dimensions(double x);
		void reset_scale();

    }; // class Flat_Shell

    Flat_Shell::Flat_Shell(double t, double E, double v, Point* p1, Point* p2, Point* p3, Point* p4) : Quadri_Lateral(p1, p2, p3, p4)
    {
        m_is_flat_shell = true;
        m_t = t;
		m_unscaled_t = t;
        m_E = E;
        m_v = v;

    } // ctor

    Flat_Shell::Flat_Shell(double t, double E, double v, Spatial_Design::Geometry::Rectangle* rec_ptr,
                           std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map)
                           : Quadri_Lateral(rec_ptr, point_map)
    {
        m_is_flat_shell = true;
        m_t = t;
		m_unscaled_t = t;
        m_E = E;
        m_v = v;
    } // ctor

    Flat_Shell::~Flat_Shell()
    {

    } // dtor

    double Flat_Shell::get_property(int n)
    {
        switch(n)
        {
        case 0:
            return m_t;
            break;
        case 1:
            return m_E;
            break;
        case 2:
            return m_v;
            break;
        default:
            std::cout << "Could not find flat shell property " << n << ", exiting now..." << std::endl;
            exit(1);
            break;
        }
    } // get_property()
	
	void Flat_Shell::scale_dimensions(double x)
	{
		m_t = x * m_unscaled_t;
	} // scale_dimensions()
	
	void Flat_Shell::reset_scale()
	{
		m_t = m_unscaled_t;
	} // reset_scale()


} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // FLAT_SHELL_COMP_HPP
