#include <Definitions.hxx>
#include <BinPackingMIPSolver.hxx>
#include <BinPackingFFD.hxx>
#include <Coordinates.hxx>
#include <GenericRouteAffectationSolver.hxx>
#include <InstanceLoader.hxx>
#include <TwoStepsCVRPSolver.hxx>
#include <SolutionExporter.hxx>
#include <MtzCVRPSolver.hxx>
#include <SweepRouteAffectationSolver.hxx>
#include <SolutionLoader.hxx>
#include <StochasticDescentCVRPSolver.hxx>
#include <OnePointExtraNeighbourhood.hxx>

#include <iostream>
#include <lemon/list_graph.h>
#include <ilcplex/ilocplex.h>

#include <lemon/nagamochi_ibaraki.h>


ILOSTLBEGIN

int main()
{
    std::cout << std::boolalpha;
    std::cout << "Hello World" << std::endl;
    
    lemon::ListDigraph g;
    
    lemon::ListDigraph::Node u = g.addNode();
    lemon::ListDigraph::Node v = g.addNode();
    lemon::ListDigraph::Arc a = g.addArc(u, v);
    
    std::cout << "So, here's lemon !" << std::endl;
    std::cout << "We have a directed graph with " << lemon::countNodes(g) << " nodes "
              << " and " << lemon::countArcs(g) << " arcs." << std::endl;
              
              
    std::cout << "Now let's test CPLEX" << std::endl;
    
    IloEnv env;
    
    try
    {
        IloModel model(env);
        IloNumVarArray vars(env);
        
        vars.add(IloNumVar(env, 0.0, 40.0));
        vars.add(IloNumVar(env));
        vars.add(IloNumVar(env));
        
        model.add(IloMaximize(env, vars[0] + 2 * vars[1] + 3 * vars[2]));
        model.add(-vars[0] + vars[1] + vars[2] <= 20);
        model.add(vars[0] - 3 * vars[1] + vars[2] <= 30);
        
        IloCplex cplex(model);
        
        if(!cplex.solve())
        {
            env.error() << "Failed to optimize LP." << std::endl;
            throw(-1);
        }
        
        IloNumArray vals(env);
        
        env.out() << "Solution status = " << cplex.getStatus() << std::endl;
        env.out() << "Solution value = " << cplex.getObjValue() << std::endl;
        cplex.getValues(vals, vars);
        
        env.out() << "Values = " << vals << std::endl;
    }
    catch(const IloException& e)
    {
        std::cerr << "Concert exception caught !" << e << std::endl;
    }
    catch(...)
    {
        std::cerr << "And unhandled exception occured !" << std::endl;
    }
    
    env.end();
    
    std::cout << "Very simple test of the Bin-Packing MIP Solver (2 buckets of capacity 5 and 2 objects of size 3 as well as 4 of size 2)" << std::endl;
    
    Solver::BinPackingMIPSolver solver{{std::vector<size_t>{7, 7}}};
    auto res = solver.solve({3, 2, 3, 2, 2, 2});
    
    std::cout << "Solvable ? : " << res.isSolvable() << std::endl;
    
    auto xx = 0;
    for(auto v : res.getAffectation())
    {
        std::cout << "Bin : " << ++xx << std::endl;
        for(auto x : v)
            std::cout << x << std::endl;
    }
    
    Solver::BinPackingFFD solver2{{std::vector<size_t>{7, 7}}};
    // std::cout << CTTI<decltype(res)>::getTypeName << std::endl;
    auto res2 = solver2.solve({3, 2, 3, 2, 2, 2});
    std::cout << "Solvable ? : " << res2.isSolvable() << std::endl;
    
    xx = 0;
    for(auto v : res2.getAffectation())
    {
        std::cout << "Bin : " << ++xx << std::endl;
        for(auto x : v)
            std::cout << x << std::endl;
    }
    
    std::cout << Coordinates(3, 5) << std::endl;
    std::cout << Coordinates() << std::endl;
    
    InstanceLoader loader{};
    auto inst = *loader.loadInstance("instances/A/A-n32-k5.vrp");
    std::cout << inst.getCoordinatesOf(inst.getNode(5)) << " : " << inst.getCoordinatesOf(inst.getNode(8)) << std::endl;
    
    size_t iddd = 0;
    std::cout << "Number of vehicles : " << inst.getNumberOfVehicles() << std::endl;
    for(auto n = inst.getNodeIt(); n != lemon::INVALID; ++n)
    {
        iddd++;
        std::cout << inst.idOf(n) << " : " << inst.getCoordinatesOf(n) << " : " << inst.getDemandOf(n) << std::endl;
    }
    
    // using FirstSolver = Solver::RouteAffectationBinPackingAdaptor<Solver::BinPackingMIPSolver>;
    using FirstSolver = Solver::SweepRouteAffectationSolver;
    Solver::TwoStepsCVRPSolver<FirstSolver, Solver::TwoOptTSPSolver> solver4({inst}, {});
    Solver::StochasticDescentCVRPSolver<Solver::TwoStepsCVRPSolver<FirstSolver, Solver::TwoOptTSPSolver>, Heuristic::OnePointExtraNeighbourhood> stochSolv{solver4, 100000};
    auto sol4 = solver4.solve(inst);
    std::cout << sol4.computeCost() << std::endl;
    //auto sol5 = stochSolv.solve(inst);
    SolutionExporter solExporter{};
    solExporter.exportSolutionGraph(sol4, "testSol.jpg");
    solExporter.exportSolution(sol4, "testSol.sol");
    //solExporter.exportSolutionGraph(sol5, "testSol2.jpg");
    //SolutionLoader solLoader{};
    //auto solLoaded = solLoader.loadSolution("testSol.sol", "instances/P/P-n19-k2.vrp");
    //solExporter.exportSolutionGraph(*solLoaded, "testSol2.jpg");
    
    /*Solver::MtzCVRPSolver solver5;
    
    std::cout << "Now solving using MTZ MIP : " << std::endl;
    auto sol4 = solver5.solve(inst);
    
    for(auto a : sol4)
    {
        for(auto b: a)
        {
            std::cout << inst.idOf(b) << " "; 
        }
        std::cout << std::endl;
    }
    std::cout << sol4.computeCost() << std::endl;
    
    SolutionExporter solExporter{};
    solExporter.exportSolutionGraph(sol4, "plot.jpg");*/
    /*Solver::RouteAffectationBinPackingAdaptater<Solver::BinPackingMIPSolver> solver3{{std::vector<size_t>{7, 7}}};
    auto res3 = solver3.solve({3, 2, 3, 2, 2, 2});
    
    std::cout << "Solvable ? : " << res3.isSolvable() << std::endl;
    
    xx = 0;
    for(auto v : res3.getAffectation())
    {
        std::cout << "Bin : " << ++xx << std::endl;
        for(auto x : v)
            std::cout << x << std::endl;
    }*/
    
    /*Solver::SweepRouteAffectationSolver sweepSolver{{inst}};
    
    for(auto& route : sweepSolver.solve(inst))
    {
        for(auto& node : route)
        {
            std::cout << inst.idOf(node) << " ";
        }
        std::cout << std::endl;
    }

    Solver::TwoStepsCVRPSolver<Solver::SweepRouteAffectationSolver, Solver::TwoOptTSPSolver> solver6({inst}, {});*/
    
    return 0;
}