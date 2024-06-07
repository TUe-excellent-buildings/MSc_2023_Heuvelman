#ifndef BSO_CLUSTERING_HPP
#define BSO_CLUSTERING_HPP

#include <BSO/Data.hpp>

#include <random>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <Eigen/Dense>

namespace BSO {
    // This file contains implementations of clustering algorithms



struct Cluster
{
    std::weak_ptr<std::vector<data_point> > m_data_set;
    data_point m_centroid;
    data_point m_mean;
    unsigned int m_size; // the number of data points currently in the cluster
    double m_variance; // the variance of all data points in this cluster with respect their centroid
    dist_function m_d_func;
    double m_dist_from_origin;
    std::vector<bool> m_bit_mask;

    Cluster (data_point p, std::weak_ptr<std::vector<data_point> > data_reference)
    {
        m_centroid = p;
        m_data_set = data_reference;
        if (auto data_set = data_reference.lock())
        {
            m_bit_mask.resize(data_set->size());
        }

    }
};

void calc_variances(std::vector<std::shared_ptr<Cluster> > x_k, dist_function d_func)
{   // calculate the variance of a clustered data set
    // dist_function is the function that determines the distances in between data points to determine the variance e.g. Euclidian

    for (unsigned int i = 0; i < x_k.size(); i++)
    { // for each cluster
        x_k[i]->m_variance = 0; // initialise cluster's variance

        data_point origin; // make a data point to resemble the origin

        if (auto data_set = x_k[i]->m_data_set.lock())
        {
            origin = data_set->front(); // initialise the origin an existing data point (for size initialisation)
            origin.setZero(); // set all values in the data point to zero

            for (unsigned int j = 0; j < data_set->size(); j++)
            { // and for each data point in the data set
                if (x_k[i]->m_bit_mask[j])
                { // if the data point belongs to the cluster
                    x_k[i]->m_variance += pow(d_func.calc(x_k[i]->m_mean, (*data_set)[j]), 2); // add the squared distance of data point j to the variance of cluster i
                }
            }
        }
        else
        {
            std::cout << "Cluster lost link to data_set (Clustering.hpp), exiting now..." << std::endl;
            exit(1);
        }

        x_k[i]->m_variance /= (double)x_k[i]->m_size;
        x_k[i]->m_dist_from_origin = d_func.calc(origin, x_k[i]->m_centroid);

    }
} // calc_variance()



std::vector<std::shared_ptr<Cluster> > k_means(std::shared_ptr<std::vector<data_point> > x, unsigned int k, dist_function d_func)
{
	// initialise the random number generator
	std::mt19937 rand_num_engine;
	rand_num_engine.seed(std::random_device()());

    // initialise centroids to random points inside the data set (i.e. select k random data_points to serve as centroids)
    std::vector<std::shared_ptr<Cluster> > x_k; // will contain all clusters

    if (k > x->size())
    {
        std::cout << "Error in k-means clustering (Clustering.hpp) number of clusters is larger than " << std::endl
                  << "the number of data points in the given data set, exiting now..." << std::endl;
        exit(1);
    }
    else if (k > x->size()/2.0)
    {
        std::vector<unsigned int> rand_array(x->size());

        for (unsigned int i = 0; i < x->size(); i++)
        { // initialise an array with the index numbers of the data set
            rand_array[i] = i;
        }
        std::shuffle(rand_array.begin(), rand_array.end(), rand_num_engine); // shuffle the indices in a random manner

        for (unsigned int i = 0; i < k; i++)
        { // the first k numbers in the shuffled list will be assigned to each clusters centroid
            x_k.push_back(std::make_shared<Cluster>((*x)[rand_array[i] ], std::weak_ptr<std::vector<data_point> >(x)));
        }
    }
    else
    { // Fisher-Yates shuffle
		std::vector<unsigned int> blacklist;
        for (unsigned int i = 0; i < k; i++)
        { // for each cluster
            unsigned int r; // will contain a (unique) random number
            do
            {
                std::uniform_int_distribution<std::mt19937::result_type> distribution(0, x->size()-1);
				r = distribution(rand_num_engine);
            }while (std::find(blacklist.begin(), blacklist.end(), r) != blacklist.end()); // continue seeding a random number untill a new unique random number is found

            blacklist.push_back(r); // if the random number is unique put it in the black list
            x_k.push_back(std::make_shared<Cluster>((*x)[r], std::weak_ptr<std::vector<data_point> >(x))); // assigned the data point with index r as a centroid to cluster k
            x_k.back()->m_d_func = d_func; // stores what dist-function was use to create this cluster
        }
    }

    bool convergence = false; // check to see if the algorithm has converged on clustering or not
    while (!convergence)
    {
        convergence = true; // set to true, and put back to false as soon a data point switches cluster

        // initialize bit masks and sizes of each cluster
        for (unsigned int i = 0; i < k; i++)
        { // for each cluster
            x_k[i]->m_size = 0; // set the variable that keeps track of the cluster size to zero
            x_k[i]->m_mean.setZero(); // set the mean of this cluster to a zero vector
            for (unsigned int j = 0; j < x->size(); j++)
            { // set the bit mask to false for each data point
                if (x_k[i]->m_bit_mask[j])
                    x_k[i]->m_bit_mask[j] = false;
            }
        }

        // assign each data point to a cluster based on its distance to each cluster
        double min_dist;
        for (unsigned int i = 0; i < x->size(); i ++)
        { // for each data point
            unsigned int closest_cluster = 0; // will hold the index to the closest cluster
            min_dist = d_func.calc((*x)[i], x_k[0]->m_centroid); // initialise with the distance to the first centroid

            for (unsigned int j = 1; j < x_k.size(); j++)
            { // for the remaining clusters
                double distance = d_func.calc((*x)[i], x_k[j]->m_centroid); // find the distance of the centroid of cluster j to data point i
                // check if the distance to centroid j is smaller
                if (min_dist > distance)
                {
                    min_dist = distance; // if it is, then this is now the smallest distance
                    closest_cluster = j; // and cluster j contains the centroid closest to data point i
                }
            } // at the end of this loop the cluster with the centroid closest to data point i is known

            x_k[closest_cluster]->m_bit_mask[i] = true; // update bit_mask index i in the cluster with its centroid closest to data point i
            x_k[closest_cluster]->m_size++; // increment the size of this cluster
            x_k[closest_cluster]->m_mean += (*x)[i]; // add this data point to the summation of the mean
        }

        for (unsigned int i = 0; i < x_k.size(); i++)
        {
            // check if a cluster is empty
            if (x_k[i]->m_size == 0)
            { // if this cluster is empty, then it will be filled with the furthest point of the largest cluster
                // initiation of trackers
                unsigned int largest_cluster_size = 0;
                unsigned int largest_cluster_index = 0;
                for (unsigned int j = 0; j < x_k.size(); j++)
                { // for all other clusters

                    if ((i != j) && (x_k[j]->m_size > largest_cluster_size))
                    { // if the size of this cluster if larger that the currently largest found cluster
                        // update the trackers for the larget cluster
                        largest_cluster_size = x_k[j]->m_size;
                        largest_cluster_index = j;
                    }
                    else
                    { // if not just keep searching for large clusters
                        continue;
                    }
                } // found the index to the largest cluster

                // now need to find the furthest point away in the largest cluster
                // to find that, first the new centroid (i.e. new mean) of this largest cluster needs to be calculated
                data_point largest_cluster_mean = x_k[largest_cluster_index]->m_mean / x_k[largest_cluster_index]->m_size; // calculate the mean

                // now the point furthest away from this mean is searched
                // initiation of trackers
                double furthest_point_distance = 0;
                unsigned int furthest_point_index = 0;

                for (unsigned int j = 0; j < x->size(); j++)
                { // for all data points in the data set
                    if (x_k[largest_cluster_index]->m_bit_mask[j])
                    { // if point j belongs to the largest cluster
                        if (furthest_point_distance < d_func.calc((*x)[j], largest_cluster_mean))
                        { // and if the distance of point j to the mean of the largest cluster is larger than the current furthest distance
                            // update the trackers for the furthest away point
                            furthest_point_distance = d_func.calc((*x)[j], largest_cluster_mean);
                            furthest_point_index = j;
                        }
                    }
                } // found the furthest away point in the largest cluster

                // remove this point from the largest cluster
                x_k[largest_cluster_index]->m_mean -= (*x)[furthest_point_index];
                x_k[largest_cluster_index]->m_size--;
                x_k[largest_cluster_index]->m_bit_mask[furthest_point_index] = false;

                // add the found point to the empty cluster
                x_k[i]->m_mean += (*x)[furthest_point_index];
                x_k[i]->m_size++;
                x_k[i]->m_bit_mask[furthest_point_index] = true;

            }
            else
            { // if this cluster is not empty then continue checking the rest of the clusters
                continue;
            }
        }

        // calculate new centroids
        for (unsigned int i = 0; i < x_k.size(); i++)
        {
            x_k[i]->m_mean /= x_k[i]->m_size; // calculate the mean

            // check if the centroid changed position
            if(x_k[i]->m_centroid != x_k[i]->m_mean)
            {
                convergence = false;
                x_k[i]->m_centroid = x_k[i]->m_mean; // assign this as the new centroid
            } // else the centroid of this cluster did not change
        } // if the centroid of none of the clusters has changed then convergence remains true
    } // convergence: true --> clustering algorithm has found a local minimum for the centroid positions

    // calculate variances
    calc_variances(x_k, d_func);
    return x_k; // return the found cluster
} // end of k_means()

std::vector<std::shared_ptr<Cluster> > clustering(std::shared_ptr<std::vector<data_point> > x, unsigned int runs, unsigned int k_min, unsigned int k_max, int dist_switch)
{ // this function tries to find the best clustering for the data set x, using a specified number of runs, and  using between k_min and k_max clusters

    if (k_min <= 1)
    {
        std::cout << "Error in function clustering() (Clustering.hpp), k_min must be larger than 1, exiting..." << std::endl;
        exit(1);
    }
    else if (k_max < k_min)
    {
        std::cout << "Error in function clustering() (Clustering.hpp), k_max must be larger than k_min, exiting..." << std::endl;
        exit(1);
    }
    else if (k_max >= x->size())
    {
        std::cout << "Error in function clustering() (Clustering.hpp), k_max must be lesser than the number of data points in the data set, exiting..." << std::endl;
        exit(1);
    }

    std::map<unsigned int, std::vector<std::shared_ptr<Cluster> > > clustered_data_sets; // will contain the best cluster for each cluster size
    dist_function d_func;

    switch (dist_switch)
    {
    case 1: // this is for the Euclidian distance between points
        d_func.m_tag = dist_function::Euclidian; // tags the dist_function to calculate the Euclidian distance between data points
        break;
    case 2: //
        d_func.m_tag = dist_function::Normalised; // tags the dist_function to calculate a normalised distance between data points
        d_func.m_max = (*x)[0]; // intialises the maximum dimension values to the first data point
        d_func.m_min = (*x)[0]; // intialises the minimum dimension values to the first data point
        for (unsigned int i = 1; i < x->size(); i++)
        { // for each remaining data point
            for (unsigned int j = 0; j < d_func.m_max.rows(); j++)
            { // and for each dimension
                if (d_func.m_max(j) < (*x)[i](j)) d_func.m_max(j) = (*x)[i](j);
                if (d_func.m_min(j) > (*x)[i](j)) d_func.m_min(j) = (*x)[i](j);
            }
        }
        break;
    default:
        std::cout << "No selection made of which distance should be used for clustering (Clustering.hpp), exiting now..." << std::endl;
        exit(1);
        break;
    }

    std::map<unsigned int, double> cluster_variances;

    for (unsigned int i = k_min-1; i <= k_max+1; i++)
    { // for each cluster size and 1 less and 1 more cluster
        double min_variance = 0; // to keep track of which cluster contains the minimal variance
        clustered_data_sets[i] = k_means(x, i, d_func);

        for (unsigned int j = 1; j < runs; j++)
        { // run the k_means algorithm 'runs' times

            std::vector<std::shared_ptr<Cluster> > temp_clustered_data_set; // will contain the clusters for each cluster size
            temp_clustered_data_set = k_means(x, i, d_func); // run the k_means algorithm and add it to the temporary set of clusters
            double variance = 0;

            for (unsigned int k = 0; k < i; k++)
            {
                variance += temp_clustered_data_set[k]->m_variance;
            }

            if (min_variance == 0)
            {
                clustered_data_sets[i] = temp_clustered_data_set;
                min_variance = variance;
            }
            else if (min_variance > variance)
            {
                clustered_data_sets[i] = temp_clustered_data_set;
                min_variance = variance;
            }
        }

    }

    // select the cluster for which the second derivative of the variances is largest (largest curvature equals maximum effect loss of clustering algorithm)

    // therefore first the angles are computed of the lines in the graph that results when the variance is plotted againt the number of clusters
    // angles here are calculated as the cosine of the angle i.e. with vector formula (v.w)/(|v|*|w|)
    std::map<unsigned int, double> cluster_effect_angles;
    for (unsigned int i = k_min; i <= k_max; i++)
    { // for each cluster size
        cluster_effect_angles[i] = -2*cluster_variances[i] + cluster_variances[i-1] + cluster_variances[i+1];


        /*// compute the vector from the previous point to point i
        Eigen::Vector2d prev_point;
        prev_point << 1, (double)(cluster_variances[i] - cluster_variances[i-1]);

        //compute the vector from i to next point
        Eigen::Vector2d next_point;
        next_point << 1, (cluster_variances[i+1] - cluster_variances[i]);

        // calculate the angle
        cluster_effect_angles[i] = prev_point.dot(next_point) / (prev_point.norm() * next_point.norm());*/

    }

    // accordingly the maximum angle is sought (i.e. minimum of the cosines of the angles that are calculated above)
    double max_cluster_effect_angle = 0;
    unsigned int best_k = 0;
    for (unsigned int i = k_min; i <= k_max; i++)
    { // for each number of clusters
        if (max_cluster_effect_angle == 0)
        { // check if the tracker has been updated after initiation
            max_cluster_effect_angle = cluster_effect_angles[i];
            best_k = i;
        }
        else if (max_cluster_effect_angle < cluster_effect_angles[i])
        {
            max_cluster_effect_angle = cluster_effect_angles[i];
            best_k = i;
        }
    }

    return clustered_data_sets[best_k];
}

}

#endif // BSO_CLUSTERING_HPP
