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


#ifndef RACK_GRAPHICS_PANEL
#define RACK_GRAPHICS_PANEL


#include <drain/image/FilePng.h>
#include <drain/image/TreeSVG.h>
#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>
#include <drain/prog/CommandBank.h>
#include <drain/util/Output.h>
#include <main/graphics-panel.h>
#include "data/SourceODIM.h" // for NOD

#include "resources.h"
//#include "fileio-svg.h"
#include "graphics-radar.h"
#include <drain/image/TreeSVG.h>
//#include "resources-image.h"

namespace rack {

using namespace drain::image;
// Move to graphics.h ?

/// Formats titles from metadata. Invoked by drain::TreeUtils::traverse()
/**
 *   In tree traversal, maintains information on metadata.
 *
 *   Invoked by, hence compatible with drain::TreeUtils::traverse()
 */
class TitleCreatorSVG : public drain::TreeVisitor<TreeSVG> {

public:


	inline
	TitleCreatorSVG(const ConfSVG & svgConf) : svgConf(svgConf) {
		//titles.set(0xff);
		/*
		if (!svgConf.mainTitle.empty()){
			titles.set(PanelConfSVG::ElemClass::MAIN_TITLE);
		}

		if (!svgConf.groupTitleSyntax.empty()){
			titles.set(PanelConfSVG::ElemClass::GROUP_TITLE);
		}

		titles.set(PanelConfSVG::ElemClass::IMAGE_TITLE);
		*/
		/*
		if (!svgConf.groupNameSyntax.empty()){
			titles.set(RackSVG::ElemClass::GROUP_TITLE);
		}
		*/
	};

	/**
	 */
	int visitPostfix(TreeSVG &root, const TreeSVG::path_t &path) override;

	/**
	 *   Useful function – as attributes of two groups are considered: panel-specific and shared.
	 */
	void formatTitle(TreeSVG & group, const NodeSVG::map_t & attributes) const;

	const ConfSVG & svgConf;


};


// ....

class CmdLayout : public drain::BasicCommand {

public:

	CmdLayout() : drain::BasicCommand(__FUNCTION__, "Set main panel alignment"){

		getParameters().link("orientation", orientation="HORZ",
				drain::sprinter(drain::Enum<orientation_enum>::dict.getKeys(), {"|"}).str());
		getParameters().link("direction", direction="INCR",
				drain::sprinter(drain::Enum<direction_enum>::dict.getKeys(), {"|"}).str());

	}

	CmdLayout(const CmdLayout & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	};


	virtual
	void exec() const override;

protected:

	typedef drain::image::AlignBase::Axis orientation_enum;
	typedef drain::image::LayoutSVG::Direction direction_enum;

	//volatile
	mutable
	std::string orientation;

	// volatile
	mutable
	std::string direction;


};

/**
 *  Examples
 *  \code
 *  rack --gAlign TOP,RIGHT
 *  rack --gAlign TOP:OUTSIDE,INSIDE:RIGHT
 *  \endcode
 *
 *  Warns if both are outside, ie. diagonally aligned to original image (or other graphical object).
 */
//class CmdAlign : public drain::SimpleCommand<std::string> {
class CmdAlign : public drain::BasicCommand {

public:

	/*
	CmdAlign() : drain::SimpleCommand<std::string>(__FUNCTION__, "Alignment of the next element", "topology", "",
			drain::sprinter(drain::Enum<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
			drain::sprinter(drain::Enum<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str() + ',' +
			drain::sprinter(drain::Enum<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
			drain::sprinter(drain::Enum<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str()
	){
		//getParameters().link("", x, drain::StringBuilder<':'>());
	}
	 */


	CmdAlign() : drain::BasicCommand(__FUNCTION__, "Alignment of the next element"){

		getParameters().link("position", position,
				drain::sprinter(drain::Enum<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
				drain::sprinter(drain::Enum<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str() + ',' +
				drain::sprinter(drain::Enum<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
				drain::sprinter(drain::Enum<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str()
		).setSeparator(':');

		getParameters().link("anchor", anchor.str(),
				std::string("<name>|<empty>") + drain::sprinter(drain::Enum<drain::image::AnchorElem::Anchor>::dict.getKeys(), {"|"}).str()
		);
		getParameters().link("anchorHorz", anchorHorz.str(),
				"..."
		);
		getParameters().link("anchorVert", anchorVert.str(),
				"..."
		);
	}

	CmdAlign(const CmdAlign & cmd) : drain::BasicCommand(cmd){
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	};


	virtual
	void exec() const override;

protected:

	std::string position;

	mutable
	drain::image::AnchorElem anchor;

	mutable
	drain::image::AnchorElem anchorHorz;

	mutable
	drain::image::AnchorElem anchorVert;

};

/*
class CmdAnchor : public drain::SimpleCommand<std::string> {

public:

	inline
	CmdAnchor() : drain::SimpleCommand<std::string>(__FUNCTION__, "Alignment of the next element", "topology", "",
			drain::sprinter(drain::Enum<drain::image::AnchorElem::Anchor>::dict.getKeys(), {"|"}).str()
	){
		//getParameters().link("", x, drain::StringBuilder<':'>());
	}

	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

	}
};
 */

/// "Virtual" command base for FontSizes and HeaderSizes
class CmdAdjustSizes : public drain::SimpleCommand<std::string> {

public:

	CmdAdjustSizes(const std::string & name, const std::string & description) : drain::SimpleCommand<std::string>(name, description) {
		// getParameters().link("sizes", fontSizes);
	}

	template <class T, size_t N=2>
	void adjust(drain::UniTuple<T,N> & tuple, float decay = 0.8) const{


		double defaultValue = decay * tuple[0];

		tuple.clear();

		drain::Reference ref(tuple);
		ref.setFill(false);
		ref = value;

		for (double & s: tuple){
			defaultValue = 0.1 * ::round(10.0 * defaultValue);
			// mout.attention("font size ",  s, ", [", defaultValue, "]");
			if (s == 0.0){
				s = defaultValue;
			}
			else {
				if (s > defaultValue){
					// mout.suspicious<LOG_WARNING>("font size increasing (",  s, '>', defaultValue, ") unexpectedly");
				}
				defaultValue = s;
			}
			defaultValue *= decay;
		}
		// mout.accept<LOG_WARNING>("new values: ", ctx.svgPanelConf.fontSizes);


	}


};


// class CmdFontSizes : public drain::BasicCommand {
class CmdFontSizes : public CmdAdjustSizes {

public:

	inline
	CmdFontSizes() : CmdAdjustSizes(__FUNCTION__, "Adjust font sizes in CSS style section.") {
	}

	virtual
	void exec() const override;

};


class CmdTitleHeights : public CmdAdjustSizes {

public:

	inline
	CmdTitleHeights() : CmdAdjustSizes(__FUNCTION__, "Set title box heights and adjust font sizes. See --gFontSizes") {
	}

	virtual
	void exec() const override;

};

class CmdLinkImage : public drain::SimpleCommand<std::string> {

public:

	inline
	CmdLinkImage() : drain::SimpleCommand<std::string>(__FUNCTION__, "Link arbitrary external image (PNG).") {
	}

	void exec() const override;

};



//class CmdGroupTitle : public drain::BasicCommand {
class CmdGroupTitle : public drain::SimpleCommand<std::string> {

public:

	inline
	CmdGroupTitle() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set titles, supporting variables", "syntax") {
		// RackContext & ctx = getContext<RackContext>();
		getParameters().separator = 0;
		// getParameters().link("syntax", ctx.svgPanelConf.groupTitleSyntax, "example: '${what:date|%Y%m} ${NOD}'");
	}

	inline
	CmdGroupTitle(const CmdGroupTitle & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		// getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	void exec() const override;

};

class CmdGroupId : public drain::SimpleCommand<std::string> {

public:

	//CmdGroupTitle() : drain::BasicCommand(__FUNCTION__, "Set titles automatically") {
	CmdGroupId() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set grouping criterion based on variables", "syntax") {
		getParameters().separator = 0;
	}

	CmdGroupId(const CmdGroupId & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		// getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	inline
	void exec() const override {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		// mout.accept<LOG_WARNING>("old value: ", value);
		drain::StringTools::replace(value, '/', '-', ctx.svgPanelConf.groupIdentifier);
		// notice if pruned/modified?
		mout.accept<LOG_WARNING>("new value: ", ctx.svgPanelConf.groupIdentifier);
	}

};


/*
class CmdImageTitle : public drain::BasicCommand {

public:

	CmdImageTitle() : drain::BasicCommand(__FUNCTION__, "Set titles automatically") {
		RackContext & ctx = getContext<RackContext>();
		getParameters().link("title", ctx.svgPanelConf.imageTitle, "'TIME'|'LOCATION'|'GENERAL'");
	}

	CmdImageTitle(const CmdImageTitle & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	void exec() const override {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	}

};
*/

class CmdTitle : public drain::BasicCommand {

public:

	// TODO: use unlinked SimpleCommand,

	inline
	CmdTitle() : drain::BasicCommand(__FUNCTION__, "Set main title") {
		RackContext & ctx = getContext<RackContext>();
		getParameters().separator = 0;
		getParameters().link("title", ctx.svgPanelConf.mainTitle, "<empty>|<string>|'auto'");
	}

	inline
	CmdTitle(const CmdTitle & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

};

/**
 *  Main title:
 *  - "NONE" – do not display
 *  - "<string>" – (implicit) apply main title issued with \c --gTitle
 *  - "AUTO" – apply automatic formatting
 *
 *  Group title:
 *  - "NONE" – use grouping to rows or columns, but do not display group titles
 *  - "AUTO:<syntax-string>" – apply group identifiers (\c --gGroup ) but use automatic formatting
 *  - USER: apply group identifiers issued with \c --gGroup  as group titles
 *
 *  Image title:
 *  - NONE: display no titles
 *  - TIME: display time and date (automatic formatting)
 *  - LOCATION: display  (automatic formatting)
 *  - GENERAL: display metadata other than date, time or location (geographic information?)
 *
 */
/*
class CmdTitles : public drain::BasicCommand {

public:

	CmdTitles() : drain::BasicCommand(__FUNCTION__, "Set titles automatically") {
		RackContext & ctx = getContext<RackContext>();
		getParameters().link("main", ctx.svgPanelConf.mainTitle,  "NONE|AUTO|USER");
		getParameters().link("group", ctx.svgPanelConf.mainTitle, "NONE|AUTO|USER");
		getParameters().link("image", ctx.svgPanelConf.mainTitle, "NONE|TIME|LOCATION|GENERAL");
	}

	CmdTitles(const CmdTitles & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

};
 */


class CmdInclude : public drain::SimpleCommand<std::string> {

public:

	CmdInclude() : drain::SimpleCommand<std::string>(__FUNCTION__, "Select images to include in SVG panel", "include",
			drain::sprinter(drain::Enum<drain::image::FileSVG::IncludePolicy>::dict.getKeys(), '|').str()) {
	}

	void exec() const override;

};


/*
class CmdTitles : public drain::SimpleCommand<std::string> {
public:

	CmdTitles() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set titles automatically", "[MAINTITLE|TIME|LOCATION|GENERAL]") {
	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		mout.unimplemented<LOG_ERR>(__FILE__, __FUNCTION__);
		//ctx.svgPanelConf.svgTitles.set(value);

	}

};
 */


} // rack::


#endif
