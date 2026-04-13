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

#ifndef RACK_SVG_INTERACTIVE
#define RACK_SVG_INTERACTIVE


#include <vector>
#include <drain/prog/Command.h>
#include <drain/util/Base64.h>

#include "hi5/Hi5.h"
#include "resources.h" // ctx
//#include "graphics-radar.h" // ctx
#include "graphics-overlay.h" // ctx



namespace rack {




class CmdRect : public CmdPolarBase { //  drain::BasicCommand,

public:

	CmdRect() : CmdPolarBase(__FUNCTION__, "Draw rectangle or circle", Graphic::GRAPHIC::RECTANGLE) {
		getParameters().separator = ':';
		//getParameters().link("bbox", bbox.tuple(100,200,300,400), "xLL,yLL,xUR,yUR").setFill(false).setSeparator(',');
		getParameters().link("bbox", bbox, "xLL,yLL,xUR,yUR[px|m|deg] or xLL,yLL,r").setSeparator(',');
		//getParameters().link("bboxUnits", bboxUnits, "[PIX|M|DEG]");
		getParameters().link("resolution", resolution.tuple(), "pixel").setFill(true).setSeparator(',');
		getParameters().link("MASK", MASK, "Render outer region with style class '.MASK'");
		// getParameters().link("panel",  panel, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::Enum<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
	}

	inline
	CmdRect(const CmdRect & cmd) : CmdPolarBase(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	void exec() const override ;

	std::string bbox = "";
	drain::Range<int> resolution;

};

class CmdCoords : public CmdPolarBase { //  drain::BasicCommand,

public:

	CmdCoords() : CmdPolarBase(__FUNCTION__, "SVG test product") {
		getParameters().link("resolution", resolution.tuple(), "pixel").setFill(true);
		getParameters().link("MASK", MASK, "Fill outside using CSS class '.MASK'");
		// getParameters().link("panel",  panel, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::Enum<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
	}

	CmdCoords(const CmdCoords & cmd) : CmdPolarBase(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	void exec() const override ;

	drain::Range<int> resolution;


};

class CmdData : public CmdPolarBase { //  drain::BasicCommand,

public:

	CmdData() : CmdPolarBase(__FUNCTION__, "Save and link encoded data for mouse read.") {
		getParameters().link("filename", filename, "PNG data file path");
	}

	CmdData(const CmdData & cmd) : CmdPolarBase(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	void exec() const override;

protected:

	std::string filename;
};



class CmdTestData : public CmdPolarBase {

public:

	CmdTestData() : CmdPolarBase(__FUNCTION__, "Test attaching numeric data as arrays using Base64 encoding") {
	}

	CmdTestData(const CmdTestData & cmd) : CmdPolarBase(cmd) {
	}

	void exec() const override ;

protected:

	template <typename T>
	void test(drain::image::TreeSVG & img, const std::vector<T> & v, drain::Logger & mout) const {

		const std::string & typeName = drain::TypeName<T>::str();
		mout.experimental<LOG_NOTICE>("Testing: vector of ", typeName);

		drain::Base64 bytes;
		bytes.convertFrom(v);
		std::string code;
		drain::Base64::base64_encode(bytes, code);
		mout.experimental<LOG_INFO>("Code: ", code);

		drain::image::TreeSVG & imgData = img[typeName](svg::METADATA);
		imgData->set("data-base64", code);
		imgData->set("data-basetype", typeName);

	}

};


// Not interactive...

class CmdDot : public CmdPolarBase { //  drain::BasicCommand,

	drain::Point2D<double> coords;
	// drain::Range<double>   radius;
	std::string id;
	std::string style;

public:

	inline
	CmdDot() : CmdPolarBase(__FUNCTION__, "Draw a marker circle", LABEL) {
		getParameters().link("lonlat", coords.tuple(25.0, 60.0), "Coordinate (lon,lat) in degrees(decimal) or metres.");
		getParameters().link("radius", radiusMetres.range.tuple(0,25000), "metres or relative").setFill(true);
		getParameters().link("id", id, "XML element id");
		getParameters().link("style", style, "XML element CSS style");
	}

	inline
	CmdDot(const CmdDot & cmd) : CmdPolarBase(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	void exec() const override;

};

} // rack::


#endif

