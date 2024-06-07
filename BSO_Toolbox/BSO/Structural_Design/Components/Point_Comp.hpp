#ifndef POINT_COMP_HPP
#define POINT_COMP_HPP

#include <BSO/Structural_Design/Components/Load.hpp>
#include <BSO/Structural_Design/Components/Constraint.hpp>
#include <BSO/Vectors.hpp>

#include <Eigen/Dense>

#include <iostream>
#include <map>

namespace Eigen {typedef Matrix<double, 6, 1> Vector6d;}

namespace BSO { namespace Structural_Design {  namespace Components {

    class Point
    {
    private:
        unsigned long m_ID;
        Eigen::Vector3d m_coordinates;

        std::map<unsigned int, Eigen::Vector6d> m_loads; // loads in point for each dof and each load case
        bool m_constraints[6]; // constraints in point for each dof

    public:
        Point(double x, double y, double z);
        Point(Vectors::Point p);
        ~Point();

        void update_loads(Load l);
        void update_constraints(Constraint c);
        void reset_settings();

        void set_ID(unsigned long ID);

        Eigen::Vector3d get_coords();
        unsigned long get_ID();
        std::vector<bool> get_constraints();
        std::map<unsigned int, Eigen::Vector6d> get_loads();

        bool operator == (Eigen::Vector3d rhs);
        Eigen::Vector3d operator - (Point rhs);
        Eigen::Vector3d operator + (Eigen::Vector3d rhs);
    };

    Point::Point(double x, double y, double z)
    {
        m_coordinates[0] = x;
        m_coordinates[1] = y;
        m_coordinates[2] = z;

        for (int i = 0; i < 6; i++)
        {
            m_constraints[i] = false;
        }
    } // ctor

    Point::Point(Vectors::Point p)
    {
        m_coordinates = p;

        for (int i = 0; i < 6; i++)
        {
            m_constraints[i] = false;
        }
    } // ctor

    Point::~Point()
    {

    } // dtor

    void Point::update_loads(Load l)
    {
        std::map<unsigned int, Eigen::Vector6d>::iterator ite;
        if (m_loads.find(l.m_lc) != m_loads.end())
        { // if this load case has already been added to the map
            m_loads[l.m_lc][l.m_dof] += l.m_value;
        }
        else
        { // if this load case has not yet been added to the map
            m_loads[l.m_lc] = Eigen::Vector6d::Zero();
            m_loads[l.m_lc][l.m_dof] += l.m_value;
        }

    } // update_loads

    void Point::update_constraints(Constraint c)
    {
        m_constraints[c.m_dof] = true;
    } // update_constraints()

    void Point::reset_settings()
    {
        m_loads.clear();
        for (int i = 0; i < 6; i++)
        {
            m_constraints[i] = false;
        }
    } // reset_settings()

    void Point::set_ID(unsigned long ID)
    {
        m_ID = ID;
    }

    Eigen::Vector3d Point::get_coords()
    {
        return m_coordinates;
    } // get_coords()

    unsigned long Point::get_ID()
    {
        return m_ID;
    }

    std::vector<bool> Point::get_constraints()
    {
        std::vector<bool> temp;
        for (int i = 0; i < 6; i++)
        {
            temp.push_back(m_constraints[i]);
        }
        return temp;
    } // get_constraints()

    std::map<unsigned int, Eigen::Vector6d> Point::get_loads()
    {
        return m_loads;
    }

    bool Point::operator == (Eigen::Vector3d rhs)
    {
        if (BSO::Vectors::is_zero(m_coordinates-rhs,0.01))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    Eigen::Vector3d Point::operator - (Point rhs)
    {
        return (m_coordinates - rhs.m_coordinates);
    }

    Eigen::Vector3d Point::operator + (Eigen::Vector3d rhs)
    {
        return (m_coordinates + rhs);
    }

} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // POINT_COMP_HPP
