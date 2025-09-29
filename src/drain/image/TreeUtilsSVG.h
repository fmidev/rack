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

#include <drain/image/AlignAnchorSVG.h>
#include <drain/util/Frame.h>
#include <drain/util/EnumFlags.h>
#include <drain/util/Rectangle.h>
#include "LayoutSVG.h"
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

template <AlignBase::Axis AX>
struct CoordSpan {

	// Starting coordinate (x or y).
	svg::coord_t pos = BBoxSVG::undefined;

	// Width or height
	svg::coord_t span = BBoxSVG::undefined;

	// ? void getTranslatedCoordSpan(const BBoxSVG & bbox);
	void copyFrom(const NodeSVG & node);

	void copyFrom(const BBoxSVG & bbox);

	/*
	inline
	bool isUndefined(){
		return std::isnan(pos) || std::isnan(span);
	}
	*/

	inline
	bool isDefined(){
		return ! (std::isnan(pos) || std::isnan(span));
	}
};


struct TreeUtilsSVG {

public:


	/// Compute the bounding box recursively in objects of type IMAGE, RECT, POLYGON and G (group).
	/**
	 *  Traverses the structure recursively, updating bounding box at each level.
	 *
	 *  Future versions may also handle CIRCLE and TEXT (location)
	 */
	static
	void detectBoxNEW(TreeSVG & group, bool debug = false);


	/// Set stack layout as a default in a subtree.
	/**
	 *  Stack layout is applied in groups (G) of class STACK_LAYOUT.
	 *  Stacking means the objects are laid subsequently next to each other,
	 *  alternating horizontal and vertical orientation in each level of STACK_LAYOUTs.
	 *
	 *  Currently, the same direction parameter is used for both horizontal and vertical layout.
	 *  In future, these could be set separately.
	 *
	 *   To actually align the objects, call also superAlignNew().	 *
	 */
	static
	void addStackLayout(TreeSVG & object, AlignBase::Axis orientation = AlignBase::Axis::HORZ, LayoutSVG::Direction direction = LayoutSVG::Direction::INCR);

	/// Sets alignment applying stack layout in a single node.
	/**
	 *   This function is invoked by TreeUtilsSVG::addStackLayout() .
	 *
	 *   To finally align (translate) the objects, call also TreeUtilsSVG::superAlignNew().	 *
	 */
	static
	void setStackLayout(NodeSVG & node, AlignBase::Axis orientation, LayoutSVG::Direction direction);

	static
	void superAlignNEW(TreeSVG & node);


	// TODO: templated
	template <AlignBase::Axis AX>
	static
	void realignObject(NodeSVG & node, const CoordSpan<AX> & span);
	//void realignObjectHorzNEW(NodeSVG & node, const CoordSpan<AlignBase::Axis::HORZ> & span);

	// static
	// void realignObjectVertNEW(NodeSVG & node, const CoordSpan<AlignBase::Axis::VERT> & span);

	// void realignObjectVertNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxVert);
	// static
	//void realignObjectVertNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxVert);

	/*
	static
	void realignObjectHorzNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxHorz);

	static
	void realignObjectVertNEW(NodeSVG & node, const Box<svg::coord_t> & anchorBoxVert);
	*/


	// ...................................................





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


	static
	void superAlign(TreeSVG & node, AlignBase::Axis orientation = AlignBase::Axis::HORZ, LayoutSVG::Direction direction = LayoutSVG::Direction::INCR);


	static
	void realignObject(const Box<svg::coord_t> & anchorBoxHorz, const Box<svg::coord_t> & anchorBoxVert, TreeSVG & obj);


	static
	void realignObjectHorz(TreeSVG & obj, const Box<svg::coord_t> & anchorBoxHorz);

	static
	void realignObjectVert(TreeSVG & obj, const Box<svg::coord_t> & anchorBoxVert);


	// UNUSED Recursively move elements with (x, y).
	static
	void translateAll(TreeSVG & group, const Point2D<svg::coord_t> &offset);

};

template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::HORZ> & span);

template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::VERT> & span);


// Deprecating. Does not handle polygons etc
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

	// Leading path, maybe partial, to be pruned
	const std::string dir;

	// String starting the modified path, for example "file://"
	const std::string prefix;

	inline
	RelativePathSetterSVG(const drain::FilePath & filepath, const std::string & prefix = "") :
		dir(filepath.dir.empty() ? "" : filepath.dir.str()+'/'),
		prefix(prefix) {
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

