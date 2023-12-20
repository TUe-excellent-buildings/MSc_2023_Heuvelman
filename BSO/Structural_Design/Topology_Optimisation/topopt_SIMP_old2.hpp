#ifndef TOPOPT_SIMP_OLD2_HPP
#define TOPOPT_SIMP_OLD2_HPP

#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Structural_Design/Analysis_Tools/FEA.hpp>
#include <BSO/Vectors.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <Eigen/Dense>
#include <Eigen/Sparse>

#include <vector>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace BSO { namespace Structural_Design {

    void topopt_SIMP_old2(FEA* fea_ptr, double f, double r_min, double penal, double x_move, double tol)
    {
        unsigned int num_el = fea_ptr->get_element_count();

        // make a map between node ID's and pointers
        std::map<int, unsigned long> node_map;

        // read file with outputs from Juan's file
        std::string file_name = "Densities.txt";
        std::ifstream input(file_name.c_str()); // initialize input stream from file: file_name

        if (!input.is_open())
        {
            std::cout << "Error, could not find file: " << file_name << ". Exiting now..." << std::endl;
            exit(1);
        }

        std::string line;
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
        std::string type_ID; // holds information about what type of information is described by the line currently read

        while (!input.eof())
        {
            getline(input,line); // get next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }

            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            type_ID = *token; // interpret first token as type ID
            boost::algorithm::trim(type_ID);

            if (type_ID == "N")
            {   // this reads a node
                token++;
                int ID = BSO::trim_and_cast_int(*token);
                token++;
                int x_coord = BSO::trim_and_cast_int(*token);
                token++;
                int y_coord = BSO::trim_and_cast_int(*token);
                token++;
                int z_coord = BSO::trim_and_cast_int(*token);

                bool node_found = false;
                typedef std::map<unsigned long, Elements::Node*>::iterator node_ite;
                for (node_ite ite = fea_ptr->m_node_map.begin(); ite != fea_ptr->m_node_map.end(); ite++)
                {
                    Eigen::Vector3d node_coord = ite->second->get_coord();
                    if (round(node_coord(0)) != x_coord)
                        continue;
                    else if (round(node_coord(1)) != y_coord)
                        continue;
                    else if (round(node_coord(2)) != z_coord)
                        continue;
                    else
                    {
                        node_found = true;
                        node_map[ID] = ite->second->get_ID();
                        break;
                    }
                }

                if (!node_found)
                {
                    std::cout << line << std::endl;
                    std::cout << "Did not find node" << std::endl;
                    exit(1);
                }

            }
            else if (type_ID == "DE")
            {   // this reads an element and its density
                token++;
                int n1 = BSO::trim_and_cast_ulong(*token);
                token++;
                int n2 = BSO::trim_and_cast_ulong(*token);
                token++;
                int n3 = BSO::trim_and_cast_ulong(*token);
                token++;
                int n4 = BSO::trim_and_cast_ulong(*token);
                token++;
                double file_density = BSO::trim_and_cast_double(*token);

                std::vector<unsigned long> element_nodes;
                element_nodes.push_back(node_map[n1]);
                element_nodes.push_back(node_map[n2]);
                element_nodes.push_back(node_map[n3]);
                element_nodes.push_back(node_map[n4]);

                bool element_found = false;

                for (unsigned int i = 0; i < fea_ptr->m_elements.size(); i++)
                {
                    if (std::find(element_nodes.begin(), element_nodes.end(), fea_ptr->m_elements[i]->get_node_ID(0)) == element_nodes.end())
                        continue;
                    else if (std::find(element_nodes.begin(), element_nodes.end(), fea_ptr->m_elements[i]->get_node_ID(1)) == element_nodes.end())
                        continue;
                    else if (std::find(element_nodes.begin(), element_nodes.end(), fea_ptr->m_elements[i]->get_node_ID(2)) == element_nodes.end())
                        continue;
                    else if (std::find(element_nodes.begin(), element_nodes.end(), fea_ptr->m_elements[i]->get_node_ID(3)) == element_nodes.end())
                        continue;
                    else
                    {
                        fea_ptr->m_elements[i]->update_density_old(file_density, 3);
                        element_found = true;
                        break;
                    }
                }

                if (!element_found)
                {
                    std::cout << "Did not find element" << std::endl;
                    exit(1);
                }


            }
        } // end of input from text file

        // FEA
        //fea_ptr->generate_GSM();
        //fea_ptr->solve();

    } // topopt_SIMP_old2()

} // namespace Structural_Design
} // namespace BSO



#endif // TOPOPT_SIMP_OLD2_HPP
