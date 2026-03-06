/*
 * Base64.h
 *
 *  Created on: Jan 26, 2026
 *      Author: mpeura
 */

#ifndef DRAIN_UTIL_BASE64
#define DRAIN_UTIL_BASE64

#include <cstdint>
#include <cstring>

#include <drain/Enum.h>
//#include <drain/Sprinter.h>
#include "Outlet.h"

namespace drain {



class Base64 : public std::vector<uint8_t>  {

public:

	enum NumType {
		Int8,
		Int16,
		Int32,
		Float16,
		Float32,
		Float64,
	};

	NumType getType(){
		return type;
	}


protected:
	NumType type;
	std::vector<uint8_t> bytes;

public:


	// static
	// void convert(const std::vector<float> & data, std::vector<uint8_t> & bytes);
	template <typename T>
	static
	void convert(const std::vector<T> & data, std::vector<uint8_t> & bytes){

		static const size_t N = sizeof(T);

		bytes.reserve(data.size() * N); // float
		for (const auto & x: data) {
			append_value_le(bytes, x);
			//append_float32_le(bytes, x);
		}
	}


	inline
	void convert(const std::vector<float> & data){
		type = Float32;
		convert(data, *this);
	}

	/*
	static
	inline void append_float32_le(std::vector<uint8_t>& out, float v) {
	    uint32_t u;
	    static_assert(sizeof(float) == 4, "float must be 32-bit");
	    std::memcpy(&u, &v, 4);

	    // Write little-endian explicitly:
	    out.push_back(uint8_t((u >> 0)  & 0xFF));
	    out.push_back(uint8_t((u >> 8)  & 0xFF));
	    out.push_back(uint8_t((u >> 16) & 0xFF));
	    out.push_back(uint8_t((u >> 24) & 0xFF));
	}
	*/

	template <typename T>
	static
	inline void append_value_le(std::vector<uint8_t>& out, T v) {

		static
		const size_t N = sizeof(T);

		uint32_t u;
		std::memcpy(&u, &v, N);

		for (size_t i=0; i<N; ++i){
			// Write little-endian explicitly:
			out.push_back(uint8_t(u & 0xFF));
			u = (u>>8);
		}
		// Write little-endian explicitly:
		/*
		out.push_back(uint8_t((u >> 0)  & 0xFF));
		out.push_back(uint8_t((u >> 8)  & 0xFF));
		out.push_back(uint8_t((u >> 16) & 0xFF));
		out.push_back(uint8_t((u >> 24) & 0xFF));
		*/
	}


	static
	void base64_encode(const std::vector<uint8_t>& data, std::string & out);

	/*
	template <typename T>
	static
	void createArray(const Outlet & code, const std::string & variableName, NumType type, const T & sequence);
	*/

protected:

	static
	void assign();

};

DRAIN_ENUM_DICT(Base64::NumType);
DRAIN_ENUM_OSTREAM(Base64::NumType);

/*
template <typename T>  // std::string & code  // std::ostream & ostr
void Base64::createArray(const Outlet & code, const std::string & variableName, NumType type, const T & sequence){
	code << "const " << variableName << " = new " << type << "Array(";
	Sprinter::toStream(code.getOStream(), sequence, Sprinter::jsLayout);
	code << ");";
}
*/


} // drain::

#endif // SRC_DRAIN_UTIL_Base64_H_
