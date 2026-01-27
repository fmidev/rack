/*
 * JavaScript.h
 *
 *  Created on: Jan 26, 2026
 *      Author: mpeura
 */

//#include <vector>
#include <cstdint>
#include <vector>
#include <cstring>

#include "JavaScript.h"

namespace drain {



DRAIN_ENUM_DICT(JavaScript::NumType) = {
	DRAIN_ENUM_ENTRY(JavaScript::NumType, Int8),
	DRAIN_ENUM_ENTRY(JavaScript::NumType, Int16),
	DRAIN_ENUM_ENTRY(JavaScript::NumType, Int32),
	DRAIN_ENUM_ENTRY(JavaScript::NumType, Float16),
	DRAIN_ENUM_ENTRY(JavaScript::NumType, Float32),
	DRAIN_ENUM_ENTRY(JavaScript::NumType, Float64),
};

static inline void append_float32_le(std::vector<uint8_t>& out, float v) {
    uint32_t u;
    static_assert(sizeof(float) == 4, "float must be 32-bit");
    std::memcpy(&u, &v, 4);

    // Write little-endian explicitly:
    out.push_back(uint8_t((u >> 0)  & 0xFF));
    out.push_back(uint8_t((u >> 8)  & 0xFF));
    out.push_back(uint8_t((u >> 16) & 0xFF));
    out.push_back(uint8_t((u >> 24) & 0xFF));
}


static
std::string base64_encode(const std::vector<uint8_t>& data) {

	static
	const char* B64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	std::string out;
    out.reserve(((data.size() + 2) / 3) * 4);

    size_t i = 0;
    while (i + 2 < data.size()) {
        uint32_t n = (uint32_t(data[i]) << 16) | (uint32_t(data[i+1]) << 8) | (uint32_t(data[i+2]));
        out.push_back(B64[(n >> 18) & 63]);
        out.push_back(B64[(n >> 12) & 63]);
        out.push_back(B64[(n >> 6) & 63]);
        out.push_back(B64[n & 63]);
        i += 3;
    }

    if (i < data.size()) {
        uint32_t n = uint32_t(data[i]) << 16;
        out.push_back(B64[(n >> 18) & 63]);

        if (i + 1 < data.size()) {
            n |= uint32_t(data[i+1]) << 8;
            out.push_back(B64[(n >> 12) & 63]);
            out.push_back(B64[(n >> 6) & 63]);
            out.push_back('=');
        }
        else {
            out.push_back(B64[(n >> 12) & 63]);
            out.push_back('=');
            out.push_back('=');
        }
    }

    return out;
}

void super_test(){

	const size_t nx = 35;
	const size_t ny = 35;

	std::vector<float> lon(nx*ny);
	std::vector<float> lat(nx*ny);
	size_t row = 0;
	for (int j = 0; j < ny; ++j) {
		row = j*nx;
		for (int i = 0; i < nx; ++i) {
			lon.at(row + i) = (rand()&255)*0.04;
			lon.at(row + i) = (rand()&255)*0.04;
		}
	}

	std::vector<uint8_t> lonBytes, latBytes;
	lonBytes.reserve(lon.size() * 4);
	latBytes.reserve(lat.size() * 4);

	for (size_t k = 0; k < lon.size(); k++) {
	    append_float32_le(lonBytes, lon[k]);
	    append_float32_le(latBytes, lat[k]);
	}

	std::string lonB64 = base64_encode(lonBytes);
	std::string latB64 = base64_encode(latBytes);

	//rain::VariableMap map;
	printf("data-nx=\"%d\" data-ny=\"%d\" ", nx, ny);
	printf("data-lon=\"%s\" data-lat=\"%s\" ", lonB64.c_str(), latB64.c_str());
}

} // drain::
