#ifndef CVRPSOLUTION_HXX
#define CVRPSOLUTION_HXX

#include <vector>

#include <CVRPInstance.hxx>

namespace Solver
{
    
class CVRPSolution; 

using CVRPSolutionData = std::vector<std::vector<Data::CVRPInstance::GraphType::Node>>;
    
template<size_t wrongCapacityPenalty>
class CVRPSolutionCostProcessor
{
    public:
    
    double computeCost(const Data::CVRPInstance& instance, const CVRPSolutionData& data) const noexcept
    {
        double totalCost = 0.0;
       
        for(const auto& route : data) 
        {
            size_t currentDemand = 0; 
           
            auto current = route.begin();
            auto next = current + 1; 
            
            if(current == route.end())
            {
                continue;
            }
            
            totalCost += instance.getCostOf(instance.getDepotNode(), *current);
            
            for(; next != route.end(); ++current, ++next)
            {
                totalCost += instance.getCostOf(*current, *next);
                currentDemand += instance.getDemandOf(*current);
            }
            
            currentDemand += instance.getDemandOf(*current);
            
            totalCost += instance.getCostOf(*current, instance.getDepotNode());
            
            if(currentDemand > instance.getVehicleCapacity())
            {
                totalCost += (currentDemand - instance.getVehicleCapacity()) * wrongCapacityPenalty;
            }   
            
            /* std::cout << currentDemand << std::endl;
            size_t actual = 0;
            for(auto node : route)
            {
                std::cout << instance.idOf(node) << " ";
                actual += instance.getDemandOf(node);
            }
            std::cout << std::endl << actual << std::endl; */
        }
        
        return totalCost;
    }
 
    bool satisfiesConstraints(const Data::CVRPInstance& instance, const CVRPSolutionData& data) const noexcept
    {
        if(data.size() > instance.getNumberOfVehicles())
        {
            return false;
        }
        
        for(const auto& route : data)
        {
            size_t currentDemand = 0;
            for(const auto& node : route)
            {
                currentDemand += instance.getDemandOf(node);
                
                if(currentDemand > instance.getVehicleCapacity())
                {
                    return false;
                }
            }
        }
        
        return true;
    }
};

class CVRPSolution
{
    private:
    using GraphType = Data::CVRPInstance::GraphType;
    using CVRPInstance = Data::CVRPInstance;
    
    public:
    using DataType = CVRPSolutionData; 
    
    public:
    CVRPSolution(const CVRPInstance& instance, const DataType& data)
    : instance_{instance},
      data_{data}
    {}
    
    CVRPSolution(const CVRPSolution&) = default;
    CVRPSolution(CVRPSolution&&) = default;
    
    CVRPSolution& operator=(const CVRPSolution&) = delete;
    CVRPSolution& operator=(CVRPSolution&&) = delete;
    
    const CVRPInstance& getOriginalInstance() const noexcept { return instance_; }
    const DataType& getData() const noexcept { return data_; }
    size_t getNumberOfRoutes() const noexcept { return data_.size(); }
    
    const std::vector<GraphType::Node>& getRoute(size_t idx) const
    {
        if(idx >= data_.size())
        {
            throw std::out_of_range(std::string{"Trying to access an out of range affectation ! Index : "} + std::to_string(idx));
        }
        
        return data_[idx];
    }
    
    double computeCost() const noexcept
    {
        return costProcessor_.computeCost(instance_, data_);
    } 
    
    auto begin() const noexcept { return data_.begin(); }
    auto end() const noexcept { return data_.end(); }
    auto cbegin() const noexcept { return data_.cbegin(); }
    auto cend() const noexcept { return data_.cend(); }
    
    auto rbegin() const noexcept { return data_.rbegin(); }
    auto rend() const noexcept { return data_.rend(); }
    auto crbegin() const noexcept { return data_.crbegin(); }
    auto crend() const noexcept { return data_.crend(); }
    
    protected:
    const CVRPInstance instance_;
    DataType data_;
    
    static constexpr size_t wrongCapacityPenalty_ = 1000;
    
    public:
    using CostProcessor = CVRPSolutionCostProcessor<wrongCapacityPenalty_>;
    
    private:
    CVRPSolutionCostProcessor<wrongCapacityPenalty_> costProcessor_;
};

}

#endif // CVRPSOLUTION_HXX