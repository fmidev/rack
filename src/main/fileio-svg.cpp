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

#include <drain/Version.h>


#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeXML.h>
#include <drain/util/TreeHTML.h>

#include <drain/image/FilePng.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/TreeUtilsSVG.h>
#include <drain/image/TreeElemUtilsSVG.h>

//#include "data/SourceODIM.h" // for NOD
//
#include "radar/PolarSector.h"

#include "graphics.h"
#include "graphics-radar.h"

#include "fileio-svg.h"


/*
namespace drain {


template <>
template <class D>
void Convert2<FlexibleVariable>::convert(const FlexibleVariable &src, D & dst){
	dst = (const D &)src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}


template <>
void Convert2<FlexibleVariable>::convert(const char *src, FlexibleVariable & dst){
	dst = src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}



template <>
template <class S>
void Convert2<FlexibleVariable>::convert(const S &src, FlexibleVariable & dst){
	dst = src;
	std::cout << "CONV..." << src << " -> " << dst << '\n';
}


}
 */

//namespace drain {


//}



namespace rack {

// typedef drain::image::TreeUtilsSVG tsvg;
// typedef drain::image::AlignSVG alignSvg;


// StyleSelectorXML :
//  Wrapper. Unused, future option
/*
template <typename E>
class ClassLabelXML : public std::string {
public:

	ClassLabelXML(const E & e): std::string(drain::EnumDict<E>::dict.getKey(e)) {
	}

	template <typename ...T>
	ClassLabelXML(const T... args) : std::string(args...){
	}

};
 */



struct GraphicsSection : public drain::CommandSection {

	inline	GraphicsSection(): CommandSection("graphics"){
	};

};




//class CmdGridAlign : public drain::SimpleCommand<std::string> {
class CmdLayout : public drain::BasicCommand {

public:

	CmdLayout() : drain::BasicCommand(__FUNCTION__, "Set main panel alignment"){

		getParameters().link("orientation", orientation="HORZ",
				drain::sprinter(drain::EnumDict<orientation_enum>::dict.getKeys(), {"|"}).str());
		getParameters().link("direction", direction="INCR",
				drain::sprinter(drain::EnumDict<direction_enum>::dict.getKeys(), {"|"}).str());

	}

	CmdLayout(const CmdLayout & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	};

	/*
	CmdGridAlign() : drain::SimpleCommand<std::string>(__FUNCTION__, "desc", "",
			drain::sprinter(orientation::dict.getKeys(), {"|"}).str() +
			drain::sprinter(direction::dict.getKeys(), {"|"}).str()
	){
	}
	 */

	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::EnumDict<orientation_enum>::setValue(orientation, ctx.mainOrientation);
		drain::EnumDict<direction_enum>::setValue(direction,     ctx.mainDirection);

		// reset
		orientation = drain::EnumDict<orientation_enum>::dict.getKey(orientation_enum::HORZ);
		direction   = drain::EnumDict<direction_enum>::dict.getKey(direction_enum::INCR);

	}

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
			drain::sprinter(drain::EnumDict<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
			drain::sprinter(drain::EnumDict<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str() + ',' +
			drain::sprinter(drain::EnumDict<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
			drain::sprinter(drain::EnumDict<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str()
	){
		//getParameters().link("", x, drain::StringBuilder<':'>());
	}
	 */


	CmdAlign() : drain::BasicCommand(__FUNCTION__, "Alignment of the next element"){

		getParameters().link("position", position,
				drain::sprinter(drain::EnumDict<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
				drain::sprinter(drain::EnumDict<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str() + ',' +
				drain::sprinter(drain::EnumDict<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
				drain::sprinter(drain::EnumDict<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str()
		).setSeparator(':');

		getParameters().link("anchor", anchor.str(),
				std::string("<name>|<empty>") + drain::sprinter(drain::EnumDict<drain::image::AnchorElem::Anchor>::dict.getKeys(), {"|"}).str()
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
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		// Anchor
		if (anchor.isSet()){
			mout.attention("setting anchor for next graphic object: ", anchor);
			mout.attention("isPrevious:\t",  anchor.isPrevious());
			mout.attention("isExtensive:\t", anchor.isCollective());
			mout.attention("isSpecific:\t",  anchor.isSpecific());
			mout.attention("isNone:\t",      anchor.isNone());
			anchorHorz = anchor;
			anchorVert = anchor;
			anchor.clear();
		}
		ctx.anchorHorz = anchorHorz;
		ctx.anchorVert = anchorVert;
		mout.attention("new anchors: ", ctx.anchorHorz, 'x', ctx.anchorVert);

		// Position
		if (!position.empty()){

			std::list<std::string> args;
			drain::StringTools::split(position, args, ':');

			for (const std::string & arg: args){

				std::list<std::string> keys;
				drain::StringTools::split(arg, keys, '.');

				CompleteAlignment<> align(AlignSVG::Topol::UNDEFINED_TOPOL, AlignBase::Pos::UNDEFINED_POS);

				for (const std::string & key: keys){
					align.set(key);
				}

				switch (align.axis) {
				case AlignBase::Axis::HORZ:
					ctx.alignHorz.set(align.topol, align.pos); // AlignBase::Axis::HORZ,
					mout.accept<LOG_NOTICE>(align.topol, AlignBase::Axis::HORZ, align.pos, " -> ", ctx.alignHorz.topol, '/', ctx.alignHorz.axis, '/', ctx.alignHorz);
					// ctx.alignHorz.set(align);
					break;
				case AlignBase::Axis::VERT:
					ctx.alignVert.set(align.topol, align.pos); // AlignBase::Axis::VERT,
					mout.accept<LOG_NOTICE>(align.topol, AlignBase::Axis::VERT, align.pos, " -> ", ctx.alignVert.topol, '/', ctx.alignVert.axis, '/', ctx.alignVert);
					// ctx.alignVert(align);
					break;
				case AlignBase::Axis::UNDEFINED_AXIS:
				default:
					mout.advice("use: ", drain::sprinter(drain::EnumDict<AlignBase::Axis>::dict.getKeys(), {"|"}).str());
					mout.advice("use: ", drain::sprinter(drain::EnumDict<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str());
					mout.advice("use: ", drain::sprinter(drain::EnumDict<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str());
					mout.advice("use: ", drain::sprinter(drain::EnumDict<Alignment<> >::dict.getKeys(), {"|"}).str());
					mout.error("could not determine axis from argument '", arg, "'");
					break;
				}


			}

		}

		// mout.accept<LOG_NOTICE>(ctx.alignHorz.topol, '/',(AlignBase &)ctx.alignHorz);
		// mout.accept<LOG_NOTICE>(ctx.alignVert.topol, '/',(AlignBase &)ctx.alignVert);

	}

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
			drain::sprinter(drain::EnumDict<drain::image::AnchorElem::Anchor>::dict.getKeys(), {"|"}).str()
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

	// CmdFontSizes() : drain::BasicCommand(__FUNCTION__, "Add or modify CSS entry") {
	CmdFontSizes() : CmdAdjustSizes(__FUNCTION__, "Adjust font sizes in CSS style section.") {
		// getParameters().link("sizes", fontSizes);
	}

	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		adjust(ctx.svgPanelConf.boxHeights, 0.8);
		mout.accept<LOG_WARNING>("new values: ", ctx.svgPanelConf.fontSizes);

	}

	/*
	static inline
	void updateFontStyles(RackContext & ctx){
		drain::image::TreeSVG & style = RackSVG::getStyle(ctx);
		style[drain::SelectorXMLcls(PanelConfSVG::MAIN_TITLE)]->set("font-size", ctx.svgPanelConf.fontSizes[0]);
		style[drain::SelectorXMLcls(PanelConfSVG::GROUP_TITLE)]->set("font-size", ctx.svgPanelConf.fontSizes[1]);
		style[drain::SelectorXMLcls(PanelConfSVG::IMAGE_TITLE)]->set("font-size", ctx.svgPanelConf.fontSizes[2]);
	}
	 */

};


class CmdTitleHeights : public CmdAdjustSizes {

public:

	CmdTitleHeights() : CmdAdjustSizes(__FUNCTION__, "Set title box heights and adjust font sizes. See --gFontSizes") {
	}

	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		adjust(ctx.svgPanelConf.boxHeights, 0.8);
		mout.accept<LOG_WARNING>("new BOX  values: ", ctx.svgPanelConf.boxHeights);

		for (size_t i= 0; i<ctx.svgPanelConf.boxHeights.size(); ++i){
			ctx.svgPanelConf.fontSizes[i] = 0.65 * ctx.svgPanelConf.boxHeights[i];
		}
		mout.accept<LOG_WARNING>("new FONT values: ", ctx.svgPanelConf.fontSizes);

		// CmdFontSizes::updateFontStyles(ctx);
	}

};

class CmdLinkImage : public drain::SimpleCommand<std::string> {

public:

	CmdLinkImage() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product") {
		//getParameters().link("level", level = 5);
	}

	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		const Composite & composite = ctx.getComposite(RackContext::CURRENT);

		drain::Frame2D<double> frame(composite.getFrameWidth(), composite.getFrameHeight());

		if (frame.empty()){
			mout.advice("Ensure --cSize <width>,<height> is called prior to this command (", getName(), ")");
			mout.hint("Use  '--format FMI-MAP --outputFile -' to obtain a background map.");
			mout.warn("Including (linking) image file without (width x height) information. Using 320x200");
			frame.set(320,200);
		}
		ctx.getUpdatedStatusMap(); // for variables in file path
		drain::FilePath filePath(ctx.getFormattedStatus(ctx.inputPrefix + this->value));
		mout.note("linking: ", filePath);
		RackSVG::addImage(ctx, frame, filePath); // , drain::StringBuilder<>(LayoutSVG::INDEPENDENT));
		// RackSVG::addImage(ctx, frame, filePath, drain::StringBuilder<>(LayoutSVG::INDEPENDENT));
		// drain::image::TreeSVG & imagePanel = getImagePanelGroup(ctx, filepath);
		// EnumDict<LayoutSVG::GroupType>::dict

	}

};



//class CmdGroupTitle : public drain::BasicCommand {
class CmdGroupTitle : public drain::SimpleCommand<std::string> {

public:

	//CmdGroupTitle() : drain::BasicCommand(__FUNCTION__, "Set titles automatically") {
	CmdGroupTitle() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set titles, supporting variables", "syntax") {
		// RackContext & ctx = getContext<RackContext>();
		getParameters().separator = 0;
		// getParameters().link("syntax", ctx.svgPanelConf.groupTitleSyntax, "example: '${what:date|%Y%m} ${NOD}'");
	}

	CmdGroupTitle(const CmdGroupTitle & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		// getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	void exec() const override {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		// drain::StringTools::replace(ctx.svgPanelConf.groupTitleSyntax, '/', '-', ctx.svgPanelConf.groupTitleSyntax);
		ctx.svgPanelConf.groupTitle = value;

		if (value.length()>=4){
			const std::string prefix(value, 0,4);
			if (prefix=="NONE"){
				ctx.svgPanelConf.groupTitle = "";
				if (value.length()==5){
					mout.suspicious("argument ", prefix, " ");
				}
				if (value.length()>4){
					// perhaps starts with some separator like ':'
					drain::StringTools::replace(value.substr(5), '/', '-', ctx.svgPanelConf.groupIdentifier);
					mout.deprecating("combination of '", prefix, "' and ID");
					mout.advice("use instead: --gGroupId '", ctx.svgPanelConf.groupIdentifier, "' --gGroupTitle '", prefix,"'");
				}
			}
			else if (prefix=="AUTO"){
				ctx.svgPanelConf.groupTitle = prefix;
				if (value.length()>4){
					mout.deprecating("combination of ", prefix, "+<title>, the latter forwarded to --gGroupId");
					// perhaps starts with some separator like ':'
					drain::StringTools::replace(value.substr(5), '/', '-', ctx.svgPanelConf.groupIdentifier);
					mout.advice("use: --gGroupId '", ctx.svgPanelConf.groupIdentifier, "' --gGroupTitle '", prefix,"'");
				}
			}
		}

		if (ctx.svgPanelConf.groupIdentifier.empty()){
			mout.revised("setting also GroupId");
			// mout.accept<LOG_WARNING>("old value: ", value);
			drain::StringTools::replace(value, '/', '-', ctx.svgPanelConf.groupIdentifier);
			mout.accept<LOG_DEBUG>("new groupID/rule: ", ctx.svgPanelConf.groupIdentifier);
		}
		// mout.attention("groupId: ", ctx.svgPanelConf.groupIdentifier);
	}

};

class CmdGroupId : public drain::SimpleCommand<std::string> {

public:

	//CmdGroupTitle() : drain::BasicCommand(__FUNCTION__, "Set titles automatically") {
	CmdGroupId() : drain::SimpleCommand<std::string>(__FUNCTION__, "Set grouping criterion based on variables", "syntax") {
		getParameters().separator = 0;
	}

	CmdGroupId(const CmdGroupTitle & cmd) : drain::SimpleCommand<std::string>(cmd) { // drain::BasicCommand(cmd) {
		// getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	void exec() const override {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		// mout.accept<LOG_WARNING>("old value: ", value);
		drain::StringTools::replace(value, '/', '-', ctx.svgPanelConf.groupIdentifier);
		// notice if pruned/modified?
		mout.accept<LOG_WARNING>("new value: ", ctx.svgPanelConf.groupIdentifier);
	}

};



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

class CmdTitle : public drain::BasicCommand {

public:

	// TODO: use unlinked SimpleCommand,

	CmdTitle() : drain::BasicCommand(__FUNCTION__, "Set main title") {
		RackContext & ctx = getContext<RackContext>();
		getParameters().separator = 0;
		getParameters().link("title", ctx.svgPanelConf.mainTitle, "<empty>|<string>|'auto'");
	}

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
			drain::sprinter(drain::EnumDict<SvgInclude>::dict.getKeys(), '|').str()) {
	}

	void exec() const override {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
		//mout.unimplemented<LOG_ERR>(__FILE__, __FUNCTION__);
		if (value.empty()){
			mout.warn("empty argument - did you mean 'NONE' ?");
		}
		else if (value == "NONE"){
			ctx.svgPanelConf.svgIncludes.reset();
		}
		else if (value == "OFF"){
			ctx.svgPanelConf.svgIncludes.unset(SvgInclude::ON);
		}
		else {
			ctx.svgPanelConf.svgIncludes.set(SvgInclude::ON); // so PNG,SVG,.. need no explicit "ON"
			ctx.svgPanelConf.svgIncludes.set(value);
		}
	}
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


class CmdPanelFoo : public drain::SimpleCommand<std::string> {

public:

	CmdPanelFoo() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "layout") {
		//getParameters().link("level", level = 5);
	}

	void exec() const override {

		// ClassLabelXML<drain::image::AlignSVG> label1(drain::image::AlignSVG::PANEL);
		// ClassLabelXML<drain::image::AlignSVG> label2("PANEL");

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::Frame2D<double> frame = {150,480};

		// drain::image::TreeSVG & group = ctx.getCurrentAlignedGroup()[value](svg::GROUP); // RackSVG::getCurrentAlignedGroup(ctx)[value](svg::GROUP);
		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[value](svg::GROUP);
		group->setId(value);
		group->addClass(LayoutSVG::INDEPENDENT);

		// rectGroup->addClass(drain::image::LayoutSVG::ALIG NED);
		const std::string ANCHOR_ELEM("myRect"); // not PanelConfSVG::MAIN
		//group->setAlignAnchorHorz(ANCHOR_ELEM);
		group->setDefaultAlignAnchor(ANCHOR_ELEM);

		RackSVG::consumeAlignRequest(ctx, group);

		/*
		if (ctx.alignHorz.topol != AlignSVG::UNDEFINED_TOPOL){
			group->setAlign(AlignBase::HORZ, ctx.alignHorz.pos, ctx.alignHorz.topol);  // simplify
			mout.unimplemented<LOG_NOTICE>("Set: ", ctx.alignHorz, " -> ", group->getAlignStr());
			ctx.alignHorz.topol  = AlignSVG::UNDEFINED_TOPOL;
		}
		else {
			group->setAlign(ctx.alignHorz);
			// group->setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); // AlignSVG::LEFT);
			mout.accept<LOG_NOTICE>("Using HORZ align: ", ctx.alignHorz, " -> ", group->getAlignStr());
		}

		if (ctx.alignVert.topol != AlignSVG::UNDEFINED_TOPOL){
			group->setAlign(AlignBase::VERT, ctx.alignVert.pos, ctx.alignVert.topol); // simplify
			mout.unimplemented<LOG_NOTICE>("Set: ", ctx.alignVert, " -> ", group->getAlignStr());
			ctx.alignVert.topol  = AlignSVG::UNDEFINED_TOPOL;
		}
		else {
			// group->setAlign(AlignSVG::TOP, AlignSVG::INSIDE); // AlignSVG::BOTTOM);
			group->setAlign(ctx.alignVert);
			// group->setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); // AlignSVG::LEFT);
			mout.accept<LOG_NOTICE>("Using VERT align: ", ctx.alignVert, " -> ", group->getAlignStr());
		}
		 */

		drain::image::TreeSVG & rect = group[ANCHOR_ELEM](svg::RECT); // +EXT!
		rect->set("width", frame.width);
		//rect->set("width", 10); //margin!
		rect->set("height", frame.height);
		rect->set("label", ANCHOR_ELEM);
		rect->setStyle("fill", "yellow");
		rect->setStyle("opacity", 0.5);
		rect->setId("textRect");

		drain::image::TreeSVG & rectTitle = group["title"](svg::RECT); // +EXT!
		// rectTitle->set("width", 50);
		rectTitle->set("height", 60);
		rectTitle->setStyle("fill", "green");
		rectTitle->setStyle("opacity", 0.5);
		rectTitle->setId("textRect");
		rectTitle->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
		rectTitle->setAlign(AlignSVG::Owner::OBJECT, AlignBase::HORZ, AlignBase::Pos::FILL);
		//rectTitle->setAlign(AlignSVG::HORZ_FILL);

		drain::image::TreeSVG & rectV = group["title2"](svg::RECT); // +EXT!
		rectV->set("width", 25);
		rectV->setStyle("fill", "red");
		rectV->setStyle("opacity", 0.5);
		rectV->setId("textV");
		rectV->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
		rectV->setAlign(AlignSVG::Owner::OBJECT, AlignBase::VERT, AlignBase::Pos::FILL);

		/*
		drain::image::TreeSVG & textGroup = group["text-group"](svg::GROUP);
		textGroup->set("width", frame.width);
		textGroup->set("height", frame.height);
		textGroup->setId("textGroup");
		textGroup->setAlign(value);
		drain::image::AlignBase::Pos horzPos = textGroup->getAlign(drain::image::AlignSVG::Owner::ANCHOR, drain::image::AlignBase::Axis::HORZ);
		 */

		drain::image::AlignBase::Pos horzPos = group->getAlign(drain::image::AlignSVG::Owner::ANCHOR, drain::image::AlignBase::Axis::HORZ);
		// AlignSVG alignSvg;


		mout.reject<LOG_NOTICE>("Main align:", ctx.alignHorz, ", ", ctx.alignVert);

		bool FIRST = true;
		int index = 0;
		for (const std::string s: {"Hello,", "world!", "My name is Test."}){
			mout.reject<LOG_NOTICE>("NOW YES ", s);

			index += 15;
			// for (const std::string & s: args){
			// drain::image::TreeSVG & text = group[s + "_text"](svg::TEXT);
			drain::image::TreeSVG & text = group.addChild()(svg::TEXT);
			//text->setId(drain::StringBuilder<'_'>("textline", ++index));
			text->setId("textline", index);
			text->setText(s);
			text->setWidth(10); // Margin + row height! TODO / FIX

			rect->setHeight(3*index);

			// Set horz alignment for every element
			if (horzPos != AlignBase::Pos::UNDEFINED_POS){
				text->setAlign(AlignBase::HORZ, horzPos, AlignSVG::Topol::INSIDE);
			}
			else {
				text->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE); // AlignSVG::LEFT);
			}

			// Set verthorz alignment for every element
			//if (ctx.alignVert.topol != AlignSVG::UNDEFINED_TOPOL){
			if (FIRST){
				FIRST = false;
			}
			else {
				// Row after row
				text->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
			}

			mout.accept<LOG_NOTICE>("TEXT ", s, " aligned: ", text->getAlignStr());
		}

		// mout.reject<LOG_NOTICE>(" ->  align:", ctx.topol, '|', ctx.halign, '/', ctx.valign);
		// mout.reject<LOG_NOTICE>("Main align:", ctx.alignHorz, ", ", ctx.alignVert);
		/*
		ctx.alignHorz.topol  = AlignSVG::UNDEFINED_TOPOL;
		ctx.alignVert.topol  = AlignSVG::UNDEFINED_TOPOL;
		 */
	}
};

class CmdStyle : public drain::SimpleCommand<std::string> {

public:

	CmdStyle() : drain::SimpleCommand<std::string>(__FUNCTION__, "Add or modify CSS entry") {
		//getParameters().link("level", level = 5);
	}

	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		//drain::image::TreeSVG & style = ctx.getStyle(); // consider static/global?
		drain::image::TreeSVG & style = RackSVG::getStyle(ctx); // consider static/global?

		std::string cssSelector, cssConf;
		if (drain::StringTools::split2(value, cssSelector, cssConf, '=')){
			// style->setStyle(cssSelector, cssConf+"; setStyle:1");
			// style->set(cssSelector, cssConf+"; attrib:1");
			// Not recommended:
			// style[cssSelector] = cssConf; // Assings string to CTEXT !? problem!
			// drain::SmartMapTools::setValues(style[cssSelector+"MIKA"]->getAttributes(), cssConf, ';', ':');
			style[cssSelector]->getAttributes().setValues(cssConf, ':', ';');
			// style[cssSelector].data = {{"koe", "black"}};      // OK but parallel..
			//drain::TreeUtils::dump(ctx.svgTrack);
		}
		else {
			mout.error("Missing '=' in assigment");
		}



	}

};


/// Currenly, value is not used.
class CmdPanel : public drain::SimpleCommand<std::string> {

public:

	CmdPanel() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "layout", "TECH") {
		//getParameters().link("level", level = 5);
	}

	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		PanelConfSVG & conf = ctx.svgPanelConf;

		drain::Frame2D<double> frame = {160,300};

		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[PanelConfSVG::SIDE_PANEL](svg::GROUP);
		group->setId(value);
		group->addClass(PanelConfSVG::SIDE_PANEL);
		//group->setAlignAnchor(RackSVG::BACKGROUND_RECT);

		RackSVG::consumeAlignRequest(ctx, group);


		drain::image::TreeSVG & rect = group[RackSVG::BACKGROUND_RECT](svg::RECT); // +EXT!
		rect->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		rect->setWidth(frame.width);
		rect->setHeight(frame.height);
		/*
		 does not work over
		rect->setAlignAnchorVert("*");
		rect->setAlign(AlignSVG::VERT_FILL);
		 */
		// rect->setAlign(AlignSVG::VERT_FILL);

		static std::string HEADER_RECT("headerRect");
		drain::VariableMap & status = ctx.getStatusMap();

		drain::image::TreeSVG & rect2 = group[HEADER_RECT](svg::RECT); // +EXT!

		VariableFormatterODIM<std::string> formatter;

		{
			rect2->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
			rect2->setMyAlignAnchor(RackSVG::BACKGROUND_RECT);
			rect2->setAlign(AlignSVG::TOP, AlignSVG::HORZ_FILL);
			rect2->setHeight(120);

			drain::image::TreeSVG & text = group.addChild()(svg::TEXT);
			text->setMyAlignAnchor(HEADER_RECT);
			text->setAlign(AlignSVG::TOP, AlignSVG::CENTER);
			text->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
			text->setFontSize(conf.fontSizes[1], conf.boxHeights[1]);
			text->setStyle("fill", "lightblue");
			text->setText(status["PLC"]);

			drain::image::TreeSVG & text2 = group.addChild()(svg::TEXT);
			text2->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
			text2->setAlign(AlignSVG::CENTER);
			text2->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
			text2->setFontSize(conf.fontSizes[0], conf.boxHeights[0]);
			text2->setText(status["NOD"]);

			std::stringstream sstr;

			drain::image::TreeSVG & date = group["date"](svg::TEXT); //addTextElem(group, "date");
			date->setMyAlignAnchor(HEADER_RECT);
			// date->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT); // CENTER);
			date->setAlign(AlignSVG::BOTTOM, AlignSVG::INSIDE);
			date->setAlign(AlignSVG::CENTER);
			date->addClass(PanelConfSVG::ElemClass::TIME);
			date->setStyle("fill", "gray");
			date->setFontSize(conf.fontSizes[1], conf.boxHeights[1]);
			formatter.formatDate(sstr, "date", status.get("what:date", ""), "%Y/%m/%d");
			date->setText(sstr.str());
			// mout.attention("DATE:", sstr.str());

			drain::image::TreeSVG & time = group["time"](svg::TEXT); // addTextElem(group, "time");
			time->setMyAlignAnchor<AlignBase::Axis::HORZ>(HEADER_RECT);
			//time->setAlign(AlignSVG::BOTTOM, AlignSVG::INSIDE);
			time->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE); // over ["date"]
			time->setAlign(AlignSVG::CENTER);
			time->setStyle("fill", "white");
			time->setFontSize(conf.fontSizes[0], conf.boxHeights[0]);
			// time->setMargin(conf.boxHeights[1]*2.1); // KLUDGE, fix later with negative HEIGHT handling
			// time->setFontSize(20,25);		//time->setMargin(15);
			sstr.str("");
			formatter.formatTime(sstr, "time", status.get("what:time", ""), "%H:%M UTC");
			time->setText(sstr.str());
			//time->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
			//time->setAlign(AlignSVG::CENTER);
		}

		std::string anchorVert(HEADER_RECT);

		// "where:BBOX",
		// svg::coord_t rotate = 0.0;
		for (const auto key: {"what:object", "what:product", "what:prodpar", "what:quantity", "how:angles", "how:lowprf", "how:hiprf",  "where:EPSG"}){

			std::string value = status.get(key, "");
			if (!value.empty()){
				// drain::image::TreeSVG & t = addTextElem(group, key);

				drain::image::TreeSVG & tkey = group[key](svg::TEXT);
				tkey->setId(key);
				tkey->setMyAlignAnchor<AlignBase::HORZ>(RackSVG::BACKGROUND_RECT);
				tkey->setMyAlignAnchor<AlignBase::VERT>(anchorVert);
				anchorVert.clear();
				tkey->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
				tkey->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
				tkey->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
				tkey->setFontSize(conf.fontSizes[2], conf.boxHeights[1]);
				tkey->setStyle("fill", "darkgray");
				tkey->setText(key);

				drain::image::TreeSVG & tval = group[tkey->getId()+"Value"](svg::TEXT);
				tval->setMyAlignAnchor<AlignBase::HORZ>(RackSVG::BACKGROUND_RECT);
				tval->setAlign(AlignSVG::RIGHT, AlignSVG::INSIDE);
				tval->setMyAlignAnchor<AlignBase::VERT>(key);
				tval->setAlign(AlignSVG::BOTTOM, AlignSVG::INSIDE);
				tval->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
				tval->setFontSize(conf.fontSizes[1], conf.boxHeights[1]);

				const std::string & format = RackSVG::guessFormat(key);
				if (!format.empty()){
					std::stringstream sstr;
					formatter.formatVariable(key, value, format, sstr);
					tval->setText(sstr.str()); //CTXX
				}
				else {
					tval->setText(value);
				}
				//t->setText(entry, ":", status[entry]);

			}
		}


	}

	// , const drain::VariableMap & vmap
	drain::image::TreeSVG & addTextElem(drain::image::TreeSVG & group, const std::string & key) const {
		TreeSVG & t = group[key](svg::TEXT);
		// t->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		t->setFontSize(10.0);
		t->setMyAlignAnchor<AlignBase::HORZ>(RackSVG::BACKGROUND_RECT);
		t->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
		t->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
		return t;
	}

};

class CmdPanelTest : public drain::SimpleCommand<std::string> {

public:

	CmdPanelTest() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "layout") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {

		// ClassLabelXML<drain::image::AlignSVG> label1(drain::image::AlignSVG::PANEL);
		// ClassLabelXML<drain::image::AlignSVG> label2("PANEL");

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::Frame2D<double> frame = {300,600};

		//drain::image::TreeSVG & group = ctx.getCurrentAlignedGroup()[value](svg::GROUP);
		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[value](svg::GROUP);
		group->setId(value);
		// rectGroup->addClass(drain::image::LayoutSVG::ALIG NED);
		const std::string ANCHOR_ELEM("anchor-elem");
		group->setDefaultAlignAnchor(ANCHOR_ELEM);
		// rectGroup->setAlign<AlignSVG::OUTSIDE>(AlignSVG::RIGHT);


		if (ctx.mainOrientation == drain::image::AlignBase::Axis::HORZ){
			group->setAlign(AlignBase::Axis::HORZ, (ctx.mainDirection==LayoutSVG::Direction::INCR) ? AlignBase::MAX : AlignBase::MIN, AlignSVG::OUTSIDE);
			group->setAlign(AlignBase::Axis::VERT, AlignBase::MIN, AlignSVG::INSIDE); // drain::image::AlignSVG::VertAlignBase::TOP);
		}
		else { // VERT  -> ASSERT? if (ctx.mainOrientation == drain::image::AlignBase::Axis::VERT){
			group->setAlign(AlignBase::Axis::HORZ, AlignBase::MIN, AlignSVG::INSIDE); // drain::image::AlignSVG::HorzAlignBase::LEFT);
			group->setAlign(AlignBase::Axis::VERT, (ctx.mainDirection==LayoutSVG::Direction::INCR) ? AlignBase::MAX : AlignBase::MIN, AlignSVG::OUTSIDE);
		}


		drain::image::TreeSVG & rect = group[ANCHOR_ELEM](svg::RECT); // +EXT!
		rect->set("width", frame.width);
		rect->set("height", frame.height);
		rect->set("label", ANCHOR_ELEM);
		rect->setStyle("fill", "red");

		// rect->addClass(LayoutSVG::INDEPENDENT);

		// rect->setAlign(AlignSVG::OBJECT, AlignBase::HORZ,  AlignBase::MAX);
		// rect->setAlign<AlignSVG::OUTSIDE>(AlignSVG::OBJECT, AlignBase::HORZ,  AlignBase::MAX);
		// rect->setAlign<AlignSVG::OUTSIDE>(AlignSVG::RIGHT);
		// rect["basename"](drain::image::svg::TITLE) = "test";
		/*
		rect->setAlign(AlignSVG2::ORIG, AlignSVG2::HORZ,  AlignSVG2::MID);
		rect->setAlign(AlignSVG2::REF, AlignSVG2::VERT,  AlignSVG2::MAX);
		rect->setAlign(AlignSVG2::REF, AlignSVG2::HORZ,  AlignSVG2::MIN);
		 */

		typedef drain::image::AlignSVG::Owner  Owner;
		typedef drain::image::AlignBase::Pos   Pos;

		const drain::EnumDict<Pos>::dict_t & dict = drain::EnumDict<Pos>::dict;


		//const std::list<Pos> pos = {AlignBase::MAX, AlignBase::MIN, AlignBase::MID};
		// for (const drain::image::LayoutSVG::Axis & ax: {AlignAxis::HORZ, AlignAxis::VERT}){
		for (const Pos & posVert: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ //pos){ // {AlignBase::MID} pos

			char pv = dict.getKey(posVert)[2];

			for (const Pos & posHorz: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ // pos

				char ph = dict.getKey(posHorz)[2];

				for (const Pos & posVertAnhor: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ // {AlignBase::MID}){

					char rv = dict.getKey(posVertAnhor)[2];

					for (const Pos & posHorzAnchor: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ //pos){

						char rh = dict.getKey(posHorzAnchor)[2];

						//const std::string label = drain::StringBuilder<'-'>(posHorzRef, posVertRef, posHorz, posVert, '-', ph, pv, rh, rv);
						const std::string label = drain::StringBuilder<'-'>(ph, pv, rh, rv);

						drain::image::TreeSVG & text = group[label + "text"](svg::TEXT);
						text->setId(label+"_T");
						text->getBoundingBox().setArea(60,30);
						text->setAlign(Owner::ANCHOR, AlignBase::HORZ, posHorzAnchor);
						text->setAlign(Owner::ANCHOR, AlignBase::VERT, posVertAnhor);
						text->setAlign(Owner::OBJECT, AlignBase::HORZ, posHorz);
						text->setAlign(Owner::OBJECT, AlignBase::VERT, posVert);
						text->setText(label);
						text->setFontSize(6.0, 8.0);

						drain::image::TreeSVG & textBox = group[label](svg::RECT);
						textBox->setId(label+"_R");
						textBox->getBoundingBox().setArea(60,30);
						//textBox->set("mika", textBox->getAlignStr()); // textBox->set("mika", textBox->getAlignStr());
						textBox->setStyle("fill", "green");
						textBox->setStyle("opacity", 0.15);
						textBox->setStyle("stroke-width", "2px");
						textBox->setStyle("stroke", "black");
						textBox->setAlign(Owner::ANCHOR, AlignBase::HORZ, posHorzAnchor);
						textBox->setAlign(Owner::ANCHOR, AlignBase::VERT, posVertAnhor);
						textBox->setAlign(Owner::OBJECT, AlignBase::HORZ, posHorz);
						textBox->setAlign(Owner::OBJECT, AlignBase::VERT, posVert);
						//textBox->addClass(LayoutSVG::INDEPENDENT);

						//text->addClass(LayoutSVG::INDEPENDENT);
						// drain::image::TreeSVG & textSpan = text["tspan"](svg::TSPAN);
						// textSpan->setText(text->getAlignStr());

					}
				}
			}
		}


	}

};

}


namespace rack {


class CmdPolarGrid : public drain::SimpleCommand<std::string> {

public:

	// double is over
	drain::Range<int> rangeMeters = {0,250000};
	// Major, minor?
	int meterStep = 25000;

	drain::Range<int> rangeDegrees = {0,360};
	// Major, minor?
	int degreeStep = 15;


	CmdPolarGrid() : drain::SimpleCommand<std::string>(__FUNCTION__, "Select (and draw) sector using natural coordinates or indices") { // __FUNCTION__, "Adjust font sizes in CSS style section.") {
	};


	virtual inline
	void parameterKeysToStream(std::ostream & ostr) const override {
		// Command::parameterKeysToStream(ostr, polarSector.getParameters().getKeyList(), ',');
	};


	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		// polarSector.reset();

		// polarSector.setParameters(value);

		const Hi5Tree & srcPolar = ctx.getHi5(RackContext::POLAR|RackContext::CURRENT); // , RackContext::CURRENT
		const Hi5Tree & srcCurr  = ctx.getHi5(RackContext::CURRENT);

		if (srcPolar.empty()){
			mout.warn("No polar data read, cannot focus on a specific radar");
			return;
		}


		RadarSVG radarSVG;
		const drain::VariableMap & where = srcPolar[ODIMPathElem::WHERE].data.attributes;

		radarSVG.radarProj.setSiteLocationDeg(where["lon"], where["lat"]);

		// RadarProj radarProj(srcPolar[ODIMPathElem::WHERE].data.attributes["lon"], srcPolar[ODIMPathElem::WHERE].data.attributes["lat"]);
		// proj.setSiteLocationDeg();

		const drain::Variable & object = srcCurr[ODIMPathElem::WHAT].data.attributes["object"];
		if ((object == "SCAN") || (object == "PVOL")){
			mout.note("Polar coordinates"); // Cartesian not "found", ie not created this.
			mout.warn("Current object is not projected (Cartesian) data, cannot focus on a specific radar");
			return;
		}
		else {
			// mout.note("Cartesian");

			const drain::VariableMap & where = srcCurr[ODIMPathElem::WHERE].data.attributes;

			// const drain::Rectangle<double> bboxComposite(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			// mout.special("Composite BBOX:", bboxComposite);

			// drain::image::GeoFrame geoFrame;
			int epsg = where.get("epsg", 0); // non-standard
			if (epsg){
				mout.attention("EPSG found: ", epsg);
				radarSVG.geoFrame.setProjectionEPSG(epsg);
			}
			else {
				radarSVG.geoFrame.setProjection(where["projdef"]);
			}
			radarSVG.geoFrame.setBoundingBoxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			radarSVG.geoFrame.setGeometry(where["xsize"], where["ysize"]);

			mout.special("GeoFrame BBOX: ", radarSVG.geoFrame);
			mout.special("GeoFrame BBOX: ", radarSVG.geoFrame.getBoundingBoxNat());

			radarSVG.radarProj.setProjectionDst(where.get("projdef", ""));
			double radius = 250000.0;
			drain::Rectangle<double> bbox;
			radarSVG.radarProj.determineBoundingBoxM(radius, bbox); // M = "native"
			// drain::Rectangle<double> bbox;
			mout.special("BBOX (250km) of the last input:", bbox);

			//
			drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx);
			//RackSVG::get


			// TODO: default group?
			drain::image::TreeSVG & geoGroup = group["geoGroup"](drain::image::svg::GROUP);

			// Set defaults...
			geoGroup->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
			geoGroup->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
			// ... but override, if explicitly set.
			RackSVG::consumeAlignRequest(ctx, geoGroup);


			// Circle as a polygon (to master projection)
			// <polygon points="100,100 150,25 150,75 200,0" fill="none" stroke="black" />
			drain::image::TreeSVG & circle = geoGroup["radarCircle"](drain::image::svg::POLYGON);
			circle->setStyle({
				{"fill", "blue"},
				{"stroke", "red"},
				{"stroke-width", 5.1},
				{"opacity", 0.65},
			});

			DRAIN_SVG_ELEM_CLS(POLYGON) polygonElem(circle);

			drain::image::TreeSVG & curve = geoGroup["radarPath"](drain::image::svg::PATH);
			curve -> setStyle({
				{"fill", "none"},
				{"stroke", "green"},
				{"stroke-width", 12.0},
				{"opacity", 0.65}
			});

			// typedef drain::image::svg::tag_t::PATH curve_t ;

			drain::svgPATH bezierElem(curve);

			// Note: polygon path has (here) integer coordinates; assuming integer frame.
			drain::Point2D<double> geoPoint;
			drain::Point2D<int> imgPoint;
			// consider generalize?
			std::list<drain::Point2D<int> > debugPath;

			const int radialResolution = 7; // steps
			radarSVG.setRadialResolution(radialResolution);

			/*

			for (int i=0; i<=radialResolution; ++i){
				azimuth = static_cast<double>(i*360/radialResolution) * drain::DEG2RAD;
				// azimuths.max = azimuth;

				radarSVG.radarProj.projectFwd(radius*::sin(azimuth), radius*::cos(azimuth), geoPoint.x, geoPoint.y);
				radarSVG.geoFrame.m2pix(geoPoint, imgPoint);
				debugPath.push_back(imgPoint);

				polygonElem.append(imgPoint);


				azimuthPrev = azimuth;

			}
			*/

			const drain::Range<double> r(rangeMeters); // int->double
			const drain::Range<double> a(rangeDegrees);// int->double
			a.min *= drain::DEG2RAD;
			a.max *= drain::DEG2RAD;

			for (int i=rangeMeters.min; i<rangeMeters.max; i += meterStep){
				radarSVG.moveTo(bezierElem, i, a.min);
				radarSVG.cubicBezierTo(bezierElem, i, a.min, a.max);
			}


			// mout.attention("Range: ", polarSector.distanceRange, " Azm:", polarSector.azmRange);
			/*
			radarSVG.moveTo(bezierElem, r.max, a.min);
			radarSVG.cubicBezierTo(bezierElem, r.max, a.min, a.max);
			radarSVG.lineTo(bezierElem, r.min, a.max);
			radarSVG.cubicBezierTo(bezierElem, r.min, a.max, a.min);
			radarSVG.lineTo(bezierElem, r.max, a.min);
			radarSVG.close(bezierElem);
			*/

			// mout.accept<LOG_NOTICE>(path);
			/*
			std::string polygonPathStr = drain::sprinter(debugPath, {" "}).str();
			mout.accept<LOG_NOTICE>(polygonPathStr);
			//circle -> set("points", polygonPathStr);
			circle[drain::image::svg::DESC]->setText(polygonPathStr);
			*/
		}

	};


};



class CmdSector : public drain::SimpleCommand<std::string> {

public:

	CmdSector() : drain::SimpleCommand<std::string>(__FUNCTION__, "Select (and draw) sector using natural coordinates or indices") { // __FUNCTION__, "Adjust font sizes in CSS style section.") {
	};


	virtual inline
	void parameterKeysToStream(std::ostream & ostr) const override {
		Command::parameterKeysToStream(ostr, polarSector.getParameters().getKeyList(), ',');
	};


	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		polarSector.reset();

		polarSector.setParameters(value);

		const Hi5Tree & srcPolar = ctx.getHi5(RackContext::POLAR|RackContext::CURRENT); // , RackContext::CURRENT
		const Hi5Tree & srcCurr  = ctx.getHi5(RackContext::CURRENT);

		if (srcPolar.empty()){
			mout.warn("No polar data read, cannot focus on a specific radar");
			return;
		}

		// prepare ("interpret") arguments
		if (polarSector.binRange.max < 0){

		}

		std::cout << polarSector.getParameters() << std::endl;

		RadarSVG radarSVG;
		const drain::VariableMap & where = srcPolar[ODIMPathElem::WHERE].data.attributes;

		radarSVG.radarProj.setSiteLocationDeg(where["lon"], where["lat"]);

		// RadarProj radarProj(srcPolar[ODIMPathElem::WHERE].data.attributes["lon"], srcPolar[ODIMPathElem::WHERE].data.attributes["lat"]);
		// proj.setSiteLocationDeg();

		const drain::Variable & object = srcCurr[ODIMPathElem::WHAT].data.attributes["object"];
		if ((object == "SCAN") || (object == "PVOL")){
			mout.note("Polar coordinates"); // Cartesian not "found", ie not created this.
		}
		else {
			mout.note("Cartesian");

			const drain::VariableMap & where = srcCurr[ODIMPathElem::WHERE].data.attributes;

			// const drain::Rectangle<double> bboxComposite(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			// mout.special("Composite BBOX:", bboxComposite);

			// drain::image::GeoFrame geoFrame;
			int epsg = where.get("epsg", 0); // non-standard
			if (epsg){
				mout.attention("EPSG found: ", epsg);
				radarSVG.geoFrame.setProjectionEPSG(epsg);
			}
			else {
				radarSVG.geoFrame.setProjection(where["projdef"]);
			}
			radarSVG.geoFrame.setBoundingBoxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			radarSVG.geoFrame.setGeometry(where["xsize"], where["ysize"]);

			mout.special("GeoFrame BBOX: ", radarSVG.geoFrame);
			mout.special("GeoFrame BBOX: ", radarSVG.geoFrame.getBoundingBoxNat());

			radarSVG.radarProj.setProjectionDst(where.get("projdef", ""));
			double radius = 250000.0;
			drain::Rectangle<double> bbox;
			radarSVG.radarProj.determineBoundingBoxM(radius, bbox); // M = "native"
			// drain::Rectangle<double> bbox;
			mout.special("BBOX (250km) of the last input:", bbox);

			//
			drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx);
			//RackSVG::get


			// TODO: default group?
			drain::image::TreeSVG & geoGroup = group["geoGroup"](drain::image::svg::GROUP);

			// Set defaults...
			geoGroup->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
			geoGroup->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
			// ... but override, if explicitly set.
			RackSVG::consumeAlignRequest(ctx, geoGroup);


			drain::image::TreeSVG & curve = geoGroup["radarPath"](drain::image::svg::PATH);
			curve -> setStyle({
				{"fill", "none"},
				{"stroke", "green"},
				{"stroke-width", 12.0},
				{"opacity", 0.65}
			});

			// TreeElemUtilsSVG.h
			drain::svgPATH bezierElem(curve);

			// Note: polygon path has (here) integer coordinates; assuming integer frame.
			// drain::Point2D<double> geoPoint;
			// drain::Point2D<int> imgPoint;

			const int radialResolution = 8; // steps
			radarSVG.setRadialResolution(radialResolution);


			const drain::Range<double> & r = polarSector.distanceRange;
			const drain::Range<double> & a = polarSector.azmRange;
			a.min *= drain::DEG2RAD;
			a.max *= drain::DEG2RAD;

			// mout.attention("Range: ", polarSector.distanceRange, " Azm:", polarSector.azmRange);
			radarSVG.moveTo(bezierElem, r.max, a.min);
			radarSVG.cubicBezierTo(bezierElem, r.max, a.min, a.max);
			radarSVG.lineTo(bezierElem, r.min, a.max);
			radarSVG.cubicBezierTo(bezierElem, r.min, a.max, a.min);
			radarSVG.lineTo(bezierElem, r.max, a.min);
			radarSVG.close(bezierElem);


		}

	};

	mutable
	PolarSector polarSector;

};


class CmdSectorTest : public drain::SimpleCommand<std::string> {

public:

	CmdSectorTest() : drain::SimpleCommand<std::string>(__FUNCTION__, "Select (and draw) sector using natural coordinates or indices") { // __FUNCTION__, "Adjust font sizes in CSS style section.") {
	};


	virtual inline
	void parameterKeysToStream(std::ostream & ostr) const override {
		Command::parameterKeysToStream(ostr, polarSector.getParameters().getKeyList(), ',');
	};


	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		polarSector.reset();

		polarSector.setParameters(value);

		const Hi5Tree & srcPolar = ctx.getHi5(RackContext::POLAR|RackContext::CURRENT); // , RackContext::CURRENT
		const Hi5Tree & srcCurr  = ctx.getHi5(RackContext::CURRENT);

		if (srcPolar.empty()){
			mout.warn("No polar data read, cannot focus on a specific radar");
			return;
		}

		// prepare ("interpret") arguments
		if (polarSector.binRange.max < 0){

		}

		std::cout << polarSector.getParameters() << std::endl;

		RadarSVG radarSVG;
		const drain::VariableMap & where = srcPolar[ODIMPathElem::WHERE].data.attributes;

		radarSVG.radarProj.setSiteLocationDeg(where["lon"], where["lat"]);

		// RadarProj radarProj(srcPolar[ODIMPathElem::WHERE].data.attributes["lon"], srcPolar[ODIMPathElem::WHERE].data.attributes["lat"]);
		// proj.setSiteLocationDeg();

		const drain::Variable & object = srcCurr[ODIMPathElem::WHAT].data.attributes["object"];
		if ((object == "SCAN") || (object == "PVOL")){
			mout.note("Polar coordinates"); // Cartesian not "found", ie not created this.
		}
		else {
			mout.note("Cartesian");

			const drain::VariableMap & where = srcCurr[ODIMPathElem::WHERE].data.attributes;

			// const drain::Rectangle<double> bboxComposite(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			// mout.special("Composite BBOX:", bboxComposite);

			// drain::image::GeoFrame geoFrame;
			int epsg = where.get("epsg", 0); // non-standard
			if (epsg){
				mout.attention("EPSG found: ", epsg);
				radarSVG.geoFrame.setProjectionEPSG(epsg);
			}
			else {
				radarSVG.geoFrame.setProjection(where["projdef"]);
			}
			radarSVG.geoFrame.setBoundingBoxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
			radarSVG.geoFrame.setGeometry(where["xsize"], where["ysize"]);

			mout.special("GeoFrame BBOX: ", radarSVG.geoFrame);
			mout.special("GeoFrame BBOX: ", radarSVG.geoFrame.getBoundingBoxNat());

			radarSVG.radarProj.setProjectionDst(where.get("projdef", ""));
			double radius = 250000.0;
			drain::Rectangle<double> bbox;
			radarSVG.radarProj.determineBoundingBoxM(radius, bbox); // M = "native"
			// drain::Rectangle<double> bbox;
			mout.special("BBOX (250km) of the last input:", bbox);

			//
			drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx);
			//RackSVG::get


			// TODO: default group?
			drain::image::TreeSVG & geoGroup = group["geoGroup"](drain::image::svg::GROUP);

			// Set defaults...
			geoGroup->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
			geoGroup->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
			// ... but override, if explicitly set.
			RackSVG::consumeAlignRequest(ctx, geoGroup);

			/* Test adapters
			{
				using namespace drain::image;
				NodeSVG::Elem<svg::CIRCLE> test(geoGroup["ZIRK"]);
				NodeSVG::Elem<svg::RECT> test2(geoGroup["REKT"]);

			}
			*/

			// Circle as a polygon (to master projection)
			// <polygon points="100,100 150,25 150,75 200,0" fill="none" stroke="black" />
			drain::image::TreeSVG & circle = geoGroup["radarCircle"](drain::image::svg::POLYGON);
			circle->setStyle({
				{"fill", "blue"},
				{"stroke", "red"},
				{"stroke-width", 5.1},
				{"opacity", 0.65},
			});
			/*
			circle -> setStyle("fill",   "blue");
			circle -> setStyle("stroke", "red");
			circle -> setStyle("stroke-width", 5.1);
			circle -> setStyle("opacity", 0.85);
			*/

			DRAIN_SVG_ELEM_CLS(POLYGON) polygonElem(circle);

			drain::image::TreeSVG & curve = geoGroup["radarPath"](drain::image::svg::PATH);
			curve -> setStyle({
				{"fill", "none"},
				{"stroke", "green"},
				{"stroke-width", 12.0},
				{"opacity", 0.65}
			});
			// curve -> setStyle("fill",   "none");
			// curve -> setStyle("stroke", "green");
			// curve -> setStyle("stroke-width", 12.0);
			// curve -> setStyle("opacity", 0.95);

			// typedef drain::image::svg::tag_t::PATH curve_t ;

			drain::svgPATH bezierElem(curve);

			// Note: polygon path has (here) integer coordinates; assuming integer frame.
			drain::Point2D<double> geoPoint;
			drain::Point2D<int> imgPoint;
			// consider generalize?
			std::list<drain::Point2D<int> > debugPath;

			const int radialResolution = 7; // steps
			radarSVG.setRadialResolution(radialResolution);

			double azimuth, azimuthPrev = 0.0;


			for (int i=0; i<=radialResolution; ++i){
				azimuth = static_cast<double>(i*360/radialResolution) * drain::DEG2RAD;
				// azimuths.max = azimuth;

				radarSVG.radarProj.projectFwd(radius*::sin(azimuth), radius*::cos(azimuth), geoPoint.x, geoPoint.y);
				radarSVG.geoFrame.m2pix(geoPoint, imgPoint);
				debugPath.push_back(imgPoint);

				polygonElem.append(imgPoint);

				/*
				// Bezier
				if (i == 0){
					radarSVG.moveTo(bezierElem, radius, azimuth);
				}
				else {
					// radarSVG.cubicBezierTo(bezierElem, imgPoint, radius, azimuthPrev, azimuth);
					radarSVG.cubicBezierTo(bezierElem, radius, azimuthPrev, azimuth);
				}
				*/

				azimuthPrev = azimuth;

			}

			const drain::Range<double> & r = polarSector.distanceRange;
			const drain::Range<double> & a = polarSector.azmRange;
			a.min *= drain::DEG2RAD;
			a.max *= drain::DEG2RAD;

			// mout.attention("Range: ", polarSector.distanceRange, " Azm:", polarSector.azmRange);
			radarSVG.moveTo(bezierElem, r.max, a.min);
			radarSVG.cubicBezierTo(bezierElem, r.max, a.min, a.max);
			radarSVG.lineTo(bezierElem, r.min, a.max);
			radarSVG.cubicBezierTo(bezierElem, r.min, a.max, a.min);
			radarSVG.lineTo(bezierElem, r.max, a.min);

			radarSVG.close(bezierElem);

			// mout.accept<LOG_NOTICE>(path);
			std::string polygonPathStr = drain::sprinter(debugPath, {" "}).str();
			mout.accept<LOG_NOTICE>(polygonPathStr);
			//circle -> set("points", polygonPathStr);
			circle[drain::image::svg::DESC]->setText(polygonPathStr);

		}

	};

	mutable
	PolarSector polarSector;

};


//#include <drain/util/TreeUtils.h>
/*
class BBoxRetrieverSVG2 : public drain::TreeVisitor<TreeSVG> {

public:

	BBoxSVG box;

	int visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path) override {


		BBoxSVG b;

		NodeSVG & node = tree(path).data;

		if (node.isAbstract()){
			return 1;
		}

		// node.get("data-bb") = 0.0;

		double r=0.0;
		switch (node.getNativeType()){
		//switch (tree->getType()){
		case svg::CIRCLE:
			r = node.get("r");
			b.setLocation(node.get("cx", 0.0)-r, node.get("cy", 0.0)-r);
			b.setArea(2.0*r, 2.0*r);
			// node.get("data-bb") = b.getLocation().tuple();
			node.get("data-bb") << b.x << b.y << b.width << b.height;
			// node.get("data-bb") << b.width << b.height;
			break;
		//case svg::RECT:
		default:
			drain::Logger(__FILE__, "BBoxRetrieverSVG2::visitPrefix").warn("unhandled type: ", tree->getNativeType());

		}

		if (box.empty()){
			box = b; // .set(b.x, b.y, b.width, b.height);
			//box.set(b.tuple());
		}
		else {
			box.expand(b);
		}


		return 0; // continue
	}

	//int visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path) override;

};
 */




// Constructionistic fine arts
drain::image::TreeSVG & addDummyObject(drain::image::TreeSVG & group){ // , double dx, double dy){

	drain::Logger mout(__FUNCTION__, __FILE__);

	using namespace drain::image;

	typedef svg::tag_t tag_t;

	const std::string name = drain::StringBuilder<>("dummy", group.getChildren().size());
	TreeSVG & subGroup = group[name](tag_t::GROUP);
	subGroup->setId(name);
	//
	subGroup->addClass(LayoutSVG::COMPOUND); // Compute (add) BBOx, skip recursion, do not align sub elements.
	// subGroup->addClass("COMPOUND"); // Compute (add) BBOx, skip recursion, do not align sub elements.
	// subGroup->addClass("DETECT_BBOX");

	TreeSVG & title = subGroup[tag_t::TITLE](tag_t::TITLE);
	title = name;

	drain::image::NodeSVG::Elem<tag_t::RECT> rect(subGroup["rectangle"]);
	rect.x = (::rand()&63); // + dx;
	rect.y = (::rand()&63); // + dy;
	rect.width  = (::rand()&255);
	rect.height = (::rand()&255);
	rect.node.setStyle("fill", "red");
	rect.node.setStyle("opacity", 0.5);

	drain::image::NodeSVG::Elem<tag_t::CIRCLE> circ(subGroup["circle"]);
	circ.cx = (::rand()&63) -20;
	circ.cy = (::rand()&63) +20;
	circ.r = int(::rand()&63);
	circ.node.setStyle("fill", "green");
	circ.node.setStyle("opacity", 0.5);

	drain::image::NodeSVG::Elem<tag_t::POLYGON> poly(subGroup["triangle"]);
	poly.node.setStyle("stroke", "blue");
	poly.node.setStyle("fill",   "cyan");
	poly.node.setStyle("opacity", 0.5);
	// poly.points.info(std::cerr); std::cerr << std::endl;
	// mout.attention("poly.path.info");
	poly.append(      (::rand()&127),      (::rand()&127));
	poly.append(-32 + (::rand()&63),  +64 +(::rand()&127));
	poly.append(+64 + (::rand()&127), +32 +(::rand()&63));

	drain::image::NodeSVG::Elem<tag_t::CIRCLE> origin(subGroup["origin"]);
	origin.cx = 0; // + dx;
	origin.cy = 0; // + dy;
	origin.r = 5.0;
	origin.node.setStyle("fill", "darkblue");
	origin.node.setStyle("opacity", 1.0);

	// drain::image::NodeSVG::Elem<tag_t::POLYGON> polyx(subGroup);
	// TreeUtilsSVG::detectBoxNEW(subGroup, true);

	return subGroup;
}

/**
 *   Only 1) direct objects 2) under group/STACK_LAYOUT will be aligned.
 *   - alignment axis (HORZ/VERT) alternates
 *   - alignment relies on bounding box: GROUP/COMPOUND, RECT, IMAGE
 *   - bbox will be adjusted: offset from origing removed, and alignment added as \c transform="translate(x,y)"
 */

// operator T() const

/*
operator const std::string &()(drain::image::AlignBase::Axis){
	static const std::string s("sksk");
	return s;
}
*/

class CmdAlignTest : public drain::BasicCommand { // drain::SimpleCommand<std::string> {

public:

	CmdAlignTest() : drain::BasicCommand(__FUNCTION__, "SVG test product") {
		getParameters().link("name",   name, "label");
		getParameters().link("panel",  panel, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::EnumDict<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
	}

	CmdAlignTest(const CmdAlignTest & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	const std::string defaultAnchor = "myRect";

	std::string panel = "playGround1";
	std::string name  = "";

	/*
	drain::image::TreeSVG & getPlayGround(RackContext & ctx) const {

		using namespace drain::image;

		drain::Logger mout(ctx.log, getName().c_str(), __FUNCTION__);

		// const std::string name = "playGround";

		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[panel]; // (svg::GROUP);

		if (group -> isUndefined()){

			group->setType(svg::GROUP);

			// Ensure style
			// This is "lazily" repeated upon repeated invocations
			drain::image::TreeSVG & style = RackSVG::getStyle(ctx);
			style[".DEBUG"]->set({
				{"stroke-width", 2},
				{"stroke", "darkgreen"},
				{"stroke-dasharray", {2,5,3}},
				{"fill", "none"},
				{"opacity", 0.7},
			});

			const drain::Frame2D<double> frame = {1280.0, 900.0}; // {640.0, 480};

			group->addClass(drain::image::LayoutSVG::STACK_LAYOUT);

			//group->setDefaultAlignAnchorHorz(ANCHOR_ELEM); // Note: axis Horz/Vert should be taken from ctx?
			group->setDefaultAlignAnchor(defaultAnchor); // Note: axis Horz/Vert should be taken from ctx?
			// AnchorElem::Anchor anchor = DRAIN_ENUM_DICT(AnchorElem::Anchor)::getValue("");
			AnchorElem::Anchor anchor = drain::EnumDict<AnchorElem::Anchor>::getValue(myAnchor);
			mout.accept<LOG_WARNING>("ANCHOR:", anchor);


			// default anchor
			drain::image::TreeSVG & rect = group[defaultAnchor](svg::RECT); // +EXT!
			rect->setId(defaultAnchor);
			rect->setName(defaultAnchor);
			//rect->getBoundingBox().setArea(frame);
			rect->setWidth(frame.width*0.75);
			rect->setHeight(frame.height*0.75);
			rect->setStyle("fill", "yellow");
			rect->setStyle("opacity", 0.5);

			// group->getBoundingBox().setArea(frame); // needed?
		}


		return group;

	}
	*/

	// template <typename T>
	// std::string static_cast<std::string>(drain::EnumDict<T>::){};

	void exec() const override {

		using namespace drain::image;

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		// drain::Version<RackContext> version(1,2,3);
		// drain::Version<CmdAlignTest> version2(5,2,3);

		const drain::Frame2D<double> frame = {768.0, 640.0};

		ctx.svgTrack->set("data-version", 2);
		ctx.svgTrack->setWidth(1.2 * frame.width);
		ctx.svgTrack->setHeight(1.2 * frame.height);

		ctx.svgTrack->setAlign(AlignSVG::TOP, AlignSVG::LEFT);

		drain::image::TreeSVG & debugStyle = RackSVG::getStyle(ctx)[".DEBUG"];
		if (debugStyle.empty()){
			debugStyle->set({
				{"stroke-width", 2},
				{"stroke", "darkgreen"},
				{"stroke-dasharray", {2,5,3}},
				{"fill", "none"},
				{"opacity", 0.7},
			});
		}

		// drain::image::TreeSVG & group = getPlayGround(ctx);
		drain::image::TreeSVG & group = RackSVG::getImagePanelGroup(ctx);

		// const drain::Frame2D<double> frame2 = group->getBoundingBox().getFrame();

		// Three sample objects, each consisting of three elements.
		drain::image::TreeSVG & compoundObject = addDummyObject(group); //, 0.5*frame.width, 0.1*frame.height );
		drain::image::NodeSVG & node = compoundObject; //, 0.5*frame.width, 0.1*frame.height );
		// node.transform.translate.set(0.5*frame.width, 0.1*frame.height);

		if (group.getChildren().size()<=1){
			// node.setMyAlignAnchor(myAnchor);
		}

		RackSVG::consumeAlignRequest(ctx, node);
		// default? node.setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);

		drain::image::NodeSVG & text = compoundObject[svg::TEXT](svg::TEXT);
		text.setFontSize(10.0, 12.0);
		text.setLocation(5,16);
		text.setText(name+'='+node.getAlignStr());
		// text.setAlignAnchor(node.getId());
		text.setMyAlignAnchor("rectangle");
		text.setAlign(drain::image::AlignSVG::MIDDLE);
		text.setAlign(drain::image::AlignSVG::CENTER);

		// BBoxSVG bbox;
		// TreeUtilsSVG::detectBoxNEW(ctx.svgTrack, true);
		mout.attention("Stacking: ", ctx.svgTrack.data);
		TreeUtilsSVG::addStackLayout(ctx.svgTrack, ctx.mainOrientation, ctx.mainDirection); // AlignBase::Axis::HORZ, LayoutSVG::Direction::INCR);
		mout.attention("Aligning: ", ctx.svgTrack.data);
		TreeUtilsSVG::superAlign(ctx.svgTrack); //, AlignBase::Axis::HORZ, LayoutSVG::Direction::INCR);

	}

};


class CmdDebug : public drain::BasicCommand { // drain::SimpleCommand<std::string> {

public:

	CmdDebug() : drain::BasicCommand(__FUNCTION__, "SVG test product") {
		getParameters().link("name",   name, "label");
		getParameters().link("panel",  panel, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::EnumDict<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
	}

	CmdDebug(const CmdDebug & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	const std::string defaultAnchor = "myRect";

	std::string panel = "playGround1";
	std::string name  = "";

	struct NodeVisitor : public drain::TreeVisitor<TreeSVG> {

		virtual inline
		int visitPrefix(TreeSVG & tree, const typename TreeSVG::path_t & path){
			return 0;
		}


	};

	void exec() const override {

		using namespace drain::image;

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		NodeVisitor visitor;
		// drain::TreeUtils::traverse(ctx.svgTrack, visitor);


	}



};

GraphicsModule::GraphicsModule(){ // : CommandSection("science"){

	// const drain::Flagger::ivalue_t section = drain::Static::get<GraphicsSection>().index;
	// const drain::bank_section_t IMAGES = drain::Static::get<drain::HiddenSection>().index;

	const drain::bank_section_t HIDDEN = drain::Static::get<drain::HiddenSection>().index;

	install<CmdLinkImage>(); //
	// install<CmdLayout>();  // Could be "CmdMainAlign", but syntax is so different. (HORZ,INCR etc)
	// install<CmdAlign>();
	DRAIN_CMD_INSTALL(Cmd, Align)();
	// DRAIN_CMD_INSTALL(Cmd, Anchor)();
	DRAIN_CMD_INSTALL(Cmd, Layout)();
	linkRelatedCommands(Align, Layout);

	install<CmdFontSizes>();
	//install<CmdGroupTitle>();
	DRAIN_CMD_INSTALL(Cmd, GroupTitle)();
	DRAIN_CMD_INSTALL(Cmd, GroupId)();
	DRAIN_CMD_INSTALL(Cmd, Title)();
	DRAIN_CMD_INSTALL(Cmd, TitleHeights)();
	// install<CmdTitleHeights>();
	linkRelatedCommands(Title, GroupTitle, TitleHeights);

	linkRelatedCommands(Layout, GroupTitle, GroupId);

	install<CmdInclude>();
	// install<CmdGroupTitle>().section = HIDDEN; // under construction
	install<CmdPanel>();
	install<CmdPanelFoo>().section = HIDDEN; // addSection(i);
	install<CmdPanelTest>().section = HIDDEN;  // addSection(i);
	// install<CmdImageTitle>(); consider
	install<CmdStyle>();

	DRAIN_CMD_INSTALL(Cmd, PolarGrid)();
	DRAIN_CMD_INSTALL(Cmd, Sector)();
	linkRelatedCommands(PolarGrid, Sector);


	install<CmdAlignTest>();
	install<CmdDebug>();

};

//Obsolete. See TreeUtilsSVG::superAlignNE
/*
void hyperAlign(drain::image::TreeSVG & group){ //, BBoxSVG & bbox){

	drain::Logger mout(__FILE__, __FUNCTION__);

	//warn("nesting SVG elem not handled");

	typedef drain::image::svg::tag_t tag_t;

	if (!group->typeIs(tag_t::GROUP, tag_t::SVG)){
		return;
	}

	drain::Point2D<svg::coord_t> point(0,0);

	if (group->hasClass(LayoutSVG::STACK_LAYOUT)){ // consider orientation in class? ALIGN_LEFT ALIGN_DOWN

		for (auto & entry: group.getChildren()){

			drain::image::NodeSVG & node = entry.second.data;

			if (node.typeIs(tag_t::GROUP)){
				BBoxSVG & bbox = node.getBoundingBox();
				drain::Point2D<svg::coord_t> translate;
				translate.x = point.x - bbox.x;
				translate.y = point.y - bbox.y;
				//svg::coord_t dx = bbox.x;
				//svg::coord_t dy = bbox.y;
				// transform="translate(0 50)
				node.transform.translate.set(translate);
				// node.set("transform", drain::StringBuilder<>("translate(", translate.x, ' ', translate.y, ")").str());
				point.x += bbox.width;
				point.y += bbox.height;
			}
		}
	}

	for (auto & entry: group.getChildren()){
		//TreeUtilsSVG::superAlignNEW(entry.second, AlignBase::Axis::HORZ, LayoutSVG::Direction::INCR);
		hyperAlign(entry.second);
	}

}
 */

/**
 *
 *   \see Palette::exportSVGLegend()
 */
/*
void CmdOutputPanel::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// mout.attention(ctx.getName());
	// mout.warn("ctx.select=", ctx.select);


	if (ctx.statusFlags.isSet(drain::Status::INPUT_ERROR)){
		mout.warn("input failed, skipping");
		return;
	}

	if (ctx.statusFlags.isSet(drain::Status::DATA_ERROR)){
		mout.warn("data error, skipping");
		return;
	}

	//TreeSVG &  // = svg["bg"];
	drain::image::TreeSVG svg(svg::SVG);
	// TreeSVG svg; // (svg::SVG); REDO this, check copy constr!
	svg->setType(svg::SVG);

	drain::image::TreeSVG & main = svg["main"];
	main->setType(svg::GROUP);
	// main->set("style", "fill:green");
	// main->set("jimbo", 126);
	// main->set("jimboz", true);

	drain::VariableMap & variables = ctx.getStatusMap();
	variables["PREFIX"] = "PANEL";


	// drain::StringMapper basename("${PREFIX}-${NOD}-${what:product}-${what:quantity}");
	// drain::BBox bboxAll;
	drain::BBox bbox;
	drain::Point2D<double> upperLeft(0,0);

	//ctx.updateCurrentImage();
	const drain::image::Image & src = ctx.getCurrentImage();
	appendImage(main, "color", variables, upperLeft, src, bbox);
	mout.attention("prev. BBOX: ", bbox);
	// bboxAll.extend(bbox);

	// variables["what:product"] = "prod";

	const drain::image::Image & src2 = ctx.getCurrentGrayImage();
	upperLeft.set(bbox.upperRight.x, 0);
	appendImage(main, "gray", variables, upperLeft, src2, bbox);
	// bboxAll.extend(bbox);
	mout.attention("prev. BBOX: ", bbox);

	// mout.attention("final BBOX: ", bboxAll);

	svg->set("viewboxFOO", bbox.tuple());
	svg->set("width",  +bbox.getWidth());
	svg->set("height", -bbox.getHeight());

	// svg->set("width",  src.getWidth());
	// svg->set("height", src.getHeight());
	ctx.getCurrentGrayImage();



	if (layout.empty() || layout == "basic"){
		//TreeSVG & radar = image["radar"];
		//radar->set("foo", 123);
	}
	else {
		mout.error("Unknown layout '", layout, "'");
	}

	const std::string s = filename.empty() ? layout+".svg" : filename;

	if (!svg::fileInfo.checkPath(s)){ // .svg
		mout.fail("suspicious extension for SVG file: ", s);
		mout.advice("extensionRegexp: ", svg::fileInfo.extensionRegexp);
	}

	drain::Output ofstr(s);
	mout.note("writing SVG file: '", s, "");
	// ofstr << svg;
	svg::toStream(ofstr, svg);


}
 */

/**
 *

void CmdOutputPanel::appendImage(TreeSVG & group, const std::string & label, drain::VariableMap & variables,
		const drain::Point2D<double> & upperLeft, const drain::image::Image & image, drain::BBox & bbox) const {

	static const drain::StringMapper basename("${outputPrefix}${PREFIX}-${NOD}-${what:product}-${what:quantity}-${LABEL}", "[a-zA-Z0-9:_]+");

	variables["LABEL"] = label;
	std::string fn = basename.toStr(variables,'X') + ".png";

	basename.toStream(std::cout, variables, 0); std::cout << '\n';
	basename.toStream(std::cout, variables, 'X'); std::cout << '\n';
	basename.toStream(std::cout, variables, -1); std::cout << '\n';


	//drain::Point2D<double> upperRight(upperLeft.x + image.getWidth(), upperLeft.y + image.getWidth(), );
	double w = image.getWidth();
	double h = image.getHeight();

	bbox.lowerLeft.x = std::min(bbox.lowerLeft.x,   upperLeft.x);
	bbox.lowerLeft.y = std::max(bbox.lowerLeft.y,   upperLeft.y + h);

	bbox.upperRight.x = std::max(bbox.upperRight.x, upperLeft.x + w);
	bbox.upperRight.y = std::min(bbox.upperRight.y, upperLeft.y);



	drain::image::TreeSVG & imageElem = group[label];
	imageElem->setType(svg::IMAGE);
	imageElem->set("x", upperLeft.x);
	imageElem->set("y", upperLeft.y);
	imageElem->set("width",  w);
	imageElem->set("height", h);
	//imageElem->set("xlink:href", fn);
	imageElem->set("href", fn);
	drain::image::FilePng::write(image, fn);

	drain::image::TreeSVG & title = imageElem["title"];
	title->setType(svg::TITLE);
	title->setText(label + " (experimental) "); //CTXX

	//title->setType(NodeSVG:);
	drain::image::TreeSVG & comment = imageElem["comment"];
	comment->setComment("label:" + label);

	// comment->setType(NodeXML::COMM)

}
 */



} // namespace rack
