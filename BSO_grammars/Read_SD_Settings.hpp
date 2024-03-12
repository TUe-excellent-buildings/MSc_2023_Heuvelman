#ifndef READ_SD_SETTINGS_HPP
#define READ_SD_SETTINGS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include <boost/algorithm/string.hpp>

#include <BSO/Trim_And_Cast.hpp>


namespace BSO { namespace Structural_Design{

void read_SD_settings(std::string input_file, Structural_Design::SD_Analysis_Vars* SD)
{
    std::fstream input(input_file.c_str()); // open a file stream
    if (!input)
    {
        std::cerr << "Error, could not open file "
                  << "\"" << input_file << "\""
                  << ", exiting..." << std::endl;
        exit(1);
    }
    std::string line; // initialise a string to hold lines being read from file
    boost::char_separator<char> sep(","); // defines what separates tokens in a string
    typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer
    char type_ID; // holds information about what type of information is described by the line currently read

    while(!input.eof())
    {
        getline(input, line); // get the next line from the file
        boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
        if (line == "") //skip empty lines (tokenizer does not like it)
        {
            continue; // continue to next line
        }
        t_tokenizer tok(line, sep); // tokenize the line
        t_tokenizer::iterator token = tok.begin(); // set iterator to first token
        type_ID = trim_and_cast_char(*token); // interpret first token as type ID

        switch (type_ID)
        {
        case 'A':
        { // number of divisions to be made by meshing
            token++;
            SD->m_mesh_division = trim_and_cast_uint(*token);
            break;
        }
        case 'B':
        { // defines an abstract loading
            Abstract_Load temp_load;
            token++; // load_ID
            unsigned int load_ID = trim_and_cast_uint(*token);
            token++; // load case
            temp_load.m_lc = trim_and_cast_uint(*token);
            token++; // load magnitude [N/mm]
            temp_load.m_magnitude = trim_and_cast_double(*token);
            token++; // azimuth [°]
            temp_load.m_azimuth = trim_and_cast_double(*token);
            token++; // altitude [°]
            temp_load.m_altitude = trim_and_cast_double(*token);
            token++; // type (string)
            temp_load.m_type = *token;
            boost::algorithm::trim(temp_load.m_type);

            temp_load.calc_direction();

            SD->m_abstract_loads[load_ID] = temp_load;

            break;
        }
        case 'C':
        { // truss element properties (ID, surface area, younbgs modulus)
            Truss_Props temp;
            token++;
            temp.m_ID = *token;
            boost::algorithm::trim(temp.m_ID);
            token++;
            temp.m_A = trim_and_cast_double(*token);
            token++;
            temp.m_E = trim_and_cast_double(*token);

            SD->m_truss_props.push_back(temp);
            break;
        }
        case 'D':
        { // beam element properties (ID, width, height, youngs modulus, poissons ratio)
            Beam_Props temp;
            token++;
            temp.m_ID = *token;
            boost::algorithm::trim(temp.m_ID);
            token++;
            temp.m_b = trim_and_cast_double(*token);
            token++;
            temp.m_h = trim_and_cast_double(*token);
            token++;
            temp.m_E = trim_and_cast_double(*token);
            token++;
            temp.m_v = trim_and_cast_double(*token);

            SD->m_beam_props.push_back(temp);
            break;
        }
        case 'E':
        { // flat shell element properties (ID, thickness, youngs modulus, poissons ratio)
            Flat_Shell_Props temp;
            token++;
            temp.m_ID = *token;
            boost::algorithm::trim(temp.m_ID);
            token++;
            temp.m_t = trim_and_cast_double(*token);
            token++;
            temp.m_E = trim_and_cast_double(*token);
            token++;
            temp.m_v = trim_and_cast_double(*token);

            SD->m_flat_shell_props.push_back(temp);
            break;
        }
        case 'F':
        { // flat shell element properties (ID, thickness, youngs modulus, poissons ratio)
            Flat_Shell_Props temp;
            token++;
            temp.m_ID = *token;
            boost::algorithm::trim(temp.m_ID);
            token++;
            temp.m_t = trim_and_cast_double(*token);
            token++;
            temp.m_E = trim_and_cast_double(*token);
            token++;
            temp.m_v = trim_and_cast_double(*token);

            SD->m_ghost_flat_shell_props.push_back(temp);
            break;
        }
        case 'G':
        { // volume element properties

            break;
        }
        case 'H':
        { // define a node (requires a node ID and the x- y- and z- cooridnates

            break;
        }
        case 'I':
        { // define a truss (needs 2 nodes)

            break;
        }
        case 'J':
        { // define a beam (needs 2 nodes)

            break;
        }
        case 'K':
        { // define a flat shell (needs 4 nodes)

            break;
        }
        case 'L':
        { // define a volume element (needs 8 nodes)

            break;
        }
        case 'M':
        { // define a constraint on a line/node or surface

            break;
        }
        case 'N':
        { // define a load on a node/line or surface

            break;
        }
        case 'O':
        { // define a released dof on a node/line or surface (e.g. hinge, or roll)

            break;
        }
        default:
            break;
        }
    } // end of while loop (i.e. end of file)

} // read_SD_settings()


} // namespace Structural_Design
} // namespace BSO

#endif // READ_SD_SETTINGS_HPP
