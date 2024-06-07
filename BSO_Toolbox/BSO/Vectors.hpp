#ifndef VECTORS_HPP
#define VECTORS_HPP

#ifndef PI
#define PI 3.14159265359
#endif // PI

#include <Eigen/Dense>

#include <iostream>
#include <cstdlib>
#include <cmath>

namespace BSO { namespace Vectors {

    typedef Eigen::Vector3d Vector; // [dx, dy, dz]
    typedef Eigen::Vector3d Point; // [x, y, z]

    /*
     * The functions in this file are vector algebra
     */

    double magnitude(Vector v)
    {
        return (pow(v(0),2) + pow(v(1),2) + pow(v(2),2));
    }

    double length(Vector v)
    {
        return sqrt(magnitude(v));
    }

    Vector normalise(Vector v)
    {
        double l = length(v);

        if (l != 0)
        {
            return (v/l);
        }
        else
        {
            return v;
        }
    }

    double dot(Vector v1, Vector v2)
    {
        return ((v1(0)*v2(0)) + (v1(1)*v2(1)) + (v1(2)*v2(2)));
    }

    Vector cross(Vector v1, Vector v2)
    {
        Vector temp;
        temp(0) = (v1(1) * v2(2)) - (v1(2) * v2(1));
        temp(1) = (v1(2) * v2(0)) - (v1(0) * v2(2));
        temp(2) = (v1(0) * v2(1)) - (v1(1) * v2(0));

        return temp;
    }

    bool is_zero(double n, double tol)
    {
        if ((n >= -tol) && (n <= tol))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool is_zero(Vector v, double tol)
    {
        if (((v(0) >= -tol) && (v(0) <= tol)) &&
            ((v(1) >= -tol) && (v(1) <= tol)) &&
            ((v(2) >= -tol) && (v(2) <= tol)))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool is_perp(Vector v1, Vector v2, double tol)
    {
        return is_zero(dot(v1, v2), tol);
    }

    bool is_paral(Vector v1, Vector v2, double tol)
    {
        return is_zero(cross(v1, v2), tol);
    }

    bool is_vert(Vector v, double tol)
    {
        Vector vt = normalise(v);

        if (is_zero(vt(0), tol) && is_zero(vt(1), tol) && is_zero(1.0-abs(vt(2)), tol))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool same_dir(Vector v1, Vector v2, double tol)
    {
        if (!is_paral(v1, v2, tol))
        {
            return false;
        }

        Eigen::Vector3d::Index max_1, max_2;
        v1.cwiseAbs().maxCoeff(&max_1);
        v2.cwiseAbs().maxCoeff(&max_2);

        if ((v1(max_1) >= 0) ^ (v2(max_2) < 0))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool intersection(Vector p1, Vector v1, Vector p2, Vector v2, Vector& intersection, double tol)
    {
        Vector v3 = p2 - p1;

        if (is_paral(v1, v2, tol) && is_paral(v1, v3, tol))
        { // two lines are colinear
            return false;
        }
        else if (is_paral(v1, v2, tol))
        { // two lines are parallel
            return false;
        }
        else if (!is_zero(dot(v3, cross(v1, v2)), tol))
        { // two lines are skew and thus not coplanar
            return false;
        }
        else
        { // the two vectors have an intersection point
            double s = length(cross(v2, v3)) / length(cross(v2, v1));
            double t = length(cross(v1, v3)) / length(cross(v1, v2));

            if (s < 1.0 && t < 1.0)
            { // if the scalars are smaller than zero then the intersection point lies on each line
                intersection = (p1 + (s * v1));
                return true;
            }
            else if ((is_zero((s-1.0),tol) && t < 1.0) || (s < 1.0 && is_zero((t-1.0),tol)))
            { // if one of the scalars is one and the other is smaller than one
                // then one line point lies on the other line (but not on a point of that line)
                std::cerr << "warning, (Vectors.hpp). Lines intersect but no new point is generated (still need to fix this issue)" << std::endl;
                return true;
            }
            else
            {
                return false;
            }
        }
    }

    bool intersection(Point p1, Vector v1, Point p2, Vector v2, double tol)
    {
        Vector v3 = p2 - p1;

        if (is_paral(v1, v2, tol) || is_paral(v1, v3, tol))
        { // two lines are colinear
            return false;
        }
        else if (is_paral(v1, v2, tol))
        { // two lines are parallel
            return false;
        }
        else if (!is_zero(dot(v3, cross(v1, v2)), tol))
        { // two lines are skew and thus not coplanar
            return false;
        }
        else
        { // the two vectors have an intersection point
            double s = length(cross(v2, v3)) / length(cross(v2, v1));
            double t = length(cross(v1, v3)) / length(cross(v1, v2));

            if (s < 1.0 && t < 1.0)
            { // if the scalars are smaller than zero then the intersection point lies on each line
                return true;
            }
            else if ((is_zero((s-1.0),tol) && t < 1.0) || (s < 1.0 && is_zero((t-1.0),tol)))
            { // if one of the scalars is one and the other is smaller than one
                // then one line point lies on the other line (but not on a point of that line)
                return true;
            }
            else
            {
                return false;
            }
        }
    }
	
	bool coplanar(Point p_1, Point p_2, Point p_3, Point p_4, double tol)
	{ // https://math.stackexchange.com/questions/1330357/show-that-four-points-are-coplanar
		if (is_zero(dot((p_4 - p_1), cross(p_2 - p_1, p_3 - p_1)), tol))
			return true;
		else
			return false;
		
	} // coplanar();
	
	double area_triangle(Point a, Point b, Point c)
	{
		return length((cross(b-a,c-a)/2.0));
	} // area_triangle()
	
	bool inside_triangle(Point p_a, Point p_b, Point p_c, Point p_p, double tol)
	{ // https://stackoverflow.com/questions/5922027/how-to-determine-if-a-point-is-within-a-quadrilateral
		
		double alpha, beta, gamma;
		double tot_area = area_triangle(p_a, p_b, p_c);
		alpha = area_triangle(p_a, p_b, p_p) / tot_area;
		beta  = area_triangle(p_a, p_p, p_c) / tot_area;
		gamma = area_triangle(p_p, p_b, p_c) / tot_area;
		
		return (is_zero(alpha + beta + gamma - 1.0, tol));
	} // inside_triangle()


    double calc_area_quadri_lat(Vector v31, Vector v42)
    { // computes area of an arbitrary planar quadrilateral given its diagonals
        return (length(cross(v31, v42)) / 2.0);
    }
	
	

    double azimuth(Vector v)
    {
        double angle = atan2(v(0), v(1)) * (180/PI);
        if (angle < 0)
            angle = 360 + angle;
        return round(angle * 100) / 100;
    } // azimuth()

    double altitude(Vector v)
    {
        double angle = asin(v(2)/length(v)) * (180/PI);
        return round(angle * 100) / 100;
    }

    Vector calc_direction(double azimuth, double altitude)
    {
        Eigen::Vector3d dir = Eigen::Vector3d::Ones();
        dir(0) *= sin(azimuth*PI/180);
        dir(1) *= cos(azimuth*PI/180);
        dir(2) *= sin(altitude*PI/180);
        dir(0) *= cos(altitude*PI/180);
        dir(1) *= cos(altitude*PI/180);
        return dir;
    }

} // namespace Vectors
} // namespace BSO


#endif // TRIM_AND_CAST_HPP

