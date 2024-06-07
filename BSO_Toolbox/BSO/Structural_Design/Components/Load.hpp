#ifndef LOAD_HPP
#define LOAD_HPP

#include <iostream>


namespace BSO { namespace Structural_Design {  namespace Components {

struct Load
{
    double m_value; // value of the load (signed magnitude)
    unsigned int m_dof; // degree of freedom [0,1,2,3,4,5] equals resp. [ux,uy,uz,rx,ry,rz]
    unsigned int m_lc; // load case ID

    Load(unsigned int lc, unsigned int dof, double value);
    ~Load();

    template<typename T> Load operator * (T rhs);
}; // struct Load

    Load::Load(unsigned int lc, unsigned int dof, double value)
    {
        m_lc = lc;
        m_dof = dof;
        m_value = value;
    } // ctor

    Load::~Load()
    {

    } // dtor

    template<typename T> Load Load::operator * (T rhs)
    {
        return  Load (m_lc, m_dof, m_value * rhs);
    }

} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // LOAD_HPP
