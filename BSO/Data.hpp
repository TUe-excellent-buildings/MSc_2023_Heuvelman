#ifndef BSO_DATA_HPP
#define BSO_DATA_HPP

#include <iostream>
#include <Eigen/Dense>
#include <map>

namespace BSO{
// this file contains data types and functions that are intended for handling and processing data

typedef Eigen::VectorXd data_point;
struct dist_function;
struct Cluster;


double calc_euclidian_distance(data_point x1, data_point x2)
{
    if (x1.rows() != x2.rows())
    {
        std::cerr << "Error when calculating Euclidian distance between data points (Clustering.hpp), " << std::endl
                  << "data points are not in same vector space, exiting..." << std::endl;
        exit(1);
    }

    double sum = 0;
    double difference;
    for (unsigned int i = 0; i < x1.rows(); i++)
    { // for each dimnesion of the data point
        difference = x2(i) - x1(i);
        sum += pow(difference, 2);
    }
    return sqrt(sum);
}

double calc_normalised_distance(data_point x1, data_point x2, data_point min, data_point max)
{
    if (x1.rows() != x2.rows())
    {
        std::cerr << "Error when calculating Normalised distance between data points (Clustering.hpp), " << std::endl
                  << "data points are not in same vector space, exiting..." << std::endl;
        exit(1);
    }

    for (unsigned int i = 0; i < x1.rows(); i++)
    { // for each dimension in the data point - normalise the distance
        x1(i) = (x1(i)-min(i))/(max(i)-min(i));
        x2(i) = (x2(i)-min(i))/(max(i)-min(i));
    }

    return calc_euclidian_distance(x1, x2); // distances are now normalised, euclidian distance can be calculated
}

struct dist_function
{ // this structure will be able to carry and calculate a specific distance function across to other structure
    data_point m_min, m_max; // for normalised distnce
    enum {Euclidian, Normalised} m_tag;
    double calc(data_point x1, data_point x2)
    {
        switch (m_tag)
        { // defines how the distance is calculated
        case dist_function::Euclidian: // this will invoke the euclidian distance function
            return calc_euclidian_distance(x1, x2); // oncly 2 data points require
            break;
        case dist_function::Normalised: // this will invoke the normalised distance function
            return calc_normalised_distance(x1, x2, m_min, m_max); // also the minimum and maxima are required in comparison with euclidian distance
            break;
        default:
            std::cerr << "Did not define a tag in dist_function (Clustering.hpp), exiting now..." << std::endl;
            exit(1);
        }
    }

    dist_function()
    {
        m_tag = Euclidian;
    }
};

double calc_variance(std::vector<data_point> x,dist_function d_func)
{
    if (x.size() == 0)
    {
        std::cerr << "WARNING! calculated variance of an empty set of data! (returned 0.0)" << std::endl;
        return 0.0;
    }
    data_point mean = Eigen::VectorXd(x[0].rows());
    mean.setZero();

    for (unsigned int i = 0; i < x.size(); i++)
    { // for each data point
        mean += x[i]; // add data point's contribution to the mean
    }
    mean /= (double)x.size(); // divide by the number of data points to compute the mean

    double m_variance = 0;

    for (unsigned int i = 0; i < x.size(); i++)
    {
        m_variance += pow(d_func.calc(mean, x[i]), 2);
    }

    m_variance /= x.size();

    return m_variance;
} // calc_variance()

unsigned int find_closest_to(std::vector<data_point> x, data_point O, dist_function d_func)
{ // finds the point in the set 'x' that is closest to point 'O'
    double min = d_func.calc(x[0], O);
    unsigned int min_index = 0;
    for (unsigned int i = 1; i < x.size(); i++)
    {
        double dist = d_func.calc(x[i], O);
        if (dist < min)
        {
            min_index = i;
            min = dist;
        }
    }

    return min_index;
}

unsigned int find_furthest_from(std::vector<data_point> x, data_point O, dist_function d_func)
{ // finds the point in the set 'x' that is closest to point 'O'
    double max = d_func.calc(x[0], O);
    unsigned int max_index = 0;
    for (unsigned int i = 1; i < x.size(); i++)
    {
        double dist = d_func.calc(x[i], O);
        if (dist > max)
        {
            max_index = i;
            max = dist;
        }
    }

    return max_index;
} // find_furthest_from()


std::map<int, data_point>::iterator find_closest_to(std::map<int, data_point>& x, data_point O, dist_function d_func)
{ // finds the point in the set 'x' that is closest to point 'O'
    std::map<int, data_point>::iterator it = x.begin();
    std::map<int, data_point>::iterator min_it = x.begin();
    double min = d_func.calc(it->second, O);
    it++;
    for (; it != x.end(); it++)
    {
        double dist = d_func.calc(it->second, O);
        if (dist < min)
        {
            min_it = it;
            min = dist;
        }
    }

    return min_it;
}

} // namespace BSO
#endif // BSO_DATA_HPP
