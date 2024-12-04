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
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#ifndef DRAIN_TREE_UTILS_SVG
#define DRAIN_TREE_UTILS_SVG

//

#include <drain/util/Frame.h>
#include <drain/util/EnumFlags.h>
#include <drain/util/Rectangle.h>
#include "AlignSVG.h"
#include "TreeSVG.h"

namespace drain {

namespace image {

class FontSizes : public drain::UniTuple<double,4> {


public:

	typedef drain::UniTuple<double,4> base_t;

	double & titles;
	double & header;
	double & leftHeader;
	double & rightHeader;

	inline
	FontSizes() : base_t(12,10,8,6), titles(next()), header(next()), leftHeader(next()), rightHeader(next()) {
	}

	inline
	FontSizes(const FontSizes & fs) : base_t(fs), titles(next()), header(next()), leftHeader(next()), rightHeader(next()) {
	}

};

struct PanelConfSVG {

	/*
	enum Orientation {UNDEFINED_ORIENTATION=0, HORZ, VERT};
	typedef drain::EnumFlagger<drain::SingleFlagger<Orientation> > OrientationFlagger;
	OrientationFlagger orientation = HORZ;

	enum Direction {UNDEFINED_DIRECTION=0, INCR, DECR};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	DirectionFlagger direction = INCR;
	*/
	LayoutSVG layout;

	enum Legend {NO_LEGEND=0, LEFT=1, RIGHT=2, DUPLEX=LEFT|RIGHT, EMBED=4};
	typedef drain::EnumFlagger<drain::MultiFlagger<Legend> > LegendFlagger;
	//typedef drain::EnumFlagger<drain::SingleFlagger<Legend> > LegendFlagger;
	LegendFlagger legend;

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	// Current
	std::string groupName;
	int maxPerGroup = 10;
	bool absolutePaths = false;

	// Currently, applications are recommended to handle "false" and "none". Or "auto"?
	std::string title;
	FontSizes fontSize;

	inline
	PanelConfSVG() : layout(LayoutSVG::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED), maxPerGroup(10), absolutePaths(false){
	}

	/*
	inline
	PanelConfSVG(const PanelConfSVG & c) : orientation(c.orientation), direction(c.direction), legend(LEFT, EMBED), maxPerGroup(10), absolutePaths(false){
	}
	*/


};

/// SVG classes marking requests of relative horizontal and vertical alignment.
/**
 *
 */
/*
enum AlignSVG_FOO { // DEPRECATING? See svgAlign...
	ALIGN_GROUP  = 0b00000, // Container (group) inside which elements will be aligned
	// Horizontal
	LEFT   = 0b00001,
	RIGHT  = 0b00010,  // Originally for swapping  LEFT <-> RIGHT
	CENTER = 0b00011,
	HORZ   = 0b00011, // yes, same
	// Vertical
	TOP    = 0b00100,
	BOTTOM = 0b01000,  // Originally for swapping
	MIDDLE = 0b01100,
	VERT   = 0b01100, // yes, same

	REF    = 0b10000,
	REF_LEFT   = (REF |   LEFT),
	REF_CENTER = (REF | CENTER),
	REF_RIGHT  = (REF |  RIGHT),
	REF_TOP    = (REF |    TOP),
	REF_MIDDLE = (REF | MIDDLE),
	REF_BOTTOM = (REF | BOTTOM),

	FLOAT    = (HORZ|VERT), // On-top. To be replaced
	ALIGN    = 0b11111,
	PANEL    = 0b100000, // Group of elements aligned together
	ANCHOR   = 0b100001, // Main element, "anchor" in a PANEL group
    RELATIVE = 0b100010, // Element aligned relative to an ANCHOR in a PANEL group
};
*/

//template <>
// const drain::EnumDict<AlignSVG_FOO>::dict_t  drain::EnumDict<AlignSVG_FOO>::dict;

struct TreeUtilsSVG {


	static PanelConfSVG defaultConf;


	/*
	enum Orientation {UNDEFINED_ORIENTATION=0, HORZ, VERT};
	typedef drain::EnumFlagger<drain::SingleFlagger<Orientation> > OrientationFlagger;
	static OrientationFlagger defaultOrientation;

	enum Direction {UNDEFINED_DIRECTION=0, INCR, DECR};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	static DirectionFlagger defaultDirection;

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	static std::string defaultGroupName;

	static std::string defaultTitle;
	*/

	/// Returns the bounding box defined here as (x,y,width,height) of an element.
	// static
	// bool getElementBounds(const TreeSVG & group, drain::Box<double> & box);

	/// NEW Returns the bounding box defined here as (x,y,width,height) of a group of elements.
	/*
	static inline
	bool getBoundsFoo(const TreeSVG & group, drain::Box<NodeSVG::coord_t> & box){
		box.x = std::numeric_limits<svg::coord_t>::max(); // lowest();
		box.y = std::numeric_limits<svg::coord_t>::max(); // lowest();
		box.width  = 0;
		box.height = 0;
		return false; // getBoundsInner(group, box);
	}
	*/

// protected:
	static
	bool computeBoundingBox(const TreeSVG & group, drain::Box<NodeSVG::coord_t> & box);

public:

	/// Computes the width and height for a bounding box  IMAGE and RECT elements.
	/**
	 *  The result is the minimal bounding box that covers the IMAGE and RECT elements aligned non-overlapping in a row (orientation \c HORZ ) or a column (orientation \c VERT ).
	 *
	 *  Future versions may also handle CIRCLE and TEXT (location)
	 */
	static
	void getBoundingFrame(const TreeSVG & group, drain::Frame2D<int> & frame, LayoutSVG::Orientation orientation=LayoutSVG::UNDEFINED_ORIENTATION);



	/// Stack IMAGE and RECT elements within a frame (width x height) to a row or column
	// alignDomain!
	static
	void alignSequenceOLD(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start={0,0},
			LayoutSVG::Orientation orientation=LayoutSVG::UNDEFINED_ORIENTATION, LayoutSVG::Direction direction=LayoutSVG::UNDEFINED_DIRECTION);


	static
	void markAligned(const TreeSVG & parentGroup, TreeSVG & alignedGroup); // TODO: frame={0,0} for margins/offsets etc from border?

	template <class ...TT>
	static inline
	void markAligned(const TreeSVG & parentGroup, TreeSVG & alignedGroup, const TT & ...args){
		markAligned(parentGroup, alignedGroup);
		alignedGroup->addClass(args...);
	};
	// TODO: frame={0,0} for margins/offsets etc from border?


	/// Moves TEXT elems to desired positions, esp corners (LEFT|RIGHT), (TOP|BOTTOM)
	static
	void alignText(TreeSVG & group);


	// NEW ---------------------
	static
	void superAlign(TreeSVG & node, const Point2D<svg::coord_t> & offset = {0,0}); // replaces alignSequence


	/// Align PANEL groups inside an ALIGN_GROUP group
	static
	void alignDomains(TreeSVG & group); // replaces alignSequence

	/// Align PANEL groups inside an ALIGN_DOMAIN group
	static
	void alignPanels(TreeSVG & alignGroup, Point2D<double> &startPos);

	// static
	// void realignElem(TreeSVG & elem, const Box<svg::coord_t> & anchorBox);

	/// Align object respect to an anchor frame.
	static
	void realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & obj);

	/// Recursively move elements with (x, y).
	static
	void translateAll(TreeSVG & group, const Point2D<svg::coord_t> &offset);



//

	/// Marker for...
	static
	const std::string attr_FRAME_REFERENCE;

protected:

	/// Align object respect to an anchor frame.
	static
	void realignObjectOLD(const Box<svg::coord_t> & anchorBox, TreeSVG & obj);

};



/**
 *
class AttributeCheckerXML {

public:

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path);

	inline
	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){
		return 0;
	}

};
 */



}  // image::

/*
template <>
const drain::EnumDict<image::LayoutSVG::Orientation>::dict_t  drain::EnumDict<image::LayoutSVG::Orientation>::dict;

template <>
const drain::EnumDict<image::LayoutSVG::Direction>::dict_t  drain::EnumDict<image::LayoutSVG::Direction>::dict;
*/

}  // drain::




#endif // TREESVG_H_

