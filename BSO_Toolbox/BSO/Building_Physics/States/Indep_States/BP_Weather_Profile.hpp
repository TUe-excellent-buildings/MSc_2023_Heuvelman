#ifndef BP_WEATHER_PROFILE_HPP
#define BP_WEATHER_PROFILE_HPP

#include <BSO/Trim_And_Cast.hpp>
#include <BSO/Building_Physics/States/Indep_States/BP_Indep_State.hpp>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <string>
#include <map>
#include <iostream>
#include <cstdlib>
#include <fstream>


namespace BSO {
namespace Building_Physics {

/*
 * BP_Weather_Data is a structure to contain a weather data point
 */

struct BP_Weather_Data
{
    double m_temp;
};

/*
 * BP_Weather_Profile reads the temperature from a weather file at given simulation date and time
 */

 // Class definition:

class BP_Weather_Profile : public BP_Indep_State
{
private:
    std::string m_weather_file_location;
    std::map<boost::posix_time::ptime, BP_Weather_Data> m_weather_data; // vector containing pointers to data of weather during the simulation
	double* m_elapsed_time;

    std::string find_weather_file(const int& year);
    double m_temperature;
public:
    BP_Weather_Profile(BP_Simulation* system, std::string weather_file_location); // initialises the weather profile to the start date, defines end date and how many time steps per hour are used
    ~BP_Weather_Profile();

	void add_weather_data(boost::posix_time::ptime begin, boost::posix_time::ptime end);
    void update_sys(double t);
    bool is_weather_profile();
}; // BP_Weather_Profile








// Implementation of the member functions:

BP_Weather_Profile::BP_Weather_Profile(BP_Simulation* system, std::string weather_file_location) : BP_Indep_State(system)
{
    m_weather_file_location = weather_file_location;
} // ctor



BP_Weather_Profile::~BP_Weather_Profile()
{

} // dtor

void BP_Weather_Profile::add_weather_data(boost::posix_time::ptime begin, boost::posix_time::ptime end)
{
	int warm_up_days = m_system->m_warm_up_days;
	if (warm_up_days*24*3600 > (end - begin).total_seconds())
	{
		end = begin + boost::posix_time::hours(warm_up_days*24);
		end += boost::posix_time::hours(1.0);
	}
	else
	{
		end += boost::posix_time::hours(1.0);
	}
	begin -= boost::posix_time::hours(1.0);

	boost::posix_time::ptime current_time;
	while (current_time != end)
    {
        std::string file_name = m_weather_file_location + find_weather_file(begin.date().year()); // find the file that contains the given year
        boost::gregorian::date current_date;

        std::fstream input(file_name.c_str());
        std::string line;
        unsigned int file_position = 0;
        boost::char_separator<char> sep(",");
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer;
        for (unsigned int i = 0; i < 33; i++)
        { // for 33 times
            getline(input, line); // to skip the heading of the weather files
        }

        while (begin != current_time) // keep reading lines in the file until the correct line has been found, this is a time consuming approach (15 seconds if the starting date is the last file entry)
        {
            file_position = input.tellg(); // position before getline, because when it is the searched line the position must be stored
            getline(input, line); // get next line from the file
            t_tokenizer tok(line, sep);
            t_tokenizer::iterator token = tok.begin();

            token++; // skip first token: STN
            current_date = boost::gregorian::date(boost::gregorian::from_undelimited_string(*token) );token++; // date
            current_time = boost::posix_time::ptime(current_date, boost::posix_time::hours(BSO::trim_and_cast_int(*token) ) ); // add time
        }

        input.seekg(file_position);

        while (end > current_time)
        {
            getline(input, line); // get next line from the file
            boost::algorithm::trim(line);

            if (input.eof())
            { // if this is the end of the file
                begin = current_time + boost::posix_time::hours(1); // then create the next time point, that should be in the next file
                break; // escape the current while loop
            }

            if (line == "")
            { // if the line is empty
                continue; // continue to the next line
            }

            t_tokenizer tok(line, sep);
            t_tokenizer::iterator token = tok.begin();

            token++; // skip first token STN

            current_date = boost::gregorian::date(boost::gregorian::from_undelimited_string(*token) ); token++; // date
            current_time = boost::posix_time::ptime(current_date, boost::posix_time::hours(BSO::trim_and_cast_int(*token) ) ); token++; // add time

            BP_Weather_Data temp; // temporary data structure to hold the tokenized data

            token++; // skip DD = Mean wind direction (in degrees) during the 10-minute period preceding the time of observation (360=north, 90=east, 180=south, 270=west, 0=calm 990=variable)
            token++; // skip FH = Hourly mean wind speed (in 0.1 m/s)
            token++; // skip FF = Mean wind speed (in 0.1 m/s) during the 10-minute period preceding the time of observation
            token++; // skip FX = Maximum wind gust (in 0.1 m/s) during the hourly division
            temp.m_temp = trim_and_cast_double(*token)/10.0;token++; // temperature (in tenths of degrees Celsius)
            /*token++; // skip T10N = Minimum temperature (in 0.1 degrees Celsius) at 0.1 m in the preceding 6-hour period
            token++; // skip TD = Dew point temperature (in 0.1 degrees Celsius) at 1.50 m at the time of observation
            token++; // skip SQ = Sunshine duration (in 0.1 hour) during the hourly division, calculated from global radiation (-1 for <0.05 hour)
            token++; // skip Q = Global radiation (in J/cm2) during the hourly division
            token++; // skip DR = Precipitation duration (in 0.1 hour) during the hourly division
            token++; // skip RH = Hourly precipitation amount (in 0.1 mm) (-1 for <0.05 mm)
            token++; // skip P = Air pressure (in 0.1 hPa) reduced to mean sea level, at the time of observation
            token++; // skip VV = Horizontal visibility at the time of observation (0=less than 100m, 1=100-200m, 2=200-300m,..., 49=4900-5000m, 50=5-6km, 56=6-7km, 57=7-8km, ..., 79=29-30km, 80=30-35km, 81=35-40km,..., 89=more than 70km)
            token++; // skip N = Cloud cover (in octants), at the time of observation (9=sky invisible)
            token++; // skip U = Relative atmospheric humidity (in percents) at 1.50 m at the time of observation
            token++; // skip WW = Present weather code (00-99), description for the hourly division. See http://www.knmi.nl/klimatologie/achtergrondinformatie/ww_lijst_engels.pdf
            token++; // skip IX = Indicator present weather code (1=manned and recorded (using code from visual observations), 2,3=manned and omitted (no significant weather phenomenon to report, not available), 4=automatically recorded (using code from visual observations), 5,6=automatically omitted (no significant weather phenomenon to report, not available), 7=automatically set (using code from automated observations)
            token++; // skip M = Fog 0=no occurrence, 1=occurred during the preceding hour and/or at the time of observation
            token++; // skip R = Rainfall 0=no occurrence, 1=occurred during the preceding hour and/or at the time of observation
            token++; // skip S = Snow 0=no occurrence, 1=occurred during the preceding hour and/or at the time of observation
            token++; // skip O = Thunder  0=no occurrence, 1=occurred during the preceding hour and/or at the time of observation
            // skip final token: Y = Ice formation 0=no occurrence, 1=occurred during the preceding hour and/or at the time of observation*/

            m_weather_data[current_time] = temp;
        }
    }
} // add_weather_data()

std::string BP_Weather_Profile::find_weather_file(const int& year)
{
    int m_year = year-((year-1)%10);

    switch (m_year)
    {
    case 1971:
        return "uurgeg_260_1971-1980.txt";
        break;
	case 1981:
        return "uurgeg_260_1981-1990.txt";
        break;
    case 1991:
        return "uurgeg_260_1991-2000.txt";
        break;
    case 2001:
        return "uurgeg_260_2001-2010.txt";
        break;
    case 2011:
        return "uurgeg_260_2011-2020.txt";
        break;
    default:
        std::cerr << "Error in reading date for weather files! Exiting now..." << std::endl;
        exit(1);
        return "error";
    }
} // weather_file()



bool BP_Weather_Profile::is_weather_profile()
{
    return true;
} // is_weather_profile()

void BP_Weather_Profile::update_sys(double t)
{ // update the external temperature using the weather data
    boost::posix_time::ptime current_time = m_system->m_sim_begin + boost::posix_time::seconds(t); // get the current time from BP_Simulation
    boost::posix_time::ptime lower_bound  = boost::posix_time::ptime(current_time.date(), boost::posix_time::hours(current_time.time_of_day().hours())); // get the current or previous temperature data entry by rounding down to hours
    boost::posix_time::ptime upper_bound  = lower_bound + boost::posix_time::hours(1);  // get next entry temperature data (i.e. one hour later)

    double d_time_tot = (double)((upper_bound-lower_bound).total_seconds()); // calculate how far time has progressed into the current data entry
    double d_temp_tot = m_weather_data[upper_bound].m_temp - m_weather_data[lower_bound].m_temp; // calculate how far temperature will progress in the current data entry
    double d_time = (double)((current_time-lower_bound).total_seconds()); // calculate how far time has progressed into the current data entry

    m_temperature = (d_temp_tot/d_time_tot)*(d_time) + m_weather_data[lower_bound].m_temp;
    m_system->m_SS_u(m_index) = m_temperature; // update the system with the current temperature
} // update_sys()


} // namespace Building_Physics
} // namespace BSO

#endif // BP_WEATHER_PROFILE_HPP
