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
        
        // Depot node
        RouteAffectationResult::NodeType depot = instance.getNode(instance.idOfDepot());
        RouteAffectationResult::NodeType referenceNode = instance.getNode(1); // !! We have to verify if it exist
        
        // Create the affectation data
        RouteAffectationResult::DataType affectation; // std::vector<std::vector<NodeType>>
        affectation.push_back({});
        
        std::vector<RouteAffectationResult::NodeType> ordonateNode = std::vector<RouteAffectationResult::NodeType>();
        ordonateNode.push_back(referenceNode);
        
        std::vector<RouteAffectationResult::NodeType> nodes = std::vector<RouteAffectationResult::NodeType>();
        
        for(auto n = instance.getNodeIt(); n != lemon::INVALID; ++n)
        {
                if(instance.idOf(n) != instance.idOfDepot() and instance.idOf(n) != instance.idOfDepot() + 1) // !! Change it
            {
                nodes.push_back(n);
            }
        }

        while(!nodes.empty())
        {

            float min = 360;
            RouteAffectationResult::NodeType n;
            
            for(RouteAffectationResult::NodeType node : nodes)
            {
                // calcul du radian
                float a = sqrt(pow(instance.getCoordinatesOf(referenceNode).x - instance.getCoordinatesOf(node).x, 2) + pow(instance.getCoordinatesOf(referenceNode).y - instance.getCoordinatesOf(node).y, 2));
                float b = sqrt(pow(instance.getCoordinatesOf(referenceNode).x - instance.getCoordinatesOf(depot).x, 2) + pow(instance.getCoordinatesOf(referenceNode).y - instance.getCoordinatesOf(depot).y, 2));
                float c = sqrt(pow(instance.getCoordinatesOf(depot).x - instance.getCoordinatesOf(node).x, 2) + pow(instance.getCoordinatesOf(depot).y - instance.getCoordinatesOf(node).y, 2));
                
                float radian = acos((pow(a, 2) - pow(b, 2) - pow(c, 2)) / (2 * b * c));
                int angle = ((int) radian * 180 / PI) % 360 ;
                if (min >= radian)
                {
                    min = radian;
                    n = node;
                }
            }       
            nodes.erase(std::find(nodes.begin(), nodes.end(), n));
            ordonateNode.push_back(n);
        }

        for(auto& node : ordonateNode)
        {
            std::cout << instance.idOf(node) << std::endl;
        }
        // For every node
        for(RouteAffectationResult::NodeType n : ordonateNode)
        {

            if(instance.idOf(n) != instance.idOfDepot() || instance.idOf(n) != instance.idOf(referenceNode))
            {

                int sum = instance.getDemandOf(n);
                for (RouteAffectationResult::NodeType node : affectation.back())
                {
                    sum += instance.getDemandOf(node);
                }
                
                // Test if the demand is superior of capacity
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
        //

        return {affectation, true, this->parameters_}; // Some modif to do
    }
    

// à toi de jouer
} ;

}
#endif // SWEEP_ROUTE_AFFECTATION_SOLVER_HXX