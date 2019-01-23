#ifndef CUT_HELPER_HXX
#define CUT_HELPER_HXX

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include <CVRPInstance.hxx>

#include <ilcplex/ilocplex.h>
#include <ilconcert/iloexpression.h>

namespace CutHelper
{

std::vector<IloRange> integerCutHelper(const IloEnv& env, const Data::CVRPInstance& instance, const std::vector<std::vector<IloBoolVar>> edgeVarArray, const std::vector<std::vector<uint8_t>> edgeValueArray)
{
    static constexpr double epsilon = 0.00001;
    std::vector<IloRange> res;
    
    std::vector<size_t> visitedNodes;
    std::vector<std::vector<size_t>> invalidTours;
    for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
    {
        size_t start = -1;
        size_t offset = 1;
        size_t current = i;
        size_t currentCapacity = 0;
        std::vector<size_t> tour;
        visitedNodes.push_back(i);
        
        for(size_t j = offset; j < instance.getNumberOfNodes(); ++j)
        {
            if(current == j) continue;
            if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
            if(edgeValueArray[std::min(current, j)][std::max(current, j)]) == 1)
            {
                currentCapacity += instance.getCostOf(instance.getNode(current), instance.getNode(j));
                if(j == start)
                {
                    j = offset;    
                    current = i;
                    start = -1;
                    
                    if(std::find(tour.begin(), tour.end(), 0) == tour.end() || (currentCapacity > instance.getVehicleCapacity()))
                    {
                        invalidTours.push_back(std::move(tour));
                    }
                    tour = {};
                    continue;
                }
                if(start == -1) 
                {
                    start = i;
                    offset = j + 1;
                    tour.push_back(i);
                }
                
                visitedNodes.push_back(j);
                tour.push_back(j);
                
                current = j;
                j = 0;
            }
        }
        
        for(const auto& tour : invalidTours)
        {
            size_t tourDemand = 0;
            IloExpr tourExpr(env);
            
            for(auto idx : tour)
            {
                tourDemand += instance.getDemandOf(instance.getNode(idx));
            }
            
            std::vector<size_t> outset;
            for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
            {
                if(std::find(tour.begin(), tour.end(), j) == tour.end()) 
                {
                    outset.push_back(j);
                }
            }
            
            for(auto source : tour)
            {
                for(auto destination : outset) 
                {
                    tourExpr += edgeVarArray[std::min(source, destination)][std::max(source, destination)];
                }
            }
            
            res.push_back(tourExpr >= std::ceil(turnDemand / instance.getVehicleCapacity()));
        }
        
        return res;
    }
}

}

#endif // CUT_HELPER_HXX