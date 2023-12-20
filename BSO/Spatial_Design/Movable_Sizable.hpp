#ifndef MOVABLE_SIZABLE_HPP
#define MOVABLE_SIZABLE_HPP

#include <BSO/Spatial_Design/Supercube.hpp> // for operator overloading, this also loads in <string> and <vector>
#include <BSO/Trim_And_Cast.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm> // for vector::sort()
#include <cmath>
#include <cstdlib>

namespace BSO { namespace Spatial_Design
{

    /*
     *
     */

    // Structure definition:

    struct MS_Space
    {
        int ID; // space identification number
        std::string m_space_type;
        double width, depth, height; // room dimensions
        double x, y, z; // room coordinates (closest to origin)
        bool surfaces_given; // TG_22-03-2017  checks if surface types of ALL cardinal directions are given, false if incomplete
        bool space_type_given;
        std::string surface_type [6]; // TG_22-03-2017  cardinal directions {north, east, south, west, top, bottom}
        double get_area()
        {
            return width*depth;
        }
        double get_volume()
        {
            return width*depth*height;
        }
        double get_aspect_ratio()
        {
            return (width>depth) ? width/depth : depth/width;
        }

        void init_zero()
        {
            ID = 0;
            width = 0;
            depth = 0;
            height = 0;
            x = 0.0;
            y = 0.0;
            z = 0.0;
            m_space_type = "";
            surfaces_given = false;
            space_type_given = false;

            for (int i = 0; i < 6; i++) // TG_22-03-2017  clear the temp_space.surface_types to obtain an empty in case of invalid amount of surface types
            {
                surface_type[i] = "";
            }
        }

        MS_Space()
        {
            init_zero();

        }

    }; // MS_Space



    /*
     *
     */

    // Class definition:

    class MS_Building
    {
    private:
        std::vector<MS_Space> m_spaces;
        int m_last_space_ID;
    public:
        MS_Building(SC_Building); // ctor, converts from Supercube to Movable Sizable
        MS_Building(std::string); // ctor, initializes the class by reading an input file
        MS_Building(); //ctor, creates an empty MS_Build object
        ~MS_Building(); // dtor

        void read_file(std::string); // reads an input file (e.g. "Filename.txt")
        void write_file(std::string); // writes an output file (e.g. "Filename.txt")

        operator SC_Building() const; // converts from Movable Sizable to Supercube

        int obtain_space_count(); // returns the size of the rooms vector
        double get_volume(); // returns the volume of the building

        MS_Space obtain_space(int space_index); // returns a MS_Space structure from the m_spaces vector at a certain index
        int get_space_index(int space_ID); // returns the index from the m_spaces vector if the space ID is found

        int get_space_ID(unsigned int space_index); // NEW returns the space ID if the index from the m_spaces vector is found
        std::string get_space_type(unsigned int space_index);
        std::string get_surface_type(int space_index, int surface_type); // NEW returns the surface_type from the m_spaces vector at a certain index
        bool get_surfaces_given(int space_index); // NEW returns boolean surfaces_given from the m_spaces vector at a certain index

        void add_space(MS_Space); // adds a space to the building
        void delete_space(int space_index); // deletes a space from the building

        void split_space(int space_index); // splits a space across its largest dimensions
        void split_space(int space_index, int axis); // splits in the middle across axis: [0,1,2] for respectively [x,y,z]
        void split_space_n(int space_index, int axis, unsigned int n_divisions); // a space into n equal parts, across axis: [0,1,2] for respectively [x,y,z]

        void scale_x(double n); // scales the building design about the x axis
        void scale_y(double n); // scales the building design about the y axis
        void scale_z(double n); // scales the building design about the z axis

        void clear_design();

        void reset_z_zero(); // resets the z-coordinates in the building design if there are no z-coordinates equal to zero

        bool check_cell(SC_Building S, int cell_index, int space_ID,
                        double x_origin, double y_origin, double z_origin) const; // checks if a cell of Supercube S belongs to a certain room_ID, also requires the x- y- and z- coordinates of the supercube (first values of x_, y_ and z_values vectors)

        // TS added functions
        int get_last_space_id(); // retrieves the last space ID used in the MS Building
        void search_last_space_id(); // searches in the MS Building for the last used space id

    }; // MS_Building



    // Implementation of the member functions:

    MS_Building::MS_Building(SC_Building S) //conversion from Supercube to MovableSizable
    {
        m_last_space_ID = -1;
        int w_origin = S.w_size(); // initialize the indexes which will contain the origin of the Movable Sizable representation
        int d_origin = S.d_size(); // these are initialized to the cell containing the largest coordinates, to be updated later
        int h_origin = S.h_size();

        std::vector<double> w_coord_values(S.w_size()+1);
        std::vector<double> d_coord_values(S.d_size()+1);
        std::vector<double> h_coord_values(S.h_size()+1);

        // find which cell willcontain the origin
        for (unsigned int cell_index = 1; cell_index <= (S.w_size()*S.d_size()*S.h_size()); cell_index++) // starts with 1, since first index is the room ID // each cell is checked for each room, whether it describes a room
        { // for each cell
            int w_index = S.get_w_index(cell_index);  // compute the grid indexes of the considered cell
            int d_index = S.get_d_index(cell_index);
            int h_index = S.get_h_index(cell_index);

            for (unsigned int i = 0; i < S.b_size(); i++) // check each room for the considered cell, whether it describes a room
            {
                if (S.request_b(i, cell_index) == 1) // if it does describe a room, then update the origin indexes
                    {
                        if (w_index < w_origin) { w_origin = w_index; } // update the indexes containing the origin of the MS representation
                        if (d_index < d_origin) { d_origin = d_index; }
                        if (h_index < h_origin) { h_origin = h_index; }
                    }
            }
        }

        // compute the global coordinates for each cell's origin
        w_coord_values[w_origin] = 0;
        for (unsigned int i = w_origin+1; i < w_coord_values.size(); i++)
        {
            w_coord_values[i] = w_coord_values[i-1] + floor(S.request_w(i-1));
        }

        d_coord_values[d_origin] = 0;
        for (unsigned int i = d_origin+1; i < d_coord_values.size(); i++)
        {
            d_coord_values[i] = d_coord_values[i-1] + floor(S.request_d(i-1));
        }

        h_coord_values[h_origin] = 0;
        for (unsigned int i = h_origin+1; i < h_coord_values.size(); i++)
        {
            h_coord_values[i] = h_coord_values[i-1] + floor(S.request_h(i-1));
        }

        // compute each MS space
        for (unsigned int i = 0; i < S.b_size(); i++)
        {
            MS_Space temp_space; // initializing all values in an object of the RoomMS structure
            temp_space.ID = i + 1; // assigning the room ID to the RoomMS object
            int maximum = 0, minimum = 0;   // initializing minimum and maximum indexes of the space with id: i+1
            for (unsigned int cell_index = 0; cell_index < S.b_row_size(i); cell_index++)   // finds the min and max indexes
            {
                if (minimum == 0 && S.request_b(i, cell_index) == 1)    // finds first cell_index with value 1 in the cell_vector, this is the cell containing the room's origin assuming spaces are cuboid
                    {minimum = cell_index;}
                if (S.request_b(i, cell_index) == 1)    // finds the last index with value 1 in the cell_array, this is the room's outmost cell assuming spaces are cuboid
                    {maximum = cell_index;}
            }

            int min_w = S.get_w_index(minimum), max_w = S.get_w_index(maximum),
                min_d = S.get_d_index(minimum), max_d = S.get_d_index(maximum),
                min_h = S.get_h_index(minimum), max_h = S.get_h_index(maximum);   // computes the grid indexes of the min and max cells

            temp_space.x = w_coord_values[min_w];
            temp_space.width = w_coord_values[max_w] + floor(S.request_w(max_w)) - temp_space.x;
            temp_space.y = d_coord_values[min_d];
            temp_space.depth = d_coord_values[max_d] + floor(S.request_d(max_d)) - temp_space.y;
            temp_space.z = h_coord_values[min_h];
            temp_space.height = h_coord_values[max_h] + floor(S.request_h(max_h)) - temp_space.z;

            m_spaces.push_back(temp_space); // stack the RoomMS structure in the rooms vector
            if (m_last_space_ID < m_spaces.back().ID)
            {
                m_last_space_ID = m_spaces.back().ID;
            }
        }
    } //ctor

    MS_Building::MS_Building(std::string filename)
    {
        m_last_space_ID = -1;

        if (filename.empty())
            throw std::runtime_error("No file name specified to initialise a MS building spatial design");


        read_file(filename);//ctor
    } // ctor

    MS_Building::MS_Building()
    {

    }


    MS_Building::~MS_Building()
    {

    } // dtor

    void MS_Building::read_file(std::string file_name)
    {
        std::ifstream input(file_name.c_str()); // initialize input stream from file: file_name
		if (!input.is_open() && file_name != "empty")
			throw std::runtime_error("Could not open file: " + file_name);
		
        std::string line;
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer

        while (!input.eof() && file_name != "empty") // continue while the End Of File has not been reached
        {

            getline(input,line); // get next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }

            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            int number_of_tokens = std::distance( tok.begin(), tok.end() ); // TG_22-03-2017  determine number of tokens within a line of the input file

            if (*token != "R")
            {
                continue; // continue to next line in text file
            }
            else // if the first token is an "R" then this line describes a space
            {
				MS_Space temp_space; // this MS_Space structure will temporarily hold the space described by the considered line
                token++; // this is the 'ID'
                temp_space.ID = trim_and_cast_int(*token);

                token++; // this is 'width'
                temp_space.width = trim_and_cast_double(*token);

                token++; // this is 'depth'
                temp_space.depth = trim_and_cast_double(*token);

                token++; // this is 'height'
                temp_space.height = trim_and_cast_double(*token);

                token++; // this is 'x-coordinate'
                temp_space.x = trim_and_cast_double(*token);

                token++; // this is 'y-coordinate'
                temp_space.y = trim_and_cast_double(*token);

                token++; // this is 'z-coordinate'
                temp_space.z = trim_and_cast_double(*token);

                switch (number_of_tokens) // TG_22-03-2017  defines boolean of surface given
                {
                case 8:
                {
                    break;
                }
                case 9:
                {
                    token++; // space_type
                    temp_space.m_space_type = *token;
                    boost::algorithm::trim(temp_space.m_space_type);
                    temp_space.space_type_given = true;
                    break;
                }
                case 15:
                {
                    token++; // space_type
                    temp_space.m_space_type = *token;
                    boost::algorithm::trim(temp_space.m_space_type);
                    temp_space.space_type_given = true;
					// NOTE no break, so we continue to the next case!
                }
                case 14:
                {
                    token++; // TG_22-03-2017  this is 'north-surface'
                    temp_space.surface_type[0] = *token;
                    boost::algorithm::trim(temp_space.surface_type[0]);

                    token++; // TG_22-03-2017  this is 'east-surface'
                    temp_space.surface_type[1] = *token;
                    boost::algorithm::trim(temp_space.surface_type[1]);

                    token++; // TG_22-03-2017  this is 'south-surface'
                    temp_space.surface_type[2] = *token;
                    boost::algorithm::trim(temp_space.surface_type[2]);

                    token++; // TG_22-03-2017  this is 'west-surface'
                    temp_space.surface_type[3] = *token;
                    boost::algorithm::trim(temp_space.surface_type[3]);

                    token++; // TG_22-03-2017  this is 'top-surface'
                    temp_space.surface_type[4] = *token;
                    boost::algorithm::trim(temp_space.surface_type[4]);

                    token++; // TG_22-03-2017  this is 'bottom-surface'
                    temp_space.surface_type[5] = *token;
                    boost::algorithm::trim(temp_space.surface_type[5]);

                    temp_space.surfaces_given  = true;
                    break;
                }
                default:
                {
                    std::cerr << "Space ID " << temp_space.ID << " contains invalid amount of tokens. (Movable_Sizable.hpp)" << std::endl;
                    exit(1);
                    break;
                }
                };

                m_spaces.push_back(temp_space); // stack the MS_Space structure in the m_spaces vector

                if (m_last_space_ID < m_spaces.back().ID)
                {
                    m_last_space_ID = m_spaces.back().ID;
                }
            }
        }
    } // read_file()

    void MS_Building::write_file(std::string filename)
    {
        std::ofstream output; // initialize output stream
        output.open(filename.c_str()); // stream to file: filename

        for (unsigned int i = 0; i < m_spaces.size(); i++)  // for each room write the description as follows:
                                                            //R,width,depth,height,x-coor,y-coor,z-coor,north-surf, east-surf, south-surf, west-surf, top-surf, bottom-surf
            {
            output << "R," << m_spaces[i].ID    << "," << m_spaces[i].width
                   << ","  << m_spaces[i].depth << "," << m_spaces[i].height
                   << ","  << m_spaces[i].x     << "," << m_spaces[i].y
                   << ","  << m_spaces[i].z;
            if (m_spaces[i].space_type_given == true)
            {
                output << "," << m_spaces[i].m_space_type;
            }
            if (m_spaces[i].surfaces_given == true) // TG_22-03-2017
            {
            output << "," << m_spaces[i].surface_type[0] << "," << m_spaces[i].surface_type[1]
                   << "," << m_spaces[i].surface_type[2] << "," << m_spaces[i].surface_type[3]
                   << "," << m_spaces[i].surface_type[4] << "," << m_spaces[i].surface_type[5];
            }
            output << std::endl;
            }

        output.close(); // terminate the output stream
    } // write_file()

    int MS_Building::obtain_space_count()
    {
        return m_spaces.size();
    } // obtain_space_count()

    double MS_Building::get_volume()
    {
        double volume = 0;
        for (unsigned int i = 0; i < m_spaces.size(); i++)
        {
            volume += m_spaces[i].get_volume();
        }
        return volume;
    } // get_volume()

    MS_Space MS_Building::obtain_space(int space_index)
    {
        return m_spaces[space_index];
    } // obtain_room()

    int MS_Building::get_space_index(int space_ID)
    {
        for (unsigned int i = 0; i < m_spaces.size(); i ++)
        {
            if (space_ID == m_spaces[i].ID)
            {
                return i;
            }
        }
        std::cerr << "Could not find space by its ID (Movable_Sizable.hpp), exiting now... " << std::endl;
        exit(1);
    }

    std::string MS_Building::get_space_type(unsigned int space_index)
    {
        return m_spaces[space_index].m_space_type;
    } // get_sapce_type()

    int MS_Building::get_space_ID(unsigned int space_index) // NEW
    {
        if (space_index >= m_spaces.size()) // NEW error if requested space index is not valid
            {
            std::cerr << "Could not find space ID by its index (Movable_Sizable.hpp), exiting now... " << std::endl;
            exit(1);
            }
        {
            return m_spaces[space_index].ID;
        }
    } // NEW get_space_ID()

    std::string MS_Building::get_surface_type(int space_index, int surface_type) // NEW
    {
        return m_spaces[space_index].surface_type[surface_type]; // cardinal directions {0=north, 1=east, 2=south, 3=west, 4=top, 5=bottom}
    } // NEW get_surface_type()

    bool MS_Building::get_surfaces_given(int space_index) // NEW
    {
        return m_spaces[space_index].surfaces_given; // returns true if surface types of ALL cardinal directions are given
    } // get_surfaces_given()

    void MS_Building::add_space(MS_Space space)
    {
        m_spaces.push_back(space);
        if (m_last_space_ID < m_spaces.back().ID)
        {
            m_last_space_ID = m_spaces.back().ID;
        }
    }

    void MS_Building::delete_space(int space_index)
    {
        m_spaces.erase(m_spaces.begin() + space_index);
    }

    void MS_Building::split_space(int space_index)
    {
        if (m_spaces[space_index].width < 0.9999 * m_spaces[space_index].depth)
        { // if the space is deeper than its width
             this->split_space_n(space_index, 1, 2);
        }
        else
        { // if the space is wider than its depth or as wide as deep
             this->split_space_n(space_index, 0, 2);
        }
    }

    void MS_Building::split_space(int space_index, int axis)
    {
        this->split_space_n(space_index, axis, 2);
    }

    void MS_Building::split_space_n(int space_index, int axis, unsigned int n_division)
    { // did not use: new coordinate = preceding_x + width*1/n_division to prevent awkward errors by double precision
        if (n_division < 2)
        {
            return;
        }
        if (axis == 0)
        { // split across x axis
            double current_x = m_spaces[space_index].x, next_x;
            for (unsigned int n = 0; n < n_division; n++)
            {
                MS_Space temp = m_spaces[space_index];
                temp.ID = ++m_last_space_ID;
                next_x = m_spaces[space_index].x + (n + 1) * m_spaces[space_index].width / (double)(n_division);
                temp.x = current_x;
                temp.width = next_x - current_x;
                m_spaces.push_back(temp);
                current_x = next_x;
            }
            delete_space(space_index);
        }
        else if (axis == 1)
        { // split across y axis
            double current_y = m_spaces[space_index].y, next_y;
            for (unsigned int n = 0; n < n_division; n++)
            {
                MS_Space temp = m_spaces[space_index];
                temp.ID = ++m_last_space_ID;
                next_y = m_spaces[space_index].y + (n + 1) * m_spaces[space_index].depth / (double)(n_division);
                temp.y = current_y;
                temp.depth = next_y - current_y;
                m_spaces.push_back(temp);
                current_y = next_y;
            }
            delete_space(space_index);
        }
        else if (axis == 2)
        { // split across z axis
            double current_z = m_spaces[space_index].z, next_z;
            for (unsigned int n = 0; n < n_division; n++)
            {
                MS_Space temp = m_spaces[space_index];
                temp.ID = ++m_last_space_ID;
                next_z = m_spaces[space_index].z + (n + 1) * m_spaces[space_index].height / (double)(n_division);
                temp.z = current_z;
                temp.height = next_z - current_z;
                m_spaces.push_back(temp);
                current_z = next_z;
            }
            delete_space(space_index);
        }
        else
        { // axis is not defined
            std::cerr << "Error in splitting space, splitting across undefined axis, exiting..." << std::endl;
            exit(1);
        }
    }

    void MS_Building::scale_x(double n)
    {
        double temp_xw;
        for (unsigned int i = 0; i < m_spaces.size(); i++)
        {
            temp_xw = ceil((m_spaces[i].x + m_spaces[i].width) * n);
            m_spaces[i].x =  ceil(m_spaces[i].x * n);
            m_spaces[i].width = temp_xw - m_spaces[i].x;

        }
    }

    void MS_Building::scale_y(double n)
    {
        double temp_yd;
        for (unsigned int i = 0; i < m_spaces.size(); i++)
        {
            temp_yd = ceil((m_spaces[i].y + m_spaces[i].depth) * n);
            m_spaces[i].y = ceil(m_spaces[i].y * n);
            m_spaces[i].depth = temp_yd - m_spaces[i].y;

        }
    }

    void MS_Building::scale_z(double n)
    {
        double temp_zh;
        for (unsigned int i = 0; i < m_spaces.size(); i++)
        {
            temp_zh = ceil((m_spaces[i].z + m_spaces[i].height) * n);
            m_spaces[i].z = ceil(m_spaces[i].z * n);
            m_spaces[i].height = temp_zh - m_spaces[i].z;

        }
    }

    MS_Building::operator SC_Building() const // conversion from MovableSizable to Supercube
    {
        SC_Building S; // initialize an object of class Supercube, information will be added and then it will be returned at the end
        std::vector<double> x_values, y_values, z_values; // these vectors will contain all x- y- and z-values of the MS representation

        for (unsigned int i = 0; i < m_spaces.size(); i++) // this stacks every x- y- and z-coordinate of every room in the respective vectors
        {
            x_values.push_back(m_spaces[i].x);
            x_values.push_back(m_spaces[i].x+m_spaces[i].width);
            y_values.push_back(m_spaces[i].y);
            y_values.push_back(m_spaces[i].y+m_spaces[i].depth);
            z_values.push_back(m_spaces[i].z);
            z_values.push_back(m_spaces[i].z+m_spaces[i].height);
        }

        sort(x_values.begin(), x_values.end()); // sorts all values in the vectors in ascending order
        sort(y_values.begin(), y_values.end());
        sort(z_values.begin(), z_values.end());
        x_values.erase(unique(x_values.begin(), x_values.end()), x_values.end()); // erases all duplicates from the vectors
        y_values.erase(unique(y_values.begin(), y_values.end()), y_values.end());
        z_values.erase(unique(z_values.begin(), z_values.end()), z_values.end());

        double x_origin = x_values[0], y_origin = y_values[0], z_origin = z_values[0]; // saves the coordinates of the super cube's origin

        for (unsigned int i = 0; i < x_values.size()-1; i++)    // computes widths of super cube grid and puts them in the w_values vector
            { S.stack_w_value(x_values[i+1] - x_values[i]); }
        for (unsigned int i = 0; i < y_values.size()-1; i++)    // computes depths of super cube grid and puts them in the d_values vector
            { S.stack_d_value(y_values[i+1] - y_values[i]); }
        for (unsigned int i = 0; i < z_values.size()-1; i++)    // computes heights of super cube grid and puts them in the h_values vector
            { S.stack_h_value(z_values[i+1] - z_values[i]); }

        int cube_size = S.w_size()*S.d_size()*S.h_size(); // initialize variable containing super cube size
        std::vector<int> b_values_row; // initializes a vector for the b_values matrix

        for (unsigned int i = 0; i < m_spaces.size(); i++)  // computes a row of the b_values matrix and adds these to the matrix for each room
        {
            b_values_row.clear(); // clears the vector's data from previous iteration
            b_values_row.push_back(i + 1); // index 0 contains the room ID, for the super cube the count starts again from 1.

            for (int cell_index = 1; cell_index <= cube_size; cell_index++) // checks each cell within the super cube if it belongs to room with ID: i
            {
                b_values_row.push_back((check_cell(S, cell_index, i, x_origin, y_origin, z_origin) ? 1 : 0)); // If a cell belongs to the room assign 1 to index, if not 0
            }

            S.stack_b_value_row(b_values_row); // adds the row to the b_values matrix
        }

        return S;
    } // operator SC_Building() const

    void MS_Building::clear_design()
    {
        m_spaces.clear();
        m_last_space_ID = -1;
    } // clear_design()

    void MS_Building::reset_z_zero()
    {
        int min = m_spaces[0].z;
        // find the minimum value of the z-coordinates in the building
        for (unsigned int i = 1; i < m_spaces.size(); i++)
        { // for each space
            if (m_spaces[i].z < min)
            { // check if this space's z is smaller than min
                min = m_spaces[i].z; // if yes, assign its value to min
            }
        }

        for (unsigned int i = 0; i < m_spaces.size(); i++)
        { // for each space
            m_spaces[i].z -= min; // shift the z-coordinates down by min
        }
    } // reset_z_zero()

    bool MS_Building::check_cell(SC_Building S, int cell_index, int space_index, double x_origin, double y_origin, double z_origin) const
    {
        int w_index = S.get_w_index(cell_index); // compute the indexes of cell with index: cell_index
        int d_index = S.get_d_index(cell_index);
        int h_index = S.get_h_index(cell_index);
        double x_coor = x_origin, y_coor = y_origin, z_coor = z_origin; // initialize the coordinates of the cell's origin to the coordinates of the super cube's origin

        for (int k = 0; k < w_index; k++)    // update the x_coordinates of the cell's origin
            { x_coor += S.request_w(k); }
        for (int k = 0; k < d_index; k++)    // update the y_coordinates of the cell's origin
            { y_coor += S.request_d(k); }
        for (int k = 0; k < h_index; k++)    // update the z_coordinates of the cell's origin
            { z_coor += S.request_h(k); }

        if((x_coor >= m_spaces[space_index].x && x_coor < m_spaces[space_index].x + m_spaces[space_index].width) &&
           (y_coor >= m_spaces[space_index].y && y_coor < m_spaces[space_index].y + m_spaces[space_index].depth) &&
           (z_coor >= m_spaces[space_index].z && z_coor < m_spaces[space_index].z + m_spaces[space_index].height))  // if a cell is within the bounds of space r return true
            { return true; }
        else
            { return false; } // if not return false
    } // check_cell()



    int MS_Building::get_last_space_id()
    {
        return m_last_space_ID;
    }

    void MS_Building::search_last_space_id()
    {
        m_last_space_ID = 0;

        for( unsigned int i = 0 ; i < m_spaces.size() ; i++)
        {
            if(m_spaces[i].ID > m_last_space_ID)
            {
                m_last_space_ID = m_spaces[i].ID;
            }
        }
        return;
    }


} // namespace Spatial_Design
} // namespace BSO
#endif // MOVABLE_SIZABLE_HPP
