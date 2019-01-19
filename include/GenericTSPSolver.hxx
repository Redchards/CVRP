#ifndef GENERIC_TSP_SOLVER_HXX
#define GENERIC_TSP_SOLVER_HXX

#include <vector>

#include <CVRPInstance.hxx>
#include <Meta.hxx>

#include <lemon/christofides_tsp.h>
#include <lemon/greedy_tsp.h>
#include <lemon/insertion_tsp.h>
#include <lemon/nearest_neighbor_tsp.h>
#include <lemon/opt2_tsp.h>

namespace Solver
{
    
using TSPResult = std::vector<Data::CVRPInstance::GraphType::Node>;

template<class Derived>
class GenericTSPSolver
{
    protected:
    using CVRPInstance = Data::CVRPInstance;
    using CostMap = CVRPInstance::CostMap;
    
    private:
    template<class T, class = void>
    struct is_solve_implemented_in : std::false_type {};
    
    template<class T>
    struct is_solve_implemented_in<T, Meta::void_t<decltype(std::declval<std::decay_t<T>>().solve(std::declval<CVRPInstance::GraphType>(), std::declval<CostMap>()))>> : std::true_type {};
  
    public:
    GenericTSPSolver()
    {
        static_assert(is_solve_implemented_in<Derived>::value, "Invalid derived class : must implement the method 'solve'");
    }
    
    TSPResult solve(const CVRPInstance::GraphType&, const CostMap&);
};

template<class Solver>
class LemonTSPSolverAdaptor : GenericTSPSolver<LemonTSPSolverAdaptor<Solver>>
{
    private:
    using CVRPInstance = Data::CVRPInstance;
    using CostMap = CVRPInstance::CostMap;
    
    public:
    using GenericTSPSolver<LemonTSPSolverAdaptor>::GenericTSPSolver;
    
    TSPResult solve(const CVRPInstance::GraphType& graph, const CostMap& costMap)
    {
        Solver solver(graph, costMap);
        solver.run();
        // std::cout << solver.tourCost() << std::endl;
        
        return solver.tourNodes();
    }
};

using ChristofidesTSPSolver = LemonTSPSolverAdaptor<lemon::ChristofidesTsp<Data::CVRPInstance::CostMap>>;
using GreedyTSPSolver = LemonTSPSolverAdaptor<lemon::GreedyTsp<Data::CVRPInstance::CostMap>>;
using InsertionTSPSolver = LemonTSPSolverAdaptor<lemon::InsertionTsp<Data::CVRPInstance::CostMap>>;
using NearestNeighbourTSPSolver = LemonTSPSolverAdaptor<lemon::NearestNeighborTsp<Data::CVRPInstance::CostMap>>;
using TwoOptTSPSolver = LemonTSPSolverAdaptor<lemon::Opt2Tsp<Data::CVRPInstance::CostMap>>;

}

#endif // GENERIC_TSP_SOLVER_HXX