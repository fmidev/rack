/*

MIT License

Copyright (c) 2017 FMI Open Development / Markus Peura, first.last@fmi.fi

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

#ifndef RACK_GRAPHICS_SVG
#define RACK_GRAPHICS_SVG

//#include <string>
//#include <drain/prog/CommandBank.h>
#include <drain/prog/CommandInstaller.h>

//#include <drain/util/FileInfo.h>
//#include "hi5/Hi5.h"
//#include "resources.h"

// Notice: role of graphics.cpp and fileio-svg.cpp is currently equivalent

#include "resources.h"

// Notice: role of graphics.cpp and fileio-svg.cpp is currently equivalent

namespace rack {



/// SVG support for creating aligned image panels
class RackSVG {


public:

	//typedef drain::StyleSelectorXML<NodeSVG> Select;

	// Identifier for the anchor background
	// static const std::string BACKGROUND_RECT; //  = "mainRect";

	enum ElemClass {
		NONE = 0,
		MAIN_TITLE  = 1,  // Main title in SVG image
		GROUP_TITLE = 2, // Group title
		IMAGE_TITLE = 4,  // Image title: small text (time, location) in corners of radar images
		// Topical
		TIME = 8,       // Date and time attributes
		LOCATION = 16,   // Place (coordinates, municipality)
		GENERAL = 32,    // Default type
		ALL = (63),
		// MAIN,
		IMAGE_PANEL,
		IMAGE_BORDER,    // RECT surrounding the image
		BACKGROUND_RECT, // invisible RECT used for aligning
		SIDE_PANEL,
		BORDER, // Overall image border (RECT, invisible by default)
		// SHARED_METADATA, // Something that should not be repeated in panels.
		// --- unused ? ---
		// TITLE,      // Default title
	};


	/// Some SVG style classes. Identifiers for IMAGE and RECT elements over which TEXT elements will be aligned
	/**
	 *  Initialize styles, if undone.
	 */
	static
	drain::image::TreeSVG & getStyle(RackContext & ctx);

	/// returns timeformat
	static
	const std::string & guessFormat(const std::string & key);

	/// Top-level GROUP used by Rack. All the graphic elements will be created inside this element.
	/**
	 *
	 *
	 */
	static
	drain::image::TreeSVG & getMainGroup(RackContext & ctx);

	static
	drain::image::TreeSVG & getCurrentAlignedGroup(RackContext & ctx);

	static
	drain::image::TreeSVG & getImagePanelGroup(RackContext & ctx, const drain::FilePath & filepath);

	static
	drain::image::TreeSVG & getImagePanelGroup(RackContext & ctx);


	/** Intermediate group "hiding" translation that moves upper left corner of the object to the origin.
	 *
	 */
	static
	drain::image::TreeSVG & getAdapterGroup(drain::image::TreeSVG & group);


	static
	//void applyAlignment(RackContext & ctx, drain::image::TreeSVG & group);
	void consumeAlignRequest(RackContext & ctx, drain::image::NodeSVG & node);

	static
	bool applyInclusion(RackContext & cxt, const drain::FilePath & filepath); // , IncludePolicy fileFormat);

	static
	// drain::image::TreeSVG &
	void addImage(RackContext & ctx, const drain::image::Image & src, const drain::FilePath & filepath);

	/// Currently, uses file link (does not embed)
	static
	// drain::image::TreeSVG &
	void addImage(RackContext & ctx, const drain::image::TreeSVG & svg, const drain::FilePath & filepath);

	/// Add external image from a file path.
	/**
	 *   \param styleClass - optional style class, possibly FLOAT
	 */
	static
	// drain::image::TreeSVG &
	void addImage(RackContext & ctx, const drain::Frame2D<drain::image::svg::coord_t> & frame, const drain::FilePath & filepath, const std::string & styleClass = "");


	/// Create a title from background rectangle and TEXT elements (MAINTITLE, LOCATION, TIME, GENERAL)
	/**
	 *  \return TEXT element located at child(GENERAL)
	 */
	static
	drain::image::TreeSVG & addTitleBox(const ConfSVG & conf, drain::image::TreeSVG & object, RackSVG::ElemClass elemClass);

	// TODO: title area "filling order", by group class.
	/// Add title elements in given group, to "reserve slots" for actual text content to be added later.

	static
	drain::image::TreeSVG & appendTitleElements(const ConfSVG &conf, drain::image::TreeSVG &group, const std::string &anchor, RackSVG::ElemClass elemClass);

	static inline
	drain::image::TreeSVG & appendTitleElements(const ConfSVG &conf, drain::image::TreeSVG &group, const char * anchor, RackSVG::ElemClass elemClass){
		return appendTitleElements(conf, group, std::string(anchor), elemClass);
	}

	template <class T>
	static inline
	drain::image::TreeSVG & appendTitleElements(const ConfSVG &conf, drain::image::TreeSVG &group, const T & anchorId, RackSVG::ElemClass elemClass){
		//const drain::StringWrapper<T> anchor(anchorId);
		const drain::EnumWrapper<T> anchor(anchorId);
		// const std::string & s = anchor;
		return appendTitleElements(conf, group, (const std::string &)anchor, elemClass);
	}


	/// Add rectangle
	static
	drain::image::TreeSVG & addRectangleGroup(RackContext & ctx, const drain::Frame2D<double> & frame = {200,200});


	/// Add
	/**
	 *  \param imagePanel
	 */
	static
	drain::image::TreeSVG & addImageBorder(drain::image::TreeSVG & imagePanel); // , const drain::Frame2D<double> & frame = {200,200});


protected:


};

} // rack



namespace drain {

DRAIN_ENUM_DICT(rack::RackSVG::ElemClass);
DRAIN_ENUM_OSTREAM(rack::RackSVG::ElemClass);

template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::RackSVG::ElemClass & key) const ;

/// Automatic conversion of elem classes to strings.
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::RackSVG::ElemClass & value) const;


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::RackSVG::ElemClass & key);

//
/*
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::GraphicsContext::ElemClass &x);
*/

/// Automatic conversion of elem classes to strings.
/**
 *
template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const image::svg::tag_t & type);
 */


//template <>
//const drain::EnumDict<RackSVG::TitleClass>::dict_t  drain::EnumDict<RackSVG::TitleClass>::dict;

/*
template <>
const std::string std::static_cast<std::string>(const RackSVG::ElemClass & e){
	return drain::EnumDict<RackSVG::ElemClass>::dict.getKey(e);
}
*/

}



namespace rack {


/*

Komentoja!

 */

// struct GraphicsSection;
struct GraphicsSection : public drain::CommandSection {

	inline	GraphicsSection(): CommandSection("graphics"){
	};

};

class GraphicsModule : public drain::CommandModule<'g',GraphicsSection> {
public:
	GraphicsModule();
};


/// SVG panel
/*
class CmdOutputPanel : public drain::BasicCommand {

public:

	inline
	CmdOutputPanel() : drain::BasicCommand(__FUNCTION__, "Save SVG panel of latest images. See also: --image, --outputRawImages.") {
		getParameters().link("filename", filename="");
		getParameters().link("layout", layout, "basic");
	};

	inline
	CmdOutputPanel(const CmdOutputPanel & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}


	void exec() const;


	std::string layout;
	std::string filename;

	void appendImage(TreeSVG & group, const std::string & prefix, drain::VariableMap & vmap,
			const drain::Point2D<double> & location, const drain::image::Image & image, drain::BBox & bbox) const;
};
*/


} // rack::


#endif

