#ifndef GENERIC_TVRP_SOLVER_HXX
#define GENERIC_TVRP_SOLVER_HXX

#include <TVRPInstance.hxx>
#include <TVRPSolution.hxx>
#include <Meta.hxx>
#include <Optional.hxx>

namespace Solver
{
    
template<class Derived>
class GenericTVRPSolver
{
    protected:
    using TVRPInstance = Data::TVRPInstance;
    
    template<class T, class = void>
    struct is_solve_implemented_in : std::false_type {};
    
    template<class T>
    struct is_solve_implemented_in<T, Meta::void_t<decltype(std::declval<std::decay_t<T>>().solve(std::declval<TVRPInstance>()))>> : std::true_type {};
    
    public:
    GenericTVRPSolver()
    {
        static_assert(is_solve_implemented_in<Derived>::value, "Invalid derived class : must implement the method 'solve'");
    }
    
    TVRPSolution solve(const TVRPInstance& instance);
};

}

#endif // GENERIC_TVRP_SOLVER_HXX