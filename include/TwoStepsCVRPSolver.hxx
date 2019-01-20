#ifndef TWO_STEPS_CVRP_SOLVER_HXX
#define TWO_STEPS_CVRP_SOLVER_HXX

#include <vector>

#include <CVRPInstance.hxx>
#include <CVRPSolution.hxx>
#include <GenericCVRPSolver.hxx>
#include <GenericRouteAffectationSolver.hxx>
#include <GenericTSPSolver.hxx>

#include <lemon/adaptors.h>

namespace Solver
{
    
template<class FirstPassSolver, class SecondPassSolver>
class TwoStepsCVRPSolver : public GenericCVRPSolver<TwoStepsCVRPSolver<FirstPassSolver, SecondPassSolver>>
{
    private:
    using GraphType = Data::CVRPInstance::GraphType;
    using CVRPInstance = Data::CVRPInstance;
    
    public:
    TwoStepsCVRPSolver(const FirstPassSolver& firstPassSolver, const SecondPassSolver& secondPassSolver)
    : firstPassSolver_{firstPassSolver},
      secondPassSolver_{secondPassSolver}
    {}
    
    CVRPSolution solve(const CVRPInstance& instance)
    {
        RouteAffectationResult firstPassResult = firstPassSolver_.solve(instance);
     
        std::vector<TSPResult> results; 
        
        for(const auto& route : firstPassResult)
        {
            GraphType::NodeMap<bool> nodeFilter(instance.getUnderlyingGraph(), true);
            
            for(const auto& node : route)
            {
                nodeFilter[node] = false;
            }
            
            lemon::FilterNodes<const GraphType> subgraph(instance.getUnderlyingGraph(), nodeFilter);
            
            GraphType tmpGraph(route.size());
            CVRPInstance::CostMap tmpCostMap(tmpGraph);
            
            for(GraphType::NodeIt n1(tmpGraph); n1 != lemon::INVALID; ++n1)
            {
                for(GraphType::NodeIt n2 = n1; n2 != lemon::INVALID; ++n2)
                {
                    if(n1 != n2)
                    {
                        const auto& origNode1 = route[tmpGraph.id(n1)];
                        const auto& origNode2 = route[tmpGraph.id(n2)];
                        tmpCostMap[tmpGraph.edge(n1, n2)] = instance.getCostMap()[instance.getEdge(origNode1, origNode2)];
                    }
                }
            }
            TSPResult tspRes = secondPassSolver_.solve(tmpGraph, tmpCostMap);
            TSPResult partialResult;
            
            for(auto node : tspRes)
            {
                partialResult.push_back(route[tmpGraph.id(node)]);
            }
            
            results.push_back(partialResult);
        }
        
        return {instance, results};
    }
    
    private:
    FirstPassSolver firstPassSolver_;
    SecondPassSolver secondPassSolver_;
};

}

#endif // TWO_STEPS_CVRP_SOLVER_HXX