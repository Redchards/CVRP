#ifndef SOLUTION_LOADER_HXX
#define SOLUTION_LOADER_HXX

#include <string>
#include <tuple>
#include <vector>

#include <CVRPSolution.hxx>
#include <FileStream.hxx>
#include <InstanceLoader.hxx>
#include <StringUtils.hxx>

class SolutionLoader
{
    private:
    using CVRPInstance = Data::CVRPInstance;
    using CVRPSolution = Solver::CVRPSolution;
    
    public:
    SolutionLoader() = default;
   
    // Tries to guess the instance file
    optional<CVRPSolution> loadSolution(const std::string& solutionFile) 
    {
        try
        {
            static constexpr char delim = '\n';
            
            FileStreamBase<StreamGoal::read> f(solutionFile, std::ios_base::in);
            std::vector<char> buffer;
            f.read(buffer, f.getFileSize());
           
            std::string data{buffer.begin(), buffer.end()}; 
            
            auto start = 0;
            auto end = data.find(delim);
            bool foundName = false; 
            std::string instanceName{};
            
            while(end != std::string::npos && !foundName)
            {
                auto current = data.substr(start, end - start);
                if(Utils::is_prefix("NAME", current))
                {
                    auto startOfName = current.find(":") + 1;
                    auto instanceName = current.substr(startOfName, current.length() -  1 - startOfName);
                    Utils::trim(instanceName);
                    foundName = true;
                }
                
                start = end + 1;
                end = data.find(delim, end + 1);
            }
            
            auto endPath = solutionFile.find(f.path_separator);
            
            if(endPath == std::string::npos)
            {
                return loadSolution(solutionFile, instanceName);
            }
            else
            {
                return loadSolution(solutionFile, solutionFile.substr(0, endPath) + instanceName + ".vrp");
            }
        }
        catch(const std::ifstream::failure& e)
        {
            std::cout << "Stream exception while trying to load the instance '" 
                      << solutionFile
                      << "' : " 
                      << e.what() 
                      << std::endl;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Argument exception while trying to load the instance '"
                      << solutionFile
                      << "' : "
                      << e.what()
                      << std::endl;
        }
        
        return {};
    }
    
    optional<CVRPSolution> loadSolution(const std::string& solutionFile, const std::string& instanceFile) 
    {
        try
        {
            static constexpr char delim = '\n';
            
            FileStreamBase<StreamGoal::read> f(solutionFile, std::ios_base::in);
            std::vector<char> buffer;
            f.read(buffer, f.getFileSize());
           
            std::string data{buffer.begin(), buffer.end()};
            
            auto start = 0;
            auto end = data.find(delim);
            
            CVRPSolution::DataType routes;
            double solutionTime = -1.0;
            
            CVRPInstance instance = *InstanceLoader().loadCVRPInstance(instanceFile);
            
            while(end != std::string::npos)
            {
                auto current = data.substr(start, end - start);
                if(Utils::is_prefix("Route", current))
                {
                    routes.push_back({});
                    auto startOfExpr = current.find(":") + 1;
                    std::string routeStr = current.substr(startOfExpr, current.length() - 1 - startOfExpr);
                    Utils::trim(routeStr);
                    
                    auto currentIt = 0;
                    auto nextIt = routeStr.find(' ');
                    auto oldNext = nextIt;
                    
                    while(oldNext != std::string::npos)
                    {
                        std::string routeNode = routeStr.substr(currentIt, nextIt - currentIt);
                        Utils::rtrim(routeNode);
                        std::cout << routeNode << std::endl;
                        routes.back().push_back(instance.getNode(std::stol(routeNode)));
                        currentIt = nextIt + 1;
                        oldNext = nextIt;
                        nextIt = routeStr.find(' ', currentIt);
                    }
                }
                
                if(Utils::is_prefix("Time", current))
                {
                    auto startOfExpr = current.find(":") + 1;
                    std::string timeStr = current.substr(startOfExpr, current.length() - 1 - startOfExpr);
                    Utils::rtrim(timeStr);
                    solutionTime = std::stod(timeStr); 
                }
                
                start = end + 1;
                end = data.find(delim, end + 1);
            }
            
            std::cout << instance.getName() << std::endl;
            return {CVRPSolution{instance, routes}};
        }
        catch(const std::ifstream::failure& e)
        {
            std::cout << "Stream exception while trying to load the instance '" 
                      << solutionFile 
                      << "' : " 
                      << e.what() 
                      << std::endl;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Argument exception while trying to load the instance '"
                      << solutionFile
                      << "' : "
                      << e.what()
                      << std::endl;
        }
        catch(const std::logic_error& e)
        {
            std::cout << "Can't load the given instance attached to the solution '"
                      << instanceFile
                      << "' : "
                      << e.what()
                      << std::endl;
        }
        
        return {};
    }
};

#endif // SOLUTION_LOADER_HXX