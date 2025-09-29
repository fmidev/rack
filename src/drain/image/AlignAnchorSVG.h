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


// Could be internal class in Adapter?
struct AnchorElem : public std::string {

	enum Anchor {
		DEFAULT = 0, // previous or group anchor
		NONE = 1,
		PREVIOUS,
		EXTENSIVE,
	};


	inline
	AnchorElem(const std::string & s="") : std::string(s){
	};

	inline
	AnchorElem(const AnchorElem & a) : std::string(a){
	};

	inline
	const std::string & str() const {
		return *this;
	}

	inline
	std::string & str() {
		return *this;
	}

	void set(const std::string & s);

	void set(const AnchorElem::Anchor & anchor);


	/// Make it dynamic, practically changed to last updated
	/*
	inline
	void setFlipping(){
		assign("*");
	}


	inline
	void setSpecific(const std::string & s){
		if (s.empty()){
			throw std::runtime_error("AnchorElem setSpecific arg empty");
		}
		else if (s == "*"){
			throw std::runtime_error("AnchorElem setSpecific arg '*' reserved for 'flipping' mode");
		}
		assign(s);
	}
	*/


	/// If not set, use default.
	inline
	bool isSet() const {
		return !empty();
	}

	/// Make it dynamic, practically changed to last updated
	inline
	bool isExtensive() const {
		return (drain::EnumDict<AnchorElem::Anchor>::getValue(*this) == Anchor::EXTENSIVE);
	}

	bool isPrevious() const {
		return (drain::EnumDict<AnchorElem::Anchor>::getValue(*this) == Anchor::PREVIOUS);
	}

	/// Named element
	inline
	bool isSpecific() const {
		return isSet() && (!isExtensive()) &&  (!isPrevious());
	}


	// Perhaps not needed. (Alignment applied only if class directs so.)
	/*
	bool isNone() const {
		return (drain::EnumDict<AnchorElem::Anchor>::getValue(*this) == Anchor::NONE);
	}
	*/

};


/// Adapter designed for NodeSVG
struct AlignAnchorSVG { // : public AlignSVG {

	typedef AnchorElem anchor_t;

	/// Mark one of the elements of this object (SVG or G) as a decisive position
	// template <class T>
	inline
	void setMyAlignAnchor(const std::string & pathElem){
		myAnchorVert = myAnchorHorz = pathElem; // getElem(pathElem);
		updateAlign();
	}

	// template <class T>
	inline
	void setMyAlignAnchorHorz(const std::string & pathElem){
		myAnchorHorz = pathElem; // getElem(pathElem);
		updateAlign();
	}

	//template <class T>
	inline
	void setMyAlignAnchorVert(const std::string & pathElem){
		myAnchorVert = pathElem; // getElem(pathElem);
		updateAlign();
	}

	inline
	void setDefaultAlignAnchor(const std::string & pathElem){
		defaultAnchorVert = defaultAnchorHorz = pathElem; // getElem(pathElem);
		updateAlign();
	}

	// template <class T>
	inline
	void setDefaultAlignAnchorHorz(const std::string & pathElem){
		defaultAnchorHorz = pathElem; // getElem(pathElem);
		updateAlign();
	}

	//template <class T>
	inline
	void setDefaultAlignAnchorVert(const std::string & pathElem){
		defaultAnchorVert = pathElem; // getElem(pathElem);
		updateAlign();
	}

	/// Store anchor object/symbol for aligning this object.
	template <AlignBase::Axis AX>
	const anchor_t & getMyAlignAnchor() const;

	/// Store anchor object/symbol for aligning object inside this G (group) or SVG element.
	template <AlignBase::Axis AX>
	const anchor_t & getDefaultAlignAnchor() const;

	/*
	inline
	const anchor_t & getAlignAnchorVert() const {
		return myAnchorVert;
	}

	inline
	const anchor_t & getAlignAnchorDefaultHorz() const {
		return defaultAnchorHorz;
	}

	inline
	const anchor_t & getAlignAnchorDefaultVert() const {
		return defaultAnchorVert;
	}
	*/


	inline virtual
	~AlignAnchorSVG(){};

protected:

	/// Redefined in NodeSVG
	/*
	virtual inline
	void updateAlign() override {
		updateAlignStr();
	}
	*/
	virtual
	void updateAlign() = 0;

	// std::string alignStr;

	// essentially std::string's
	anchor_t myAnchorHorz;
	anchor_t myAnchorVert;

	anchor_t defaultAnchorHorz;
	anchor_t defaultAnchorVert;

	//void updateAlignStr();

};


template <>
inline
const AlignAnchorSVG::anchor_t & AlignAnchorSVG::getMyAlignAnchor<AlignBase::Axis::HORZ>() const {
	return myAnchorHorz;
};

template <>
inline
const AlignAnchorSVG::anchor_t & AlignAnchorSVG::getMyAlignAnchor<AlignBase::Axis::VERT>() const {
	return myAnchorVert;
};

/// Store anchor object/symbol for aligning object inside this G (group) or SVG element.
template <>
inline
const AlignAnchorSVG::anchor_t & AlignAnchorSVG::getDefaultAlignAnchor<AlignBase::Axis::HORZ>() const {
	return defaultAnchorHorz;
};

template <>
inline
const AlignAnchorSVG::anchor_t & AlignAnchorSVG::getDefaultAlignAnchor<AlignBase::Axis::VERT>() const {
	return defaultAnchorVert;
};




}  // image::

DRAIN_ENUM_DICT(drain::image::AnchorElem::Anchor);
DRAIN_ENUM_OSTREAM(drain::image::AnchorElem::Anchor);

}  // drain::



#endif // DRAIN_ALIGN_SVG_H_

