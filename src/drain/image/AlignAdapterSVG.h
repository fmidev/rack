/*

MIT License

Copyright (c) 2023 FMI Open Development / Markus Peura, first.last@fmi.fi

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
/*
 * TreeSVG.h
 *
 *  Created on: Nov, 2024
 *      Author: mpeura
 */

#ifndef DRAIN_ALIGN_ADAPTER_SVG
#define DRAIN_ALIGN_ADAPTER_SVG

//#include "drain/util/FileInfo.h"
// #include "drain/util/Frame.h"
//
#include <string>
//#include "drain/util/EnumFlags.h"

#include "AlignSVG.h"

namespace drain {

namespace image {


/// Adapter designed for NodeSVG
struct AlignAdapterSVG : public AlignSVG {

	/// Mark one of the elements of this object (SVG or G) as a decisive position
	// template <class T>
	inline
	void setAlignAnchor(const std::string & pathElem){
		anchorVert = anchorHorz = pathElem; // getElem(pathElem);
		updateAlign();
	}

	// template <class T>
	inline
	void setAlignAnchorHorz(const std::string & pathElem){
		anchorHorz = pathElem; // getElem(pathElem);
		updateAlign();
	}

	//template <class T>
	inline
	void setAlignAnchorVert(const std::string & pathElem){
		anchorVert = pathElem; // getElem(pathElem);
		updateAlign();
	}

	///
	inline
	const std::string & getAlignAnchorHorz() const {
		return anchorHorz;
	}

	inline
	const std::string & getAlignAnchorVert() const {
		return anchorVert;
	}

	inline
	const std::string & getAlignStr() const {
		return alignStr;
	}

	inline virtual
	~AlignAdapterSVG(){};

protected:

	/*
	static inline
	const std::string & getElem(const std::string &s){
		return s;
	};

	static inline
	const char * getElem(const char *s){
		return s;
	};
	*/

	/*
	template <class T>
	static inline
	const std::string & getElem(const T & type){
		// return EnumDict<T>::getKey(type, false);
		return EnumDict<T>::dict.getKey(type, false);
	}
	*/


	virtual inline
	void updateAlign() override {
		updateAlignStr();
	}

	std::string alignStr;

	std::string anchorHorz;
	std::string anchorVert;

	void updateAlignStr();

};






}  // image::

}  // drain::



#endif // DRAIN_ALIGN_SVG_H_

