#pragma once
//
#include <iosfwd>
#include <iomanip>
#include <string_view>
#include "input_reader.h"
//
#include "transport_catalogue.h"
//

using namespace std::literals;

namespace stat_reader{

template <typename Stream>
void ParseAndPrintStat(
        const transport_catalogue::TransportCatalogue &transport_catalogue,
        std::string_view request, Stream &output) {
    int space_pos = request.find(' ');
        if (space_pos != -1) {
            std::string command = std::string {request.substr(0, space_pos)}; // @suppress("Symbol is not resolved")
            if(command == "Bus"s) {
                const std::string &bus_number = std::string {request.substr(space_pos+1, request.size() - space_pos)}; // @suppress("Symbol is not resolved")
                domain::BusInfo bus_info = transport_catalogue.GetBusInfo(bus_number);
                if (!bus_info.found) {
                    output << "Bus "s << bus_number << ": not found\n"s;
                }
                else {

                    double route_length = bus_info.route_length;

                    output << "Bus "s << bus_number<< ": "s << bus_info.no_unique_stops_count
                    << " stops on route, "s << bus_info.unique_stops_count << " unique stops, "s
                    << std::setprecision(6) << route_length << " route length, "s  << bus_info.curvature << " curvature\n";
                }

            }
            else {
                const std::string &stop_name = std::string {request.substr(space_pos+1, request.size() - space_pos)}; // @suppress("Symbol is not resolved")

                if (transport_catalogue.FindStop(stop_name) == nullptr) {
                      output << "Stop "s << stop_name << ": not found\n"s;
                }
                else{
                    const std::set<std::string> &stop_info = transport_catalogue.GetStopInfo(stop_name);
                    if(stop_info.size() == 0){
                        output << "Stop "s << stop_name << ": no buses\n"s;
                    }
                    else{
                        output << "Stop "s << stop_name << ": buses"s;
                        for (auto& str : stop_info){
                            output << " " << str ;
                        }
                        output << "\n";
                    }
                }
            }
        }
}


void ParseCommadStream(std::istringstream& command,
                       const transport_catalogue::TransportCatalogue& catalogue,
                       std::ostringstream& ostream);

}
