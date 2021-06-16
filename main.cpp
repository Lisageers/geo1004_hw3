#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"

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

//template<typename T>
//void Remove( std::basic_string<T> & Str, const T * CharsToRemove )
//{
//    std::basic_string<T>::size_type pos = 0;
//    while (( pos = Str.find_first_of( CharsToRemove, pos )) != std::basic_string<T>::npos )
//    {
//        Str.erase( pos, 1 );
//    }
//}

json initCityJSON(json buildings, json footprints) {
    buildings["type"] = "CityJSON";
    buildings["version"] = "1.0";

    buildings["metadata"]["referenceSystem"] = "urn:ogc:def:crs:EPSG::7415";
    buildings["CityObjects"] = {};
//    buildings["vertices"] = [];
    for(int i=0;i<footprints["features"].size();i++)
    {
        std::string id = footprints["features"][i]["properties"]["identificatie"];
        buildings["CityObjects"]["id-"+ id]["Type"] = "Building";
        buildings["CityObjects"]["id-"+ id]["lod"] = "1.3";
        buildings["CityObjects"]["id-"+ id]["attributes"]["yearOfConstruction"] = footprints["features"][i]["properties"]["bouwjaar"];
        buildings["CityObjects"]["id-"+ id]["attributes"]["measuredHeight"] = 0;
        buildings["CityObjects"]["id-"+ id]["attributes"]["storeysAboveGround"] = 0;

        buildings["CityObjects"]["id-"+ id]["geometry"]["type"] = "MultiSurface";
        buildings["CityObjects"]["id-"+ id]["geometry"]["lod"] = "1.3";
        buildings["CityObjects"]["id-"+ id]["geometry"]["boundaries"] = footprints["features"][i]["geometry"]["coordinates"];
    }

    return buildings;
}

std::unordered_map<json, std::vector<std::vector<float>>> parse_coordinates(json footprints)
{
    std::unordered_map<json, std::vector<std::vector<float>>> coordinate_floats;

    for(int i=0;i<footprints["features"].size();i++)
    {
        std::string id = footprints["features"][i]["properties"]["identificatie"];
        json cordlist = footprints["features"][i]["geometry"]["coordinates"];
        std::string cordstr = cordlist.dump(1);

        char chars[] = "[],";

//        for (unsigned int i = 0; i < strlen(chars); ++i)
//        {
//            cordstr.erase (std::remove(cordstr.begin(), cordstr.end(), chars[i]), cordstr.end());
//        }


        std::cout << cordstr << "\n";

    }
    return coordinate_floats;
}

std::vector<std::vector<float>> update_geometry(json footprints)
{
    std::vector<std::vector<float>> vertices;


//    std::vector<int>::iterator ip;
//    ip = std::unique(vertices.begin(), vertices.begin() + 12);
//    // Now v becomes {1 3 10 1 3 7 8 * * * * *}
//    // * means undefined
//
//    // Resizing the vector so as to remove the undefined terms
//    vertices.resize(std::distance(vertices.begin(), ip));
    return vertices;
}



int main() {
    const char *file_in = "../footprints.geojson";
    const char *file_out = "../extruded_footprints.json";
    // read json
    std::ifstream i(file_in);
    json footprints;
    i >> footprints;

    std::cout << footprints["type"];
    json buildings;
    buildings = initCityJSON(buildings, footprints);
    std::unordered_map<json, std::vector<std::vector<float>>> parsed_cords = parse_coordinates(footprints);
    std::vector<std::vector<float>> verticeslist = update_geometry(parsed_cords);
    // function with references to veritceslist
    // write json
    std::ofstream o(file_out);
    o << std::setw(4) << buildings << std::endl;
    return 0;
}
