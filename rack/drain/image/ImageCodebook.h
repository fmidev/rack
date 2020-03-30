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
#ifndef DRAIN_CODEBOOK
#define DRAIN_CODEBOOK

//#include "Geometry.h"
//#include "File.h"
//#include "TreeSVG.h"

//#include "util/JSONtree.h"
#include <ostream>
#include <vector>
#include <map>

#include "../util/Log.h"
#include "../util/ValueScaling.h"

namespace drain
{

///
/**   \tparam double - type of lower bound of the value range associated with an entry
 *    \tparam T - entry type (LegendEntry, PaletteEntry, HistogramEntry)
 *
 */
template <class T>
//class ImageCodeMap : public std::vector<T> {
class ImageCodeMap : public std::map<double,T> {

public:

	typedef std::map<double,T> cont_t;
	//typedef typename cont_t::size_type size_t;
	typedef typename cont_t::key_type     key_t;
	typedef typename cont_t::value_type entry_t;



	/// Default constructor
	//ImageCodeMap(size_t n=0, const T & value=T()): cont_t(n, value), separator(0) {
	ImageCodeMap() : separator(0) {
	};

	/// Copy constructor //
	// ImageCodeMap(const cont_t & v): cont_t(v), separator(0) {
	ImageCodeMap(const cont_t & map): separator(0) { // COPY?
	};


	/// LOOK-UP table, consider outsourcing this

	typedef std::vector<typename cont_t::const_iterator> lookup_t;

	mutable
	lookup_t lookUp;

	lookup_t & createLookUp(size_t n, const ValueScaling & scaling, int shiftBits = 0) const {

		drain::Logger mout(__FUNCTION__, __FILE__);

		typename cont_t::const_iterator itLower = this->begin();

		lookUp.resize(n, itLower);

		// Signed, because scaling physical values may cause underflow
		int index, indexLower=0;

		for (typename cont_t::const_iterator it=this->begin(); it!=this->end(); ++it){

			index = (static_cast<int>(scaling.inv(it->first)) >> shiftBits);

			if (index < 0){
				mout.warn() << "threshold " << it->first << " mapped to index (" << index << ") < 0, skipping " << mout.endl;
				continue;
			}

			//if (static_cast<size_t>(index) >= n){
			if (index >= n){
				mout.warn() << "threshold " << it->first << " mapped to index (" << index << ") > max (" << (n-1) << "), skipping " << mout.endl;
				continue;
			}

			if (indexLower < index){
				mout.debug() << "adding [" << indexLower << '-' << index << "[ -> \t";
				mout << '[' << itLower->first << "] // " << itLower->second << mout.endl;
			}

			//for (key_t i=itLower->first; i<it->first; ++i){
			for (int i=indexLower; i<index; ++i){
				lookUp[i] = itLower;
			}

			indexLower = index;
			itLower    = it;

		}

		index = n;
		if (indexLower < index){
			mout.debug() << "padding [" << indexLower << '-' << index << "[ -> \t";
			mout << '[' << itLower->first << "] // " << itLower->second << mout.endl;
		}
		for (int i=indexLower; i<index; ++i){
			lookUp[i] = itLower;
		}

		return lookUp;
	}

	// inline
	typename cont_t::const_iterator retrieve(double d) const {

		typename cont_t::const_iterator it = this->begin();
		typename cont_t::const_iterator rit = it; // result

		while (it != this->end()){
			if (it->first > d)
				return rit;
			rit = it;
			++it;
		}

		return rit; // may be invalid

	}

	// inline
	typename cont_t::iterator retrieve(double d) {

		typename cont_t::iterator  it = this->begin();
		typename cont_t::iterator rit = it;

		while (it != this->end()){
			if (it->first > d)
				return rit;
			rit = it;
			++it;
		}

		return rit; // may be invalid

	}


	/*
	inline
	const entry_t & retrieve(double intensity) const {

		key_t index;

		if (scaling.isScaled()){
			index = static_cast<key_t>(scaling.inv(intensity));
		}
		else {
			index = static_cast<key_t>(intensity);
		}

		return operator [](index);

	}

	inline
	entry_t & retrieve(double intensity) {

		key_t index;

		if (scaling.isScaled()){
			index = static_cast<key_t>(scaling.inv(intensity));
		}
		else {
			index = static_cast<key_t>(intensity);
		}

		return operator [](index);

	}
	*/

	// Rename to entryEmpty?
	/*
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
	*/

	///
	/*
	 *    \param equal - typically =, :, or -
	 *    \param start - typically hyphen or leading parenthesis (, {, [
	 *    \param end   - typically hyphen or trailing parenthesis ), }, [
	 *    \param separator - typically comma or semicolon
	 */

	// ValueScaling scaling; more like property of image

	char separator;
};

/*
class ImageCodeEntry {
public:

	virtual inline
	~ImageCodeEntry(){};

	virtual
	bool empty() const = 0;
};
*/


} // drain::

#endif
