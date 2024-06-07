#ifndef GRID_HPP
#define GRID_HPP

#include <vector>
#include <algorithm>
#include <iostream>
#include <map>

#include <BSO/Structural_Design/SD_Analysis.hpp>
#include <BSO/Structural_Design/Components/Point_Comp.hpp>

namespace BSO { namespace Structural_Design { namespace Stabilization
{
    using namespace std;
    struct coord
    {
        int x;
        int y;
        int z;

        bool operator == (coord& rhs)
        {
            return (x == rhs.x && y == rhs.y && z == rhs.z);
        }
    }; //coord

    class Grid
    {
    private:
        SD_Analysis_Vars* SD;
        vector<coord> coords;
        vector<vector<vector<coord*>>> grid;
        vector<Components::Point*> m_points;
        map<vector<unsigned int>, vector<int> > grid_coords;
        map<pair<vector<unsigned int>, vector<int> >, Components::Point* > grid_coords_points;
        map<Components::Point*, vector<unsigned int> > points_grid;
		map<vector<unsigned int>, Components::Point*> grid_points;
		unsigned int x_size, y_size, z_size;

    public:
        Grid(SD_Analysis_Vars*); // ctor
		Grid(vector<Components::Point*>); // ctor
        ~Grid(); // dtor
        vector<vector<vector<coord*>>> get_grid();
        void show_grid();
        map<vector<unsigned int>, vector<int> > get_grid_coords();
        map<pair<vector<unsigned int>, vector<int> >, Components::Point* > get_grid_coords_points();
        map<Components::Point*, vector<unsigned int> > get_points_grid();
		map<vector<unsigned int>, Components::Point*> get_grid_points();
		unsigned int get_grid_size(char);
    };


    Grid::Grid(SD_Analysis_Vars* SD)
    {
        // create grid vector
        m_points = SD->get_points();

        vector<int> x;
        vector<int> y;
        vector<int> z;

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            x.push_back(m_points[i]->get_coords()[0]);
            y.push_back(m_points[i]->get_coords()[1]);
            z.push_back(m_points[i]->get_coords()[2]);
        }

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            coord temp;
            temp.x = x[i];
            temp.y = y[i];
            temp.z = z[i];
            coords.push_back(temp);
        }

        sort(x.begin(),x.end());
        x.erase( unique(x.begin(),x.end() ), x.end());
        sort(y.begin(),y.end());
        y.erase( unique(y.begin(),y.end() ), y.end());
        sort(z.begin(),z.end());
        z.erase( unique(z.begin(),z.end() ), z.end());

        for (unsigned int i = 0; i < x.size(); i++)
        {
            grid.push_back(vector<vector<coord*> >());
            for (unsigned int j = 0; j < y.size(); j++)
            {
                grid[i].push_back(vector<coord*>());
                for (unsigned int k = 0; k < z.size(); k++)
                {
                    grid[i][j].push_back(nullptr);
                }
            }
        }

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            auto l = distance(x.begin(), find(x.begin(), x.end(), coords[i].x));
            auto m = distance(y.begin(), find(y.begin(), y.end(), coords[i].y));
            auto n = distance(z.begin(), find(z.begin(), z.end(), coords[i].z));
            grid[l][m][n] = &coords[i];
        }

        // create grid-coords map
        vector<unsigned int> temp_grid;
        vector<int> temp_coords;
        for (unsigned int i = 0; i < grid.size(); i ++)
        {
            for (unsigned int j = 0; j < grid[i].size(); j++)
            {
                for (unsigned int k = 0; k < grid[i][j].size(); k++)
                {
                    if (grid[i][j][k] != nullptr)
                    {
                        temp_grid.push_back(i);
                        temp_grid.push_back(j);
                        temp_grid.push_back(k);
                        temp_coords.push_back(grid[i][j][k]->x);
                        temp_coords.push_back(grid[i][j][k]->y);
                        temp_coords.push_back(grid[i][j][k]->z);

                        grid_coords[temp_grid] = temp_coords;
                        temp_grid.clear();
                        temp_coords.clear();
                    }
                }
            }
        }

        // create grid_coords_points map
        pair<vector<unsigned int>, vector<int> > temp_pair;
        map<vector<unsigned int>, vector<int> >::iterator it;

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            for (it = grid_coords.begin() ; it != grid_coords.end() ; it++)
            {
                if (m_points[i]->get_coords()[0] == it->second[0] &&
                    m_points[i]->get_coords()[1] == it->second[1] &&
                    m_points[i]->get_coords()[2] == it->second[2])
                {
                    temp_pair = make_pair(it->first, it->second);
                    grid_coords_points[temp_pair] = m_points[i];
                    break;
                }
            }
        }

        // create points_grid map
        map<pair<vector<unsigned int>, vector<int> >, Components::Point* >::iterator it_2; // grid_coords_points

        for (it_2 = grid_coords_points.begin(); it_2 != grid_coords_points.end(); it_2++)
        {
            points_grid[it_2->second] = it_2->first.first;
        }

		// create grid_points map
		map<Components::Point*, vector<unsigned int> >::iterator it_3; // points_grid
		for (it_3 = points_grid.begin(); it_3 != points_grid.end(); it_3++)
		{
			grid_points[it_3->second] = it_3->first;
		}
		
		// determine grid sizes
		x_size = x.size();
		y_size = y.size();
		z_size = z.size();
		
		

    } // ctor
	
    Grid::Grid(vector<Components::Point*> points)
    {
        // create grid vector
        m_points = points;

        vector<int> x;
        vector<int> y;
        vector<int> z;

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            x.push_back(m_points[i]->get_coords()[0]);
            y.push_back(m_points[i]->get_coords()[1]);
            z.push_back(m_points[i]->get_coords()[2]);
        }

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            coord temp;
            temp.x = x[i];
            temp.y = y[i];
            temp.z = z[i];
            coords.push_back(temp);
        }

        sort(x.begin(),x.end());
        x.erase( unique(x.begin(),x.end() ), x.end());
        sort(y.begin(),y.end());
        y.erase( unique(y.begin(),y.end() ), y.end());
        sort(z.begin(),z.end());
        z.erase( unique(z.begin(),z.end() ), z.end());

        for (unsigned int i = 0; i < x.size(); i++)
        {
            grid.push_back(vector<vector<coord*> >());
            for (unsigned int j = 0; j < y.size(); j++)
            {
                grid[i].push_back(vector<coord*>());
                for (unsigned int k = 0; k < z.size(); k++)
                {
                    grid[i][j].push_back(nullptr);
                }
            }
        }

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            auto l = distance(x.begin(), find(x.begin(), x.end(), coords[i].x));
            auto m = distance(y.begin(), find(y.begin(), y.end(), coords[i].y));
            auto n = distance(z.begin(), find(z.begin(), z.end(), coords[i].z));
            grid[l][m][n] = &coords[i];
        }

        // create grid-coords map
        vector<unsigned int> temp_grid;
        vector<int> temp_coords;
        for (unsigned int i = 0; i < grid.size(); i ++)
        {
            for (unsigned int j = 0; j < grid[i].size(); j++)
            {
                for (unsigned int k = 0; k < grid[i][j].size(); k++)
                {
                    if (grid[i][j][k] != nullptr)
                    {
                        temp_grid.push_back(i);
                        temp_grid.push_back(j);
                        temp_grid.push_back(k);
                        temp_coords.push_back(grid[i][j][k]->x);
                        temp_coords.push_back(grid[i][j][k]->y);
                        temp_coords.push_back(grid[i][j][k]->z);

                        grid_coords[temp_grid] = temp_coords;
                        temp_grid.clear();
                        temp_coords.clear();
                    }
                }
            }
        }

        // create grid_coords_points map
        pair<vector<unsigned int>, vector<int> > temp_pair;
        map<vector<unsigned int>, vector<int> >::iterator it;

        for (unsigned int i = 0; i < m_points.size(); i++)
        {
            for (it = grid_coords.begin() ; it != grid_coords.end() ; it++)
            {
                if (m_points[i]->get_coords()[0] == it->second[0] &&
                    m_points[i]->get_coords()[1] == it->second[1] &&
                    m_points[i]->get_coords()[2] == it->second[2])
                {
                    temp_pair = make_pair(it->first, it->second);
                    grid_coords_points[temp_pair] = m_points[i];
                    break;
                }
            }
        }

        // create points_grid map
        map<pair<vector<unsigned int>, vector<int> >, Components::Point* >::iterator it_2; // grid_coords_points

        for (it_2 = grid_coords_points.begin(); it_2 != grid_coords_points.end(); it_2++)
        {
            points_grid[it_2->second] = it_2->first.first;
        }

		// create grid_points map
		map<Components::Point*, vector<unsigned int> >::iterator it_3; // points_grid
		for (it_3 = points_grid.begin(); it_3 != points_grid.end(); it_3++)
		{
			grid_points[it_3->second] = it_3->first;
		}
		
		// determine grid sizes
		x_size = x.size();
		y_size = y.size();
		z_size = z.size();
		
		

    } // ctor

    Grid::~Grid()
    {

    } // dtor

    vector<vector<vector<coord*>>> Grid::get_grid()
    {
        return grid;
    }


    void Grid::show_grid()
    {

        for (unsigned int i = 0; i < grid.size(); i ++)
        {
            for (unsigned int j = 0; j < grid[i].size(); j++)
            {
                for (unsigned int k = 0; k < grid[i][j].size(); k++)
                {
                    if (grid[i][j][k] != nullptr)
                    {
                        cout << i << "," << j << "," << k << " - " << grid[i][j][k]->x << "," << grid[i][j][k]->y << "," << grid[i][j][k]->z << endl;
                    }
                }
            }
        }
    }

    map<vector<unsigned int>, vector<int> > Grid::get_grid_coords()
    {
        return grid_coords;
    }

    map<pair<vector<unsigned int>, vector<int> >, Components::Point* > Grid::get_grid_coords_points()
    {
        return grid_coords_points;
    }

    map<Components::Point*, vector<unsigned int> > Grid::get_points_grid()
    {
        return points_grid;
    }

	map<vector<unsigned int>, Components::Point*> Grid::get_grid_points()
	{
		return grid_points;
	}
	
	unsigned int Grid::get_grid_size(char axis)
	{
		if (axis == 'x')
			return x_size;
		else if (axis == 'y')
			return y_size;
		else
			return z_size;		
	}
	
} // namespace Stabilization
} // namespace Structural_Design
} // namespace BSO

#endif //GRID_HPP
