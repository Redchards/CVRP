#ifndef INSTANCE_LOADER_HXX
#define INSTANCE_LOADER_HXX

#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

#include <Coordinates.hxx>
#include <CVRPInstance.hxx>
#include <FileStream.hxx>
#include <StringUtils.hxx>
#include <Optional.hxx>

class InstanceLoader
{
    private:
    using CVRPInstance = Data::CVRPInstance;
    using VehicleData = Data::VehicleData;
    
    public:
    InstanceLoader()
    : costFunctions_{{"EUC_2D", 
                     [](Coordinates p1, Coordinates p2){
                        auto x = p2.x - p1.x;
                        auto y = p2.y - p1.y;
                        return std::sqrt(x*x + y*y);
                     }}}
    {}
    
    optional<CVRPInstance> loadInstance(const std::string& filename)
    {
        try 
        {
            constexpr char delim = '\n';
            FileStreamBase<StreamGoal::read> f(filename, std::ios_base::in);
            std::vector<char> buffer;
            std::cout << "File size : " << f.getFileSize() << std::endl;
            f.read(buffer, f.getFileSize());
            
            const std::string data{buffer.begin(), buffer.end()};
            std::cout << data << std::endl;
            
            size_t start = 0;
            size_t end = data.find(delim);
            
            std::string instanceName;
            bool inCoordSection = false;
            bool inDemandSection = false;
            bool preprocessingDone = false;
            size_t graphSize;
            
            while(end != std::string::npos && !preprocessingDone)
            {
                auto current = data.substr(start, end - start);
                Utils::trim(current);
                
                if(Utils::is_prefix("DIMENSION", current))
                {
                    graphSize = std::stoi(current.substr(12, current.length() - 12)); // HARD CODED, WRONG !!
                    preprocessingDone = true;
                }
                
                start = end + 1;
                end = data.find(delim, end + 1);
            }
            std::cout << graphSize << std::endl;
            start = 0;
            end = data.find(delim);
            
            CVRPInstance::GraphType g(graphSize);
            CVRPInstance::CoordinatesMap coordinatesMap{g};
            CVRPInstance::DemandMap demandMap{g};
            std::function<CVRPInstance::CostFunctionType> costFunction;
            size_t vehicleNum = 0;
            size_t vehicleCapacity = 0;
            
            while(end != std::string::npos)
            {
                auto current = data.substr(start, end - start);
                Utils::trim(current);
                
                if(Utils::is_prefix("NODE_COORD_SECTION", current))
                {
                    inCoordSection = true;
                    inDemandSection = false;
                }
                else if(Utils::is_prefix("DEMAND_SECTION", current))
                {
                    inDemandSection = true;
                    inCoordSection = false;
                }
                else if(Utils::is_prefix("DEPOT_SECTION", current))
                {
                    inDemandSection = false;
                    inCoordSection = false;
                }
                else if(inCoordSection)
                {
                    auto parsedResult = parseCoordinates(current);
                    coordinatesMap.set(g.nodeFromId(std::get<0>(parsedResult) - 1), std::get<1>(parsedResult));
                }
                else if(inDemandSection)
                {
                    auto parsedResult = parseDemand(current);
                    std::cout << std::get<0>(parsedResult) << std::endl;
                    demandMap.set(g.nodeFromId(std::get<0>(parsedResult) - 1), std::get<1>(parsedResult));
                }
                else if(Utils::is_prefix("NAME", current)) 
                {
                    auto pos = current.find(":") + 1;
                    instanceName = current.substr(pos, current.length() - pos);
                }
                else if(Utils::is_prefix("EDGE_WEIGHT_TYPE", current))
                {
                    auto pos = current.find(":") + 1;
                    std::cout << current.substr(pos, current.length() - pos) << std::endl;
                    std::string type = current.substr(pos, current.length() - pos);
                    Utils::trim(type);
                    costFunction = costFunctions_.at(type);
                }
                else if(current.find("No of trucks") != std::string::npos)
                {
                    auto pos1 = current.find("No of trucks");
                    auto pos2 = current.find(",", pos1);
                    auto numStr = current.substr(pos1 + 13, pos2 - pos1 - 13);
                    std::cout << numStr << std::endl;
                    vehicleNum = std::stod(numStr);
                }
                else if(Utils::is_prefix("CAPACITY", current))
                {
                    auto pos = current.find(":") + 1;
                    std::string numStr = current.substr(pos, current.length() - pos);
                    vehicleCapacity = std::stod(numStr);
                }
                
                start = end + 1;
                end = data.find(delim, end + 1);
            }
            std::cout << "Finish" << std::endl;

            return CVRPInstance{g, instanceName, VehicleData{vehicleNum, vehicleCapacity}, demandMap, coordinatesMap, costFunction};
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
   
    std::tuple<size_t, Coordinates> parseCoordinates(const std::string& data)
    {
        std::cout << data << std::endl;
        size_t start = 0;
        size_t end = data.find(' ');
        std::vector<double> res;
        res.reserve(3);
        
        while(end != std::string::npos)
        {
            res.push_back(std::stod(data.substr(start, end - start)));
            start = end + 1;
            end = data.find(' ', start);
        }
        res.push_back(std::stod(data.substr(start, data.size() - start)));
        
        std::cout << res[0] << " : " << res[1] << " : " << res[2] << std::endl;
        
        return std::tuple<size_t, Coordinates>{res[0], {res[1], res[2]}};
    }
    
    std::tuple<size_t, size_t> parseDemand(const std::string& data)
    {
        std::cout << data << std::endl;
        size_t start = 0;
        size_t end = data.find(' ');
        std::vector<size_t> res;
        res.reserve(2);
        
        while(end != std::string::npos)
        {
            res.push_back(std::stol(data.substr(start, end - start)));
            start = end + 1;
            end = data.find(' ', start);
        }
        res.push_back(std::stol(data.substr(start, data.size() - start)));
        std::cout << res[0] << " : " << res[1] << std::endl;
        
        return std::tuple<size_t, size_t>{res[0], res[1]};
    }
    
    private:
    const std::unordered_map<std::string, std::function<CVRPInstance::CostFunctionType>> costFunctions_;
};

#endif // INSTANCE_LOADER_HXX