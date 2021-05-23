#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

int main() {
    const char *file_in = "../footprints.geojson";
    const char *file_out = "../extruded_footprints.json";
    // read json
    std::ifstream i(file_in);
    json footprints;
    i >> footprints;

    std::cout << footprints["type"];

    // write json
//    std::ofstream o("pretty.json");
//    o << std::setw(4) << footprints << std::endl;
    return 0;
}
