#ifndef READ_GRAMMAR_SETTINGS_HPP
#define READ_GRAMMAR_SETTINGS_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <boost/algorithm/string.hpp>

namespace BSO { namespace Grammar {

struct Rectangle_Setting
{
    std::string m_side_1; // indicates what space type is at side_1
    std::string m_side_2; // indicates what space type is at side_2
    std::string m_type; // optional, indicates what type of construction (e.g. glass or construction in BP require different property sets)
    std::string m_type_ID; // indicates the ID of the property set for that

    bool compare(const std::pair<std::string, std::string> rhs)
    { // comparison operator to compare with a pair of side ID's
        return ((m_side_1 == rhs.first && m_side_2 == rhs.second) ||
                (m_side_1 == rhs.second && m_side_2 == rhs.first));
    }
}; // struct Rectangle_Setting

struct Space_Setting
{
    std::string m_BP_space_type; // defines which space building physics space type it gets
    std::string m_type_ID; // identifier for space_type
};

struct Grammar_Settings
{
    std::vector<Rectangle_Setting> m_SD_rectangles_wall; // holds the conversion table for SD properties of rectangles in the conformal model
    std::vector<Rectangle_Setting> m_SD_rectangles_floor; // holds the conversion table for SD properties of rectangles in the conformal model
    std::vector<Rectangle_Setting> m_BP_rectangles_wall; // holds the conversion table for BP properties of rectangles in the conformal model
    std::vector<Rectangle_Setting> m_BP_rectangles_floor; // holds the conversion table for BP properties of rectangles in the conformal model
    std::vector<Space_Setting> m_BP_space_types;
}; // Grammar_settings

Grammar_Settings read_grammar_settings(std::string input_file)
{
    Grammar_Settings grammar_settings;

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
        { // SD Settings
            std::string temp_str;
            Rectangle_Setting temp_setting;
            token++; // next token: type_ID 1
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_1 = temp_str;

            token++; // next token: type_ID 2
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_2 = temp_str;

            token++; // next token: construction ID
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type = temp_str;

            token++; // next token: construction ID
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type_ID = temp_str;

            grammar_settings.m_SD_rectangles_wall.push_back(temp_setting);

            break;
        }
        case 'B':
        { // SD Settings
            std::string temp_str;
            Rectangle_Setting temp_setting;
            token++; // next token: type_ID 1
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_1 = temp_str;

            token++; // next token: type_ID 2
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_2 = temp_str;

            token++; // next token: construction ID
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type = temp_str;

            token++; // next token: construction ID
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type_ID = temp_str;

            grammar_settings.m_SD_rectangles_floor.push_back(temp_setting);

            break;
        }
        case 'C':
        {
            std::string temp_string;
            Space_Setting temp_setting;
            token++;// space_type_ID
            temp_string = *token;
            boost::algorithm::trim(temp_string);
            temp_setting.m_type_ID = temp_string;

            token++;// BP_Space_Set_ID
            temp_string = *token;
            boost::algorithm::trim(temp_string);
            temp_setting.m_BP_space_type = temp_string;

            grammar_settings.m_BP_space_types.push_back(temp_setting);
            break;
        }
        case 'D':
        { // BP settings
            std::string temp_str;
            Rectangle_Setting temp_setting;
            token++; // next token: type_ID 1
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_1 = temp_str;

            token++; // next token: type_ID 2
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_2 = temp_str;

            token++; // next token: construction type
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type = temp_str;

            token++; // next token: construction ID
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type_ID = temp_str;

            grammar_settings.m_BP_rectangles_wall.push_back(temp_setting);

            break;
        }
        case 'E':
        { // BP settings
            std::string temp_str;
            Rectangle_Setting temp_setting;
            token++; // next token: type_ID 1
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_1 = temp_str;

            token++; // next token: type_ID 2
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_side_2 = temp_str;

            token++; // next token: construction type
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type = temp_str;

            token++; // next token: construction ID
            temp_str = *token;
            boost::algorithm::trim(temp_str);
            temp_setting.m_type_ID = temp_str;

            grammar_settings.m_BP_rectangles_floor.push_back(temp_setting);

            break;
        }
        default:
        { // do nothing, it is probably a comment or something similar
            break;
        }
        } // end of switch statement
    } // end of while statement (read file)

    return grammar_settings;
} // read_grammar_settings()


} // namespace Grammar
} // namespace Grammar

#endif // READ_GRAMMAR_SETTINGS_HPP
