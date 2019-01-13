#ifndef STOCHASTIC_DESCENT_CVRP_SOLVER_HXX
#define STOCHASTIC_DESCENT_CVRP_SOLVER_HXX

#include <random>
#include <tuple>
#include <type_traits>

#include <CVRPInstance.hxx>
#include <CVRPSolution.hxx>
#include <GenericCVRPSolver.hxx>

namespace Solver
{
    
template<class BaseSolver, class ... Neighbourhoods>
class StochasticDescentCVRPSolver : public GenericCVRPSolver<StochasticDescentCVRPSolver<BaseSolver, Neighbourhoods>>
{
    private:
    using NeighbourhoodTupleType = std::tuple<Neighbourhoods...>;
    
    template<class T, size_t first, size_t ... next>
    static auto dynamic_get_impl(const T& container, size_t index)
    {
        if(first == index) return std::get<first>(container);
        else return dynamic_get_impl<T, next...>(container, index);
    }
    
    template<class T, size_t ... indices>
    static auto dynamic_get_aux(const T&, container, size_t index, std::index_sequence<indices...>)
    {
        return dynamic_get_impl(container, index);
    }
    
    template<class T>
    static auto dynamic_get(const T& container, size_t index)
    {
        return dynamic_get_aux(container, index, std::make_index_sequence<std::tuple_size<T>::value>());
    }
    
    public:
    StochasticDescentCVRPSolver(const BaseSolver& baseSolver, size_t steps) 
    : GenericCVRPSolver<StochasticDescentCVRPSolver>(),
      baseSolver_{baseSolver},
      steps_{steps},
      neighbourhoods_{}
    {}
    
    CVRPSolution solve(const CVRPInstance& instance)
    {
        constexpr numberOfNeighbourhoods = std::tuple_size<NeighbourhoodTupleType>::value;
        
        std::random_engine en;
        std::default_random_engine randomEngine(en);
        std::uniform_int_distribution<unsigned int> distrib(0, numberOfNeighbourhoods - 1);
        
        auto origSol = baseSolver_.solve(instance);
        
        for(size_t i = 0; i < steps_; ++i)
        {
            auto newSol =  dynamic_get(neighbourhoods_, distrib(randomEngine)).randomNeighbour();
        }
    }
    
    private:
    const BaseSolver baseSolver_;
    const NeighbourhoodTupleType neighbourhoods_;
    size_t steps_;
};

}

#endif // STOCHASTIC_DESCENT_CVRP_SOLVER_HXX