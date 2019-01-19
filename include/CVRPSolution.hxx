#ifndef CVRPSOLUTION_HXX
#define CVRPSOLUTION_HXX

#include <vector>

#include <CVRPInstance.hxx>

namespace Solver
{

class CVRPSolution
{
    private:
    using GraphType = Data::CVRPInstance::GraphType;
    using CVRPInstance = Data::CVRPInstance;
    
    public:
    using DataType = std::vector<std::vector<GraphType::Node>>;
    
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
    
    double computeCost() const noexcept
    {
        double totalCost = 0.0;
       
        for(const auto& route : data_) 
        {
            size_t currentDemand = 0; 
           
            auto current = route.begin();
            auto next = current + 1; 
            
            if(current == route.end())
            {
                continue;
            }
            
            totalCost += instance_.getCostOf(instance_.getDepotNode(), *current);
            currentDemand += instance_.getDemandOf(*current);
            
            for(; next != route.end(); ++current, ++next)
            {
                totalCost += instance_.getCostOf(*current, *next);
                currentDemand += instance_.getDemandOf(*current);
            }
            
            totalCost += instance_.getCostOf(*current, instance_.getDepotNode());
            
            if(currentDemand > instance_.getVehicleCapacity())
            {
                totalCost += (currentDemand - instance_.getVehicleCapacity()) * wrongCapacityPenalty_;
            }
        }
        
        return totalCost;
    }
    
    auto begin() const noexcept { return data_.begin(); }
    auto end() const noexcept { return data_.end(); }
    auto cbegin() const noexcept { return data_.cbegin(); }
    auto cend() const noexcept { return data_.cend(); }
    
    auto rbegin() const noexcept { return data_.rbegin(); }
    auto rend() const noexcept { return data_.rend(); }
    auto crbegin() const noexcept { return data_.crbegin(); }
    auto crend() const noexcept { return data_.crend(); }
    
    private:
    const CVRPInstance& instance_;
    DataType data_;
    
    static constexpr double wrongCapacityPenalty_ = 100.0;
};

}

#endif // CVRPSOLUTION_HXX