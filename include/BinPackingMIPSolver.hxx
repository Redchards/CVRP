#ifndef BIN_PACKING_MIP_SOLVER_HXX

// #include <cstddef>
#include <numeric>
#include <string>
#include <vector>

#include <ilcplex/ilocplex.h>
#include <ilconcert/iloexpression.h>

#include <GenericBinPackingSolver.hxx>

namespace Solver
{

class BinPackingMIPSolver : public GenericBinPackingSolver<BinPackingMIPSolver>
{
    public:
    
    using GenericBinPackingSolver<BinPackingMIPSolver>::GenericBinPackingSolver;
    
    BinPackingResults solve(const std::vector<size_t>& objectList)
    {
        IloEnv env;
        
        try
        {
            IloModel model(env);
            
            std::vector<IloBoolVar> varArray{};
            
            for(size_t i = 0; i < objectList.size(); i++)
            {
                for(size_t j = 0; j < this->parameters_.getNumberOfBins(); j++)
                {
                    varArray.emplace_back(env, 0, 1, (std::string{"x"} + std::to_string(i) + "," + std::to_string(j)).c_str());
                }
            }
            
            model.add(IloMaximize(env, std::accumulate(varArray.begin(), varArray.end(), IloExpr(env))));

            IloRangeArray capacityConditions(env);
            IloRangeArray consistencyConditions(env);
            
            for(size_t j = 0; j < this->parameters_.getNumberOfBins(); j++)
            {
                IloExpr cond(env);
                
                for(size_t i = 0; i < objectList.size(); i++)
                {
                    cond += varArray[i * this->parameters_.getNumberOfBins() + j] * static_cast<IloInt>(objectList[i]);
                }
                
                capacityConditions.add(cond <= static_cast<IloInt>(parameters_.getBinCapacity(j)));
            }
            
            for(size_t i = 0; i < objectList.size(); i++)
            {
                IloExpr cond(env);
                
                for(size_t j = 0; j < this->parameters_.getNumberOfBins(); j++)
                {
                    cond += varArray[i * this->parameters_.getNumberOfBins() + j];
                }
                
                consistencyConditions.add(cond == 1);
            }
            
            model.add(capacityConditions);
            model.add(consistencyConditions);
            
            IloCplex cplex(model);
            // cplex.exportModel("sortie.lp");

        
            if ( !cplex.solve() ) {
                env.error() << "Failed to optimize LP" << std::endl;
                return {{}, false, parameters_};
            }
            
            std::vector<std::vector<size_t>> affectation(this->parameters_.getNumberOfBins());
            
            for(size_t j = 0; j < this->parameters_.getNumberOfBins(); j++)
            {
                for(size_t i = 0; i < objectList.size(); i++)
                {
                    if(cplex.getValue(varArray[i * this->parameters_.getNumberOfBins() + j]) == 1)
                    {
                        affectation[j].push_back(i);
                    }
                }
            }
            
             
            env.out() << "Solution status = " << cplex.getStatus() << std::endl;
            env.out() << "Solution value  = " << cplex.getObjValue() << std::endl;

            for(const auto& var : varArray)
            {
                std::cout << var << ": " << cplex.getValue(var) << std::endl;
            }
            // TODO : put the number of bins used
            return {affectation, true, parameters_};
        }
        catch(const IloException& e)
        {
            std::cerr << "Concert exception caught !" << e << std::endl;
        }
        catch(...)
        {
            std::cerr << "And unhandled exception occured !" << std::endl;
        }
        
        return {{}, false, parameters_};
    }
    
};

}

#endif // BIN_PACKING_MIP_SOLVER_HXX