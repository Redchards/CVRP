#ifndef GENERIC_CVRP_SOLVER_HXX
#define GENERIC_CVRP_SOLVER_HXX

#include <CVRPInstance.hxx>
#include <CVRPSolution.hxx>
#include <Meta.hxx>
#include <Optional.hxx>

namespace Solver
{
    
template<class Derived>
class GenericCVRPSolver
{
    protected:
    using CVRPInstance = Data::CVRPInstance;
    
    template<class T, class = void>
    struct is_solve_implemented_in : std::false_type {};
    
    template<class T>
    struct is_solve_implemented_in<T, Meta::void_t<decltype(std::declval<std::decay_t<T>>().solve(std::declval<CVRPInstance>()))>> : std::true_type {};
    
    public:
    GenericCVRPSolver()
    {
        static_assert(is_solve_implemented_in<Derived>::value, "Invalid derived class : must implement the method 'solve'");
    }
    
    CVRPSolution solve(const CVRPInstance& instance);
};

}

#endif // GENERIC_CVRP_SOLVER_HXX