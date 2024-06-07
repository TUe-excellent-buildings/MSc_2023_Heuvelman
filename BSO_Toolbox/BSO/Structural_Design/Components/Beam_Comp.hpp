#ifndef BEAM_COMP_HPP
#define BEAM_COMP_HPP

#include <BSO/Structural_Design/Components/Point_Comp.hpp>

#include <iostream>
#include <cstdlib>


namespace BSO { namespace Structural_Design {  namespace Components {

    class Beam : public Line
    {
    private:
        double m_b; // beam width [mm]
		double m_unscaled_b;
        double m_h; // beam height [mm]
		double m_unscaled_h;
        double m_E; // youngs modulus [N/mm²]
        double m_v; // poisson ratio [-]

    public:
        Beam(double b, double h, double E, double v, Point* p1, Point* p2);
        Beam(double b, double h, double E, double v, Spatial_Design::Geometry::Line* line_ptr,
             std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map );
        ~Beam();

        double get_property(int n);
		void scale_dimensions(double x);
		void reset_scale();
    }; // class Beam

    Beam::Beam(double b, double h, double E, double v, Point* p1, Point* p2) : Line(p1, p2)
    {
        m_is_beam = true;
        m_b = b;
		m_unscaled_b = b;
        m_h = h;
		m_unscaled_h = h;
        m_E = E;
        m_v = v;
    } // ctor

    Beam::Beam(double b, double h, double E, double v, Spatial_Design::Geometry::Line* line_ptr,
               std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map ) : Line(line_ptr, point_map)
    {
        m_is_beam = true;
        m_b = b;
        m_unscaled_b = b;
        m_h = h;
		m_unscaled_h = h;
        m_E = E;
        m_v = v;
    } // ctor

    Beam::~Beam()
    {

    } // dtor

    double Beam::get_property(int n)
    {
        switch(n)
        {
        case 0:
            return m_b;
            break;
        case 1:
            return m_h;
            break;
        case 2:
            return m_E;
            break;
        case 3:
            return m_v;
            break;
        default:
            std::cout << "Could not find beam property " << n << ", exiting now..." << std::endl;
            exit(1);
            break;
        }
    } // get_property()
	
	
	void Beam::scale_dimensions(double x)
	{
		m_b = sqrt(x) * m_unscaled_b;
        m_h = sqrt(x) * m_unscaled_h;
	} // scale_dimensions()
	
	void Beam::reset_scale()
	{
		m_b = m_unscaled_b;
        m_h = m_unscaled_h;
	} // reset_scale()



} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // BEAM_COMP_HPP
