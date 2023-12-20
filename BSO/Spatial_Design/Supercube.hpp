#ifndef SUPERCUBE_HPP
#define SUPERCUBE_HPP

#include <BSO/Trim_And_Cast.hpp>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

namespace BSO { namespace Spatial_Design
{
    /*
     *
     */

    // Class definition:

    class SC_Building
    {
    private:
        std::vector<double> w_values; // contains all widths of the cube's grid in x-direction
        std::vector<double> d_values; // contains all depths of the cube's grid in y-direction
        std::vector<double> h_values; // contains all heights of the cube's grid in z-direction
        std::vector<std::vector<int> > b_values; // contains all room information, for each room it is described whether a cell belongs to that room or not

        friend class MS_Building;

        void stack_w_value(double w_value); // adds a value to the w_values vector
        void stack_d_value(double d_value); // adds a value to the d_values vector
        void stack_h_value(double h_value); // adds a value to the h_values vector
        void stack_b_value_row(std::vector<int> b_value_row); // adds a value to the b_values vector
    protected:

    public:
        SC_Building(std::string filename); // ctor, initializes by reading an input file
        SC_Building(std::string line, int n); // ctor, intializes by parsing one string
        SC_Building(); //ctor
        ~SC_Building(); //dtor

        void read_file(std::string); // reads an input file (e.g. "Filename.txt")
        void write_file(std::string); // writes an output file (e.g. "Filename.txt")
        std::string write_string(); // writes the SC to a string (tab separated)

        int get_w_index(int cell_index); // returns the index of the w-grid of the specified cell
        int get_d_index(int cell_index); // returns the index of the d-grid of the specified cell
        int get_h_index(int cell_index); // returns the index of the h-grid of the specified cell
        int get_cell_index(int w_index, int d_index, int h_index); // returns the cell index belonging to the w,d,h indices

        unsigned int w_size(); // returns size of the w_values vector
        unsigned int d_size(); // returns size of the d_values vector
        unsigned int h_size(); // returns size of the h_values vector
        unsigned int b_size(); // returns size of the b_values vector
        unsigned int b_row_size(int room_ID); // returns size of the specified b_values row
        int get_space_id(int cell_index); // returns the space ID if the cell belongs the space WARNING exits when the cell does not belong to a space

        double request_w(int index); // returns a value from the w_values vector
        double request_d(int index); // returns a value from the d_values vector
        double request_h(int index); // returns a value from the h_values vector
        int request_b(int room_ID, int cell_index); // returns a value from the b_values vector

        void add_padding(unsigned int w, unsigned int d, unsigned int h, double magnitude);

        //TS added functions
        bool empty_cell(int cell_index); // returns true if a cell is empty
        std::vector<int> adjacent_cells(int cell_index); // returns a list with all cells adjacent to the given index
    }; // SC_Building

    // Implementation of member functions:

    SC_Building::SC_Building(std::string filename) // see header
    {
		if (filename.empty())
			throw std::runtime_error("No file name specified when trying to initialise a building spatial design in SC representation");
		
        read_file(filename);
    } // ctor

    SC_Building::SC_Building(std::string line, int n) // see header
    {
        if (n < -1) std::cout << "";

        boost::char_separator<char> sep("\t; "); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer

        t_tokenizer tok(line, sep); // tokenize the line
        t_tokenizer::iterator token = tok.begin(); // set iterator to first token

        unsigned int w = BSO::trim_and_cast_uint(*token); token++;
        unsigned int d = BSO::trim_and_cast_uint(*token); token++;
        unsigned int h = BSO::trim_and_cast_uint(*token); token++;
        unsigned int b = BSO::trim_and_cast_uint(*token); token++;

        w_values = std::vector<double>(w);
        d_values = std::vector<double>(d);
        h_values = std::vector<double>(h);
        b_values = std::vector<std::vector<int> >(b);

        for (unsigned int i = 0; i < w; i++)
        { // for each w_value
            w_values[i] = (int)1000*BSO::trim_and_cast_double(*token); // add the dimension
            token++;
        }

        for (unsigned int i = 0; i < d; i++)
        { // for each d_value
            d_values[i] = (int)1000*BSO::trim_and_cast_double(*token); // add the dimension
            token++;
        }

        for (unsigned int i = 0; i < h; i++)
        { // for each h_value
            h_values[i] = (int)1000*BSO::trim_and_cast_double(*token); // add the dimension
            token++;
        }

        for (unsigned int i = 0; i < b; i++)
        { // for each space
            b_values[i].push_back(i+1); // add a space ID
            for (unsigned int j = 0; j < w*d*h; j++)
            { // and for each cell in the supercube
                b_values[i].push_back(BSO::trim_and_cast_double(*token)); // add the value of cell j for space i to the b_values container
                token++;
            }
        }
    } // ctor

    SC_Building::SC_Building()
    {
        //ctor
    }

    SC_Building::~SC_Building()
    {
        //dtor
    }

    void SC_Building::read_file(std::string file_name)
    {
        std::ifstream input(file_name.c_str()); // initialize input stream from file: file_name

        if (!input.is_open())
            throw std::runtime_error("Could not open file: " + file_name);

        std::string line;
        std::string temp; // to temporarily hold tokens
        char type_ID = ' ';
        boost::char_separator<char> sep(","); // defines what separates tokens in a string
        typedef boost::tokenizer< boost::char_separator<char> > t_tokenizer; // settings for the boost::tokenizer

        while (!input.eof()) // continue while the End Of File has not been reached
        {
            getline(input,line); // get next line from the file
            boost::algorithm::trim(line); // remove white space from start and end of line (to see if it is an empty line, remove any incidental white space)
            if (line == "") //skip empty lines (tokenizer does not like it)
            {
                continue; // continue to next line
            }

            t_tokenizer tok(line, sep); // tokenize the line
            t_tokenizer::iterator token = tok.begin(); // set iterator to first token
            type_ID = BSO::trim_and_cast_char(*token); // interpret first token as type ID
            token++;

            switch (type_ID)
            {
            case 'w':
                {
                    while (token != tok.end())
                    {
                        temp = *token;
                        w_values.push_back(trim_and_cast_double(temp));
                        token++;
                    }
                    break;
                }
            case 'd':
                {
                    while (token != tok.end())
                    {
                        temp = *token;
                        d_values.push_back(trim_and_cast_double(temp));
                        token++;
                    }
                    break;
                }
            case 'h':
                {
                    while (token != tok.end())
                    {
                        temp = *token;
                        h_values.push_back(trim_and_cast_double(temp));
                        token++;
                    }
                    break;
                }
            case 'b':
                {
                    temp = *token;
                    int ID = trim_and_cast_int(temp);
                    std::vector<int> row; row.push_back(ID);
                    b_values.push_back(row);
                    token++;

                    while (token != tok.end())
                    {
                        temp = *token;
                        b_values[ID-1].push_back(trim_and_cast_int(temp));
                        token++;
                    }
                    break;
                }
            default:
                {
                    break;
                }
            } // end of switch statement
        } // end of while
    } // read_file()

    void SC_Building::write_file(std::string filename)
    {
        std::ofstream output; // initialize an output stream
        output.open(filename.c_str()); // stream to file: filename

        output << "w,";
        for (unsigned int i = 0; i < w_values.size(); i++) // stream all w_values
        {
            output << w_values[i];
            if (i < w_values.size() -1) output << ",";
        }

        output << std::endl << "d,";
        for (unsigned int i = 0; i < d_values.size(); i++) // stream all d_values
        {
            output << d_values[i];
            if (i < d_values.size() -1) output << ",";
        }

        output << std::endl << "h,";
        for (unsigned int i = 0; i < h_values.size(); i++) // stream all h_values
        {
            output << h_values[i];
            if (i < h_values.size() -1) output << ",";
        }

        output << std::endl;
        for (unsigned int i = 0; i < b_values.size(); i++) // stream all b_values, each line containing 1 room vector
        {
            output << "b," << b_values[i][0] << ",";

            for(unsigned int j = 1; j < b_values[i].size(); j++)
            {
                output << b_values[i][j];
                if (j < b_values[i].size() -1) output << ", ";
            }
            output << std::endl;
        }

        output.close(); // terminate the output stream
    } // write_file()

    std::string SC_Building::write_string()
    {
        std::string output;

        output += std::to_string(w_values.size()) + " ";
        output += std::to_string(d_values.size()) + " ";
        output += std::to_string(h_values.size()) + " ";
        output += std::to_string(b_values.size()) + " ";

        for (unsigned int i = 0;  i < w_values.size(); i++)
        {
            output += std::to_string(w_values[i]/1000) + " ";
        }

        for (unsigned int i = 0;  i < d_values.size(); i++)
        {
            output += std::to_string(d_values[i]/1000) + " ";
        }

        for (unsigned int i = 0;  i < h_values.size(); i++)
        {
            output += std::to_string(h_values[i]/1000) + " ";
        }

        for (unsigned int i = 0;  i < b_values.size(); i++)
        {
            for (unsigned int j = 1; j < b_values[i].size(); j++)
            {
                output += std::to_string(b_values[i][j]) + " ";
            }
        }

        return output;
    } // write_string()

    unsigned int SC_Building::w_size() // see header
    {
        return w_values.size();
    } // w_size()

    unsigned int SC_Building::d_size() // see header
    {
        return d_values.size();
    } // d_size()

    unsigned int SC_Building::h_size() // see header
    {
        return h_values.size();
    } // h_size()

    unsigned int SC_Building::b_size() // see header
    {
        return b_values.size();
    } // b_size()

    unsigned int SC_Building::b_row_size(int room_ID) // see header
    {
        return b_values[room_ID].size();
    } // b_row_size()

    int SC_Building::get_space_id(int cell_index)
    {
        for ( unsigned int i = 0 ; i < b_values.size() ; i++ )
        {
            if(b_values[i][cell_index] == 1)
                {return b_values[i][0];}
        }

        std::cerr<< "Error, could not find space using cell index: " << cell_index <<"exiting now... (Supercube.hpp)"<<std::endl;
        exit(1);
    } // get_space_id()

    void SC_Building::add_padding(unsigned int w, unsigned int d, unsigned int h, double magnitude)
    {
        for (unsigned int i = 0; i < w; i++)
        { // for each w_value to add
            if (i%2 == 0)
            { // i is even or zero
                for (unsigned int j = 0; j < b_values.size(); j++)
                { // for each space
                    b_values[j].insert(b_values[j].end(), h_values.size()*d_values.size(), 0); // add the padding to the bit mask
                }

                w_values.push_back(magnitude); // add a w_value to the back (i.e. the right; +x face)
            }
            else
            { // i is odd
                for (unsigned int j = 0; j < b_values.size(); j++)
                { // for each space
                    b_values[j].insert(b_values[j].begin()+1, h_values.size()*d_values.size(), 0); // add the padding to the bit mask
                }

                w_values.insert(w_values.begin(), magnitude); // add a w_value to the front (i.e. the left; -x face)
            }
        }

        for (unsigned int i = 0; i < d; i++)
        { // for each d_value to add
            if (i%2 == 0)
            { // i is even or zero
                for (unsigned int j = 0; j < b_values.size(); j++)
                { // for each space
                    for (int k = w_values.size()-1; k >= 0; k--)
                    { // for each w_value (i.e. for each w_row)
                        b_values[j].insert(b_values[j].begin()+1+((k+1)*d_values.size()*h_values.size()), h_values.size(), 0); // add the padding to the bit mask
                    }
                }

                d_values.push_back(magnitude); // add a d_value to the back (i.e. the back; +y face)
            }
            else
            { // i is odd
                for (unsigned int j = 0; j < b_values.size(); j++)
                { // for each space
                    for (int k = w_values.size()-1; k >= 0; k--)
                    { // for each w_value (i.e. for each w_row)
                        b_values[j].insert(b_values[j].begin()+1+((k+0)*d_values.size()*h_values.size()), h_values.size(), 0); // add the padding to the bit mask
                    }
                }

                d_values.insert(d_values.begin(), magnitude); // add a d_value to the front (i.e. the front; -y face)
            }
        }

        for (unsigned int i = 0; i < h; i++)
        { // for each h_value to add
            for (unsigned int j = 0; j < b_values.size(); j++)
            { // for each space
                for (int k = w_values.size()*d_values.size()-1; k >= 0; k--)
                { // for each w-,d-indexed column
                    b_values[j].insert(b_values[j].begin()+1+((k+1)*h_values.size()), 1, 0); // add the padding to the bit mask
                }
            }

            h_values.push_back(magnitude); // always add padding to the back (i.e. the top; +z face)
        }
    } // add_padding()

    double SC_Building::request_w(int index) // see header
    {
        return w_values[index];
    } // request_w()

    double SC_Building::request_d(int index) // see header
    {
        return d_values[index];
    } // request_d()

    double SC_Building::request_h(int index) // see header
    {
        return h_values[index];
    } // request_h()

    int SC_Building::request_b(int room_ID, int cell_index) // see header
    {
        return b_values[room_ID][cell_index];
    } // request_b()

    void SC_Building::stack_w_value(double w_value) // see header
    {
        w_values.push_back(w_value);
    } // stack_w_value()

    void SC_Building::stack_d_value(double d_value) // see header
    {
        d_values.push_back(d_value);
    } // stack_d_value()

    void SC_Building::stack_h_value(double h_value) // see header
    {
        h_values.push_back(h_value);
    } // stack_h_value()

    void SC_Building::stack_b_value_row(std::vector<int> b_value_row)
    {
        b_values.push_back(b_value_row);
    } // stack_b_value_row()

    int SC_Building::get_w_index(int cell_index)
    {
        return ((cell_index-1)/(h_values.size()*d_values.size()));
    } // get_w_index()

    int SC_Building::get_d_index(int cell_index)
    {
        return (((cell_index-1) - get_w_index(cell_index)*(h_values.size()*d_values.size()))/(h_values.size()));
    } // get_d_index()

    int SC_Building::get_h_index(int cell_index)
    {
        return (cell_index-1) - get_w_index(cell_index)*(h_values.size()*d_values.size()) - get_d_index(cell_index)*h_values.size();
    } // get_h_index()

    int SC_Building::get_cell_index(int w_index, int d_index, int h_index)
    {
        return (w_index * (h_values.size() * d_values.size())) + (d_index * h_values.size()) + h_index + 1;
    } // get_cell_index()

    bool SC_Building::empty_cell(int cell_index)
    {
        for ( unsigned int i = 0 ; i < b_values.size() ; i++ )
        {

            if ( b_values[i][cell_index] == 1)
            {
                return false;
            }
        }
        return true;
    } // empty_cell()

    std::vector<int> SC_Building::adjacent_cells(int cell_index)
    {
        unsigned int h_index = get_h_index(cell_index);
        unsigned int w_index = get_w_index(cell_index);
        unsigned int d_index = get_d_index(cell_index);

        std::vector<int> adjacent_cell_indices;

        if (h_index < h_values.size())
        {
            adjacent_cell_indices.push_back(get_cell_index(w_index, d_index, h_index + 1 ));
        }
        if (h_index > 1)
        {
            adjacent_cell_indices.push_back(get_cell_index(w_index, d_index, h_index - 1 ));
        }

        if (w_index < w_values.size())
        {
            adjacent_cell_indices.push_back(get_cell_index(w_index + 1, d_index, h_index ));
        }
        if(w_index > 1 )
        {
            adjacent_cell_indices.push_back(get_cell_index(w_index - 1, d_index, h_index ));
        }

        if (d_index > 1)
        {
            adjacent_cell_indices.push_back(get_cell_index(w_index, d_index - 1, h_index));
        }
        if (d_index < d_values.size())
        {
            adjacent_cell_indices.push_back(get_cell_index(w_index, d_index + 1, h_index));
        }

        return adjacent_cell_indices;
    } //adjacent_cells()

} // namespace Spatial_Design
} // namespace BSO

#endif // SUPERCUBE_H
