#ifndef ONE_POINT_EXTRA_NEIGHBOURHOOD_HXX
#define ONE_POINT_EXTRA_NEIGHBOURHOOD_HXX

#include <random>

#include <CVRPInstance.hxx>
#include <GenericNeighbourhoodGenerator.hxx>

namespace Heuristic
{
    
class OnePointExtraNeighbourhood : public GenericNeighbourhoodGenerator
{
    private:
    using CVRPSolutionData = Solver::CVRPSolutionData;
    public:
    using GenericNeighbourhoodGenerator::GenericNeighbourhoodGenerator;
    
    public:
    CVRPSolutionData randomNeighbour(const CVRPSolutionData& sol)
    {
        if(sol.size() == 0)
        {
            return {};
        }
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> routePicker(0, sol.size() - 1); 
        
        auto newData = sol;
        
        auto r1 = 0;
        do 
        {
            r1 = routePicker(gen);
        } while(sol[r1].size() == 0);
        
        std::uniform_int_distribution<> nodePicker1(0, sol[r1].size() - 1);
        auto n1Id = nodePicker1(gen);
        
        auto n1It = newData[r1].begin() + n1Id;
        auto n1 = *n1It;
        newData[r1].erase(n1It);
        
        auto r2 = routePicker(gen);
        std::uniform_int_distribution<> nodePicker2(0, newData[r2].size());
        auto n2Id = nodePicker2(gen);
        
        auto n2It = newData[r2].begin() + n2Id;
        
        newData[r2].insert(n2It, n1);
        
        // std::cout << "Switch " << sol.getOriginalInstance().idOf(n1) << " from route " << r1 << " to " << r2 << " position " << n2Id << std::endl;
        
        return newData;
    }
};

}

#endif // ONE_POINT_EXTRA_NEIGHBOURHOOD_HXX