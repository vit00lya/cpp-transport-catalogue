#pragma once

#include <cmath>
#include <stdint.h>

namespace geo {

    const uint32_t radius_earth = 6371000;

    struct Coordinates {
        double lat;
        double lng;

        bool operator==(Coordinates const &other) const {
            return lat == other.lat && lng == other.lng;
        }

        bool operator!=(Coordinates const &other) const {
                return !(*this == other);
            }
    };

    inline double ComputeDistance(const Coordinates& from, const Coordinates& to) {
        using namespace std;
        if (from == to) {
            return 0;
        }
        static const double dr = 3.1415926535 / 180.;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
                    + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * radius_earth;
    }

}
