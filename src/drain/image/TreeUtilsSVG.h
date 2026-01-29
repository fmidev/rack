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


/// SVG classes marking requests of relative horizontal and vertical alignment.
/**
 *
 */



// TODO: separate TreeLayoutUtilsSVG

struct TreeUtilsSVG {

public:

	enum Roles {
		MAIN,
	};

	/// Get header object, which is often but not always unique. If nonexistent, is created and prepended in the document.
	/**
	 *  \param root - top-level element of the SVG document (tag type "svg").
	 */
	static
	TreeSVG & getHeaderObject(TreeSVG & root, svg::tag_t, const std::string & key = "");

	/// Create a new entry, unless already defined.
	static
	TreeSVG & ensureStyle(TreeSVG & root, const SelectXML<svg::tag_t> & selector, const std::initializer_list<std::pair<const char *,const Variable> > & styleDef);


};

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

	/// Ensures a clipping path o f type RECT of given size.
	/**
	 *   Only a single entry is created for each (width,height) combination
	 */
	static
	TreeSVG & getClippingRect(TreeSVG & root, size_t width, size_t height);

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

