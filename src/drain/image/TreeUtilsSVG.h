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
#include <drain/util/Flags.h>
#include <drain/util/Rectangle.h>
#include "TreeSVG.h"

namespace drain {

namespace image {

struct PanelConfSVG {

	enum Orientation {UNDEFINED_ORIENTATION=0, HORZ, VERT};
	typedef drain::EnumFlagger<drain::SingleFlagger<Orientation> > OrientationFlagger;
	OrientationFlagger orientation = HORZ;

	enum Direction {UNDEFINED_DIRECTION=0, INCR, DECR};
	typedef drain::EnumFlagger<drain::SingleFlagger<Direction> > DirectionFlagger;
	DirectionFlagger direction = INCR;

	enum Legend {NO_LEGEND=0, LEFT=1, RIGHT=2, EMBED=4};
	typedef drain::EnumFlagger<drain::MultiFlagger<Legend> > LegendFlagger;
	//LegendFlagger legend = LEFT | EMBED;
	LegendFlagger legend; // (PanelConfSVG::Legend::LEFT, PanelConfSVG::Legend::EMBED);

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	// Current
	std::string groupName;
	int maxPerGroup;
	bool relativePaths;

	std::string title;

	inline
	PanelConfSVG() : orientation(HORZ), direction(INCR), legend(LEFT, EMBED), maxPerGroup(10), relativePaths(true){
	}


};


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

	/// Returns the bounding box defined here as (x,y,width,height) of a single element.
	static
	bool getRect(TreeSVG & group, drain::Box<double> & rect);

	/// Computes the width and height for a bounding box  IMAGE and RECT elements.
	/**
	 *  The result is the minimal bounding box that covers the IMAGE and RECT elements aligned non-overlapping in a row (orientation \c HORZ ) or a column (orientation \c VERT ).
	 *
	 */
	static
	void determineBBox(TreeSVG & group, drain::Frame2D<int> & frame, PanelConfSVG::Orientation orientation=PanelConfSVG::UNDEFINED_ORIENTATION);

	/// Stack IMAGE and RECT elements within a frame (width x height) to a row or column
	static
	void alignSequence(TreeSVG & group, const drain::Frame2D<int> & frame, const drain::Point2D<int> & start={0,0},
			PanelConfSVG::Orientation orientation=PanelConfSVG::UNDEFINED_ORIENTATION, PanelConfSVG::Direction direction=PanelConfSVG::UNDEFINED_DIRECTION);

	/// Moves TEXT elems to desired positions, esp corners (LEFT|RIGHT), (TOP|BOTTOM)
	static
	void alignText(TreeSVG & group);


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

template <>
const drain::EnumDict<image::PanelConfSVG::Orientation>::dict_t  drain::EnumDict<image::PanelConfSVG::Orientation>::dict;

template <>
const drain::EnumDict<image::PanelConfSVG::Direction>::dict_t  drain::EnumDict<image::PanelConfSVG::Direction>::dict;

}  // drain::




#endif // TREESVG_H_

