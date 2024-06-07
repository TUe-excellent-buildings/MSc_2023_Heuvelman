#ifndef READ_ZONING_SETTINGS_HPP
#define READ_ZONING_SETTINGS_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>
#include <BSO/Trim_And_Cast.hpp>

namespace BSO { namespace Grammar {

struct Zoning_Settings
{
	unsigned int max_span;
	unsigned int min_span;
	bool whole_space_zones;
	bool delete_expanded_designs;
	bool zone_floors;
	bool adaptive_thickness;
	
	bool unzoned;
}; // struct Zoning_Settings

Zoning_Settings read_zoning_settings(std::string input_file)
{
    Zoning_Settings zoning_settings;

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
        { // Span settings
		token++; // maximum span
            zoning_settings.max_span = trim_and_cast_uint(*token);
		token++; // minimum span
            zoning_settings.min_span = trim_and_cast_uint(*token);
            break;
        }
        case 'B':
        { // Solution space settings
			token++; // large?
			if (trim_and_cast_char(*token) == 'Y')
				zoning_settings.delete_expanded_designs = false;
			else
				zoning_settings.delete_expanded_designs = true;
		token++; // whole-space zones only?
			if (trim_and_cast_char(*token) == 'Y')
				zoning_settings.whole_space_zones = true;
			else
				zoning_settings.whole_space_zones = false;
            break;
        }
        case 'C':
        { // Alternative grammar settings
			token++; // structural floors?
			if (trim_and_cast_char(*token) == 'Y')
				zoning_settings.zone_floors = true;
			else
				zoning_settings.zone_floors = false;
		token++; // adaptive thickness?
			if (trim_and_cast_char(*token) == 'Y')
				zoning_settings.adaptive_thickness = true;
			else
				zoning_settings.adaptive_thickness = false;
            break;
        }
        case 'D':
        { // Check the unzoned design
			token++;
			if (trim_and_cast_char(*token) == 'Y')
				zoning_settings.unzoned = true;
			else
				zoning_settings.unzoned = false;
            break;
        }
        default:
        { // do nothing, it is probably a comment or something similar
            break;
        }
        } // end of switch statement
    } // end of while statement (read file)

    return zoning_settings;
} // read_zoning_settings()


} // namespace Grammar
} // namespace BSO

#endif // READ_ZONING_SETTINGS_HPP
