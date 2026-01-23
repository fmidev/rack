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

#include <drain/util/EnumUtils.h>
#include <drain/util/Frame.h>
#include <drain/util/Rectangle.h>

#include "AlignAnchorSVG.h"
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

	inline
	CoordSpan(svg::coord_t pos = 0, svg::coord_t span = 0) : pos(pos), span(span) {
	}

	inline
	CoordSpan(const CoordSpan & cspan) : pos(cspan.pos), span(cspan.span) {
	}

	// Starting coordinate (x or y).
	svg::coord_t pos = 0; //BBoxSVG::undefined;

	// Width or height
	svg::coord_t span = 0; // BBoxSVG::undefined;

	// ? void getTranslatedCoordSpan(const BBoxSVG & bbox);
	void copyFrom(const NodeSVG & node);

	void copyFrom(const BBoxSVG & bbox);

	inline
	bool isDefined(){
		return ! (std::isnan(pos) || std::isnan(span));
	}
};


struct TreeUtilsSVG {

public:

	enum Roles {
		MAIN,
	};

	/// Create a new entry, unless already defined.
	/**
	 *
	 */
	static
	TreeSVG & ensureStyle(TreeSVG & root, const SelectXML<svg::tag_t> & selector, const std::initializer_list<std::pair<const char *,const Variable> > & styleDef);

	/// Compute the bounding box recursively in objects of type IMAGE, RECT, POLYGON and G (group).
	/**
	 *  Traverses the structure recursively, updating bounding box at each level.
	 *
	 *  Future versions may also handle CIRCLE and TEXT (location)
	 */
	static
	void detectBox(TreeSVG & group, bool debug = false);

	static inline
	void getAdjustedBBox(const NodeSVG & node, BBoxSVG & bbox) { //, bool debug = false){
		//detectBoxNEW(group, debug);
		bbox = node.getBoundingBox();
		bbox.x += node.transform.translate.x;
		bbox.y += node.transform.translate.y;
	}

	template <AlignBase::Axis AX>
	static
	void adjustLocation(TreeSVG & group, NodeSVG & node, CoordSpan<AX> anchorSpan);

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
	void addStackLayout(TreeSVG & object, AlignBase::Axis orientation = AlignBase::Axis::HORZ, LayoutSVG::Direction direction = LayoutSVG::Direction::INCR, unsigned short depth=0);

	/// Sets alignment applying stack layout in a single node.
	/**
	 *   This function is invoked by TreeUtilsSVG::addStackLayout() .
	 *
	 *   To finally align (translate) the objects, call also TreeUtilsSVG::superAlignNew().	 *
	 */
	static
	void setStackLayout(NodeSVG & node, AlignBase::Axis orientation, LayoutSVG::Direction direction);

	static
	void superAlign(TreeSVG & node);


	// Why templated, and not two separate?
	template <AlignBase::Axis AX>
	static
	void realignObject(NodeSVG & node, const CoordSpan<AX> & span);




};

template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::HORZ> & span);

template <>
void TreeUtilsSVG::realignObject(NodeSVG & node, const CoordSpan<AlignBase::Axis::VERT> & span);

DRAIN_ENUM_DICT(TreeUtilsSVG::Roles);
DRAIN_ENUM_OSTREAM(TreeUtilsSVG::Roles);

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
 */
class MetaDataCollectorSVG : public drain::TreeVisitor<TreeSVG> {

public:

	static
	const std::string LOCAL;

	static
	const std::string SHARED;

	/*
	enum MetaDataType {
		LOCAL,
		SHARED
	};
	*/

	inline
	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override {
		// always continue
		return 0;
	}

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) override;

	// Also
	//GraphicsContext::TitleFlagger titles;

protected:

	typedef std::map<std::string, unsigned short> variableStat_t;

};


/**
 *
 */
class ClipperSVG : public drain::TreeVisitor<TreeSVG> {

public:

	static
	const std::string CLIP;

	/*
	enum MyDataType {
		FIRST,
		SECOND
	};
	*/

	inline
	ClipperSVG(TreeSVG & root) : root(root) {
	}

	ClipperSVG(const ClipperSVG & clipper) : root(clipper.root) { // ???
	}

	inline
	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override {
		// always continue
		return 0;
	}

	int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) override;

	TreeSVG & root;

};


// DRAIN_ENUM_DICT(MetaDataCollectorSVG::MetaDataType);

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

}  // drain::




#endif // TREESVG_H_

