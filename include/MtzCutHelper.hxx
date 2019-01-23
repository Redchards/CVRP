#ifndef MTZ_CUT_HELPER_HXX
#define MTZ_CUT_HELPER_HXX

#include <algorithm>
#include <cmath>
#include <cstdint>

#include <CVRPInstance.hxx>
#include <Optional.hxx>

#include <lemon/full_graph.h>
#include <lemon/hao_orlin.h>
#include <lemon/nagamochi_ibaraki.h>

#include <ilcplex/ilocplex.h>
#include <ilconcert/iloexpression.h>

namespace CutHelper
{
    
optional<IloRange> MtzSymmetricUserCut(const IloEnv& env, const Data::CVRPInstance& instance, const std::vector<IloBoolVar>& arcVarArray, const std::vector<double>& arcValueArray)
{
    using GraphType = Data::CVRPInstance::GraphType;   
    using CapacityMap = GraphType::EdgeMap<int>;
    using NodeMap = GraphType::NodeMap<bool>;
    
    static constexpr double REGULARIZATION_FACTOR = 1000.0;
    static constexpr double epsilon = 0.01;
    static constexpr size_t cutThreshold = 1;
    
    const GraphType& graph = instance.getUnderlyingGraph();
    
    CapacityMap flowCapacityMap{graph}; 
    
    for(auto edge = instance.getEdgeIt(); edge != lemon::INVALID; ++edge)
    {
        size_t id1 = instance.idOf(graph.u(edge));
        size_t id2 = instance.idOf(graph.v(edge));
        
        auto v1 = arcValueArray[id1 * instance.getNumberOfNodes() + id2 - 1];
        auto v2 = arcValueArray[id2 * instance.getNumberOfNodes() + id1 - 1];
        auto v = std::max(v1, v2);
        
        flowCapacityMap[edge] = v > epsilon ? v * REGULARIZATION_FACTOR : 0.0;

    }
    
    lemon::NagamochiIbaraki<GraphType, CapacityMap> minCutAlgo{graph, flowCapacityMap};
    minCutAlgo.run(); 
    
    NodeMap minCut{graph};
    IloExpr expr{env};
    auto minCutValue = minCutAlgo.minCutMap(minCut) / REGULARIZATION_FACTOR;
    if(minCutValue < (cutThreshold - epsilon))
    {
        std::cout << "CUT FOUND ! " << std::endl;
        std::vector<size_t> firstSet;
        std::vector<size_t> secondSet;
        
        for(auto n = instance.getNodeIt(); n != lemon::INVALID; ++n)
        {
            if(minCut[n] == true) secondSet.push_back(instance.idOf(n));
            else                     firstSet.push_back(instance.idOf(n));
        }
        
        for(auto id1 : firstSet)
        {
            for(auto id2 : secondSet)
            {
                expr += arcVarArray[id1 * instance.getNumberOfNodes() + id2 - 1];
            }
        }
       
        return expr >= static_cast<IloInt>(cutThreshold);
    }
    
    return {};
}


optional<IloRange> MtzAssymmetricUserCut(const IloEnv& env, const Data::CVRPInstance& instance, const std::vector<IloBoolVar>& arcVarArray, const std::vector<double>& arcValueArray)
{
    using GraphType = lemon::FullDigraph;   
    using CapacityMap = GraphType::ArcMap<int>;
    using NodeMap = GraphType::NodeMap<bool>;
    
    static constexpr double REGULARIZATION_FACTOR = 1000000.0;
    static constexpr double epsilon = 0.01;
    static constexpr size_t cutThreshold = 1;
    
    const GraphType& graph{static_cast<int>(instance.getNumberOfNodes())};
    
    CapacityMap flowCapacityMap{graph}; 
    
    for(GraphType::ArcIt arc{graph}; arc != lemon::INVALID; ++arc)
    {
        size_t id1 = graph.id(graph.source(arc));
        size_t id2 = graph.id(graph.target(arc));
        
        auto v = arcValueArray[id1 * instance.getNumberOfNodes() + id2 - 1];
        
        flowCapacityMap[arc] = v > epsilon ? v * REGULARIZATION_FACTOR : 0.0;
    }
    
    lemon::HaoOrlin<GraphType, CapacityMap> minCutAlgo{graph, flowCapacityMap};
    minCutAlgo.run(); 
    
    NodeMap minCut{graph};
    IloExpr expr{env};
    auto minCutValue = minCutAlgo.minCutMap(minCut) / REGULARIZATION_FACTOR;
    // std::cout << minCutValue << std::endl;
    if(minCutValue < (cutThreshold - epsilon))
    {
        std::cout << "CUT FOUND ! " << minCutValue << " " << cutThreshold << std::endl;
        std::vector<size_t> firstSet;
        std::vector<size_t> secondSet;
        
        for(GraphType::NodeIt n{graph}; n != lemon::INVALID; ++n)
        {
            if(minCut[n] == true) secondSet.push_back(graph.id(n));
            else                     firstSet.push_back(graph.id(n));
        }
        
        for(auto id1 : firstSet)
        {
            for(auto id2 : secondSet)
            {
                expr += arcVarArray[id1 * instance.getNumberOfNodes() + id2 - 1];
                // std::cout << arcValueArray[id1 * instance.getNumberOfNodes() + id2 - 1] << " " << std::endl;
            }
        }
        
        /*std::cout << "First Set" << std::endl;
        for(auto id1 : firstSet)
        {
            std::cout << id1 << " ";
        }
        std::cout << std::endl << "Second Set" << std::endl;
        
        for(auto id2 : secondSet)
        {
            std::cout << id2 << " ";
        }
        std::cout << std::endl;*/
        
        return expr >= static_cast<IloInt>(cutThreshold); 
    }
    
    return {};
}

IloNumArray MtzPrimalHeuristic(const IloEnv& env, const Data::CVRPInstance& instance, const std::vector<IloBoolVar>& arcVarArray, const std::vector<double>& arcValueArray)
{
    IloNumArray resArray(env, arcVarArray.size());
    size_t offset = 1;
    bool offsetUpdated = false;
    for(size_t routeId = 0; routeId < instance.getNumberOfVehicles(); ++routeId)
    {
        //std::cout << routeId << std::endl;
        size_t i = 0;
        size_t currentBestVal = 0;
        size_t currentBestNode = 0;       
        size_t currentDemand = 0;
        offsetUpdated = false;
            
        bool done = false;
        while(!done)
        {
            if(offset >= instance.getNumberOfVehicles())
            {
                break;
            }
            
            for(size_t j = offset; j < instance.getNumberOfNodes(); ++j)
            {
                if(i == j) continue;
                
                auto currentVal = arcValueArray[i * instance.getNumberOfNodes() + j - 1];
                
                // std::cout << instance.getVehicleCapacity() << " : " << currentDemand << std::endl;
                if(currentDemand > instance.getVehicleCapacity())
                {
                    //std::cout << "True" << std::endl;
                    resArray[i * instance.getNumberOfNodes() - 1] = 1;
                    done = true;
                    break;
                }
                
                if(currentVal > currentBestVal)
                {
                    currentBestVal = currentVal;
                    currentBestNode = j;
                }
            }                
            
            if(i == 0 && currentBestNode == 0)
            {
                currentBestNode = 1;
            }
            
            if(!done)
            {
                resArray[i * instance.getNumberOfNodes() + currentBestNode - 1] = 1;
                currentDemand += instance.getDemandOf(instance.getNode(currentBestNode));
                i = currentBestNode;
            }
            
            if(!offsetUpdated) 
            {
                offset = currentBestNode + 1;
            }
        }
 
    }
    
    return resArray;
    
}
    
}

#endif // MTZ_CUT_HELPER_HXX