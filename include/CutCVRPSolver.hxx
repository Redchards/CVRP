#ifndef CUT_CVRP_SOLVER_HXX
#define CUT_CVRP_SOLVER_HXX

#include <cstdint>
#include <queue>
#include <vector>

//#include <CutHelper.hxx>
#include <CVRPInstance.hxx>
#include <CVRPSolution.hxx>
#include <GenericCVRPSolver.hxx>

ILOLAZYCONSTRAINTCALLBACK2(CutCVRPLazyCut, const Data::CVRPInstance&, instance, const std::vector<std::vector<IloIntVar>>&, edgeVarArray)
{
    //std::vector<IloRange> violatedConstraints = CutHelper::integerCutHelper(getEnv(), *this, instance, edgeVarArray);
    //std::vector<std::vector<uint8_t>> edgeValueArray;
    
    static constexpr double epsilon = 0.00001;
    auto env = getEnv();
    std::vector<IloRange> violatedConstraints;
                /*for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = i + 1; j < instance.getNumberOfNodes(); ++j)
                {
                    std::cout << edgeVarArray[i][j] << " : " << getValue(edgeVarArray[i][j]) << std::endl;
                }
            }*/
    
    std::vector<size_t> visitedNodes;
    std::vector<std::vector<size_t>> invalidTours;
    for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
    {
        size_t start = 0;
        bool isInitialized = false;
        size_t offset = 0;
        size_t currentCapacity = 0;
        std::vector<size_t> tour;
        std::queue<size_t> q;
        
        for(size_t j = i + 1; j < instance.getNumberOfNodes(); ++j)
        {
            if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
            if(getValue(edgeVarArray[i][j]) > epsilon)
            {
                q.push(i);
                q.push(j);
                tour.push_back(i);
                tour.push_back(j);
                currentCapacity += instance.getDemandOf(instance.getNode(i));
                currentCapacity += instance.getDemandOf(instance.getNode(j));
                visitedNodes.push_back(i);
                visitedNodes.push_back(j);
                break;
            }
        }
        while(!q.empty())
        {
            size_t current = q.front();
            q.pop();
            
            for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
                if(getValue(edgeVarArray[std::min(current, j)][std::max(current, j)]) > epsilon)
                {
                    q.push(j);
                    tour.push_back(j);
                    currentCapacity += instance.getDemandOf(instance.getNode(j));
                    visitedNodes.push_back(j);
                }
            }
        }
        isInitialized = false;
        
        //std::cout << "Cut ?" << std::endl;
        /*for(auto dd : tour)
        {
            std::cout << dd << " ";
        }
        std::cout << std::endl;*/
        
        // TODO : Sometimes the tours can be "out of order", we should test the links to the depot for every node and then check if == 2
        size_t nbLinkedToDepot = 0;
        for(auto n : tour)
        {
            if(getValue(edgeVarArray[0][n]) > epsilon)
            {
                ++nbLinkedToDepot;
            }
        }
        bool isLinkedToDepot = tour.size() > 0 && nbLinkedToDepot == 2;//(getValue(edgeVarArray[0][tour.front()]) > epsilon && getValue(edgeVarArray[0][tour.back()]) > epsilon); 
        bool isInvalid = tour.size() > 0 && (!isLinkedToDepot || (currentCapacity > instance.getVehicleCapacity()));
            
        if(isInvalid)
        {
            //std::cout << "INVALID " << currentCapacity << std::endl;
            /*if(isLinkedToDepot)
            {
                tour.push_back(0);
            }*/
            invalidTours.push_back(std::move(tour));
        }
        tour = {};
    }
        
    for(const auto& tour : invalidTours)
    {
        /*std::cout << "Found tour" << std::endl;
                          for(auto dd : tour)
                {
                    std::cout << dd << " ";
                }
                std::cout << std::endl;*/
        size_t tourDemand = 0;
        IloExpr tourExpr(env);
        
        for(auto idx : tour)
        {
            tourDemand += instance.getDemandOf(instance.getNode(idx));
        }
        
        std::vector<size_t> outset;
        //std::cout << "outset" << std::endl;
        for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
        {
            if(std::find(tour.begin(), tour.end(), j) == tour.end()) 
            {
                outset.push_back(j);
                //std::cout << j << " ";
            }
        }
        //std::cout << std::endl;
        
        if(outset.size() == 0) continue;
        
        for(auto source : tour)
        {
            for(auto destination : outset) 
            {
                tourExpr += edgeVarArray[std::min(source, destination)][std::max(source, destination)];
            }
        }
        
        //std::cout << "Hoi " << std::ceil(static_cast<double>(tourDemand) / instance.getVehicleCapacity()) << std::endl;
        violatedConstraints.push_back(tourExpr >= 2 * std::ceil(static_cast<double>(tourDemand) / instance.getVehicleCapacity()));
        //std::cout << (tourExpr >= 2 * std::ceil(static_cast<double>(tourDemand) / instance.getVehicleCapacity())) << std::endl;
    }
    
    for(const auto& cstr : violatedConstraints)
    {
        add(cstr, IloCplex::UseCutForce);
    }
}

namespace Solver
{
    
class CutCVRPSolver : public GenericCVRPSolver<CutCVRPSolver>
{
    private:
    using CVRPInstance = Data::CVRPInstance;
    using GraphType = CVRPInstance::GraphType;
    
    public:
    using GenericCVRPSolver<CutCVRPSolver>::GenericCVRPSolver;
    
    CVRPSolution solve(const CVRPInstance& instance)
    {
        static constexpr double epsilon = 0.00001;
        IloEnv env;
        
        try
        {
            IloModel model(env);
            
            std::vector<std::vector<IloIntVar>> edgeVarArray; 
            IloExpr objective(env);
            
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                edgeVarArray.push_back({});
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i == 0)
                    {
                        edgeVarArray.back().emplace_back(env, 0, 2, (std::string{"x"} + std::to_string(i) + "," + std::to_string(j)).c_str());
                    }
                    else
                    {
                        edgeVarArray.back().emplace_back(env, 0, 1, (std::string{"x"} + std::to_string(i) + "," + std::to_string(j)).c_str());
                    }
                    
                    if(i < j)
                    {
                        auto n1 = instance.getNode(i);
                        auto n2 = instance.getNode(j);
                        double cost = instance.getCostOf(instance.getEdge(n1, n2));
                        
                        objective += static_cast<IloInt>(cost) * edgeVarArray.back().back();
                    }
                }
            }
            
            model.add(IloMinimize(env, objective));
            
            IloExpr depotConstraint(env);
            for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                depotConstraint += edgeVarArray[0][j];
            }
            model.add(depotConstraint <= static_cast<IloInt>(2 * instance.getNumberOfVehicles()));
            
            for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
            {
                IloExpr consistencyConstraint(env);
                for(size_t j = 0; j < instance.getNumberOfNodes(); ++j)
                {
                    if(i > j) 
                    {
                        std::cout << edgeVarArray[i][j] << std::endl;
                        consistencyConstraint += edgeVarArray[j][i];
                    }
                    else if(i < j)
                    {
                        std::cout << edgeVarArray[i][j] << std::endl;
                        consistencyConstraint += edgeVarArray[i][j];
                    }
                }
                model.add(consistencyConstraint == static_cast<IloInt>(2));
            }
            
            IloCplex cplex(model);
            
            cplex.add(CutCVRPLazyCut(env, instance, edgeVarArray));
            cplex.exportModel("sortie.lp");
            
            if(!cplex.solve()) {
                env.error() << "Failed to optimize LP" << std::endl;
                return {instance, {}};
            }
            
            for(size_t i = 0; i < instance.getNumberOfNodes(); ++i)
            {
                for(size_t j = i + 1; j < instance.getNumberOfNodes(); ++j)
                {
                    std::cout << edgeVarArray[i][j] << " : " << cplex.getValue(edgeVarArray[i][j]) << std::endl;
                }
            }
            
            std::vector<std::vector<GraphType::Node>> res;
            std::vector<size_t> visitedNodes;
            for(size_t i = 1; i < instance.getNumberOfNodes(); ++i)
            {
                size_t current = i; 
                if(std::find(visitedNodes.begin(), visitedNodes.end(), i) != visitedNodes.end()) continue;
                
                if(cplex.getValue(edgeVarArray[0][i]) > epsilon)
                {
                    res.push_back({instance.getNode(i)});
                    visitedNodes.push_back(i);
                    for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
                    {
                        if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
                        if(cplex.getValue(edgeVarArray[std::min(current, j)][std::max(current, j)]) > epsilon)
                        {
                            res.back().push_back(instance.getNode(j));
                            visitedNodes.push_back(j);
                            current = j;
                            j = 0;
                        }
                    }
                }
                /*bool isInitialized = false;
                std::vector<GraphType::Node> tour;
                std::queue<size_t> q1;
                std::queue<size_t> q2;
                
                for(size_t j = i + 1; j < instance.getNumberOfNodes(); ++j)
                {
                    if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
                    if(cplex.getValue(edgeVarArray[i][j]) > epsilon)
                    {
                        q1.push(i);
                        q2.push(j);
                        tour.push_back(instance.getNode(i));
                        tour.push_back(instance.getNode(j));
                        visitedNodes.push_back(i);
                        visitedNodes.push_back(j);
                    }
                }
                while(!q1.empty())
                {
                    size_t current = q1.front();
                    q1.pop();
                    
                    for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
                    {
                        if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
                        if(cplex.getValue(edgeVarArray[std::min(current, j)][std::max(current, j)]) > epsilon)
                        {
                            q1.push(j);
                            tour.insert(tour.begin(), instance.getNode(j));
                            visitedNodes.push_back(j);
                            break;
                        }
                    }
                }
                while(!q2.empty())
                {
                    size_t current = q2.front();
                    q2.pop();
                    
                    for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
                    {
                        if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
                        if(cplex.getValue(edgeVarArray[std::min(current, j)][std::max(current, j)]) > epsilon)
                        {
                            q2.push(j);
                            tour.push_back(instance.getNode(j));
                            visitedNodes.push_back(j);
                            break;
                        }
                    }
                }
                
                if(!tour.empty())
                {
                    res.push_back(std::move(tour));
                    tour = {};
                }*/
                std::cout << "pass" << std::endl;
            }
            /*for(size_t j = 1; j < instance.getNumberOfNodes(); ++j)
            {
                if(std::find(visitedNodes.begin(), visitedNodes.end(), j) != visitedNodes.end()) continue;
                if(cplex.getValue(edgeVarArray[0][j]) > epsilon)
                {
                    res.push_back({instance.getNode(j)});
                }
            }*/
            
            std::cout << "Routes num " << res.size() << std::endl;
            for(const auto& route : res)
            {
                for(auto i : route)
                {
                    std::cout << instance.idOf(i) << " ";
                }
                std::cout << std::endl;
            }
            
            return {instance, res};
        
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

#endif // CUT_CVRP_SOLVER_HXX