#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include <iostream>


namespace BSO { namespace Structural_Design {  namespace Components {

struct Constraint
{
    unsigned int m_dof; // degree of freedom [0,1,2,3,4,5] equals resp. [ux,uy,uz,rx,ry,rz]

    Constraint(unsigned int dof);
    ~Constraint();
}; // struct Constraint

Constraint::Constraint(unsigned int dof)
{
    m_dof = dof;
} // ctor

Constraint::~Constraint()
{

} // dtor

} // namespace Component
} // namespace Structural_Design
} // namespace BSO



#endif // CONSTRAINT_HPP
