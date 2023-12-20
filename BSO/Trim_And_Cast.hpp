#ifndef TRIM_AND_CAST_HPP
#define TRIM_AND_CAST_HPP

#include <string>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

namespace BSO
{

    /*
     * The functions in this file are used to convert data from files
     * e.g. string to in, string to double, string to char
     */


    // Function declarations

    int trim_and_cast_int(std::string); // trims all white space at the end and beginning of a string, and the converts it to an integer
    double trim_and_cast_double(std::string); // trims all white space at the end and beginning of a string, and the converts it to a double
    unsigned long trim_and_cast_ulong(std::string); // trims all white space at the end and beginning of a string, and the converts it to an unsigned long
    char trim_and_cast_char(std::string); // trims all white space at the end and beginning of a string, and the converts it to an integer


    //Function implementations:

    int trim_and_cast_int(std::string s) // trims all white space at the end and beginning of a string, and the converts it to an integer
    {
        boost::algorithm::trim(s); // trims white space at beginning and end of a string
        return boost::lexical_cast<int>(s); // casts a string as an int
    } // trim_and_cast_int()

    unsigned int trim_and_cast_uint(std::string s) // trims all white space at the end and beginning of a string, and the converts it to an unsigned integer
    {
        boost::algorithm::trim(s); // trims white space at beginning and end of a string
        return boost::lexical_cast<unsigned int>(s); // casts a string as an int
    } // trim_and_cast_int()

    unsigned long trim_and_cast_ulong(std::string s) // trims all white space at the end and beginning of a string, and the converts it to an unsigned long
    {
        boost::algorithm::trim(s); // trims white space at beginning and end of a string
        return boost::lexical_cast<unsigned long>(s); // casts a string as an int
    } // trim_and_cast_int()

    double trim_and_cast_double(std::string s) // trims all white space at the end and beginning of a string, and the converts it to a double
    {
        boost::algorithm::trim(s); // trims white space at beginning and end of a string
        return boost::lexical_cast<double>(s); // casts a string as a double
    } // trim_and_cast_double()

    char trim_and_cast_char(std::string s) // trims all white space at the end and beginning of a string, and the converts it to an integer
    {
        boost::algorithm::trim(s); // trims white space at beginning and end of a string
        return s.at(0); // casts a string as a char
    } // trim_and_cast_char()

} // namespace BSO


#endif // TRIM_AND_CAST_HPP

