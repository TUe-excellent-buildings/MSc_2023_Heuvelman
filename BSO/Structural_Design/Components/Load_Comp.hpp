#ifndef LOAD_COMP_HPP
#define LOAD_COMP_HPP

#include <BSO/Structural_Design/Components/Component.hpp>

namespace BSO { namespace Structural_Design {  namespace Components {

    class Line_Load : public Line
    {
    private:

    public:
        Line_Load(Load l, Point* p1, Point* p2);
        Line_Load(Load l, Spatial_Design::Geometry::Line* line_ptr,
                  std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map );

        double get_property(int n);
		void scale_dimensions(double x);
		void reset_scale();
    };

    class Quadrilateral_Load : public Quadri_Lateral
    {
    private:

    public:
        Quadrilateral_Load(Load l, Point* p1, Point* p2, Point* p3, Point* p4);
        Quadrilateral_Load(Load l, Spatial_Design::Geometry::Rectangle* rec_ptr,
                           std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map);

        double get_property(int n);
		void scale_dimensions(double x);
		void reset_scale();
    };

    class Line_Constraint : public Line
    {
    private:

    public:
        Line_Constraint(Constraint c, Point* p1, Point* p2);
        Line_Constraint(Constraint c, Spatial_Design::Geometry::Line* line_ptr,
                  std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map );

        double get_property(int n);
		void scale_dimensions(double x);
		void reset_scale();
    };




    Line_Load::Line_Load(Load l, Point* p1, Point* p2) : Line(p1, p2)
    {
        m_is_line_load = true;
        m_loads.push_back(l);
    }

    Line_Load::Line_Load(Load l, Spatial_Design::Geometry::Line* line_ptr,
                  std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map ) : Line(line_ptr, point_map)
    {
        m_is_line_load = true;
        m_loads.push_back(l);
    }

    double Line_Load::get_property(int n)
    {
        std::cerr << "Cannot request property from a line_load, exiting now... (Load_Comp.hpp)" << std::endl;
        exit(1);
    }
	
	void Line_Load::scale_dimensions(double x)
	{
		// do nothing
	} // scale_dimensions
	
	void Line_Load::reset_scale()
	{
		// do nothing
	} // reset_scale()



    Quadrilateral_Load::Quadrilateral_Load(Load l, Point* p1, Point* p2, Point* p3, Point* p4) :Quadri_Lateral(p1, p2, p3, p4)
    {
        m_is_quadri_load = true;
        m_loads.push_back(l);
    }

    Quadrilateral_Load::Quadrilateral_Load(Load l, Spatial_Design::Geometry::Rectangle* rec_ptr,
                                           std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map)
                                           : Quadri_Lateral(rec_ptr, point_map)
    {
        m_is_quadri_load = true;
        m_loads.push_back(l);
    }

    double Quadrilateral_Load::get_property(int n)
    {
        std::cerr << "Cannot request property from a line_load, exiting now... (Load_Comp.hpp)" << std::endl;
        exit(1);
    }
	
	void Quadrilateral_Load::scale_dimensions(double x)
	{
		// do nothing
	} // scale_dimensions
	
	void Quadrilateral_Load::reset_scale()
	{
		// do nothing
	} // reset_scale()

    Line_Constraint::Line_Constraint(Constraint c, Point* p1, Point* p2) : Line(p1, p2)
    {
        m_is_line_constraint = true;
        m_constraints.push_back(c);
    }

    Line_Constraint::Line_Constraint(Constraint c, Spatial_Design::Geometry::Line* line_ptr,
                  std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map ) : Line(line_ptr, point_map)
    {
        m_is_line_constraint = true;
        m_constraints.push_back(c);
    }

    double Line_Constraint::get_property(int n)
    {
        std::cerr << "Cannot request property from a line_load, exiting now... (Load_Comp.hpp)" << std::endl;
        exit(1);
    }
	
	void Line_Constraint::scale_dimensions(double x)
	{
		// do nothing
	} // scale_dimensions
	
	void Line_Constraint::reset_scale()
	{
		// do nothing
	} // reset_scale()

} // namespace Components
} // namespace Structural_Design
} // BSO

#endif // LOAD_COMP_HPP
