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
	enum Axis {UNDEFINED_ORIENTATION=0, HORZ, VERT};
	typedef drain::EnumFlagger<drain::SingleFlagger<Axis> > AxisFlagger;
	AxisFlagger orientation = HORZ;

	enum Direction {UNDEFINED_DIRECTION=0, INCR, DECR};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	DirectionFlagger direction = INCR;
	*/
	// LayoutSVG layout;

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	//  Current
	std::string groupName;
	int maxPerGroup = 10;
	bool absolutePaths = false;

	// Currently, applications are recommended to handle "false" and "none". Or "auto"?
	std::string title;
	FontSizes fontSize;

	/*
	enum Legend {NO_LEGEND=0, LEFT=1, RIGHT=2, DUPLEX=LEFT|RIGHT, EMBED=4};
	typedef drain::EnumFlagger<drain::MultiFlagger<Legend> > LegendFlagger;
	LegendFlagger legend;
	*/
	//typedef drain::EnumFlagger<drain::SingleFlagger<Legend> > LegendFlagger;


	inline  // layout(AlignCoord::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED),
	PanelConfSVG() :  maxPerGroup(10), absolutePaths(false){
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


//template <>
// const drain::EnumDict<AlignSVG_FOO>::dict_t  drain::EnumDict<AlignSVG_FOO>::dict;

struct TreeUtilsSVG {


	static PanelConfSVG defaultConf;

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
	void getBoundingFrame(const TreeSVG & group, drain::Frame2D<int> & frame, AlignBase::Axis orientation=AlignBase::Axis::HORZ); // UNDEFINED_ORIENTATION=2


	// NEW ---------------------
	static
	void superAlign(TreeSVG & node, AlignBase::Axis orientation = AlignBase::Axis::HORZ, LayoutSVG::Direction direction = LayoutSVG::Direction::INCR, const Point2D<svg::coord_t> & offset = {0,0}); // replaces alignSequence



	// static
	// void realignElem(TreeSVG & elem, const Box<svg::coord_t> & anchorBox);

	/// Align object respect to an anchor frame.
	static
	void realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & obj);

	static
	void realignObject(const Box<svg::coord_t> & anchorBoxHorz, const Box<svg::coord_t> & anchorBoxVert, TreeSVG & obj);

	static
	void realignObject(AlignBase::Axis axis, svg::coord_t pos, svg::coord_t width, TreeSVG & obj, svg::coord_t & newPos); // , Point2D<svg::coord_t> & newLocation);

	/// Recursively move elements with (x, y).
	static
	void translateAll(TreeSVG & group, const Point2D<svg::coord_t> &offset);



	/// Marker for...
	//  static
	//  const std::string attr_FRAME_REFERENCE;

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
const drain::EnumDict<image::LayoutSVG::Axis>::dict_t  drain::EnumDict<image::LayoutSVG::Axis>::dict;

template <>
const drain::EnumDict<image::LayoutSVG::Direction>::dict_t  drain::EnumDict<image::LayoutSVG::Direction>::dict;
*/

}  // drain::




#endif // TREESVG_H_

