#ifndef BIN_PACKING_FFD_HXX
#define BIN_PACKING_FFD_HXX

#include <algorithm>
#include <functional>
#include <vector>

#include <GenericBinPackingSolver.hxx>

namespace Solver
{
    
class BinPackingFFD : public GenericBinPackingSolver<BinPackingFFD>
{
    public:
    
    using GenericBinPackingSolver<BinPackingFFD>::GenericBinPackingSolver;
    
    BinPackingResults solve(const std::vector<size_t>& objectList)
    {
        const auto numBins = this->parameters_.getNumberOfBins();
        
        std::vector<std::vector<size_t>> affectation(numBins);
        std::vector<size_t> freeSpace(this->parameters_.getUnderlying());
        auto objList = objectList;
        std::sort(objList.begin(), objList.end(), std::greater<decltype(objList)::value_type>{});
        
        bool isValid = true;
        for(size_t objIdx = 0; objIdx < objList.size(); ++objIdx)
        {
            bool fits = false;
            for(size_t i = 0; i < affectation.size(); ++i)
            {
                if(freeSpace[i] >= objList[objIdx])
                {
                    affectation[i].push_back(objIdx);
                    freeSpace[i] -= objList[objIdx];
                    fits = true;
                    break;
                }
            }
            
            if(!fits)
            {
                isValid = false;
                break;
            }
        }
        
        return {affectation, isValid, parameters_};
    }
};

}

#endif // BIN_PACKING_FFD_HXX