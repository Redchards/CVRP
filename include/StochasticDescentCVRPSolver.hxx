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
class StochasticDescentCVRPSolver : public GenericCVRPSolver<StochasticDescentCVRPSolver<BaseSolver, Neighbourhoods...>>
{
    private:
    using NeighbourhoodTupleType = std::tuple<Neighbourhoods...>;
    using CVRPInstance = Data::CVRPInstance;
    
    template<class T, size_t first, size_t ... next>
    struct dynamic_get_impl
    {
        auto get(const T& container, size_t index)
        {
            if(first == index) return std::get<first>(container);
            else return dynamic_get_impl<T, next...>{}.get(container, index);
        }
    };
    
    template<class T, size_t last>
    struct dynamic_get_impl<T, last>
    {
        auto get(const T& container, size_t)
        {
            return std::get<last>(container);
        }
    };
    
    template<class T, size_t ... indices>
    static auto dynamic_get_aux(const T& container, size_t index, std::index_sequence<indices...>)
    {
        return dynamic_get_impl<T, indices...>{}.get(container, index);
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
        constexpr auto numberOfNeighbourhoods = std::tuple_size<NeighbourhoodTupleType>::value;
        
        std::random_device en;
        std::mt19937 randomEngine(en());
        std::uniform_int_distribution<unsigned int> distrib(0, numberOfNeighbourhoods - 1);
        
        auto origSol = baseSolver_.solve(instance);
        CVRPSolution::CostProcessor costProcessor;
        auto bestSolData = origSol.getData();
        std::cout << costProcessor.computeCost(instance, bestSolData) << std::endl;
        
        for(size_t i = 0; i < steps_; ++i)
        {
            auto newSolData = dynamic_get(neighbourhoods_, distrib(randomEngine)).randomNeighbour(bestSolData);
            if(costProcessor.computeCost(instance, newSolData) < costProcessor.computeCost(instance, bestSolData))
            {
                bestSolData = newSolData;
                std::cout << "FOUND ! " << std::endl;
            }
        }
        
        std::cout << "Done" << std::endl;
        CVRPSolutionCostProcessor<1000000> finalCostProcessor;
        std::cout << finalCostProcessor.satisfiesConstraints(instance, bestSolData) << std::endl;
        /*while(!finalCostProcessor.satisfiesConstraints(instance, bestSolData))
        {
            auto newSolData = dynamic_get(neighbourhoods_, distrib(randomEngine)).randomNeighbour(bestSolData);
            if(costProcessor.computeCost(instance, newSolData) < costProcessor.computeCost(instance, bestSolData))
            {
                bestSolData = newSolData;
                std::cout << "FOUND ! " << std::endl;
            }
        }*/
        
        return {instance, bestSolData};
    }
    
    private:
    BaseSolver baseSolver_;
    size_t steps_;
    const NeighbourhoodTupleType neighbourhoods_;
};

}

#endif // STOCHASTIC_DESCENT_CVRP_SOLVER_HXX