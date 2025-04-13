#include "stat_reader.h"

void stat_reader::ParseCommadStream(std::istringstream &command,
        const transport_catalogue::TransportCatalogue &catalogue,
        std::ostringstream &out) {
    int stat_request_count;
    command >> stat_request_count;
    for (int i = 0; i <= stat_request_count; ++i) {
        std::string line;
        getline(command, line);
        ParseAndPrintStat(catalogue, line, out);
    }
}

