#include <GL/freeglut.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <fstream>

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Conformation.hpp>
#include <BSO/Spatial_Design/Zoning.hpp>
#include <AEI_Grammar/Grammar_zoning.hpp>


BSO::Spatial_Design::MS_Building MS1("JH_Zoning_Assignment_GUI/MS_Input.txt");
BSO::Spatial_Design::MS_Conformal CF1(MS1, &(BSO::Grammar::grammar_zoning));

BSO::Spatial_Design::MS_Building MS2("JH_Zoning_Assignment_GUI/Designs and settings/Design 3/MS_Input.txt");
BSO::Spatial_Design::MS_Conformal CF2(MS2, &(BSO::Grammar::grammar_zoning));

void makeConformal1() {
    CF1.make_conformal();
}

void makeConformal2() {
    CF2.make_conformal();
}

BSO::Spatial_Design::Zoning::Zoned_Design makeZoning1() {
    makeConformal1();
    BSO::Spatial_Design::Zoning::Zoned_Design ZD1(&CF1);
    ZD1.make_zoning();
    return ZD1;
}

BSO::Spatial_Design::Zoning::Zoned_Design makeZoning2() {
    makeConformal2();
    BSO::Spatial_Design::Zoning::Zoned_Design ZD2(&CF2);
    ZD2.make_zoning();
    return ZD2;
}

std::vector<BSO::Spatial_Design::Geometry::Vertex*> boundaryVertices(std::vector<BSO::Spatial_Design::Geometry::Vertex*> vertices) {
    std::unordered_map<BSO::Spatial_Design::Geometry::Vertex*, int> vertexCount;

    // Count each vertex's appearances
    for (auto vertex : vertices) {
        vertexCount[vertex]++;
    }

    std::vector<BSO::Spatial_Design::Geometry::Vertex*> uniqueVertices;
    // Add vertices that appear exactly once to the result
    for (const auto& pair : vertexCount) {
        if (pair.second == 1) {
            uniqueVertices.push_back(pair.first);
        }
    }

    return uniqueVertices;
}

std::pair<BSO::Spatial_Design::Geometry::Vertex*, BSO::Spatial_Design::Geometry::Vertex*> getDiagonal(std::vector<BSO::Spatial_Design::Geometry::Vertex*> points) {
    // Based on the provided 8 points for a 3D figure, we want to get the diagonal in the 2D projection
    // The diagonal is the line that connects the two points that are farthest from each other
    BSO::Spatial_Design::Geometry::Vertex* v1 = points[0];
    BSO::Spatial_Design::Geometry::Vertex* v2 = points[1];
    for(auto point : points) {
        double p2x = v2->get_coords()[0];
        double p2y = v2->get_coords()[1];
        double p1x = v1->get_coords()[0];
        double p1y = v1->get_coords()[1];

        double p3x = point->get_coords()[0];
        double p3y = point->get_coords()[1];

        if ((p1x - p2x) * (p1x - p2x) + (p1y - p2y) * (p1y - p2y) < (p1x - p3x) * (p1x - p3x) + (p1y - p3y) * (p1y - p3y)) {
            v2 = point;
        }
    }
    return std::make_pair(v1, v2);
}

bool areNonCollinear(BSO::Spatial_Design::Geometry::Vertex* a, BSO::Spatial_Design::Geometry::Vertex* b, BSO::Spatial_Design::Geometry::Vertex* c) {
    double x1 = b->get_coords()[0] - a->get_coords()[0];
    double y1 = b->get_coords()[1] - a->get_coords()[1];
    double x2 = c->get_coords()[0] - a->get_coords()[0];
    double y2 = c->get_coords()[1] - a->get_coords()[1];
    double determinant = x1 * y2 - y1 * x2;
    return std::abs(determinant) > 1e-5; // Small epsilon to handle floating point precision issues
}

void printPrisms(std::vector<BSO::Spatial_Design::Geometry::Vertex*> points, 
                 std::pair<BSO::Spatial_Design::Geometry::Vertex*, BSO::Spatial_Design::Geometry::Vertex*> diag, 
                 std::ofstream& outfile, int design, int zone) {
    // Base and difference calculations for z coordinates
    double z_base = diag.first->get_coords()[2];
    double z_difference = diag.second->get_coords()[2] - z_base;

    // Min and max calculations for x and y coordinates
    double x_min = std::min(diag.first->get_coords()[0], diag.second->get_coords()[0]);
    double x_max = std::max(diag.first->get_coords()[0], diag.second->get_coords()[0]);
    double y_min = std::min(diag.first->get_coords()[1], diag.second->get_coords()[1]);
    double y_max = std::max(diag.first->get_coords()[1], diag.second->get_coords()[1]);

    // Calculating two additional vertices for the prisms
    BSO::Spatial_Design::Geometry::Vertex* p3 = new BSO::Spatial_Design::Geometry::Vertex(x_min, y_max, z_base);
    BSO::Spatial_Design::Geometry::Vertex* p4 = new BSO::Spatial_Design::Geometry::Vertex(x_max, y_min, z_base);

    // Output each prism with 19 numbers
    // Prism with p3 as the third vertex
    outfile << zone << "," 
            << diag.first->get_coords()[0] << "," << diag.first->get_coords()[1] << "," << diag.first->get_coords()[2] << ","
            << diag.second->get_coords()[0] << "," << diag.second->get_coords()[1] << "," << diag.first->get_coords()[2] << ","
            << p3->get_coords()[0] << "," << p3->get_coords()[1] << "," << diag.first->get_coords()[2] << ","
            << diag.first->get_coords()[0] << "," << diag.first->get_coords()[1] << "," << diag.first->get_coords()[2] + z_difference << ","
            << diag.second->get_coords()[0] << "," << diag.second->get_coords()[1] << "," << diag.first->get_coords()[2] + z_difference << ","
            << p3->get_coords()[0] << "," << p3->get_coords()[1] << "," << diag.first->get_coords()[2] + z_difference << std::endl;

    // Prism with p4 as the third vertex
    outfile << zone << ","
            << diag.first->get_coords()[0] << "," << diag.first->get_coords()[1] << "," << diag.first->get_coords()[2] << ","
            << diag.second->get_coords()[0] << "," << diag.second->get_coords()[1] << "," << diag.first->get_coords()[2] << ","
            << p4->get_coords()[0] << "," << p4->get_coords()[1] << "," << diag.first->get_coords()[2] << ","
            << diag.first->get_coords()[0] << "," << diag.first->get_coords()[1] << "," << diag.first->get_coords()[2] + z_difference << ","
            << diag.second->get_coords()[0] << "," << diag.second->get_coords()[1] << "," << diag.first->get_coords()[2] + z_difference << ","
            << p4->get_coords()[0] << "," << p4->get_coords()[1] << "," << diag.first->get_coords()[2] + z_difference << std::endl;

    // Cleaning up dynamically allocated vertices
    delete p3;
    delete p4;
}


void printZonedDesign(BSO::Spatial_Design::Zoning::Zoned_Design ZD, int design) {
    std::ofstream outFile("comparison" + std::to_string(design) + ".txt");

    std::cout << "Zoned Design" << std::endl;
    std::cout << "Zones: " << ZD.get_zones().size() << std::endl;
    int k = 0;
    for (auto zone : ZD.get_zones()) {
        std::cout << "Zone" << std::endl;
        std::vector<BSO::Spatial_Design::Geometry::Vertex*> vertices;
        for (auto cuboid : zone->get_cuboids()) {
            for (auto vertex : cuboid->get_vertices()) {
                vertices.push_back(vertex);
            }
        }
        std::vector<BSO::Spatial_Design::Geometry::Vertex*> bound = boundaryVertices(vertices);
        std::cout << "Zone vertices: " << bound.size() << std::endl;
        std::pair<BSO::Spatial_Design::Geometry::Vertex*, BSO::Spatial_Design::Geometry::Vertex*> diag = getDiagonal(bound);
        printPrisms(bound, diag, outFile, design, k);
        k++;
    }
}

int main() {
    std::cout << "Models constructed" << std::endl;
    BSO::Spatial_Design::Zoning::Zoned_Design ZD1 = makeZoning1();
    BSO::Spatial_Design::Zoning::Zoned_Design ZD2 = makeZoning2();
    std::cout << "Zonings made" << std::endl;
    printZonedDesign(ZD1, 1);
    printZonedDesign(ZD2, 2);

    return 0;
}