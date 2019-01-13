#ifndef GENERIC_BIN_PACKING_SOLVER_HXX
#define GENERIC_BIN_PACKING_SOLVER_HXX

#include <Meta.hxx>

// #include <cstddef>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>


namespace Solver
{

class BinPackingParameters
{
    public:
    BinPackingParameters(const std::vector<size_t>& binParams) noexcept
    : binParams_{binParams}
    {}
    
    BinPackingParameters(const BinPackingParameters& other) = default;
    BinPackingParameters(BinPackingParameters&&) = default;
    
    BinPackingParameters& operator=(const BinPackingParameters&) = delete;
    BinPackingParameters& operator=(BinPackingParameters&&) = delete;
    
    size_t getBinCapacity(size_t idx)
    {
        if(idx >= binParams_.size())
        {
            throw std::out_of_range(std::string{"Trying to access an out of range bin ! Index : "} + std::to_string(idx));
        }
        
        return binParams_[idx];
    }
    
    private:
    
    const std::vector<size_t> binParams_;
    
    public: 
    
    const typename std::add_lvalue_reference<decltype(binParams_)>::type getUnderlying() noexcept { return binParams_; }
    size_t getNumberOfBins() noexcept { return binParams_.size(); }
    
    auto begin() const noexcept { return binParams_.begin(); }
    auto end() const noexcept { return binParams_.end(); }
    auto cbegin() const noexcept { return binParams_.cbegin(); }
    auto cend() const noexcept { return binParams_.cend(); }
    
    auto rbegin() const noexcept { return binParams_.rbegin(); }
    auto rend() const noexcept { return binParams_.rend(); }
    auto crbegin() const noexcept { return binParams_.crbegin(); }
    auto crend() const noexcept { return binParams_.crend(); }
};


class BinPackingResults
{
    public:
    
    BinPackingResults(const std::vector<std::vector<size_t>>& affectation, bool solvable, const BinPackingParameters& problemParameters) noexcept
    : affectation_{affectation},
      solvable_{solvable},
      problemParameters_{problemParameters}
    {}
    
    BinPackingResults(std::vector<std::vector<size_t>>&& affectation, bool solvable, const BinPackingParameters& problemParameters) noexcept
    : affectation_{std::move(affectation)},
      solvable_{solvable},
      problemParameters_{problemParameters}
    {}
    
    BinPackingResults(const BinPackingResults&) = default;
    BinPackingResults(BinPackingResults&&) = default;
    
    BinPackingResults& operator=(const BinPackingResults&) = delete;
    BinPackingResults& operator=(BinPackingResults&&) = delete;
    
    auto begin() const noexcept { return affectation_.begin(); }
    auto end() const noexcept { return affectation_.end(); }
    auto cbegin() const noexcept { return affectation_.cbegin(); }
    auto cend() const noexcept { return affectation_.cend(); }
    
    auto rbegin() const noexcept { return affectation_.rbegin(); }
    auto rend() const noexcept { return affectation_.rend(); }
    auto crbegin() const noexcept { return affectation_.crbegin(); }
    auto crend() const noexcept { return affectation_.crend(); }
    
    private:
    const std::vector<std::vector<size_t>> affectation_;
    const bool solvable_;
    const BinPackingParameters problemParameters_;
    
    public:
    const typename std::add_lvalue_reference<decltype(affectation_)>::type getAffectation() noexcept { return affectation_; }
    bool isSolvable() noexcept { return solvable_; }
    const BinPackingParameters& getProblemParameters() noexcept { return problemParameters_; }
};

template<class Derived>
class GenericBinPackingSolver
{
    private:
    template<class T, class = void>
    struct is_solve_implemented_in : std::false_type {};
    
    template<class T>
    struct is_solve_implemented_in<T, Meta::void_t<decltype(std::declval<std::decay_t<T>>().solve({}))>> : std::true_type {};
    
    public:
    
    
    GenericBinPackingSolver(const BinPackingParameters& parameters) 
    : parameters_{parameters}
    {
        static_assert(is_solve_implemented_in<Derived>::value, "Invalid derived class : must implement the method 'solve'");
    }
    
    GenericBinPackingSolver(const GenericBinPackingSolver&) = default;
    GenericBinPackingSolver(GenericBinPackingSolver&&) = default;
    
    GenericBinPackingSolver& operator=(const GenericBinPackingSolver&) = default;
    GenericBinPackingSolver& operator=(GenericBinPackingSolver&&) = default;
    
    void setParameters(const BinPackingParameters& params) noexcept
    {
        parameters_ = params;
    }
    
    BinPackingResults solve(const std::vector<size_t>& objectList);
    
    protected:
    
    BinPackingParameters parameters_;
};

}

#endif // GENERIC_BIN_PACKING_SOLVER_HXX