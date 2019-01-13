#ifndef ROUTE_AFFECTATION_SOLVER_HXX
#define ROUTE_AFFECTATION_SOLVER_HXX

#include <vector>

#include <CVRPInstance.hxx>
#include <GenericBinPackingSolver.hxx>

namespace Solver
{

class RouteAffectationParameters
{
    private:
    using VehicleData = Data::VehicleData;
    using CVRPInstance = Data::CVRPInstance;
    
    public:
    RouteAffectationParameters(size_t vehicleNum, size_t vehicleCapacity)
    : vehicleData_{vehicleNum, vehicleCapacity}
    {}
    
    RouteAffectationParameters(VehicleData vehicleData)
    : vehicleData_{vehicleData}
    {}
    
    RouteAffectationParameters(const CVRPInstance& instance)
    : vehicleData_{instance.getVehicleData()}
    {}
    
    RouteAffectationParameters(const RouteAffectationParameters&) = default;
    RouteAffectationParameters(RouteAffectationParameters&&) = default;
    
    RouteAffectationParameters& operator=(const RouteAffectationParameters&) = delete;
    RouteAffectationParameters& operator=(RouteAffectationParameters&&) = delete;
    
    size_t getVehicleCapacity() const noexcept { return vehicleData_.getVehicleCapacity(); }
    size_t getNumberOfVehicles() const noexcept { return vehicleData_.getNumberOfVehicles(); }
    
    private:
    const VehicleData vehicleData_;
    
};

class RouteAffectationResult
{
    public:
    using NodeType = Data::CVRPInstance::GraphType::Node;
    using DataType = std::vector<std::vector<NodeType>>;
    
    public:
    RouteAffectationResult(const DataType& routeAffectation, bool solvable, const RouteAffectationParameters& parameters) noexcept
    : routeAffectation_{routeAffectation},
      solvable_{solvable},
      parameters_{parameters}
    {}
    
    RouteAffectationResult(const RouteAffectationResult&) = default;
    RouteAffectationResult(RouteAffectationResult&&) = default;
    
    RouteAffectationResult& operator=(const RouteAffectationResult&) = delete;
    RouteAffectationResult& operator=(RouteAffectationResult&&) = delete;
    
    const DataType& getRouteAffectations() const noexcept
    {
        return routeAffectation_;
    }
    
    const std::vector<NodeType>& getRouteAffectationFor(size_t idx) const
    {
        if(idx >= routeAffectation_.size())
        {
            throw std::out_of_range(std::string{"Trying to access an out of range affectation ! Index : "} + std::to_string(idx));
        }
        
        return routeAffectation_[idx];
    }
    
    bool isSolvable() const noexcept
    {
        return solvable_;
    }
    
    const RouteAffectationParameters& getParameters() const noexcept
    {
        return parameters_;
    }
    
    auto begin() const noexcept { return routeAffectation_.begin(); }
    auto end() const noexcept { return routeAffectation_.end(); }
    auto cbegin() const noexcept { return routeAffectation_.cbegin(); }
    auto cend() const noexcept { return routeAffectation_.cend(); }
    
    auto rbegin() const noexcept { return routeAffectation_.rbegin(); }
    auto rend() const noexcept { return routeAffectation_.rend(); }
    auto crbegin() const noexcept { return routeAffectation_.crbegin(); }
    auto crend() const noexcept { return routeAffectation_.crend(); }
    
    private:
    const DataType routeAffectation_;
    const bool solvable_;
    const RouteAffectationParameters parameters_;
    
};

template<class Derived>
class GenericRouteAffectationSolver
{
    protected:
    using CVRPInstance = Data::CVRPInstance;
    
    template<class T, class = void>
    struct is_solve_implemented_in : std::false_type {};
    
    template<class T>
    struct is_solve_implemented_in<T, Meta::void_t<decltype(std::declval<std::decay_t<T>>().solve(std::declval<CVRPInstance>()))>> : std::true_type {};
    
    public:
    GenericRouteAffectationSolver(const RouteAffectationParameters& parameters) noexcept
    : parameters_{parameters}
    {
        static_assert(is_solve_implemented_in<Derived>::value, "Invalid derived class : must implement the method 'solve'");
    }
    
    GenericRouteAffectationSolver(const GenericRouteAffectationSolver&) = default;
    GenericRouteAffectationSolver(GenericRouteAffectationSolver&&) = default;
    
    GenericRouteAffectationSolver& operator=(const GenericRouteAffectationSolver&) = default;
    GenericRouteAffectationSolver& operator=(GenericRouteAffectationSolver&&) = default;
    
    RouteAffectationResult solve(const CVRPInstance&);
    
    public:
    RouteAffectationParameters parameters_;
};

template<class BinPackingSolver>
class RouteAffectationBinPackingAdaptor : public GenericRouteAffectationSolver<RouteAffectationBinPackingAdaptor<BinPackingSolver>>
{
    private:
    using CVRPInstance = Data::CVRPInstance;
    
    public:
    using GenericRouteAffectationSolver<RouteAffectationBinPackingAdaptor<BinPackingSolver>>::GenericRouteAffectationSolver;
    
    RouteAffectationResult solve(const CVRPInstance& instance)
    {
        std::vector<size_t> binParams(this->parameters_.getNumberOfVehicles(), this->parameters_.getVehicleCapacity());
        BinPackingParameters params(binParams);
        std::vector<size_t> items(instance.getNumberOfNodes() - 1);
        
        
        for(auto n = instance.getNodeIt(); n != lemon::INVALID; ++n)
        {
            if(instance.idOf(n) != instance.idOfDepot())
            {
                items[instance.idOf(n) - 1] = instance.getDemandOf(n);
            }
        }
        
        BinPackingSolver solver{params};
        BinPackingResults res = solver.solve(items);
        RouteAffectationResult::DataType affectation;
        
        for(auto bin : res)
        {
            affectation.push_back({});
            for(auto id : bin)
            {
                affectation.back().push_back(instance.getNode(id + 1));
            }
        }
        
        return {affectation, res.isSolvable(), this->parameters_};
    }
};

}

#endif // ROUTE_AFFECTATION_SOLVER_HXX