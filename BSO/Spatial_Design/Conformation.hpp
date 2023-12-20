#ifndef MS_CONFORMAL_HPP
#define MS_CONFORMAL_HPP

#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Spatial_Design/Movable_Sizable.hpp> //ms_building en of niet ms_space
#include <BSO/Spatial_Design/Geometry/Geometry.hpp> //cf_buildin en miss geometry from utilities

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>

namespace BSO { namespace Spatial_Design
{
/*
 *
 */

// forward declaration of classes
class MS_Conformal;

} // namespace Spatial_Design
namespace Building_Physics {
    struct BP_Simulation_Vars;
} // namespace Building_Physics
namespace Structural_Design {
    struct SD_Analysis_Vars;
} // namespace Structural_Design
namespace Spatial_Design {

// typedef for function pointers to grammars
typedef void (*Grammar_Ptr)(MS_Conformal*);
typedef void (*BP_Grammar_Ptr)(MS_Conformal*, BSO::Building_Physics::BP_Simulation_Vars*);
typedef void (*SD_Grammar_Ptr)(MS_Conformal*, Structural_Design::SD_Analysis_Vars*);


// Class definition:
class MS_Conformal : public Geometry::Vertex_Store
{
private:
    std::vector<Geometry::Point*> m_points;
    std::vector<Geometry::Edge*> m_edges;
    std::vector<Geometry::Surface*> m_surfaces;
    std::vector<Geometry::Space*> m_spaces;

    BP_Grammar_Ptr m_BP_grammar;
    SD_Grammar_Ptr m_SD_grammar;


public:
    MS_Conformal(std::string file_name, Grammar_Ptr);
    MS_Conformal(MS_Building&, Grammar_Ptr);
    ~MS_Conformal();

    void add_grammars(BP_Grammar_Ptr, SD_Grammar_Ptr);
    void add_grammars(BP_Grammar_Ptr);
    void add_grammars(SD_Grammar_Ptr);
    BP_Grammar_Ptr request_BP_grammar();
    SD_Grammar_Ptr request_SD_grammar();

    void make_conformal();

    Geometry::Point* add_point(Geometry::Vertex* vertex_ptr);

    Geometry::Edge* add_edge(Geometry::Line* line_ptr,
                             Geometry::Point* p_1,
                             Geometry::Point* p_2);

    Geometry::Surface* add_surface(Geometry::Rectangle* rectangle_ptr,
                                   Geometry::Edge* e_1,
                                   Geometry::Edge* e_2,
                                   Geometry::Edge* e_3,
                                   Geometry::Edge* e_4);

    void add_point(double x, double y, double z);
    void add_edge(double x1, double y1, double z1,
                  double x2, double y2, double z2); // orthogonal only
    void add_surface(double x1, double y1, double z1,
                     double x2, double y2, double z2); // orthogonal only
    void add_space(MS_Space); // orthogonal only
	
	Geometry::Point* get_point(unsigned int n);
    unsigned int get_point_count();
    Geometry::Edge* get_edge(unsigned int n);
    unsigned int get_edge_count();
    Geometry::Surface* get_surface(unsigned int n);
    unsigned int get_surface_count();
    Geometry::Space* get_space(unsigned int n);
    unsigned int get_space_count();

    Geometry::Cuboid* get_cuboid(unsigned int n);
    unsigned int get_cuboid_count();
    Geometry::Rectangle* get_rectangle(unsigned int n);
    unsigned int get_rectangle_count();
    Geometry::Line* get_line(int n);
    unsigned int get_line_count();
    Geometry::Vertex* get_vertex(unsigned int n);
    unsigned int get_vertex_count();
}; // MS_Conformal




// Implementation of member functions

MS_Conformal::MS_Conformal(std::string file_name, Grammar_Ptr grammar)
{
    MS_Building S(file_name);

    for (int i = 0; i < S.obtain_space_count(); i++)
    {
        this->add_space(S.obtain_space(i));
    }

    // execute grammar
    grammar(this);

} // ctor

MS_Conformal::MS_Conformal(BSO::Spatial_Design::MS_Building& S, Grammar_Ptr grammar)
{
    for (int i = 0; i < S.obtain_space_count(); i++)
    {
        this->add_space(S.obtain_space(i));
    }

    // execute grammar
    grammar(this);

} // ctor

MS_Conformal::~MS_Conformal()
{
    for (auto i : m_rectangles)
    {
        i->deassociate_lines();
    }

    for (unsigned int i = 0; i < m_points.size(); i++)
    {
        delete m_points[i];
        m_points.clear();
    }

    for (unsigned int i = 0; i < m_edges.size(); i++)
    {
        delete m_edges[i];
        m_edges.clear();
    }

    for (unsigned int i = 0; i < m_surfaces.size(); i++)
    {
        delete m_surfaces[i];
        m_surfaces.clear();
    }

    for (unsigned int i = 0; i < m_spaces.size(); i++)
    {
        delete m_spaces[i];
        m_spaces.clear();
    }

} // dtor

void MS_Conformal::add_grammars(BP_Grammar_Ptr BP_grammar, SD_Grammar_Ptr SD_grammar)
{
    m_BP_grammar = BP_grammar;
    m_SD_grammar = SD_grammar;
} // add_grammar

void MS_Conformal::add_grammars(BP_Grammar_Ptr BP_grammar)
{
    m_BP_grammar = BP_grammar;
} // add_grammar()

void MS_Conformal::add_grammars(SD_Grammar_Ptr SD_grammar)
{
    m_SD_grammar = SD_grammar;
} // add_grammar()

BP_Grammar_Ptr MS_Conformal::request_BP_grammar()
{
    return m_BP_grammar;
} // request_BP_grammar

SD_Grammar_Ptr MS_Conformal::request_SD_grammar()
{
    return m_SD_grammar;
} // request_SD_grammar


void MS_Conformal::make_conformal()
{
    // check for intersections
    Geometry::Vertex* temp_ptr = new Geometry::Vertex; // to store possible intersection points
    for (unsigned int i = 0; i < m_rectangles.size(); i++) // check line-rectangle intersections
    {
        for (unsigned int j = 0; j < m_lines.size(); j++)
        {
            if (m_rectangles[i]->check_line_intersect(m_lines[j], temp_ptr))
            {
                Vertex_Store::add_vertex(temp_ptr->get_coords());
            }
        }
    }

    for (unsigned int i = 0; i < m_lines.size(); i++) // check line-line intersections
    {
        for (unsigned int j = 0; j < m_lines.size(); j++)
        {
            if ((i != j) && (m_lines[i]->check_line_intersect(m_lines[j], temp_ptr)))
            {
                Vertex_Store::add_vertex(temp_ptr->get_coords());
            }
        }
    }
    delete temp_ptr;

    // check if vertices interfere with spaces
    for (unsigned int i = 0; i < m_spaces.size(); i++)
    {
        for (unsigned int j = 0; j < m_vertices.size(); j++)
        {
            m_spaces[i]->check_vertex(m_vertices[j]);
        }
    }


    // delete all lines, rectangles and cuboids that are tagged for deletion
    for (unsigned int i = 0; i < m_cubes.size(); i++)
    {
        if (m_cubes[i]->check_deletion())
        {
            Vertex_Store::delete_cuboid(m_cubes[i]);
            i--;
        }
    }

    for (unsigned int i = 0; i < m_rectangles.size(); i++)
    {
        if (m_rectangles[i]->check_deletion())
        {
            Vertex_Store::delete_rectangle(m_rectangles[i]);
            i--;
        }
    }

    for (unsigned int i = 0; i < m_lines.size(); i++)
    {
        if (m_lines[i]->check_deletion())
        {
            Vertex_Store::delete_line(m_lines[i]);
            i--;
        }
    }

    // make associations
    for (auto i :m_rectangles)
    {
        i->associate_lines();
    }

} // make_conformal

Geometry::Point* MS_Conformal::add_point(Geometry::Vertex* vertex_ptr)
{
    using namespace Geometry;
    m_points.push_back(new Point(vertex_ptr));
    return m_points.back();
} // add_point()

void MS_Conformal::add_point(double x, double y, double z)
{
    using namespace Geometry;
    Vertex* v_1 = Vertex_Store::add_vertex(x, y, z);
    Point* p_1 = new Point(v_1);
    m_points.push_back(p_1);
    v_1->add_point(p_1);
} // add_point()

Geometry::Edge* MS_Conformal::add_edge(Geometry::Line* line_ptr,
                                       Geometry::Point* p_1,
                                       Geometry::Point* p_2)
{
    using namespace Geometry;
    m_edges.push_back(new Edge(line_ptr, p_1, p_2));
    return m_edges.back();
} // add_edge()

void MS_Conformal::add_edge(double x1, double y1, double z1,
                            double x2, double y2, double z2)
{
    using namespace Geometry;
    Vertex* v_1, * v_2;
    Point* p_1, * p_2;
    if (((x1 == x2) && (y1 == y2)) && (z1 != z2))
    {
        v_1 = Vertex_Store::add_vertex(x1,     y1,      z1   );    p_1 = add_point(v_1);    v_1->add_point(p_1);
        v_2 = Vertex_Store::add_vertex(x1,     y1,      z2   );    p_2 = add_point(v_2);    v_2->add_point(p_2);
    }
    else if((x1 != x2) && ((y1 == y2) && (z1 == z1)))
    {
        v_1 = Vertex_Store::add_vertex(x1,     y1,      z1   );    p_1 = add_point(v_1);    v_1->add_point(p_1);
        v_2 = Vertex_Store::add_vertex(x2,     y1,      z1   );    p_2 = add_point(v_2);    v_2->add_point(p_2);
    }
    else if ((y1 == y2) && ((x1 != x2) && (z1 == z1)))
    {
        v_1 = Vertex_Store::add_vertex(x1,     y1,      z1   );    p_1 = add_point(v_1);    v_1->add_point(p_1);
        v_2 = Vertex_Store::add_vertex(x1,     y2,      z1   );    p_2 = add_point(v_2);    v_2->add_point(p_2);
    }
    else
    {
        std::cerr << "Error, edge is not defined orthogonal, exiting..." << std::endl;
        exit(1);
    }

    Line* l_1 = Vertex_Store::add_line(v_1, v_2);
    Edge* e_1 = new Edge(l_1, p_1, p_2);
    m_edges.push_back(e_1);
    l_1->add_edge(e_1);

    p_1->add_edge(e_1); p_2->add_edge(e_1);
} // add_edge()

Geometry::Surface* MS_Conformal::add_surface(Geometry::Rectangle* rectangle_ptr,
                                             Geometry::Edge* e_1, Geometry::Edge* e_2,
                                             Geometry::Edge* e_3, Geometry::Edge* e_4)
{
    using namespace Geometry;
    m_surfaces.push_back(new Surface(rectangle_ptr, e_1, e_2, e_3, e_4));
    return m_surfaces.back();
} // add_surface()

void MS_Conformal::add_surface(double x1, double y1, double z1,
                               double x2, double y2, double z2)
{
    using namespace Geometry;
    Vertex* v_1, * v_2, * v_3, * v_4;
    Point* p_1, * p_2, * p_3, * p_4;
    if ((x1 == x2) && ((y1 != y2) && (z1 != z2)))
    {
        v_1 = Vertex_Store::add_vertex(x1,     y1,      z1  );    p_1 = add_point(v_1);    v_1->add_point(p_1);
        v_2 = Vertex_Store::add_vertex(x1,     y1,      z2  );    p_2 = add_point(v_2);    v_2->add_point(p_2);
        v_3 = Vertex_Store::add_vertex(x1,     y2,      z1  );    p_3 = add_point(v_3);    v_3->add_point(p_3);
        v_4 = Vertex_Store::add_vertex(x1,     y2,      z2  );    p_4 = add_point(v_4);    v_4->add_point(p_4);
    }
    else if ((y1 == y2) && ((x1 != x2) && (z1 != z2)))
    {
        v_1 = Vertex_Store::add_vertex(x1,     y1,      z1  );    p_1 = add_point(v_1);    v_1->add_point(p_1);
        v_2 = Vertex_Store::add_vertex(x1,     y1,      z2  );    p_2 = add_point(v_2);    v_2->add_point(p_2);
        v_3 = Vertex_Store::add_vertex(x2,     y1,      z1  );    p_3 = add_point(v_3);    v_3->add_point(p_3);
        v_4 = Vertex_Store::add_vertex(x2,     y1,      z2  );    p_4 = add_point(v_4);    v_4->add_point(p_4);
    }
    else if ((z1 == z2) && ((x1 != x2) && (y1 != y2)))
    {
        v_1 = Vertex_Store::add_vertex(x1,     y1,      z1  );    p_1 = add_point(v_1);    v_1->add_point(p_1);
        v_2 = Vertex_Store::add_vertex(x1,     y2,      z1  );    p_2 = add_point(v_2);    v_2->add_point(p_2);
        v_3 = Vertex_Store::add_vertex(x2,     y1,      z1  );    p_3 = add_point(v_3);    v_3->add_point(p_3);
        v_4 = Vertex_Store::add_vertex(x2,     y2,      z1  );    p_4 = add_point(v_4);    v_4->add_point(p_4);
    }
    else
    {
        std::cerr << "Error, surface is not an orthogonal plane, exiting..." << std::endl;
        exit(1);
    }

    Line* l_01 = Vertex_Store::add_line(v_1,  v_2);     Edge* e_01 = add_edge(l_01, p_1,  p_2);    l_01->add_edge(e_01);   p_1->add_edge(e_01);    p_2->add_edge(e_01);
    Line* l_02 = Vertex_Store::add_line(v_2,  v_4);     Edge* e_02 = add_edge(l_02, p_2,  p_4);    l_02->add_edge(e_02);   p_2->add_edge(e_01);    p_4->add_edge(e_01);
    Line* l_03 = Vertex_Store::add_line(v_4,  v_3);     Edge* e_03 = add_edge(l_03, p_4,  p_3);    l_03->add_edge(e_03);   p_4->add_edge(e_01);    p_3->add_edge(e_01);
    Line* l_04 = Vertex_Store::add_line(v_3,  v_1);     Edge* e_04 = add_edge(l_04, p_3,  p_1);    l_04->add_edge(e_04);   p_3->add_edge(e_01);    p_1->add_edge(e_01);

    Rectangle* r_1 = Vertex_Store::add_rectangle(l_01,  l_02,   l_03,   l_04);
    Surface* s_1 = new Surface(r_1, e_01,  e_02,   e_03,   e_04);
    m_surfaces.push_back(s_1);
    r_1->add_surface(s_1);

    e_01->add_surface(s_1); e_02->add_surface(s_1); e_03->add_surface(s_1); e_04->add_surface(s_1);
    p_1->add_surface(s_1);  p_2->add_surface(s_1);  p_3->add_surface(s_1);  p_4->add_surface(s_1);
} // add_surface()

void MS_Conformal::add_space(MS_Space S)
{
    using namespace Geometry;
    Vertex* v_1 = Vertex_Store::add_vertex(S.x,         S.y,            S.z         );    Point* p_1 = add_point(v_1);    v_1->add_point(p_1);
    Vertex* v_2 = Vertex_Store::add_vertex(S.x,         S.y,            S.z+S.height);    Point* p_2 = add_point(v_2);    v_2->add_point(p_2);
    Vertex* v_3 = Vertex_Store::add_vertex(S.x,         S.y+S.depth,    S.z         );    Point* p_3 = add_point(v_3);    v_3->add_point(p_3);
    Vertex* v_4 = Vertex_Store::add_vertex(S.x,         S.y+S.depth,    S.z+S.height);    Point* p_4 = add_point(v_4);    v_4->add_point(p_4);
    Vertex* v_5 = Vertex_Store::add_vertex(S.x+S.width, S.y,            S.z         );    Point* p_5 = add_point(v_5);    v_5->add_point(p_5);
    Vertex* v_6 = Vertex_Store::add_vertex(S.x+S.width, S.y,            S.z+S.height);    Point* p_6 = add_point(v_6);    v_6->add_point(p_6);
    Vertex* v_7 = Vertex_Store::add_vertex(S.x+S.width, S.y+S.depth,    S.z         );    Point* p_7 = add_point(v_7);    v_7->add_point(p_7);
    Vertex* v_8 = Vertex_Store::add_vertex(S.x+S.width, S.y+S.depth,    S.z+S.height);    Point* p_8 = add_point(v_8);    v_8->add_point(p_8);


    Line* l_01 = Vertex_Store::add_line(v_1,  v_3);     Edge* e_01 = add_edge(l_01, p_1,  p_3);    l_01->add_edge(e_01);   p_1->add_edge(e_01);    p_3->add_edge(e_01);
    Line* l_02 = Vertex_Store::add_line(v_3,  v_7);     Edge* e_02 = add_edge(l_02, p_3,  p_7);    l_02->add_edge(e_02);   p_3->add_edge(e_02);    p_7->add_edge(e_02);
    Line* l_03 = Vertex_Store::add_line(v_7,  v_5);     Edge* e_03 = add_edge(l_03, p_7,  p_5);    l_03->add_edge(e_03);   p_7->add_edge(e_03);    p_5->add_edge(e_03);
    Line* l_04 = Vertex_Store::add_line(v_5,  v_1);     Edge* e_04 = add_edge(l_04, p_5,  p_1);    l_04->add_edge(e_04);   p_5->add_edge(e_04);    p_1->add_edge(e_04);
    Line* l_05 = Vertex_Store::add_line(v_2,  v_4);     Edge* e_05 = add_edge(l_05, p_2,  p_4);    l_05->add_edge(e_05);   p_2->add_edge(e_05);    p_4->add_edge(e_05);
    Line* l_06 = Vertex_Store::add_line(v_4,  v_8);     Edge* e_06 = add_edge(l_06, p_4,  p_8);    l_06->add_edge(e_06);   p_4->add_edge(e_06);    p_8->add_edge(e_06);
    Line* l_07 = Vertex_Store::add_line(v_8,  v_6);     Edge* e_07 = add_edge(l_07, p_8,  p_6);    l_07->add_edge(e_07);   p_8->add_edge(e_07);    p_6->add_edge(e_07);
    Line* l_08 = Vertex_Store::add_line(v_6,  v_2);     Edge* e_08 = add_edge(l_08, p_6,  p_2);    l_08->add_edge(e_08);   p_6->add_edge(e_08);    p_2->add_edge(e_08);
    Line* l_09 = Vertex_Store::add_line(v_1,  v_2);     Edge* e_09 = add_edge(l_09, p_1,  p_2);    l_09->add_edge(e_09);   p_1->add_edge(e_09);    p_2->add_edge(e_09);
    Line* l_10 = Vertex_Store::add_line(v_3,  v_4);     Edge* e_10 = add_edge(l_10, p_3,  p_4);    l_10->add_edge(e_10);   p_3->add_edge(e_10);    p_4->add_edge(e_10);
    Line* l_11 = Vertex_Store::add_line(v_7,  v_8);     Edge* e_11 = add_edge(l_11, p_7,  p_8);    l_11->add_edge(e_11);   p_7->add_edge(e_11);    p_8->add_edge(e_11);
    Line* l_12 = Vertex_Store::add_line(v_5,  v_6);     Edge* e_12 = add_edge(l_12, p_5,  p_6);    l_12->add_edge(e_12);   p_5->add_edge(e_12);    p_6->add_edge(e_12);


    Rectangle* r_1 = Vertex_Store::add_rectangle(l_01,  l_02,   l_03,   l_04);      Surface* s_1 = add_surface(r_1, e_01,  e_02,   e_03,   e_04);   r_1->add_surface(s_1);
    Rectangle* r_2 = Vertex_Store::add_rectangle(l_05,  l_06,   l_07,   l_08);      Surface* s_2 = add_surface(r_2, e_05,  e_06,   e_07,   e_08);   r_2->add_surface(s_2);
    Rectangle* r_3 = Vertex_Store::add_rectangle(l_10,  l_05,   l_09,   l_01);      Surface* s_3 = add_surface(r_3, e_10,  e_05,   e_09,   e_01);   r_3->add_surface(s_3);
    Rectangle* r_4 = Vertex_Store::add_rectangle(l_11,  l_06,   l_10,   l_02);      Surface* s_4 = add_surface(r_4, e_11,  e_06,   e_10,   e_02);   r_4->add_surface(s_4);
    Rectangle* r_5 = Vertex_Store::add_rectangle(l_12,  l_07,   l_11,   l_03);      Surface* s_5 = add_surface(r_5, e_12,  e_07,   e_11,   e_03);   r_5->add_surface(s_5);
    Rectangle* r_6 = Vertex_Store::add_rectangle(l_09,  l_08,   l_12,   l_04);      Surface* s_6 = add_surface(r_6, e_09,  e_08,   e_12,   e_04);   r_6->add_surface(s_6);

    s_1->assign_surface_type(S.surface_type[5]); s_2->assign_surface_type(S.surface_type[4]); s_3->assign_surface_type(S.surface_type[3]);
    s_4->assign_surface_type(S.surface_type[0]); s_5->assign_surface_type(S.surface_type[1]); s_6->assign_surface_type(S.surface_type[2]);

    e_01->add_surface(s_1); e_02->add_surface(s_1); e_03->add_surface(s_1); e_04->add_surface(s_1); p_1->add_surface(s_1);  p_3->add_surface(s_1);  p_5->add_surface(s_1);  p_7->add_surface(s_1);
    e_05->add_surface(s_2); e_06->add_surface(s_2); e_07->add_surface(s_2); e_08->add_surface(s_2); p_2->add_surface(s_2);  p_4->add_surface(s_2);  p_8->add_surface(s_2);  p_6->add_surface(s_2);
    e_10->add_surface(s_3); e_05->add_surface(s_3); e_09->add_surface(s_3); e_01->add_surface(s_3); p_1->add_surface(s_3);  p_3->add_surface(s_3);  p_4->add_surface(s_3);  p_2->add_surface(s_3);
    e_11->add_surface(s_4); e_06->add_surface(s_4); e_10->add_surface(s_4); e_02->add_surface(s_4); p_7->add_surface(s_4);  p_8->add_surface(s_4);  p_4->add_surface(s_4);  p_3->add_surface(s_4);
    e_12->add_surface(s_5); e_07->add_surface(s_5); e_11->add_surface(s_5); e_03->add_surface(s_5); p_5->add_surface(s_5);  p_6->add_surface(s_5);  p_8->add_surface(s_5);  p_7->add_surface(s_5);
    e_09->add_surface(s_6); e_08->add_surface(s_6); e_12->add_surface(s_6); e_04->add_surface(s_6); p_1->add_surface(s_6);  p_2->add_surface(s_6);  p_5->add_surface(s_6);  p_6->add_surface(s_6);


    Cuboid* c_1 = Vertex_Store::add_cuboid(r_1,   r_2,    r_3,    r_4,    r_5,    r_6);
    Space* space_1 = new Space(S.ID, c_1, s_1,   s_2,    s_3,    s_4,    s_5,    s_6);
    space_1->assign_space_type(S.m_space_type);
    m_spaces.push_back(space_1);
    c_1->add_space(space_1);

    p_1->add_space(space_1);    p_2->add_space(space_1);    p_3->add_space(space_1);    p_4->add_space(space_1);
    p_5->add_space(space_1);    p_6->add_space(space_1);    p_7->add_space(space_1);    p_8->add_space(space_1);

    e_01->add_space(space_1);   e_02->add_space(space_1);   e_03->add_space(space_1);   e_04->add_space(space_1);   e_05->add_space(space_1);   e_06->add_space(space_1);
    e_07->add_space(space_1);   e_08->add_space(space_1);   e_09->add_space(space_1);   e_10->add_space(space_1);   e_11->add_space(space_1);   e_12->add_space(space_1);

    s_1->add_space(space_1);    s_2->add_space(space_1);    s_3->add_space(space_1);
    s_4->add_space(space_1);    s_5->add_space(space_1);    s_6->add_space(space_1);
} // add_space()

Geometry::Point* MS_Conformal::get_point(unsigned int n)
{
    return m_points[n];
} // get_point()

unsigned int MS_Conformal::get_point_count()
{
    return m_points.size();
} // get_point_count()

Geometry::Edge* MS_Conformal::get_edge(unsigned int n)
{
    return m_edges[n];
} // get_edge()

unsigned int MS_Conformal::get_edge_count()
{
    return m_edges.size();
} // get_edge_count()

Geometry::Surface* MS_Conformal::get_surface(unsigned int n)
{
    return m_surfaces[n];
} // get_surface()

unsigned int MS_Conformal::get_surface_count()
{
    return m_surfaces.size();
} // get_surface_count()

Geometry::Space* MS_Conformal::get_space(unsigned int n)
{
    return m_spaces[n];
} // get_space()

unsigned int MS_Conformal::get_space_count()
{
    return m_spaces.size();
} // get_space_count()

Geometry::Cuboid* MS_Conformal::get_cuboid(unsigned int n)
{
    return Vertex_Store::m_cubes[n];
} // get_cuboid()

unsigned int MS_Conformal::get_cuboid_count()
{
    return Vertex_Store::m_cubes.size();
}

Geometry::Rectangle* MS_Conformal::get_rectangle(unsigned int n)
{
    return Vertex_Store::m_rectangles[n];
} // get_rectangle()

unsigned int MS_Conformal::get_rectangle_count()
{
    return Vertex_Store::m_rectangles.size();
} // get_rectangle_count()

Geometry::Line* MS_Conformal::get_line(int n)
{
    return Vertex_Store::m_lines[n];
} // get_line()

unsigned int MS_Conformal::get_line_count()
{
    return Vertex_Store::m_lines.size();
} // get_line_count()

Geometry::Vertex* MS_Conformal::get_vertex(unsigned int n)
{
    return Vertex_Store::m_vertices[n];
} // get_vertex()

unsigned int MS_Conformal::get_vertex_count()
{
    return Vertex_Store::m_vertices.size();
} // get_vertex_count()

} // namespace Spatial_Design
} // namespace BSO

#endif // MS_CONFORMAL_HPP
