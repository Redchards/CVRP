#ifndef SOLUTION_EXPORTER_HXX
#define SOLUTION_EXPORTER_HXX

#include <array>
#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

#include <CVRPSolution.hxx>
#include <FileStream.hxx>

class SolutionExporter
{
    private:
    using CVRPSolution = Solver::CVRPSolution;
    
    public:
    SolutionExporter() = default;
    
    SolutionExporter(const SolutionExporter&) = default;
    SolutionExporter(SolutionExporter&&) = default;
    
    SolutionExporter& operator=(const SolutionExporter&) = default;;
    SolutionExporter& operator=(SolutionExporter&&) = default;
    
    void exportSolution(const CVRPSolution& solution, const std::string& filename) const
    {
        FileStreamBase<StreamGoal::write> stream(filename, std::ios_base::in);
        std::string res;
        
        size_t idx = 0;
        for(const auto& route : solution)
        {
            idx++;
            res += std::string{"Route #"} + std::to_string(idx) + ": ";
            for(auto& node : route)
            {
                res += std::to_string(solution.getOriginalInstance().idOf(node)) + " ";
            }
            res += "\n";
        }
        
        stream.write(res);
    }
    
    void exportSolutionGraph(const CVRPSolution& solution, const std::string& filename) const
    {
        std::string graphData{};
        
        for(const auto& route : solution)
        {
            std::string partialGraphData{};
            
            auto depotNode = solution.getOriginalInstance().getDepotNode();
            auto depotPos = solution.getOriginalInstance().getCoordinatesOf(depotNode);
            std::string depotNodeData = std::to_string(depotPos.x) + " " + std::to_string(depotPos.y) + " " + std::to_string(solution.getOriginalInstance().idOfDepot()) + "\n";
            
            partialGraphData += depotNodeData;
            for(const auto& node : route)
            {
                auto pos = solution.getOriginalInstance().getCoordinatesOf(node);
                partialGraphData += std::to_string(pos.x) + " " + std::to_string(pos.y) + " " + std::to_string(solution.getOriginalInstance().idOf(node)) + "\n";
            }
            partialGraphData += depotNodeData; 
            partialGraphData += "e";
            
            for(size_t i = 0; i < 3; ++i)
            {
                graphData += std::string("\n") + partialGraphData; 
            }
        }
        
        auto extPos = filename.find('.');
        
        if(extPos == std::string::npos)
        {
            throw std::out_of_range(std::string("The file name'") + filename + "provided does not have a proper extension.");
        }
        
        ++extPos;
        std::string ext = filename.substr(extPos, filename.length() - extPos);
        
        if(ext == "jpg") ext = "jpeg";
        
        
        
        /*for(auto n = solution.getOriginalInstance().getNodeIt(); n != lemon::INVALID; ++n)
        {
            auto pos = solution.getOriginalInstance().getCoordinatesOf(n);
            graphData += std::to_string(pos.x) + " " + std::to_string(pos.y) + "\n";
        }*/
        std::string plotTitle = solution.getOriginalInstance().getName() + " - value : " + std::to_string(solution.computeCost());
        std::string plotCommand = std::string("bash -c \"gnuplot -p <(echo -e 'set terminal ") + ext + ";set title \\\"" + plotTitle + "\\\";set key outside;set output \\\"" + filename + "\\\";";
        
        bool firstPass = true; 
        for(size_t routeId = 0; routeId < solution.getNumberOfRoutes(); ++routeId)
        {
            std::string firstCommand = ",\\\"\\\" ";
            if(firstPass) 
            {
                firstCommand = "plot \\\"-\\\" ";
                firstPass = false;
            }
            plotCommand += firstCommand + linePlotCommand1_ + colors_[routeId] + linePlotCommand2_ + "\\\"Route " + std::to_string(routeId + 1) + "\\\"";
            plotCommand += std::string(",\\\"\\\" ") + nodePlotCommand1_ + colors_[routeId] + nodePlotCommand2_;
            plotCommand += std::string(",\\\"\\\" ") + labelPlotCommand1_ + labelColor_ + labelPlotCommand2_;
        }
        
        plotCommand += graphData + "\n\')\"";
        std::cout << plotCommand << std::endl;
        system(plotCommand.c_str());
        
    }
    
    private:
    static constexpr std::array<const char*, 8> colors_{{
        "red",
        "orange",
        "yellow",
        "green",
        "cyan",
        "blue",
        "violet",
        "magenta"
    }};
    
    const char* labelColor_ = "black";
    
    const char* linePlotCommand1_ = "using 1:2 with lines lc rgb \\\"";
    const char* linePlotCommand2_ = "\\\" lw 2 title ";
    
    const char* nodePlotCommand1_ = "using 1:2:(0.3) with circles fill solid lc rgb \\\"";
    const char* nodePlotCommand2_ = "\\\" notitle";
    
    const char* labelPlotCommand1_ = "using 1:2:3 with labels tc rgb \\\"";
    const char* labelPlotCommand2_ = "\\\" offset (0,0) notitle";
};

#endif // SOLUTION_EXPORTER_HXX