#ifndef READ_STABILIZE_SETTINGS_HPP
#define READ_STABILIZE_SETTINGS_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <BSO/Trim_And_Cast.hpp>

namespace BSO { namespace Grammar {

struct Stabilize_Settings
{
	unsigned int method;
	double singular;
	unsigned int point_it_unzoned;
	unsigned int zone_it;
	unsigned int point_it_zoned;
	bool delete_superfluous_trusses;
}; // struct Stabilize_Settings

Stabilize_Settings read_stabilize_settings(std::string input_file)
{
    Stabilize_Settings stabilize_settings;

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
        { // Method settings
            std::string temp_str;
            token++;
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            if (temp_str == "Unzoned")
				stabilize_settings.method = 0;
            else if (temp_str == "Partially_Zoned")
				stabilize_settings.method = 1;
            else if (temp_str == "Fully_Zoned")
				stabilize_settings.method = 2;
            break;
        }
        case 'B':
        { // SVD settings
            token++;
            stabilize_settings.singular = trim_and_cast_double(*token);
            break;
        }
        case 'C':
        { // Point iteration unzoned
            token++;
            stabilize_settings.point_it_unzoned = trim_and_cast_uint(*token);
            break;
        }
        case 'D':
        { // Zone iteration
            token++;
            stabilize_settings.zone_it = trim_and_cast_uint(*token);
            break;
        }
        case 'E':
        { // Point iteration zoned
            token++;
            stabilize_settings.point_it_zoned = trim_and_cast_uint(*token);
            break;
        }
        case 'F':
        { // Method settings
			token++; // delete superfluous trusses?
			if (trim_and_cast_char(*token) == 'Y')
				stabilize_settings.delete_superfluous_trusses = true;
			else
				stabilize_settings.delete_superfluous_trusses = false;
            break;
        }
        default:
        { // do nothing, it is probably a comment or something similar
            break;
        }
        } // end of switch statement
    } // end of while statement (read file)

    return stabilize_settings;
} // read_stabilize_settings()


} // namespace Grammar
} // namespace BSO

#endif // READ_STABILIZE_SETTINGS_HPP
