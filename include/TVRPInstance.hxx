#ifndef TVRP_INSTANCE_HXX
#define TVRP_INSTANCE_HXX

#include <vector>

#include <CVRPInstance.hxx>

namespace Data
{
    
class TechnicianData
{
    public:
    TechnicianData(const std::vector<std::vector<bool>>& data)
    : data_{data},
      numberOfSkills_{data[0].size()}
    {} 
    
    TechnicianData(const TechnicianData&) = default;
    TechnicianData(TechnicianData&&) = default;
    
    TechnicianData& operator=(const TechnicianData&) = default;
    TechnicianData& operator=(TechnicianData&&) = default;
    
    const std::vector<bool> getSkillsOf(size_t idx) const
    {
        if(idx >= data_.size())
        {
            throw std::out_of_range(std::string{"Trying to access an out of range technician ! Index : "} + std::to_string(idx));
        }
        
        return data_[idx];
    }
    
    bool hasSkill(size_t technicianIdx, size_t skillIdx) const
    {
        const auto& skillSet = getSkillsOf(technicianIdx);
        
        if(skillIdx >= skillSet.size())
        {
            throw std::out_of_range(std::string{"Trying to access an out of range skill ! Index : "} + std::to_string(skillIdx));
        }
        
        return data_[technicianIdx][skillIdx];
    }
    
    size_t getNumberOfSkills() const noexcept
    {
        return numberOfSkills_;
    }
    
    size_t getNumberOfTechnicians() const noexcept
    {
        return data_.size();
    }
    
    private:
    std::vector<std::vector<bool>> data_;
    size_t numberOfSkills_;
};
    
class TVRPInstance : public CVRPInstance
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
    using SkillMap = GraphType::NodeMap<std::vector<bool>>;
    using SkillType = size_t;
    using CoordinatesMap = GraphType::NodeMap<Coordinates>;
    using CoordinatesType = Coordinates;
    
    public: 
    TVRPInstance(const GraphType& graph,
                 const std::string& name,
                 VehicleData vehicleData,
                 TechnicianData technicianData,
                 const DemandMap& demandMap, 
                 const SkillMap& skillMap,
                 const CoordinatesMap& coordinatesMap,
                 const std::function<CostFunctionType>& costFunction) noexcept
    : CVRPInstance(graph, name, vehicleData, demandMap, coordinatesMap, costFunction),
      technicianData_{technicianData},
      skillMap_{graph_}
    {  
        copyMaps(skillMap);
    }
    
    TVRPInstance(const TVRPInstance& other)
    : CVRPInstance(other),
      technicianData_{other.technicianData_},
      skillMap_{graph_}
    {
        copyMaps(other.skillMap_);
    }
    
    bool canServe(size_t technicianIdx, const Node& node) const
    {
        bool res = true;
        const auto& requiredSkillset = skillMap_[node];
        
        for(size_t i = 0; i < requiredSkillset.size(); ++i)
        {
            if(requiredSkillset[i])
            {
                res &= technicianData_.hasSkill(technicianIdx, i);
            }
        }
        
        return res;
    }
    
    const std::vector<bool>& getSkillset(const Node& node) const noexcept
    {
        return skillMap_[node];
    }
    
    size_t getNumberOfTechnicians() const noexcept
    {
        return technicianData_.getNumberOfTechnicians();
    }
    
    private:
    void copyMaps(const SkillMap& skillMap)
    {
        lemon::mapCopy(graph_, skillMap, skillMap_);
    }
    
    protected:
    TechnicianData technicianData_;
    SkillMap skillMap_;
};

}

#endif // TVRP_INSTANCE_HXX