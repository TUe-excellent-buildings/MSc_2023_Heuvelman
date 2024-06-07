#ifndef TRUSS_COMP_HPP
#define TRUSS_COMP_HPP

#include <BSO/Structural_Design/Components/Point_Comp.hpp>
#include <BSO/Structural_Design/Components/Component.hpp>

#include <iostream>
#include <cstdlib>


namespace BSO { namespace Structural_Design {  namespace Components {

    class Truss : public Line
    {
    private:
        double m_E; // youngs modulus [N/mm²]
        double m_A; // cross sectional area [mm²]
		double m_unscaled_A;
		
		std::vector<Point*> m_points;

    public:
        Truss(double E, double A, Point* p1, Point* p2);
        Truss(double E, double A, Spatial_Design::Geometry::Line* line_ptr,
             std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map );
        ~Truss();

        double get_property(int n);
		void scale_dimensions(double x);
		void reset_scale();
		
		bool find_points(Point*, Point*);
    }; // class Truss

    Truss::Truss(double E, double A, Point* p1, Point* p2) : Line(p1, p2)
    {
        m_is_truss = true;
        m_E = E;
        m_A = A;
		m_unscaled_A = A;
		
		m_points.push_back(p1);
		m_points.push_back(p2);
    } // ctor

    Truss::Truss(double E, double A, Spatial_Design::Geometry::Line* line_ptr,
             std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map ) : Line(line_ptr, point_map)
    {
        m_is_truss = true;
        m_E = E;
        m_A = A;
		m_unscaled_A = A;

		m_points.push_back(point_map[line_ptr->get_vertex_ptr(0)]);
		m_points.push_back(point_map[line_ptr->get_vertex_ptr(1)]);
    } // ctor

    Truss::~Truss()
    {

    } // dtor

    double Truss::get_property(int n)
    {
        switch(n)
        {
        case 0:
            return m_A;
            break;
        case 1:
            return m_E;
            break;
        default:
            std::cout << "Could not find truss property " << n << ", exiting now..." << std::endl;
            exit(1);
            break;
        }
    } // get_property()
	
	void Truss::scale_dimensions(double x)
	{
		m_A = x * m_unscaled_A;
	} // scale_dimensions()
	
	void Truss::reset_scale()
	{
		m_A = m_unscaled_A;
	} // reset_scale()
	
	bool Truss::find_points(Point* p1, Point* p2)
	{
		if (std::find(m_points.begin(), m_points.end(), p1) != m_points.end() && std::find(m_points.begin(), m_points.end(), p2) != m_points.end() )
			return true;
		else
			return false;
	}
	
} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // TRUSS_COMP_HPP
