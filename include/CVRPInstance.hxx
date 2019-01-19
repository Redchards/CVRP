#ifndef CVRPINSTANCE_HXX
#define CVRPINSTANCE_HXX

#include <functional>

#include <lemon/maps.h>
#include <lemon/full_graph.h>

#include <Coordinates.hxx>

namespace Data
{
    
class VehicleData
{
    public:
    explicit VehicleData(size_t vehicleNum, size_t vehicleCapacity)
    : vehicleNum_{vehicleNum},
      vehicleCapacity_{vehicleCapacity}
    {}
    
    VehicleData(const VehicleData&) = default;
    VehicleData(VehicleData&&) = default;
    
    VehicleData& operator=(const VehicleData&) = default;
    VehicleData& operator=(VehicleData&&) = default;
    
    size_t getNumberOfVehicles() const noexcept { return vehicleNum_; }
    size_t getVehicleCapacity() const noexcept { return vehicleCapacity_; }
    void setNumberOfVehicles(size_t vehicleNum) noexcept { vehicleNum_ = vehicleNum; }
    void setvehicleCapacity(size_t vehicleCapacity) noexcept { vehicleCapacity_ = vehicleCapacity; }
    
    private:
    size_t vehicleNum_;
    size_t vehicleCapacity_;
    
};

class CVRPInstance
{
    public:
    using GraphType = lemon::FullGraph;
    using Node = GraphType::Node;
    using Edge = GraphType::Edge;
    
    using CostMap = GraphType::EdgeMap<double>;
    using CostType = CostMap::Value;
    using CostFunctionType = double(Coordinates, Coordinates);
    using DemandMap = GraphType::NodeMap<size_t>;
    using DemandType = size_t;
    using CoordinatesMap = GraphType::NodeMap<Coordinates>;
    using CoordinatesType = Coordinates;
    
    public:
    CVRPInstance(const GraphType& graph,
                 const std::string& name,
                 VehicleData vehicleData,
                 const DemandMap& demandMap, 
                 const CoordinatesMap& coordinatesMap,
                 const std::function<CostFunctionType>& costFunction) noexcept
    : graph_{graph.nodeNum()},
      name_{name},
      vehicleData_{vehicleData}, 
      demandMap_{graph_},
      coordinatesMap_{graph_},
      costMap_{graph_},
      costFunction_{costFunction}
    {
        //lemon::graphCopy(graph, graph_).nodeMap(coordinatesMap, coordinatesMap_).nodeMap(demandMap, demandMap_).run();
        // I wish I could do differently, but EdgeMap's copy constructor is deleted as well as the copy operator
        copyMaps(coordinatesMap, demandMap);
        initializeCostMap(costFunction);
    }
    
    CVRPInstance(const CVRPInstance& other)
    : graph_{other.graph_.nodeNum()},
      name_{other.name_},
      vehicleData_{other.getVehicleData()},
      demandMap_{graph_},
      coordinatesMap_{graph_},
      costMap_{graph_},
      costFunction_{other.costFunction_}
    {        
        //lemon::graphCopy(other.graph_, graph_).nodeMap(other.coordinatesMap_, coordinatesMap_).nodeMap(other.demandMap_, demandMap_).run();
        copyMaps(other.coordinatesMap_, other.demandMap_);
        initializeCostMap(costFunction_);
    }
    
    
    /*CVRPInstance(CVRPInstance&& other)
    : graph_{std::move(other.graph_)},
      name_{std::move(other.name_)},
      demandMap_{std::move(other.demandMap_)},
      coordinatesMap_{std::move(other.coordinatesMap_)},
      costMap_{graph_},
      costFunction_{std::move(other.costFunction_)}
    {
        initializeCostMap(costFunction_);
    }*/
    
    CVRPInstance& operator=(const CVRPInstance& other) = delete;
    CVRPInstance& operator=(CVRPInstance&& other) = delete;
    
    const std::string& getName() const noexcept
    {
        return name_;
    }
    
    void setName(const std::string& name) noexcept
    {
        name_ = name;
    }
    
    size_t getNumberOfVehicles() const noexcept
    {
        return vehicleData_.getNumberOfVehicles();
    }
    
    size_t getVehicleCapacity() const noexcept
    {
        return vehicleData_.getVehicleCapacity();
    }
    
    VehicleData getVehicleData() const noexcept
    {
        return vehicleData_;
    }
    
    CoordinatesType getCoordinatesOf(const Node& node) const noexcept
    {
        return coordinatesMap_[node];
    }
    
    const CoordinatesMap& getCoordinatesMap() const noexcept
    {
        return coordinatesMap_;
    }
    
    DemandType getDemandOf(const Node& node) const noexcept
    {
        return demandMap_[node];
    }
    
    const DemandMap& getDemandMap() const noexcept
    {
        return demandMap_;
    }
    
    CostType getCostOf(const Edge& edge) const noexcept
    {
        return costMap_[edge];
    }
    
    CostType getCostOf(const Node& n1, const Node& n2) const noexcept
    {
        return costMap_[getEdge(n1, n2)];
    }
    
    const CostMap& getCostMap() const noexcept
    {
        return costMap_;
    }
    
    GraphType::NodeIt getNodeIt() const noexcept
    {
        return GraphType::NodeIt{graph_};
    }
    
    GraphType::Node getNode(size_t id) const noexcept
    {
        return graph_(id);
    }
    
    GraphType::EdgeIt getEdgeIt() const noexcept
    {
        return GraphType::EdgeIt{graph_};
    }
    
    Edge getEdge(const Node& n1, const Node& n2) const noexcept
    {
        return graph_.edge(n1, n2);
    }
    
    size_t getNumberOfNodes() const noexcept
    {
        return graph_.nodeNum();
    }
    
    size_t getNumberOfEdges() const noexcept
    {
        return graph_.edgeNum();
    }
    
    auto idOf(const GraphType::Node& node) const noexcept
    {
        return graph_.id(node);
    }
    
    auto idOfDepot() const noexcept
    {
        return 0;
    }
    
    Node getDepotNode() const noexcept
    {
        return getNode(idOfDepot());
    }
    
    const GraphType& getUnderlyingGraph() const noexcept
    {
        return graph_;
    }
    
    private:
    void copyMaps(const CoordinatesMap& coordinatesMap, const DemandMap& demandMap)
    {
        lemon::mapCopy(graph_, coordinatesMap, coordinatesMap_);
        lemon::mapCopy(graph_, demandMap, demandMap_);
    }
    
    void initializeCostMap(const std::function<CostFunctionType>& costFunction)
    {
        for(GraphType::NodeIt n1(graph_); n1 != lemon::INVALID; ++n1)
        {
            for(GraphType::NodeIt n2(graph_); n2 != lemon::INVALID; ++n2)
            {
                if(n1 != n2)
                {
                    costMap_.set(graph_.edge(n1, n2), costFunction(coordinatesMap_[n1], coordinatesMap_[n2]));
                }
            }
        }
    }
    
    private:
    const GraphType graph_;
    std::string name_;
    VehicleData vehicleData_;
    DemandMap demandMap_;
    CoordinatesMap coordinatesMap_;
    CostMap costMap_; // Can't be const due to implementation quirks of LEMON, but should not be modified !!
    std::function<CostFunctionType> costFunction_;
};

}

#endif // CVRPINSTANCE_HXX