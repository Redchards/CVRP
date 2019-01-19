#ifndef MTZ_CUT_HELPER_HXX
#define MTZ_CUT_HELPER_HXX

#include <algorithm>
#include <cmath>

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
        
        return expr >= 1; 
    }
    
    return {};
}


optional<IloRange> MtzAssymmetricUserCut(const IloEnv& env, const Data::CVRPInstance& instance, const std::vector<IloBoolVar>& arcVarArray, const std::vector<double>& arcValueArray)
{
    using GraphType = lemon::FullDigraph;   
    using CapacityMap = GraphType::ArcMap<int>;
    using NodeMap = GraphType::NodeMap<bool>;
    
    static constexpr double REGULARIZATION_FACTOR = 1000.0;
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
        // std::cout << "CUT FOUND ! " << minCutValue << std::endl;
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
            }
        }
        
        return expr >= 1; 
    }
    
    return {};
}
    
}

#endif // MTZ_CUT_HELPER_HXX