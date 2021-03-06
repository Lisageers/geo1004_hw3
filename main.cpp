#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"
#include <sstream>

using json = nlohmann::json;

typedef std::pair<int,int> pair;
struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &pair) const
    {
        return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
    }
};


json initCityJSON(json buildings, json footprints, std::vector<std::vector<float>> verticeslist, std::unordered_map<std::string, std::vector<int>> referencedcords) {
    buildings["type"] = "CityJSON";
    buildings["version"] = "1.0";

    buildings["metadata"]["referenceSystem"] = "urn:ogc:def:crs:EPSG::7415";
    for(int i=0;i<footprints["features"].size();i++)
    {
        std::string id = footprints["features"][i]["properties"]["identificatie"];
        buildings["CityObjects"]["id-"+ id]["Type"] = "Building";
        buildings["CityObjects"]["id-"+ id]["attributes"]["yearOfConstruction"] = footprints["features"][i]["properties"]["bouwjaar"];

        buildings["CityObjects"]["id-"+ id]["attributes"]["measuredHeight"] = 0;
        buildings["CityObjects"]["id-"+ id]["attributes"]["storeysAboveGround"] = 0;

        // make a list for the geometries
        std::vector<json> geometrylist;
        json geometry;
        geometry["type"] = "MultiSurface";
        geometry["lod"] = "0.1";
        geometry["semantics"]["surfaces"]["type"] = "GroundSurface";

        std::vector<std::vector<std::vector<int>>> cordsformat = {{referencedcords[id]}};
        geometry["boundaries"] = cordsformat;
        geometrylist.push_back(geometry);
        buildings["CityObjects"]["id-"+ id]["geometry"] = geometrylist;
    }
    buildings["vertices"] = verticeslist;

    return buildings;
}

std::unordered_map<std::string, std::vector<std::vector<float>>> parse_coordinates(json footprints)
{
    std::unordered_map<std::string, std::vector<std::vector<float>>> coordinate_floats;

    for(int i=0;i<footprints["features"].size();i++)
    {
        // initialise dict
        std::string id = footprints["features"][i]["properties"]["identificatie"];
        coordinate_floats.emplace(id, std::vector<std::vector<float>>());

        json cordlist = footprints["features"][i]["geometry"]["coordinates"];
        std::string cordstr = cordlist.dump();

        // remove [ and ]
        char chars[] = "[]";
        for (unsigned int i = 0; i < strlen(chars); ++i)
        {
            cordstr.erase (std::remove(cordstr.begin(), cordstr.end(), chars[i]), cordstr.end());
        }

        // parse coordinates into vector one at the time by splitting at ,
        std::stringstream ss(cordstr);
        std::vector<float> result;
        while(ss.good())
        {
            std::string substr;
            std::getline(ss, substr, ',' );
            result.push_back(stof(substr));
        }

        // put in dictionary
        for(int j=0;j<result.size();j+=2)
        {
            coordinate_floats[id].push_back({result[j], result[j+1], 0.0});
        }
    }
    return coordinate_floats;
}

std::vector<std::vector<float>> create_verticeslist(std::unordered_map<std::string, std::vector<std::vector<float>>> cordsdict)
{
    std::vector<std::vector<float>> vertices;

    // get values from dictionary and put in vector
    for(auto &values : cordsdict)
    {
        for(auto &cords : values.second)
        {
            vertices.push_back(cords);
        }
    }
    // make unique -- didnt work
//    std::vector<std::vector<float>>::iterator itr = unique(vertices.begin(), vertices.end());

    return vertices;
}

std::unordered_map<std::string, std::vector<int>> reference_coordinates(std::unordered_map<std::string, std::vector<std::vector<float>>> cordsdict, std::vector<std::vector<float>> cordslist)
{
    std::unordered_map<std::string, std::vector<int>> referencesdict;

    for(auto &dict : cordsdict)
    {
        std::string id = dict.first;
        referencesdict.emplace(id, std::vector<int>());
        for(auto &cords : dict.second)
        {
            for(int i=0;i<cordslist.size();i++)
            {
                if(cords[0] == cordslist[i][0] && cords[1] == cordslist[i][1])
                {
                    int index = i;
                    referencesdict[id].push_back(index);
                    break;
                }
            }
        }
    }

    return referencesdict;
}


int main() {
    const char *file_in = "../footprints.geojson";
    const char *file_out = "../extruded_footprints.json";
    // read json
    std::ifstream i(file_in);
    json footprints;
    i >> footprints;

    std::unordered_map<std::string, std::vector<std::vector<float>>> parsed_cords = parse_coordinates(footprints);
    std::vector<std::vector<float>> verticeslist = create_verticeslist(parsed_cords);
    std::unordered_map<std::string, std::vector<int>> referencedcords = reference_coordinates(parsed_cords, verticeslist);

    json buildings;
    buildings = initCityJSON(buildings, footprints, verticeslist, referencedcords);

    // write json
    std::ofstream o(file_out);
    o << std::setw(4) << buildings << std::endl;
    return 0;
}
