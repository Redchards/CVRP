#ifndef SOLUTION_LOADER_HXX
#define SOLUTION_LOADER_HXX

#include <string>
#include <vector>

#include <CVRPSolution.hxx>
#include <FileStream.hxx>
#include <Optional.hxx>
#include <StringUtils.hxx>

class SolutionLoader
{
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
           
            std::string data{buffer.begin(), buffer.end()} 
            
            auto start = 0;
            auto end = data.find(delim);
            bool foundName = false; 
            std::string instanceName{};
            
            while(end != std::string::npos && !foundName)
            {
                auto current = data.substr(start, end);
                if(Utils::is_prefix("NAME", current))
                {
                    auto startOfName = current.find(":");
                    instanceName = Utils::trim(current.substr(0, startOfName));
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
                      << filename 
                      << "' : " 
                      << e.what() 
                      << std::endl;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Argument exception while trying to load the instance '"
                      << filename
                      << "' : "
                      << e.what()
                      << std::endl;
        }
        
        return {};
    }
    
    CVRPSolution loadSolution(const std::string& solutionFile, const std::string& instanceFile) 
    {
        try
        {
            static constexpr char delim = '\n';
            
            FileStreamBase<StreamGoal::read> f(solutionFile, std::ios_base::in);
            std::vector<char> buffer;
            f.read(buffer, f.getFileSize());
           
            std::string data{buffer.begin(), buffer.end()} 
            
            auto start = 0;
            auto end = data.find(delim);
            bool foundName = false; 
            std::string instanceName{};
            
            while(end != std::string::npos && !foundName)
            {
                auto current = data.substr(start, end);
                if(Utils::is_prefix("NAME", current))
                {
                    auto startOfName = current.find(":");
                    instanceName = Utils::trim(current.substr(0, startOfName));
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
                      << filename 
                      << "' : " 
                      << e.what() 
                      << std::endl;
        }
        catch(const std::invalid_argument& e)
        {
            std::cout << "Argument exception while trying to load the instance '"
                      << filename
                      << "' : "
                      << e.what()
                      << std::endl;
        }
        
        return {};
    }
};

#endif // SOLUTION_LOADER_HXX