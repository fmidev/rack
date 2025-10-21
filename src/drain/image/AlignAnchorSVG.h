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

/*  Semantics:
 *
 *  if myAnchor is set, use it.
 *  if myAnchor is unset, use group anchor; it servers as a default anchor
 *  if groupAnchor is unset -> use previous object as anchor.
 *
 *  GroupAnchor (default anchor):
    DEFAULT(=UNSET): use previous object
	NONE: use nothing, don't align
	PREVIOUS: use previous (unneeded, this is the default for group)
	COLLECTIVE: use compound bounding bbox

	MyAnchor (specific anchor, always overrides)
	DEFAULT(=UNSET) -> use GroupAnchor
	NONE: don't align me! (raise error if Align set?)
	PREVIOUS:
	COLLECTIVE: use compound bounding box

	SPECIAL: both DEFAULT: use PREVIOUS
 *
 */

// Could be internal class in Adapter?
struct AnchorElem : public std::string {

	// More like a symbol.
	enum Anchor {
		DEFAULT = 0, // use group's default anchor, or if that is not set, use previous
		NONE = 1,    // don't align at alla
		PREVIOUS,	 // use previous object
		NEXT,	     // use next object - applied typically by background map adapting to actual overlay image (requires revisiting loop)
		COLLECTIVE_CURRENT,  // use dynamically growing bounding box or the compound
		COLLECTIVE_FINAL,    // use dynamically growing bounding box or the compound - in the end.
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

	/// If not set, use default.
	inline
	bool isSet() const {
		return !empty();
	}

	// Use the bounding box of the compound, "accumulated" object as anchor.
	inline
	bool isCollective() const {
		return (drain::EnumDict<AnchorElem::Anchor>::getValue(*this) == Anchor::COLLECTIVE_CURRENT);
	}

	// Use the bounding box of the latest object as anchor.
	bool isPrevious() const {
		return (drain::EnumDict<AnchorElem::Anchor>::getValue(*this) == Anchor::PREVIOUS);
	}

	// Explicitly states that an object uses no anchor, hence the position is absolute (not aligned)
	bool isNone() const {
		return (drain::EnumDict<AnchorElem::Anchor>::getValue(*this) == Anchor::NONE);
	}

	// Use the bounding box of a named object as anchor.
	inline
	bool isSpecific() const {
		return isSet() && (!isCollective()) &&  (!isPrevious()) &&  (!isNone());
	}


};

}

}

/*
DRAIN_ENUM_DICT(drain::image::AnchorElem::Anchor);

DRAIN_ENUM_OSTREAM(drain::image::AnchorElem::Anchor);
*/


namespace drain {

DRAIN_ENUM_DICT(image::AnchorElem::Anchor);
DRAIN_ENUM_OSTREAM(image::AnchorElem::Anchor);

namespace image {

/// Adapter designed for NodeSVG
struct AlignAnchorSVG { // : public AlignSVG {


public:

	typedef AnchorElem anchor_t; // needed?

	template <class T>
	inline
	void setMyAlignAnchor(const T & value){
		adjustAnchor(myAnchorHorz, value);
		myAnchorVert = myAnchorHorz;
		updateAlign();
	}

	template <AlignBase::Axis AX, class T>
	inline
	void setMyAlignAnchor(const T & value){
		adjustAnchor(getMyAlignAnchor<AX>(), value);
		updateAlign();
	}

	template <class T>
	inline
	void setDefaultAlignAnchor(const T & value){
		adjustAnchor(defaultAnchorHorz, value);
		defaultAnchorVert = defaultAnchorHorz;
		updateAlign();
	}

	template <AlignBase::Axis AX, class T>
	inline
	void setDefaultAlignAnchor(const T & value){
		AnchorElem & defaultAnchor = getDefaultAlignAnchor<AX>();
		adjustAnchor(defaultAnchor, value);
		if (defaultAnchor.isPrevious()){
			Logger(__FILE__, __FUNCTION__, __LINE__).suspicious("Removing explicit ", AnchorElem::Anchor::PREVIOUS, " as non-informative for group default");
			// getDefaultAlignAnchor<AX>() = AnchorElem::Anchor::DEFAULT;
			adjustAnchor(defaultAnchor, AnchorElem::Anchor::DEFAULT);
		}
		updateAlign();
	}


	/// Store anchor object/symbol for aligning this object.
	template <AlignBase::Axis AX>
	const anchor_t & getMyAlignAnchor() const;

	///
	template <AlignBase::Axis AX>
	const anchor_t & getDefaultAlignAnchor() const;

	///
	template <AlignBase::Axis AX>
	anchor_t & getMyAlignAnchor();

	///
	template <AlignBase::Axis AX>
	anchor_t & getDefaultAlignAnchor();



	inline virtual
	~AlignAnchorSVG(){};

	void swapAnchors(AlignAnchorSVG & anchors);

protected:

	// "string detectors"

	static inline
	void adjustAnchor(AnchorElem & anchor, const std::string & value){
		anchor.assign(value);
	}

	static inline
	void adjustAnchor(AnchorElem & anchor, const char * value){
		anchor.assign(value);
	}

	static inline
	void adjustAnchor(AnchorElem & anchor, const AnchorElem & elem){
		anchor.assign(elem);
	}

	template <class T>
	static inline
	void adjustAnchor(AnchorElem & anchor, const T & value){
		anchor.assign(EnumDict<T>::getKey(value));
	}

	/// Redefined in NodeSVG
	virtual
	void updateAlign() = 0;

	// essentially std::string's
	anchor_t myAnchorHorz;
	anchor_t myAnchorVert;

	anchor_t defaultAnchorHorz;
	anchor_t defaultAnchorVert;


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

template <>
inline
AlignAnchorSVG::anchor_t & AlignAnchorSVG::getMyAlignAnchor<AlignBase::Axis::HORZ>() {
	return myAnchorHorz;
};

template <>
inline
AlignAnchorSVG::anchor_t & AlignAnchorSVG::getMyAlignAnchor<AlignBase::Axis::VERT>() {
	return myAnchorVert;
};

/// Store anchor object/symbol for aligning object inside this G (group) or SVG element.
template <>
inline
AlignAnchorSVG::anchor_t & AlignAnchorSVG::getDefaultAlignAnchor<AlignBase::Axis::HORZ>() {
	return defaultAnchorHorz;
};

template <>
inline
AlignAnchorSVG::anchor_t & AlignAnchorSVG::getDefaultAlignAnchor<AlignBase::Axis::VERT>() {
	return defaultAnchorVert;
};



}  // image::



}  // drain::

//DRAIN_ENUM_DICT(drain::image::AnchorElem::Anchor);
//DRAIN_ENUM_OSTREAM(drain::image::AnchorElem::Anchor);

#endif // DRAIN_ALIGN_SVG_H_

