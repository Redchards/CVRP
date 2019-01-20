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
                if(instance.idOf(n) != instance.idOfDepot() and instance.idOf(n) != instance.idOf(referenceNode)) // !! Change it
            {
                nodes.push_back(n);
            }
        }

        while(!nodes.empty())
        {

            float min = 5000;
            RouteAffectationResult::NodeType n;
            
            for(RouteAffectationResult::NodeType node : nodes)
            {
                float x1 = instance.getCoordinatesOf(referenceNode).x - instance.getCoordinatesOf(depot).x;
                float y1 = instance.getCoordinatesOf(referenceNode).y - instance.getCoordinatesOf(depot).y;
                float x2 = instance.getCoordinatesOf(node).x - instance.getCoordinatesOf(depot).x;
                float y2 = instance.getCoordinatesOf(node).y - instance.getCoordinatesOf(depot).y;
                
                float theta1 = 2 * atan(y1/ (x1 + sqrt(pow(x1, 2) + pow(y1, 2)))) / M_PI;
                float theta2 = 2 * atan(y2/ (x2 + sqrt(pow(x2, 2) + pow(y2, 2)))) / M_PI;
                float radian = theta1 - theta2;
                if (radian < 0)
                {
                    radian = 2 + radian;
                }
                if (min >= radian)
                {
                    min = radian;
                    n = node;
                }
            }       
            nodes.erase(std::find(nodes.begin(), nodes.end(), n));
            ordonateNode.push_back(n);
        }

        // For every node
        for(RouteAffectationResult::NodeType n : ordonateNode)
        {

            if(instance.idOf(n) != instance.idOfDepot())
            {

                int sum = instance.getDemandOf(n);
                for (RouteAffectationResult::NodeType node : affectation.back())
                {
                    sum += instance.getDemandOf(node);
                }
                
                // Test if the demand is superior of capacity
                if (sum <= instance.getVehicleCapacity())
                {
                    affectation.back().push_back(n);
                }
                else
                {
                    affectation.push_back({n});
                }
            }
        }
        
        
        // We have find a solution with a certain number of vehicules
        
        int diff = affectation.size() - instance.getNumberOfVehicles();
        // If they are to many vehicules
        if (diff > 0)
        {
            // For every route not desire
            for (int i=affectation.size()-1; i >= instance.getNumberOfVehicles(); i--)
            {
                std::cout << "enter " << i << std::endl;
                std::vector<RouteAffectationResult::NodeType> supressNodes = std::vector<RouteAffectationResult::NodeType>();
                
                // For every node in the route
                for (RouteAffectationResult::NodeType n : affectation[i])
                {
                    for (int j=0; j < instance.getNumberOfVehicles(); ++j)
                    {
                        int sum = instance.getDemandOf(n);   
                        for (RouteAffectationResult::NodeType node : affectation[j])
                        {
                            sum += instance.getDemandOf(node);
                        }
                        std::cout << sum << std::endl;
                        if (sum <= instance.getVehicleCapacity())
                        {
                            affectation[j].push_back(n);
                            supressNodes.push_back(n);
                            break;
                        }
                        
                    }
                }
                
                for (RouteAffectationResult::NodeType node : supressNodes)
                {
                    affectation[i].erase(std::find(affectation[i].begin(), affectation[i].end(), node));
                }
            }
        }
        diff = affectation.size() - instance.getNumberOfVehicles();
        if (diff > 0)
        {        
            return {affectation, false, this->parameters_};
        }
            // We need to test if it is possible and modify it in the case of not
        
        return {affectation, true, this->parameters_}; // Some modif to do
    }
    

// à toi de jouer
} ;

}
#endif // SWEEP_ROUTE_AFFECTATION_SOLVER_HXX