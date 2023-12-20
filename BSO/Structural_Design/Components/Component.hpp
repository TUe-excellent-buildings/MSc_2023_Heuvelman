#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Structural_Design/Components/Point_Comp.hpp>
#include <BSO/Structural_Design/Components/Point_Comp.hpp>
#include <BSO/Structural_Design/Components/Load.hpp>
#include <BSO/Structural_Design/Components/Constraint.hpp>
#include <BSO/Structural_Design/Elements/Element.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <vector>
#include <map>

namespace BSO { namespace Structural_Design {  namespace Components {

    class Component
    {
    protected:
        std::vector<Point*> m_points; // original points
        std::vector<Point*> m_point_list; // meshed + original points
        std::vector<std::vector<Point*> > m_elements; // each set forms one element of the component (after meshing)
        std::vector<Elements::Element*> m_element_ptrs; // pointers to each of the component's elements

        std::vector<Spatial_Design::Geometry::Space*> m_space_ptrs; // all the ID's of the spaces that this component is related to

        std::vector<Load> m_loads; // loads acting on all nodes of an instance of this component
        std::vector<Constraint> m_constraints; // constraints acting on all nodes of an instances of this component

        bool m_is_truss;
        bool m_is_beam;
        bool m_is_flat_shell;
        bool m_is_line_load;
        bool m_is_quadri_load;
        bool m_is_line_constraint;
        bool m_mesh_switch;
        bool m_is_ghost;

        bool m_visualisation_transparancy;

    public:
        Component();
        virtual ~Component();

        virtual void add_load(Load l);
        virtual void add_constraint(Constraint c);

        virtual void mesh(unsigned int n, std::vector<Point*>& point_store) = 0;
        virtual void clear_mesh();
        virtual void set_element_ptr(Elements::Element*);
        virtual unsigned int get_element_ptr_count();
        virtual Elements::Element* get_element_ptr(unsigned int n);

        virtual bool is_truss();
        virtual bool is_beam();
        virtual bool is_flat_shell();
        virtual bool is_line_load();
        virtual bool is_quadri_load();
        virtual bool is_line_constraint();
        bool is_ghost_component();
        void set_ghost_component(bool);
        bool get_vis_transparancy();
        void set_vis_transparancy(bool);

        void set_mesh_switch(bool);
        bool get_mesh_switch();

        virtual unsigned int get_element_count();
        virtual std::vector<unsigned long> get_node_IDs(unsigned int n) = 0;
        virtual unsigned int get_space_ptr_count();
        virtual Spatial_Design::Geometry::Space* get_space_ptr(unsigned int n);

        virtual std::vector<Eigen::Vector3d> get_vis_points();

        virtual double get_property(int n) = 0;
		virtual void scale_dimensions(double x) = 0;
		virtual void reset_scale() = 0;

        virtual void add_line_load(Load line_load, Point* p1, Point* p2);
        virtual void add_line_constraint(Constraint line_constraint, Point* p1, Point* p2);
		
		virtual bool find_points(Point*, Point*);

    }; // class Component



    class Line : public Component
    {
    protected:

    public:
        Line(Point* p1, Point* p2);
        Line(Spatial_Design::Geometry::Line* line_ptr,
             std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map);
        virtual ~Line();

        virtual void mesh(unsigned int n, std::vector<Point*>& point_store);
        virtual std::vector<unsigned long> get_node_IDs(unsigned int n);

        virtual double get_property(int n) = 0;
		virtual void scale_dimensions(double x) = 0;
		virtual void reset_scale() = 0;
    }; // class Line




    class Quadri_Lateral : public Component
    {
    protected:
        std::vector<std::pair<Point*, Point*> > m_lines; // pairs of pointers Point objects that form an edge of the quadrilateral
        std::vector<std::vector<Point*> > m_line_point_list; // lists the nodes for each of the quadrilaterals edges
        std::map<unsigned int, std::vector<Load> > m_line_loads; // key values are indices of m_lines
        std::map<unsigned int, std::vector<Constraint> > m_line_constraints; // key values are indices of m_lines
    public:
        Quadri_Lateral(Point* p1, Point* p2, Point* p3, Point* p4);
        Quadri_Lateral(Spatial_Design::Geometry::Rectangle* rec_ptr,
                       std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map);
        virtual ~Quadri_Lateral();

        virtual void mesh(unsigned int n, std::vector<Point*>& point_store);
        virtual void add_line_load(Load line_load, Point* p1, Point* p2);
        virtual void add_line_constraint(Constraint line_constraint, Point* p1, Point* p2);

        virtual std::vector<unsigned long> get_node_IDs(unsigned int n);

        virtual double get_property(int n) = 0;
		virtual void scale_dimensions(double x) = 0;
		virtual void reset_scale() = 0;
    }; // class Quadri_Lateral





/*
    class Hexahedron : public Component
    {
    protected:
        std::vector<std::pair<Point*, Point*> > m_lines; // pairs of pointers Point objects that form an edge of the hexahedron
        //std::vector<std::vector<Point*> > m_line_point_list; // lists the nodes for each of the quadrilaterals edges
        std::vector<Point*[4] > m_faces; // set of pointers to Point objects that form a face of the hexahedron

        std::map<unsigned int, std::vector<Load> > m_line_loads; // key values are indices of m_lines
        std::map<unsigned int, std::vector<Constraint> > m_line_constraints; // key values are indices of m_lines
        std::map<unsigned int, std::vector<Load> > m_area_loads; // key values are indices of m_faces
        std::map<unsigned int, std::vector<Constraint> > m_area_constraints; // key values are indices of m_faces
    public:
        Hexahedron(Point* p1, Point* p2, Point* p3, Point* p4,
                   Point* p5, Point* p6, Point* p7, Point* p8);
        Hexahedron(Spatial_Design::Geometry::Cuboid* cuboid_ptr,
                   std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map);
        virtual ~Hexahedron();

        virtual void mesh(unsigned int n, std::vector<Point*>& point_store);
        virtual void add_line_load(Load line_load, Point* p1, Point* p2);
        virtual void add_line_constraint(Constraint line_constraint, Point* p1, Point* p2);
        virtual void add_area_load(Load area_load, Point* p1, Point* p2, Point* p3, Point* p4);
        virtual void add_area_constraint(Constraint area_constraint, Point* p1, Point* p2, Point* p3, Point* p4);

        virtual std::vector<unsigned long> get_node_IDs(unsigned int n);

        virtual double get_property(int n) = 0;
    }; // class Hexahedron

*/






    Component::Component()
    {
        m_is_truss = false;
        m_is_beam = false;
        m_is_flat_shell = false;
        m_is_line_load = false;
        m_is_quadri_load = false;
        m_is_line_constraint = false;
        m_is_ghost = false;

        m_visualisation_transparancy = false;

        m_mesh_switch = true;
    } // ctor

    Component::~Component()
    {

    } // dtor

    void Component::add_load(Load l)
    {
        if (l.m_value > 1e-9 || l.m_value < -1e-9)
        {
            m_loads.push_back(l);
        }

    } // add_load

    void Component::add_constraint(Constraint c)
    {
        m_constraints.push_back(c);
    } // add_constraint

    void Component::clear_mesh()
    {
        m_point_list.clear();
        m_elements.clear();
        m_element_ptrs.clear();
    }

    void Component::set_element_ptr(Elements::Element* ele_ptr)
    {
        m_element_ptrs.push_back(ele_ptr);
    } // set_element_ptr()

    unsigned int Component::get_element_ptr_count()
    {
        return m_element_ptrs.size();
    } // get_element_ptr_count()

    Elements::Element* Component::get_element_ptr(unsigned int n)
    {
        return m_element_ptrs[n];
    } // get_element_ptr()

    bool Component::is_truss()
    {
        return m_is_truss;
    } // is_truss()

    bool Component::is_beam()
    {
        return m_is_beam;
    } // is_beam()

    bool Component::is_flat_shell()
    {
        return m_is_flat_shell;
    } // is_flat_shell()

    bool Component::is_line_load()
    {
        return m_is_line_load;
    } // is_line_load()

    bool Component::is_quadri_load()
    {
        return m_is_quadri_load;
    } // is_quadri_load()

    bool Component::is_line_constraint()
    {
        return m_is_line_constraint;
    } // is_line_constraint()

    bool Component::is_ghost_component()
    {
        return m_is_ghost;
    } // is_ghost_component()

    void Component::set_ghost_component(bool is_ghost)
    {
        m_is_ghost = is_ghost;
    } // set_ghost_component()

    bool Component::get_vis_transparancy()
    {
        return m_visualisation_transparancy;
    } // get_vis_transparancy()

    void Component::set_vis_transparancy(bool val)
    {
        m_visualisation_transparancy = val;
    } // set_vis_transparancy()

    void Component::set_mesh_switch(bool b)
    {
        m_mesh_switch = b;
    } // set_mesh_switch()

    bool Component::get_mesh_switch()
    {
        return m_mesh_switch;
    } // get_mesh_switch()

    unsigned int Component::get_element_count()
    {
        return m_elements.size();
    } // get_element_count()

    unsigned int Component::get_space_ptr_count()
    {
        return m_space_ptrs.size();
    } // get_space_ptr_count()

    Spatial_Design::Geometry::Space* Component::get_space_ptr(unsigned int n)
    {
        return m_space_ptrs[n];
    } // get_space_ptr()

    std::vector<Eigen::Vector3d> Component::get_vis_points()
    {
        std::vector<Eigen::Vector3d> temp;

        if(this->is_flat_shell())
        {
            for (int i = 0; i < 4; i++)
            {
                temp.push_back(m_points[i]->get_coords());
            }
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                temp.push_back(m_points[i]->get_coords());
            }
        }

        return temp;
    }

    void Component::add_line_load(Load line_load, Point* p1, Point* p2)
    {
        (void)line_load; // does nothing
        (void)p1; // does nothing
        (void)p2; // does nothing
        std::cout << "Trying to add a line load to a bad construction component on initialisation of SD model, exiting..." << std::endl;
        exit(1);
    }

    void Component::add_line_constraint(Constraint line_constraint, Point* p1, Point* p2)
    {
        (void)line_constraint; // does nothing
        (void)p1; // does nothing
        (void)p2; // does nothing
        std::cout << "Trying to add a line constraint to a bad construction component on initialisation of SD model, exiting..." << std::endl;
        exit(1);
    }
	
	bool Component::find_points(Point* p1, Point* p2)
	{
		if (std::find(m_points.begin(), m_points.end(), p1) != m_points.end() && std::find(m_points.begin(), m_points.end(), p2) != m_points.end() )
			return true;
		else
			return false;
	}











    Line::Line(Point* p1, Point* p2) : Component()
    {
        m_points.clear();
        m_points.resize(2);
        m_points[0] = p1;
        m_points[1] = p2;

    } // ctor

    Line::Line(Spatial_Design::Geometry::Line* line_ptr,
               std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map) : Component()
    {
        m_points.clear();
        m_points.resize(2);
        m_points[0] = point_map[line_ptr->get_vertex_ptr(0)];
        m_points[1] = point_map[line_ptr->get_vertex_ptr(1)];

    } // ctor

    Line::~Line()
    {

    } // dtor

    void Line::mesh(unsigned int n, std::vector<Point*>& point_store)
    { // divides the line in n parts
        BSO::Vectors::Vector vec = *m_points[1] - *m_points[0];

        m_point_list.clear();
        m_point_list.resize(n+1);

        m_point_list[0] = m_points[0]; // add start and end point to the point list
        m_point_list[n] = m_points[1];

        // get the new nodes on this line
        for (unsigned int i = 1; i < n; i++)
        { // for each new point

            Eigen::Vector3d new_point = *m_points[0] + (vec * (i/((double)n)));

            for (unsigned int j = 0; j < point_store.size(); j++)
            {

                if (*point_store[j] == new_point)
                { // if the point is not in the point store yet
                    m_point_list[i] = point_store[j];
                    break; // break the for loop (value has been found, prevents the else statement to be executed)
                }
                else if (j == point_store.size() - 1)
                { // if the point is in the point store already
                    point_store.push_back(new Point(new_point[0], new_point[1], new_point[2]));
                    m_point_list[i] = point_store.back();
                    break;  // break the for loop (vector size has increased, i != point_store.size(), prevents the else statement to be executed again)
                }
            }
        }

        // make the pairs of points for each element
        std::vector<Point*> temp_set; // temporary set of points of one element
        for (unsigned int i = 0; i < n; i++)
        { // for each element i
            temp_set.push_back(m_point_list[i]); // add the nodes of this element to temp_set
            temp_set.push_back(m_point_list[i+1]);

            m_elements.push_back(temp_set); // add the set to the element vector
            temp_set.clear(); // clear the temporary set
        }

        // add the present line constraints to the points on this line
        for (unsigned int i = 0; i < n+1; i++)
        { // for each point

            for (unsigned int j = 0; j < m_constraints.size(); j++)
            { // for each line constraint (add constraint to each node)
                m_point_list[i]->update_constraints(m_constraints[j]);
            }
        }

        // add the present line loads to the points on this line
        for (unsigned int i = 0; i < m_elements.size(); i++)
        {
            BSO::Vectors::Vector v1 = m_elements[i][1]->get_coords() -
                                      m_elements[i][0]->get_coords();

            double length = BSO::Vectors::length(v1);

            for (unsigned int j = 0; j < m_loads.size(); j++)
            {
                m_elements[i][0]->update_loads(m_loads[j] * (length / 2.0));
                m_elements[i][1]->update_loads(m_loads[j] * (length / 2.0));
            }
        }
    } // mesh()

    std::vector<unsigned long> Line::get_node_IDs(unsigned int n)
    {
        std::vector<unsigned long> temp;
        temp.push_back(m_elements[n][0]->get_ID());
        temp.push_back(m_elements[n][1]->get_ID());
        return temp;
    }










    Quadri_Lateral::Quadri_Lateral(Point* p1, Point* p2, Point* p3, Point* p4)
    {
        // sort the points in CC or CCC wise order:
        // first initialise all possible vectors between the points
        BSO::Vectors::Vector v21 = *p2 - *p1;
        BSO::Vectors::Vector v31 = *p3 - *p1;
        BSO::Vectors::Vector v41 = *p4 - *p1;
        BSO::Vectors::Vector v32 = *p3 - *p2;
        BSO::Vectors::Vector v42 = *p4 - *p2;
        BSO::Vectors::Vector v43 = *p4 - *p3;

        if (BSO::Vectors::intersection(p1->get_coords(), v41, p2->get_coords(), v32, 0.001))
        { // v41 and v32 are the two diagonals
            std::swap(p4, p3);
            std::swap(v31, v41);
            std::swap(v32, v42);
            v43 *= -1; // swaps 43 with 34
        }
        else if (BSO::Vectors::intersection(p1->get_coords(), v21, p3->get_coords(), v43, 0.001))
        { // v31 and v42 are the two diagonals
            std::swap(p3, p2);
            std::swap(v31, v21);
            std::swap(v43, v42);
            v32 *= -1; // swaps 32 with 23
        } // endif v31 and v42 are now the diagonals, nodes are now ordered either clock wise or counter clockwise : 1,2,3,4 or 1,4,3,2


        // store the points in the instance of the component:
        m_points.clear();
        m_points.resize(4);
        m_points[0] = p1;
        m_points[1] = p2;
        m_points[2] = p3;
        m_points[3] = p4;

        // store the lines that can be created with the points
        m_lines.push_back(std::make_pair(p1, p2));
        m_lines.push_back(std::make_pair(p2, p3));
        m_lines.push_back(std::make_pair(p3, p4));
        m_lines.push_back(std::make_pair(p4, p1));
    } // ctor

    Quadri_Lateral::Quadri_Lateral(Spatial_Design::Geometry::Rectangle* rec_ptr,
                                   std::map<Spatial_Design::Geometry::Vertex*, Components::Point*>& point_map)
    {
        // obtain the structural points for the rectangle from the point map
        Point* p1 = point_map[rec_ptr->get_vertex_ptr(0)];
        Point* p2 = point_map[rec_ptr->get_vertex_ptr(1)];
        Point* p3 = point_map[rec_ptr->get_vertex_ptr(2)];
        Point* p4 = point_map[rec_ptr->get_vertex_ptr(3)];

        // add space information to this component
        for (unsigned int i = 0; i < rec_ptr->get_surface_count(); i++)
        {
            for (unsigned int j = 0; j < rec_ptr->get_surface_ptr(i)->get_space_count(); j++)
            {
                Spatial_Design::Geometry::Space* space_ptr = rec_ptr->get_surface_ptr(i)->get_space_ptr(j);
                if (std::find(m_space_ptrs.begin(), m_space_ptrs.end(), space_ptr) == m_space_ptrs.end())
                { // if the space_ID has not yet been added to the component, then add it
                    m_space_ptrs.push_back(space_ptr);
                }
            }
        }

        // sort the points in CC or CCC wise order:
        // first initialise all possible vectors between the points
        BSO::Vectors::Vector v21 = *p2 - *p1;
        BSO::Vectors::Vector v31 = *p3 - *p1;
        BSO::Vectors::Vector v41 = *p4 - *p1;
        BSO::Vectors::Vector v32 = *p3 - *p2;
        BSO::Vectors::Vector v42 = *p4 - *p2;
        BSO::Vectors::Vector v43 = *p4 - *p3;

        if (BSO::Vectors::intersection(p1->get_coords(), v41, p2->get_coords(), v32, 0.001))
        { // v41 and v32 are the two diagonals
            std::swap(p4, p3);
            std::swap(v31, v41);
            std::swap(v32, v42);
            v43 *= -1; // swaps 43 with 34
        }
        else if (BSO::Vectors::intersection(p1->get_coords(), v21, p3->get_coords(), v43, 0.001))
        { // v31 and v42 are the two diagonals
            std::swap(p3, p2);
            std::swap(v31, v21);
            std::swap(v43, v42);
            v32 *= -1; // swaps 32 with 23
        } // endif v31 and v42 are now the diagonals, nodes are now ordered either clock wise or counter clockwise : 1,2,3,4 or 1,4,3,2

        // store the points in the instance of the component:
        m_points.clear();
        m_points.resize(4);
        m_points[0] = p1;
        m_points[1] = p2;
        m_points[2] = p3;
        m_points[3] = p4;

        // store the lines that can be created with the points
        m_lines.push_back(std::make_pair(p1, p2));
        m_lines.push_back(std::make_pair(p2, p3));
        m_lines.push_back(std::make_pair(p3, p4));
        m_lines.push_back(std::make_pair(p4, p1));
    } // ctor

    Quadri_Lateral::~Quadri_Lateral()
    {

    } // dtor

    void Quadri_Lateral::mesh(unsigned int x, std::vector<Point*>& point_store)
    { // divides the quadrilateral in n by n parts
        // allocate memory for the point list of this quadrilateral
        m_point_list.clear();
        m_point_list.resize((x+1)*(x+1));

        for (unsigned int i = 0; i < m_lines.size(); i++)
        {
            std::vector<Point*> temp(x+1);
            m_line_point_list.push_back(temp);
        }

        // initialise the point_list of two opposing lines (non adjacent lines)
        std::vector<Eigen::Vector3d> point_list_line_1(x+1);
        std::vector<Eigen::Vector3d> point_list_line_3(x+1);

        point_list_line_1[0] = m_lines[0].first->get_coords();
        point_list_line_1[x] = m_lines[0].second->get_coords();
        BSO::Vectors::Vector vec_line_1 = point_list_line_1[x] - point_list_line_1[0];

        point_list_line_3[0] = m_lines[2].first->get_coords();
        point_list_line_3[x] = m_lines[2].second->get_coords();
        BSO::Vectors::Vector vec_line_3 = point_list_line_3[x] - point_list_line_3[0];

        // mesh points to the point lists of the two opposing lines
        for (unsigned int i = 1; i < x; i++)
        {
            point_list_line_1[i] =point_list_line_1[0] + ((i/(double)x) * vec_line_1);
            point_list_line_3[i] =point_list_line_3[0] + ((i/(double)x) * vec_line_3);
        }

        // get all (existing and new) points on this quadrilateral and add to store if necessary
        Eigen::Vector3d new_point; // temporary vector to hold new points
        for (unsigned int m = 0; m < (x + 1); m++)
        { // for each row m

            BSO::Vectors::Vector vec = point_list_line_3[(x-m)] - point_list_line_1[m];

            for (unsigned int n = 0; n < (x + 1);  n++)
            { // and for each column n

                new_point = point_list_line_1[m] + (vec * (n/((double)x)));

                for (unsigned int i = 0; i < point_store.size(); i++)
                {
                    if (*(point_store[i]) == new_point)
                    { // if the point is in the point store already
                        m_point_list[(m * (x+1)) + n] = point_store[i];
                        break; // break the for loop (value has been found, prevents the else statement form being executed)
                    }
                    else if (i == point_store.size() - 1)
                    { // if the point is not in the point store yet
                        point_store.push_back(new Point(new_point[0], new_point[1], new_point[2]));
                        m_point_list[(m * (x+1)) + n] = point_store.back();
                        break; // break the for loop (vector size has increased, i != point_store.size(), prevents the else statement to be executed again)
                    }
                }

                if (n == 0)
                {
                    m_line_point_list[0][m] = m_point_list[(m * (x+1)) + n];
                }
                else if (n == x)
                {
                    m_line_point_list[2][x-m] = m_point_list[(m * (x+1)) + n];
                }

                if (m == x)
                {
                    m_line_point_list[1][n] = m_point_list[(m * (x+1)) + n];
                }
                else if (m == 0)
                {
                    m_line_point_list[3][x-n] = m_point_list[(m * (x+1)) + n];
                }
            }
        }

        // make the pairs of points for each element
        std::vector<Point*> temp_set; // temporary set of points of one element
        for (unsigned int m = 0; m < (x); m++)
        { // for each row of elements m
            for (unsigned int n = 0; n < (x); n++)
            { // and for each column of elements n
                temp_set.push_back(m_point_list[(((m)   * (x+1)) + (n))] ); // add the nodes of this element to temp_set
                temp_set.push_back(m_point_list[(((m)   * (x+1)) + (n+1))] );
                temp_set.push_back(m_point_list[(((m+1) * (x+1)) + (n))] );
                temp_set.push_back(m_point_list[(((m+1) * (x+1)) + (n+1))] );

                m_elements.push_back(temp_set); // add the set to the element vector
                temp_set.clear(); // clear the temporary set
            }
        }

        // add the present area constraints to the points on this quadrilateral
        for (unsigned int i = 0; i < ((x+1)*(x+1)); i++)
        { // for each point

            for (unsigned int j = 0; j < m_constraints.size(); j++)
            { // for each line constraint (add constraint to each node)
                m_point_list[i]->update_constraints(m_constraints[j]);
            }
        }

        // add the present area loads to the points on this quadrilateral
        for (unsigned int i = 0; i < m_elements.size(); i++)
        { // for each element
            Eigen::Vector3d center_point;
            center_point.setZero();

            // calculate the center point
            center_point += m_elements[i][0]->get_coords();
            center_point += m_elements[i][1]->get_coords();
            center_point += m_elements[i][2]->get_coords();
            center_point += m_elements[i][3]->get_coords();
            center_point /= 4.0;

            for (unsigned int j = 0; j < 4; j++)
            { // and for each node of that element
                unsigned int j_plus;
                unsigned int j_minus;

                if (j == 0) {j_minus = 2; j_plus = 1;}
                else if (j == 1) {j_minus = 0; j_plus = 3;}
                else if (j == 2) {j_minus = 3; j_plus = 0;}
                else {j_minus = 1; j_plus = 2;}


                Eigen::Vector3d p1 = m_elements[i][j]->get_coords();
                Eigen::Vector3d p2 = (p1 + m_elements[i][j_minus]->get_coords()) / 2.0;
                Eigen::Vector3d p3 = (p1 + m_elements[i][j_plus]->get_coords()) / 2.0;

                BSO::Vectors::Vector v1 = center_point - p1;
                BSO::Vectors::Vector v2 = p3 - p2;
                double area = BSO::Vectors::calc_area_quadri_lat(v1, v2);

                for (unsigned int k = 0; k < m_loads.size(); k++)
                { // and each load acting on the instance of this quadrilateral
                    m_elements[i][j]->update_loads(m_loads[k] * area);
                }
            }
        }

        // add the present line constraints to the points on this quadrilateral
        typedef std::map<unsigned int, std::vector<Constraint> >::iterator lconstr_ite;
        for (lconstr_ite ite = m_line_constraints.begin(); ite != m_line_constraints.end(); ite++)
        { // for each line of the quadrilateral with one or more loads acting on it
            for (unsigned int i = 0; i < ite->second.size(); i++)
            { // and for each constraint acting on that line
                for (unsigned int j = 0; j < x + 1; j++)
                { // and for each node on that line
                    m_line_point_list[ite->first][j]->update_constraints(ite->second[i]);
                }
            }
        }

        // add the present line loads to the points on this quadrilateral
        typedef std::map<unsigned int, std::vector<Load> >::iterator lloads_ite;
        for (lloads_ite ite = m_line_loads.begin(); ite != m_line_loads.end(); ite++)
        { // for each line of the quadrilateral with one or more loads acting on it
            for (unsigned int i = 0; i < ite->second.size(); i++)
            { // and for each load acting on that line
                for (unsigned int j = 0; j < x; j++)
                { // and for each element division in that line
                    double length = BSO::Vectors::length(m_line_point_list[ite->first][j+1]->get_coords()
                                                         - m_line_point_list[ite->first][j]->get_coords());

                    m_line_point_list[ite->first][j]->update_loads(ite->second[i]*(length/2.0));
                    m_line_point_list[ite->first][j+1]->update_loads(ite->second[i]*(length/2.0));
                }
            }
        }

    } // mesh()

    void Quadri_Lateral::add_line_load(Load line_load, Point* p1, Point* p2)
    {
        for (unsigned int i = 0; i < m_lines.size(); i++)
        {
            if ((m_lines[i].first == p1 && m_lines[i].second == p2) ||
                (m_lines[i].first == p2 && m_lines[i].second == p1))
            {
                m_line_loads[i].push_back(line_load);
            }
            else if (i == m_lines.size() - 1)
            {
                std::cout << "Could not find line in quadrilateral when assigning line load, exiting..." << std::endl;
                exit(1);
            }
        }

    } // add_line_load()

    void Quadri_Lateral::add_line_constraint(Constraint line_constraint, Point* p1, Point* p2)
    {
        for (unsigned int i = 0; i < m_lines.size(); i++)
        { // for each line of this quadrilateral
            if ((m_lines[i].first == p1 && m_lines[i].second == p2) ||
                (m_lines[i].first == p2 && m_lines[i].second == p1))
            { // if the two endpoints of line segment 'i' correspond with point p1 and p2
                if (m_line_constraints.find(i) == m_line_constraints.end())
                { // if no line constraints have added to this line yet
                    std::vector<Constraint> temp;
                    m_line_constraints[i] = temp;
                }

                m_line_constraints[i].push_back(line_constraint);
                break;
            }
            else if (i == m_lines.size() - 1)
            {
                std::cout << "Could not find line in quadrilateral when assigning line constraint, exiting..." << std::endl;
                exit(1);
            }
        }
    } // add_line_constraint()


    std::vector<unsigned long> Quadri_Lateral::get_node_IDs(unsigned int n)
    {
        std::vector<unsigned long> temp;
        temp.push_back(m_elements[n][0]->get_ID());
        temp.push_back(m_elements[n][1]->get_ID());
        temp.push_back(m_elements[n][2]->get_ID());
        temp.push_back(m_elements[n][3]->get_ID());

        return temp;
    }





/*
    Hexahedron::Hexahedron(Point* p1, Point* p2, Point* p3, Point* p4,
                           Point* p5, Point* p6, Point* p7, Point* p8)
    {
        BSO::Vectors::Point center = (p1->get_coords() + p2->get_coords() +
                                      p3->get_coords() + p4->get_coords() +
                                      p5->get_coords() + p6->get_coords() +
                                      p7->get_coords() + p8->get_coords())/8;

        Point* points[8];
        points[0] = p1;
        points[1] = p2;
        points[2] = p3;
        points[3] = p4;
        points[4] = p5;
        points[5] = p6;
        points[6] = p7;
        points[7] = p8;

        BSO::Vectors::Vector center_vectors[8];
        for (int i = 0; i < 8; i++)
        {
            center_vectors[i] = *points[i] - center;
        }

        for (int i = 0; i < 8; i++)
        { // apply signum function to each of the vectors in 'center_vectors'
            center_vectors[i](0) = (center_vectors[i](0) > 0) - (center_vectors[i](0) < 0);
            center_vectors[i](1) = (center_vectors[i](1) > 0) - (center_vectors[i](1) < 0);
            center_vectors[i](2) = (center_vectors[i](2) > 0) - (center_vectors[i](2) < 0);
        }

        m_points.clear();
        m_points.resize(8);

        for (int i = 0; i < 8; i++)
            m_points[i] = nullptr;

        for (int i = 0; i < 8; i++)
        { // put the points in the right order into m_points
            if      (center_vectors[i] == BSO::Vectors::Vector(-1,-1,-1))
                m_points[0] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector( 1,-1,-1))
                m_points[1] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector( 1, 1,-1))
                m_points[2] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector(-1, 1,-1))
                m_points[3] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector(-1,-1, 1))
                m_points[4] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector( 1,-1, 1))
                m_points[5] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector( 1, 1, 1))
                m_points[6] = points[i];
            else if (center_vectors[i] == BSO::Vectors::Vector(-1, 1, 1))
                m_points[7] = points[i];
        }

        for (int i = 0; i < 8; i++)
        {
            if (m_points[i] == nullptr)
            {
                std::cout << "Structural component is not a hexahedron or is severely distorted (Component.hpp), exiting now..." << std::endl;
                exit(1);
            }
        }

        m_lines.clear();
        m_lines.resize(12);
        m_lines[0]  = std::make_pair(m_points[0], m_points[1]);
        m_lines[1]  = std::make_pair(m_points[1], m_points[2]);
        m_lines[2]  = std::make_pair(m_points[2], m_points[3]);
        m_lines[3]  = std::make_pair(m_points[3], m_points[0]);

        m_lines[4]  = std::make_pair(m_points[4], m_points[5]);
        m_lines[5]  = std::make_pair(m_points[5], m_points[6]);
        m_lines[6]  = std::make_pair(m_points[6], m_points[7]);
        m_lines[7]  = std::make_pair(m_points[7], m_points[4]);

        m_lines[8]  = std::make_pair(m_points[0], m_points[4]);
        m_lines[9]  = std::make_pair(m_points[1], m_points[5]);
        m_lines[10] = std::make_pair(m_points[2], m_points[6]);
        m_lines[11] = std::make_pair(m_points[3], m_points[7]);

        m_faces.clear();
        m_faces.resize(6);
        m_faces[0][0] = m_points[0];
        m_faces[0][1] = m_points[1];
        m_faces[0][2] = m_points[2];
        m_faces[0][3] = m_points[3];

        m_faces[1][0] = m_points[4];
        m_faces[1][1] = m_points[5];
        m_faces[1][2] = m_points[6];
        m_faces[1][3] = m_points[7];

        m_faces[2][0] = m_points[0];
        m_faces[2][1] = m_points[1];
        m_faces[2][2] = m_points[5];
        m_faces[2][3] = m_points[4];

        m_faces[3][0] = m_points[1];
        m_faces[3][1] = m_points[2];
        m_faces[3][2] = m_points[6];
        m_faces[3][3] = m_points[5];

        m_faces[4][0] = m_points[2];
        m_faces[4][1] = m_points[3];
        m_faces[4][2] = m_points[7];
        m_faces[4][3] = m_points[6];

        m_faces[5][0] = m_points[3];
        m_faces[5][1] = m_points[0];
        m_faces[5][2] = m_points[4];
        m_faces[5][3] = m_points[7];

        //check if each of the points on the faces are coplanar
        for (unsigned int i = 0; i < 6; i++)
        { // for each face
            BSO::Vectors::Vector v_1 = m_faces[i][2]->get_coords() - m_faces[i][0]->get_coords();
            BSO::Vectors::Vector v_2 = m_faces[i][3]->get_coords() - m_faces[i][1]->get_coords();

            if(!BSO::Vectors::intersection(m_faces[i][0]->get_coords(), v_1, m_faces[i][0]->get_coords(), v_2, 0.001))
            {
                std::cout << "Component is not a hexahedron, faces are not a plane (Component.hpp), exiting now..." << std::endl;
                exit(1);
            }
        }
    }
*/


} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // COMPONENT_HPP
