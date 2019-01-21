#ifndef AGG_TVRP_SOLVER_HXX
#define AGG_TVRP_SOLVER_HXX

#include <GenericTVRPSolver.hxx>
#include <Optional.hxx>
#include <TVRPInstance.hxx>
#include <TVRPSolution.hxx>

#include <ilcplex/ilocplex.h>
#include <ilconcert/iloexpression.h>

namespace Solver
{
    
class AggTVRPSolver : public GenericTVRPSolver<AggTVRPSolver>
{
    private:
    using TVRPInstance = Data::TVRPInstance;
    using Node = TVRPInstance::GraphType::Node;
    
    public:
    using GenericTVRPSolver<AggTVRPSolver>::GenericTVRPSolver;
    
    TVRPSolution solve(const TVRPInstance& instance)
    {
        IloEnv env;
        
        try
        {
            IloModel model(env);
            
            std::vector<std::vector<std::vector<IloBoolVar>>> arcVarArray;
            std::vector<std::vector<IloNumVar>> flowVarArray;
            
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                arcVarArray.push_back({});
                flowVarArray.push_back({});
                auto& currentI = arcVarArray.back();
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                { 
                    currentI.push_back({});
                    auto& currentJ = currentI.back();
                    
                    for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
                    {
                        currentJ.emplace_back(env, 0, 1, (std::string{"x"} + std::to_string(i) + "," + std::to_string(j) + "," + std::to_string(t)).c_str());
                    }
                    flowVarArray.back().emplace_back(env, 0, IloInfinity, (std::string{"y"} + std::to_string(i) + "," + std::to_string(j)).c_str());
                }
            }
            
            IloExpr objective(env);
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
                    {
                        if(instance.canServe(t, instance.getNode(i)) && instance.canServe(t, instance.getNode(j)))
                        {
                            auto n1 = instance.getNode(i);
                            auto n2 = instance.getNode(j);
                            auto cost = instance.getCostOf(instance.getEdge(n1, n2));
                            
                            objective += static_cast<IloNum>(cost) * arcVarArray[i][j][t];
                        }
                    }
                }
            }
            model.add(IloMinimize(env, objective));
            
            for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                IloExpr tmpExpr(env);
                for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
                {
                    for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
                    {
                        if(i == j) continue;
                        
                        if(instance.canServe(t, instance.getNode(i)) && instance.canServe(t, instance.getNode(j)))
                        {
                            tmpExpr += arcVarArray[i][j][t];
                        }
                    }
                }
                model.add(tmpExpr == 1);
            }
            
            for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
            {
                IloExpr tmpExpr(env);
                for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
                {
                    if(instance.canServe(t, instance.getNode(j)))
                    {
                        tmpExpr += arcVarArray[0][j][t];
                    }
                }
                model.add(tmpExpr == 1);
            }
            
            for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
                {
                    IloExpr leftExpr(env);
                    IloExpr rightExpr(env);
                    
                    for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
                    {
                        if(i == j) continue;
                        
                        if(instance.canServe(t, instance.getNode(i)) && instance.canServe(t, instance.getNode(j)))
                        {
                            leftExpr += arcVarArray[i][j][t];
                            rightExpr += arcVarArray[j][i][t];
                        }
                    }
                    
                    model.add(leftExpr == rightExpr);
                }
            }
            
            IloExpr flowExpr1(env);
            for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                flowExpr1 += flowVarArray[0][j];
                //model.add(flowVarArray[0][j] <= static_cast<IloInt>(instance.getVehicleCapacity()));
            }
            model.add(flowExpr1 == (instance.getVehicleCapacity() * instance.getNumberOfTechnicians()));
            
            for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                IloExpr flowExpr2(env);
                IloExpr flowExpr3(env);
                for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
                {
                    if(i == j) continue;
                    
                    flowExpr2 += flowVarArray[i][j];
                    flowExpr3 += flowVarArray[j][i];
                }
                model.add((flowExpr2 - flowExpr3) == instance.getDemandOf(instance.getNode(j)));
            }
            
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == j) continue;
                    
                    IloExpr tmpExpr(env);
                    
                    for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
                    {
                        if(instance.canServe(t, instance.getNode(i)) && instance.canServe(t, instance.getNode(j)))
                        {
                            tmpExpr += arcVarArray[i][j][t];
                        }
                    }
                    
                    model.add(flowVarArray[i][j] <= (static_cast<IloInt>(instance.getVehicleCapacity()) * tmpExpr));
                }
            }
            
            IloCplex cplex(model);
            cplex.exportModel("sortie.lp");
            
            if(!cplex.solve()) {
                env.error() << "Failed to optimize LP" << std::endl;
                return {instance, {}};
            }
            
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == j) continue;
                    for(size_t t = 0; t < instance.getNumberOfTechnicians(); ++t)
                    {
                        if(instance.canServe(t, instance.getNode(i)) && instance.canServe(t, instance.getNode(j)))
                        {
                            std::cout << arcVarArray[i][j][t] << " : " << cplex.getValue(arcVarArray[i][j][t]) << std::endl;
                        }
                    }
                }
            }
            
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i) 
            {
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == j) continue;
                    std::cout << flowVarArray[i][j] << " : " << cplex.getValue(flowVarArray[i][j]) << std::endl;
                }
            }
            
           
            std::vector<std::vector<Node>> result(instance.getNumberOfTechnicians());
            bool offsetUpdated = false;
            size_t offset = 0;
            for(size_t routeId = 0; routeId < instance.getNumberOfTechnicians(); ++routeId)
            {
                //std::cout << routeId << std::endl;
                size_t i = 0;
                size_t currentTechnician = 0;
                offsetUpdated = false;
                for(size_t j = offset; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == j) continue;
                    //std::cout << cplex.getValue(arcVarArray[i * instance.getNumberOfNodes() + j - 1]) << std::endl;
                    for(size_t t = 0; t < instance.getNumberOfTechnicians() && !offsetUpdated; ++t)
                    {
                        
                        //std::cout << i << " : " << j << " : " << t << std::endl;
                        if(instance.canServe(t, instance.getNode(i)) && instance.canServe(t, instance.getNode(j)) && cplex.getValue(arcVarArray[0][j][t]) == 1)
                        {
                            currentTechnician = t;
                        }
                        //std::cout << "Hey" << std::endl;
                    }
                    if(instance.canServe(currentTechnician, instance.getNode(j)) && cplex.getValue(arcVarArray[i][j][currentTechnician]) > 0)
                    {
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

#endif // AGG_TVRP_SOLVER_HXX