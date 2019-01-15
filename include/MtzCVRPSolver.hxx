#ifndef MTZ_CVRP_SOLVER_HXX
#define MTZ_CVRP_SOLVER_HXX

#include <numeric>
#include <string>
#include <vector>

#include <CVRPInstance.hxx>
#include <CVRPSolution.hxx>
#include <GenericCVRPSolver.hxx>
#include <Optional.hxx>
#include <MtzCutHelper.hxx>

#include <ilcplex/ilocplex.h>
#include <ilconcert/iloexpression.h>


ILOUSERCUTCALLBACK2(UserCutMtzCVRPSeparation, const Data::CVRPInstance&, instance, const std::vector<IloBoolVar>&, arcVarArray)
{
    std::vector<double> arcValueArray(arcVarArray.size());
    const auto& graph = instance.getUnderlyingGraph();
    
    for(auto edge = instance.getEdgeIt(); edge != lemon::INVALID; ++edge)
    {
        size_t id1 = instance.idOf(graph.u(edge));
        size_t id2 = instance.idOf(graph.v(edge));
        
        arcValueArray[id1 * instance.getNumberOfNodes() + id2 - 1] = getValue(arcVarArray[id1 * instance.getNumberOfNodes() + id2 - 1]);
        arcValueArray[id2 * instance.getNumberOfNodes() + id1 - 1] = getValue(arcVarArray[id2 * instance.getNumberOfNodes() + id1 - 1]);
    }
    
    optional<IloRange> newExpr = CutHelper::MtzUserCut(getEnv(), instance, arcVarArray, arcValueArray);
    
    if(newExpr)
    {
        add(*newExpr, IloCplex::UseCutForce);
    }
}

namespace Solver
{
    
class MtzCVRPSolver : GenericCVRPSolver<MtzCVRPSolver>
{
    private:
    using Node = Data::CVRPInstance::GraphType::Node;
    
    public:
    using GenericCVRPSolver<MtzCVRPSolver>::GenericCVRPSolver;
    
    CVRPSolution solve(const CVRPInstance& instance)
    {
        IloEnv env;
        
        try
        {
            IloModel model(env);
            
            std::vector<IloBoolVar> arcVarArray{};
            std::vector<IloNumVar> mtzVarArray{};
            std::vector<IloExpr> capacitatedArcArray{};
           
            std::cout << "Hi1" << std::endl;
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                { 
                    if(i == 0 && j == 0)
                    {
                        continue;
                    }
                    
                    auto n1 = instance.getNode(i);
                    auto n2 = instance.getNode(j);
                    auto edge = instance.getEdge(n1, n2);
                    
                    arcVarArray.emplace_back(env, 0, 1, (std::string{"x"} + std::to_string(i) + "," + std::to_string(j)).c_str());
                    
                    if(i == j) continue;
                    
                    capacitatedArcArray.push_back(instance.getCostOf(edge) * arcVarArray.back());
                    
                    /*arcVarArray.emplace_back(env, 0, 1, (std::string{"x"} + std::to_string(j) + "," + std::to_string(i)).c_str());
                    capacitatedArcArray.push_back(IloExpr(env) * instance.getCostOf(edge) * arcVarArray.back());*/
                }
            }
           
            std::cout << "Hi2" << std::endl;
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i) 
            {
                mtzVarArray.emplace_back(env, 0, instance.getVehicleCapacity(), ILOFLOAT, (std::string{"w"} + std::to_string(i)).c_str()); 
            }
            
            model.add(IloMinimize(env, std::accumulate(capacitatedArcArray.begin(), capacitatedArcArray.end(), IloExpr(env))));
            
            IloRangeArray depotConstraints(env);
            IloRangeArray consistencyConstraints(env);
            IloConstraintArray mtzConstraints1(env);
            IloConstraintArray mtzConstraints2(env);
            
            IloExpr depotConstraint1(env);
            IloExpr depotConstraint2(env);
            
            std::cout << "Hi3" << std::endl;
            for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
            {
                depotConstraint1 += arcVarArray[i * instance.getNumberOfNodes() - 1];
                depotConstraint2 += arcVarArray[i - 1];
            }
            
            depotConstraints.add(depotConstraint1 <= static_cast<IloInt>(instance.getNumberOfVehicles()));
            depotConstraints.add(depotConstraint2 <= static_cast<IloInt>(instance.getNumberOfVehicles()));
            
            
            std::cout << "Hi4" << std::endl;
            for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
            {   
                IloExpr consistencyConstraint1(env);
                IloExpr consistencyConstraint2(env);
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    //std::cout << arcVarArray.size() << " : " << (i * instance.getNumberOfNodes() + j) << " : " << (j * instance.getNumberOfNodes() + i) << std::endl;
                    if(i == j) continue;
                    consistencyConstraint1 += arcVarArray[i * instance.getNumberOfNodes() + j - 1];
                    consistencyConstraint2 += arcVarArray[j * instance.getNumberOfNodes() + i - 1];
                }
                
                consistencyConstraints.add(consistencyConstraint1 == static_cast<IloInt>(1));
                consistencyConstraints.add(consistencyConstraint2 == static_cast<IloInt>(1));
            }
            
            
            std::cout << "Hi5" << std::endl;
            for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
            {
                auto ni = instance.getNode(i);
                IloInt di = instance.getDemandOf(ni);
                //std::cout << i << " : " << di << std::endl;
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == j) continue;
                    
                    //std::cout << "(" << i << ", " << j << ") : " << arcVarArray[i * instance.getNumberOfNodes() + j - 1].getName() << " : " << arcVarArray[j * instance.getNumberOfNodes() + i - 1].getName() << std::endl;
                    mtzConstraints1.add((mtzVarArray[i] - mtzVarArray[j]) >= (di - static_cast<IloInt>(instance.getVehicleCapacity() + di) * (1 - arcVarArray[i * instance.getNumberOfNodes() + j - 1])));
                }
            }
            
            for(auto& var : mtzVarArray)
            {
                mtzConstraints2.add(static_cast<IloInt>(0) <= var <= static_cast<IloInt>(instance.getVehicleCapacity()));
            }
            
            model.add(depotConstraints);
            model.add(consistencyConstraints);
            model.add(mtzConstraints1);
            //model.add(mtzConstraints2);
            
            IloCplex cplex(model);
            
            cplex.add(UserCutMtzCVRPSeparation(env, instance, arcVarArray));
            
            cplex.exportModel("sortie.lp");
            std::cout << "Model built ... Ready to solve" << std::endl;
            if (!cplex.solve()) {
                env.error() << "Failed to optimize LP" << std::endl;
                return {instance, {}};
            }
            
            std::vector<std::vector<Node>> result(instance.getNumberOfVehicles());
            
            // cplex.getValue 
            std::cout << "SOLVED ! " << std::endl;
            
            size_t offset = 1;
            bool offsetUpdated = false;
            
            /*for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j) 
                {
                    if(i == j) continue;
                    std::cout << i << " : " << j << std::endl;
                    std::cout << arcVarArray.size() << " : " << (i * instance.getNumberOfNodes() + j - 1)  << std::endl;
                    std::cout << cplex.getValue(arcVarArray[i * instance.getNumberOfNodes() + j - 1]) << std::endl;
                    if(cplex.getValue(arcVarArray[i * instance.getNumberOfNodes() + j - 1]))
                    {
                        std::cout << "(" << i << ", " << j << ")" << std::endl;
                    }
                }
            }*/
            for(size_t routeId = 0; routeId < instance.getNumberOfVehicles(); ++routeId)
            {
                //std::cout << routeId << std::endl;
                size_t i = 0;
                offsetUpdated = false;
                auto origOffset = offset;
                for(size_t j = offset; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == j) continue;
                    //std::cout << i << " : " << j << std::endl;
                    //std::cout << cplex.getValue(arcVarArray[i * instance.getNumberOfNodes() + j - 1]) << std::endl;
                    if(cplex.getValue(arcVarArray[i * instance.getNumberOfNodes() + j - 1]) > 0)
                    {
                        std::cout << j << std::endl;   
                        result[routeId].push_back(instance.getNode(j));
                        i = j;
                        if(!offsetUpdated)
                        {
                            offsetUpdated = true;
                            offset = j + 1;
                        }
                        j = 0;
                    }
                }
            }
            
            return {instance, result};
        }       
        catch(const IloException& e)
        {
            std::cerr << "Concert exception caught !" << e << std::endl;
        }
        catch(...)
        {
            std::cerr << "And unhandled exception occured !" << std::endl;
        }
        
        return {instance, {}};
    }
};

}

#endif // MTZ_CVRP_SOLVER_HXX