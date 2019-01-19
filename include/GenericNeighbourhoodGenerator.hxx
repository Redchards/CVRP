#ifndef GENERIC_NEIGHBOURHOOD_GENERATOR_HXX
#define GENERIC_NEIGHBOURHOOD_GENERATOR_HXX

#include <CVRPSolution.hxx>

namespace Heuristic
{

class GenericNeighbourhoodGenerator
{
    private:
    using CVRPSolution = Solver::CVRPSolution;
    
    public:
    /*GenericNeighbourhoodGenerator(const CVRPSolution& solution)
    : initial_{solution}
    {}*/
    GenericNeighbourhoodGenerator() = default;
    
    GenericNeighbourhoodGenerator(const GenericNeighbourhoodGenerator&) = default;
    GenericNeighbourhoodGenerator(GenericNeighbourhoodGenerator&&) = default;
    
    GenericNeighbourhoodGenerator& operator=(const GenericNeighbourhoodGenerator&) = default;
    GenericNeighbourhoodGenerator& operator=(GenericNeighbourhoodGenerator&) = default;
    
    //void setSolution(const CVRPSolution& solution) const noexcept { initial_ = solution; }
    
    CVRPSolution randomNeighbour() const; 
    
    private:
    //CVRPSolution initial_;
};

}

#endif // GENERIC_NEIGHBOURHOOD_GENERATOR_HXX