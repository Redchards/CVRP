#ifndef SWEEP_ROUTE_AFFECTATION_SOLVER_HXX
#define SWEEP_ROUTE_AFFECTATION_SOLVER_HXX

#include <algorithm>
#include <functional>
#include <vector>
#include <math.h>

#include <GenericRouteAffectationSolver.hxx>

namespace Solver
{
    
class SweepRouteAffectationSolver : public GenericRouteAffectationSolver<SweepRouteAffectationSolver>
{
    public:
    using GenericRouteAffectationSolver<SweepRouteAffectationSolver>::GenericRouteAffectationSolver;

    RouteAffectationResult solve(const CVRPInstance& instance)
    {
        std::vector<size_t> binParams(this->parameters_.getNumberOfVehicles(), this->parameters_.getVehicleCapacity());
        BinPackingParameters params(binParams);
        std::vector<size_t> items(instance.getNumberOfNodes() - 1);
        
        \\ Depot node
        NodeType depot = instance.idOfDepot();
        NodeType referenceNode = instance.idOfDepot()+1; \\ !! We have to verify if it exist
        
        \\ Create the affectation data
        RouteAffectationResult::DataType affectation; \\ std::vector<std::vector<NodeType>>
        
        
        std::vector<NodeType> ordonateNode = std::vector<NodeType>();
        ordonateNode.push_back(referenceNode);
        
        std::vector<NodeType> nodes = std::vector<NodeType>();
        
        for(auto n = instance.getNodeIt(); n != lemon::INVALID; ++n)
        {
                if(instance.idOf(n) != instance.idOfDepot() or instance.idOf(n) != instance.idOfDepot() + 1) \\ !! Change it
            {
                nodes.push-back(n)
            }
        }
        
        while(!v.empty())
        {

            float min = 5;
            NodeType n;
            
            for(NodeType node : nodes)
            {
                \\ calcul du radian
                float a = sqrt(pow(instance.getCoordinatesOf(referenceNode).x - instance.getCoordinatesOf(node).x, 2) + pow(instance.getCoordinatesOf(referenceNode).y - instance.getCoordinatesOf(node).y, 2));
                float b = sqrt(pow(instance.getCoordinatesOf(referenceNode).x - instance.getCoordinatesOf(depot).x, 2) + pow(instance.getCoordinatesOf(referenceNode).y - instance.getCoordinatesOf(depot).y, 2));
                float c = sqrt(pow(instance.getCoordinatesOf(depot).x - instance.getCoordinatesOf(node).x, 2) + pow(instance.getCoordinatesOf(depot).y - instance.getCoordinatesOf(node).y, 2));
                
                float radian = acos((pow(a, 2) - pow(b, 2) - pow(c, 2)) / (2 * b * c));
                if (min >= radian)
                {
                    min = radian;
                    n = node;
                }
            }       
            nodes.erase(n);
            ordonateNode.push_back(n)
        }
        
        
        \\ For every node
        for(NodeType n : ordonateNode)
        {
            if(instance.idOf(n) != instance.idOfDepot())
            {
                int sum = instance.getDemandOf(n);
                for (NodeType node : affectation.back())
                {
                    sum += instance.getDemandOf(node);
                }
                
                \\ Test if the demand is superior of capacity
                if (sum < instance.getVehicleCapacity())
                {
                    affectation.back().push_back(n);
                }
                else
                {
                    affectation.push_back({n});
                }
            }
        }

        
        return {affectation, True, this->parameters_}; \\ Some modif to do
    }
    
    bool functionTrie (NodeType n1, NodeType n2) { return (<j); }

// à toi de jouer
} ;

}
#endif // SWEEP_ROUTE_AFFECTATION_SOLVER_HXX