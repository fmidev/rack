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
/*
struct PanelConfSVG {

	/// SVG file may contain several "modules", for example rows or columns of IMAGE:s. This is the name of the current module, contained in a GROUP.
	bool absolutePaths = true;

	// Currently, applications are recommended to handle "false" and "none". Or "auto"?
	// std::string title;
	// FontSizes fontSize;

	inline  // maxPerGroup(10), layout(Alignment::HORZ, LayoutSVG::INCR), legend(LEFT, EMBED),
	PanelConfSVG() : absolutePaths(true){
	}

};
*/

/// SVG classes marking requests of relative horizontal and vertical alignment.
/**
 *
 */


//template <>
// const drain::EnumDict<AlignSVG_FOO>::dict_t  drain::EnumDict<AlignSVG_FOO>::dict;

struct TreeUtilsSVG {

public:

	// static PanelConfSVG defaultConf; // Consider separating Rack-specific properties.

	/*
	static
	const std::set<XML::intval_t> abstractTags;

	static
	bool isAbstract(XML::intval_t tag);
	*/

	/// Compute bounding box of the whole structure.
	/**
	 *
	 */
	static
	bool computeBoundingBox(const TreeSVG & group, drain::Box<svg::coord_t> & box);

	/// Compute bounding box and set the top-level SVG width, height and viewBox properties.
	/**
	 *
	 */
	static
	void finalizeBoundingBox(TreeSVG & svg);


	/// Computes the width and height for a bounding box  IMAGE and RECT elements.
	/**
	 *  The result is the minimal bounding box that covers the IMAGE and RECT elements aligned non-overlapping in a row (orientation \c HORZ ) or a column (orientation \c VERT ).
	 *
	 *  Future versions may also handle CIRCLE and TEXT (location)
	 */
	// static
	// void getBoundingFrame(const TreeSVG & group, drain::Frame2D<int> & frame, AlignBase::Axis orientation=AlignBase::Axis::HORZ);

	static
	void setRelativePaths(drain::image::TreeSVG & object, const drain::FilePath & filepath);


	// NEW ---------------------
	static
	void superAlign(TreeSVG & node, AlignBase::Axis orientation = AlignBase::Axis::HORZ, LayoutSVG::Direction direction = LayoutSVG::Direction::INCR);
	//const Point2D<svg::coord_t> & offset = {0,0}); // replaces alignSequence



	// static
	// void realignElem(TreeSVG & elem, const Box<svg::coord_t> & anchorBox);

	/// Align object respect to an anchor frame.
	// static
	// void realignObject(const Box<svg::coord_t> & anchorBox, TreeSVG & obj);

	static
	void realignObject(const Box<svg::coord_t> & anchorBoxHorz, const Box<svg::coord_t> & anchorBoxVert, TreeSVG & obj);

	// static
	// void realignObject(AlignBase::Axis axis, svg::coord_t pos, svg::coord_t width, TreeSVG & obj, svg::coord_t & newPos); // , Point2D<svg::coord_t> & newLocation);

	// static
	// void realignObjectHorz(TreeSVG & obj, const Box<svg::coord_t> & anchorBoxHorz, svg::coord_t & coord);

	static
	void realignObjectHorz(TreeSVG & obj, const Box<svg::coord_t> & anchorBoxHorz);

	// static
	 //void realignObjectVert(TreeSVG & obj, const Box<svg::coord_t> & anchorBoxVert, svg::coord_t & coord);

	static
	void realignObjectVert(TreeSVG & obj, const Box<svg::coord_t> & anchorBoxVert);


	/// Recursively move elements with (x, y).
	static
	void translateAll(TreeSVG & group, const Point2D<svg::coord_t> &offset);

};


class TranslatorSVG : public drain::TreeVisitor<TreeSVG> {

public:

	const Point2D<svg::coord_t> offset;

	template <class T>
	inline
	TranslatorSVG(const Point2D<T> & offset) : offset(offset){};

	template <typename T>
	inline
	TranslatorSVG(T dx, T dy) : offset(dx, dy){};

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override;

	// int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) override;
};


class BBoxRetrieverSVG : public drain::TreeVisitor<TreeSVG> {

public:

	BBoxSVG box;
	// Box<svg::coord_t> box;

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override;

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) override;
};


class RelativePathSetterSVG : public drain::TreeVisitor<TreeSVG> {

public:

	const std::string dir;

	inline
	RelativePathSetterSVG(const drain::FilePath & filepath) :
		dir(filepath.dir.empty() ? "" : filepath.dir.str()+'/') {
	}

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override;


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

