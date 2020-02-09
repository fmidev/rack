/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/
/*
Part of Rack development has been done in the BALTRAD projects part-financed
by the European Union (European Regional Development Fund and European
Neighbourhood Partnership Instrument, Baltic Sea Region Programme 2007-2013)
*/
#ifndef DRAIN_CODEMAP
#define DRAIN_CODEMAP

//#include "Geometry.h"
//#include "File.h"
//#include "TreeSVG.h"

//#include "util/JSONtree.h"
#include <ostream>
//#include <vector>
#include <map>

#include "../util/ValueScaling.h"

namespace drain
{


template <class T>
class ImageCodeMap : public std::vector<T> {
//class ImageCodeMap : public std::map<int,T> {

public:

	typedef T entry_t;

	//typedef std::map<int,T> cont_t;
	typedef std::vector<T> cont_t;
	typedef typename cont_t::size_type size_t;
	typedef T value_t;

	/// Default constructor
	ImageCodeMap(size_t n=0, const T & value=T()): cont_t(n, value), separator(0) {
	//ImageCodeMap() : separator(0) {
	};

	/// Copy constructor //
	ImageCodeMap(const cont_t & v): cont_t(v), separator(0) {
	//ImageCodeMap(const cont_t & map): separator(0) { // COPY?
	};


	inline
	const entry_t & retrieve(double intensity) const {

		size_t index;

		if (scaling.isScaled()){
			index = static_cast<size_t>(scaling.inv(intensity));
		}
		else {
			index = static_cast<size_t>(intensity);
		}

		return operator [](index);

	}

	inline
	entry_t & retrieve(double intensity) {

		size_t index;

		if (scaling.isScaled()){
			index = static_cast<size_t>(scaling.inv(intensity));
		}
		else {
			index = static_cast<size_t>(intensity);
		}

		return operator [](index);

	}

	static inline
	bool empty(unsigned short int i){
		return (i==0);
	}
	static inline
	bool empty(short int i){
		return (i==0);
	}

	static inline
	bool empty(int i){
		return (i==0);
	}

	static inline
	bool empty(long int i){
		return (i==0);
	}

	static inline
	bool empty(unsigned int i){
		return (i==0);
	}


	template <class E>
	static
	bool empty(const E & e){
		return e.empty();
	}

	///
	/*
	 *    \param equal - typically =, :, or -
	 *    \param start - typically hyphen or leading parenthesis (, {, [
	 *    \param end   - typically hyphen or trailing parenthesis ), }, [
	 *    \param separator - typically comma or semicolon
	 */

	ValueScaling scaling;

	char separator;
};


class ImageCodeEntry {
public:

	virtual inline
	~ImageCodeEntry(){};

	virtual
	bool empty() const = 0;
};






// -----------------------

class LegendEntry { //: public std::string {

public:

	std::string label;

};

inline
std::ostream & operator<<(const LegendEntry & entry, std::ostream &ostr){
	//return ostr << (const std::string &)entry;
	return ostr << entry.label;
}


class Legend : public ImageCodeMap<std::string> {
public:

	Legend() : ImageCodeMap<std::string>(256){};

};


template <>
std::ostream & JSONwriter::toStream(const Legend & l, std::ostream &ostr, unsigned short indentation){
	//return JSONwriter::mapToStream(l, ostr, indentation);
	return JSONwriter::sparseSequenceToStream(l, ostr);
}


/*
template <class T>
inline
std::ostream & operator<<(std::ostream &ostr, const ImageCodeMap<T> & m){
	m.toOStream(ostr);
	return ostr;
}
*/



} // drain::

#endif
