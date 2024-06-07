#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include <BSO/Vectors.hpp>

#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

/*
 * This file serves as a forward declaration of the required classes in the conformation process
 */

namespace BSO { namespace Spatial_Design { namespace Geometry
{
    // forward declaration of the classes
    class Vertex;
    class Point;
    class Line;
    class Edge;
    class Rectangle;
    class Surface;
    class Cuboid;
    class Space;
    class Vertex_Store;
} // Geometry
} // Spatial_Design
} // BSO

// implementation of class structure and declaration of member function of the classes:
#include <BSO/Spatial_Design/Geometry/Vertex.hpp>
#include <BSO/Spatial_Design/Geometry/Point.hpp>
#include <BSO/Spatial_Design/Geometry/Line.hpp>
#include <BSO/Spatial_Design/Geometry/Edge.hpp>
#include <BSO/Spatial_Design/Geometry/Rectangle.hpp>
#include <BSO/Spatial_Design/Geometry/Surface.hpp>
#include <BSO/Spatial_Design/Geometry/Cuboid.hpp>
#include <BSO/Spatial_Design/Geometry/Space.hpp>
#include <BSO/Spatial_Design/Geometry/Vertex_Store.hpp>

//Implementation of member functions of the classes
#include <BSO/Spatial_Design/Geometry/Vertex.cpp>
#include <BSO/Spatial_Design/Geometry/Point.cpp>
#include <BSO/Spatial_Design/Geometry/Line.cpp>
#include <BSO/Spatial_Design/Geometry/Edge.cpp>
#include <BSO/Spatial_Design/Geometry/Rectangle.cpp>
#include <BSO/Spatial_Design/Geometry/Surface.cpp>
#include <BSO/Spatial_Design/Geometry/Cuboid.cpp>
#include <BSO/Spatial_Design/Geometry/Space.cpp>
#include <BSO/Spatial_Design/Geometry/Vertex_Store.cpp>

#endif // GEOMETRY_HPP
