#ifndef PARETO_SELECTION_HPP
#define PARETO_SELECTION_HPP

#include <BSO/Spatial_Design/Movable_Sizable.hpp>
#include <BSO/Spatial_Design/Supercube.hpp>
#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Data.hpp>

#include <Eigen/Dense>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <map>

namespace BSO {

data_point find_utopia_point(std::map<int, data_point> pareto_front);

enum class p_front_selection
{
    KNEE_POINT,
    CLOSEST_TO_ZERO
};

std::vector<Spatial_Design::MS_Building> pareto_selection(std::string file_name, unsigned int n_disciplines, p_front_selection select_option)
{
    std::vector<Spatial_Design::MS_Building> selected_designs;
    std::map<int, data_point> SC_output;
    std::map<int, data_point>::iterator SC_it;

    // start reading in the pareto front data points
    std::ifstream input(file_name.c_str());

    if (!input.is_open())
    {
        std::cout << "Could not find input: " << file_name << " while reading SC output for conversion to MS input, exiting now." << std::endl;
        exit(1);
    }

    unsigned int file_ptr;
    std::string line; // will hold the next line in the file
    boost::char_separator<char> sep("\t; "); // defines what separates tokens in a string
    typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer

    while(!input.eof())
    { // read in all the data points
        file_ptr = input.tellg(); // safes the location of this line
        getline(input, line); // get the next line from the file
        boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
        if (line == "") //skip empty lines (tokenizer does not like it)
        {
            continue; // continue to next line
        }

        t_tokenizer tok(line, sep); // tokenize the line
        t_tokenizer::iterator token = tok.begin(); // set iterator to first token

        // initialise the data point
        data_point d_point = Eigen::VectorXd(n_disciplines);
        token++; // skip the stamp

        for (unsigned int i = 0; i < n_disciplines; i++)
        { // for each disciplinary performance
            d_point[i] = trim_and_cast_double(*token);
            token++;
        }

        SC_output[file_ptr] = d_point;
    }

    // initialise what distance function is used to select the design
    dist_function d_func; // default euclidian;

    // find the desired point in the pareto front
    data_point closest_to_this;
    if (select_option == p_front_selection::KNEE_POINT)
    { // if the knee point should be selected
        closest_to_this = find_utopia_point(SC_output); // this is the utopia point
    }
    else if (select_option == p_front_selection::CLOSEST_TO_ZERO)
    { // if the point closest to the origin should be selected
        closest_to_this = Eigen::VectorXd::Zero(n_disciplines); // this is the origin
    }
    else
    { // if no selection method has been selected
        std::cerr << "ERROR in Pareto_Selection.hpp: no selection method found, exiting now" << std::endl;
        exit(1);
    }


    unsigned int selected_point_ptr = BSO::find_closest_to(SC_output, closest_to_this, d_func)->first;

    // retrieve the design that has been selected from the pareto front from "file_name"
    input.close();
    input = std::ifstream(file_name.c_str());
    input.seekg(selected_point_ptr);
    getline(input, line);

    // remove the data that is not needed from the string
    boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
    unsigned int split_index = 0;
    unsigned int delim_count = 0;


    for (unsigned int i = 0; i < line.size(); i++)
    { // for the first few elements: stamp + objective evaluations (n_disciplines + 1)
        if (line[i] == ' ' || line[i] == '\t')
        {
            split_index = i+1;
            delim_count++;
        }
        if (delim_count == n_disciplines + 1)
        {
            break;
        }
    }

    line.erase(line.begin(), line.begin()+split_index);
    input.close();

    // initialise the design that has been selected into MS format
    BSO::Spatial_Design::SC_Building SC(line,1);
    selected_designs.push_back(BSO::Spatial_Design::MS_Building(SC));

    return selected_designs;
} // pareto_selection()

data_point find_utopia_point(std::map<int, BSO::data_point> pareto_front)
{
    std::map<int, BSO::data_point>::iterator it = pareto_front.begin();
    BSO::data_point utopia_point = it->second;
    it++;

    for (; it != pareto_front.end(); it++)
    { // for each data_point in the pareto front
        for (unsigned int i = 0; i < it->second.rows(); i++)
        { // and for each dimension of the data_point
            if (it->second[i] < utopia_point[i])
            { // if the value of the considered dimension 'i' in the considered data point '*it' is smaller than that of the current utopia point
                utopia_point[i] = it->second[i]; // assign the smaller value to dimension 'i' of the utopia point
            }
        }
    }

    return utopia_point; // return the found utopia point
} // find_utopia_point()

}// namespace BSO
#endif // PARETO_SELECTION_HPP
