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

//#include <drain/util/Convert.h>
#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeXML.h>
#include <drain/util/TreeHTML.h>

#include <drain/image/FilePng.h>
#include <drain/image/TreeUtilsSVG.h>

//#include "data/SourceODIM.h" // for NOD

#include "fileio-svg.h"
#include "graphics.h"


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
class CmdAlign : public drain::SimpleCommand<std::string> {

public:

	// Like INSIDE:RIGHT or RIGHT:OUTSIDE

	// drain::StringBuilder<':'>(topol_enum::dict.getKeys())
	CmdAlign() : drain::SimpleCommand<std::string>(__FUNCTION__, "Alignment of the next element", "topology", "",
			drain::sprinter(drain::EnumDict<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
			// drain::sprinter(drain::EnumDict<AlignBase::Axis>::dict.getKeys(), {"|"}).str() + ':'+
			drain::sprinter(drain::EnumDict<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str() + ',' +
			drain::sprinter(drain::EnumDict<AlignSVG::Topol>::dict.getKeys(), {"|"}).str() + ':' +
			drain::sprinter(drain::EnumDict<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str()
			// drain::sprinter(drain::EnumDict<AlignBase::Pos>::dict.getKeys(), {"|"}).str()
	){
		//getParameters().link("", x, drain::StringBuilder<':'>());
	}

	virtual
	void exec() const override {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		std::list<std::string> args;
		drain::StringTools::split(value, args, ',');

		for (const std::string & arg: args){

			std::list<std::string> keys;
			drain::StringTools::split(arg, keys, ':');

			CompleteAlignment<> align(AlignSVG::Topol::UNDEFINED_TOPOL, AlignBase::Pos::UNDEFINED_POS); //(AlignSVG::Topol::UNDEFINED_TOPOL, AlignBase::Axis::UNDEFINED_AXIS, AlignBase::Pos::UNDEFINED_POS);

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

			//mout.accept<LOG_NOTICE>(align.topol, '/',(Align &)align);
			// ctx.topol = align.topol;

		}

		// mout.accept<LOG_NOTICE>(ctx.alignHorz.topol, '/',(AlignBase &)ctx.alignHorz);
		// mout.accept<LOG_NOTICE>(ctx.alignVert.topol, '/',(AlignBase &)ctx.alignVert);

	}

protected:


};

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

	void exec() const {
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
		drain::FilePath filePath(ctx.getFormattedStatus(this->value));
		mout.note("linking: ", filePath);
		RackSVG::addImage(ctx, frame, filePath); // , drain::StringBuilder<>(LayoutSVG::GroupType::FLOAT));
		// EnumDict<LayoutSVG::GroupType>::dict

	}

};



class CmdGroupTitle : public drain::BasicCommand {

public:

	CmdGroupTitle() : drain::BasicCommand(__FUNCTION__, "Set titles automatically") {
		RackContext & ctx = getContext<RackContext>();
		getParameters().separator = 0;
		getParameters().link("syntax", ctx.svgPanelConf.groupTitleSyntax, "example: '${what:date|%Y%m} ${NOD}'");
	}

	CmdGroupTitle(const CmdGroupTitle & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this, drain::ReferenceMap::LINK);
	}

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		drain::StringTools::replace(ctx.svgPanelConf.groupTitleSyntax, '/', '-', ctx.svgPanelConf.groupTitleSyntax);
		mout.accept<LOG_WARNING>("new value: ", ctx.svgPanelConf.groupTitleSyntax);
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

	void exec() const {
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

	void exec() const {
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

	void exec() const {

		// ClassLabelXML<drain::image::AlignSVG> label1(drain::image::AlignSVG::PANEL);
		// ClassLabelXML<drain::image::AlignSVG> label2("PANEL");

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::Frame2D<double> frame = {150,480};

		// drain::image::TreeSVG & group = ctx.getCurrentAlignedGroup()[value](svg::GROUP); // RackSVG::getCurrentAlignedGroup(ctx)[value](svg::GROUP);
		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[value](svg::GROUP);
		group->setId(value);
		group->addClass(LayoutSVG::FLOAT);

		// rectGroup->addClass(drain::image::LayoutSVG::ALIG NED);
		const std::string ANCHOR_ELEM("myRect"); // not PanelConfSVG::MAIN
		group->setAlignAnchorHorz(ANCHOR_ELEM);

		RackSVG::applyAlignment(ctx, group);

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

		/*
		std::string arg(value);
		drain::StringTools::replace({{"\\n", "\n"}}, arg);
		std::list<std::string> args;
		drain::StringTools::split(arg, args, '\n');
		*/

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

	void exec() const {

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



class CmdPanel : public drain::SimpleCommand<std::string> {

public:

	CmdPanel() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "layout") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::Frame2D<double> frame = {120,300};

		//drain::image::TreeSVG & group = ctx.getCurrentAlignedGroup()[value](svg::GROUP);
		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[PanelConfSVG::SIDE_PANEL](svg::GROUP);
		group->setId(value);

		//group->setAlignAnchor(RackSVG::BACKGROUND_RECT);
		RackSVG::applyAlignment(ctx, group);


		drain::image::TreeSVG & rect = group[RackSVG::BACKGROUND_RECT](svg::RECT); // +EXT!
		rect->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		rect->setWidth(frame.width);
		rect->setHeight(frame.height);
		// rect->setStyle("fill", "red");
		/*
		 does not work over
		rect->setAlignAnchorVert("*");
		rect->setAlign(AlignSVG::VERT_FILL);
		*/
		// rect->setAlign(AlignSVG::VERT_FILL);

		static std::string HEADER_RECT("headerRect");

		drain::image::TreeSVG & rect2 = group[HEADER_RECT](svg::RECT); // +EXT!
		rect2->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		rect2->setAlignAnchor(RackSVG::BACKGROUND_RECT);
		rect2->setAlign(AlignSVG::TOP, AlignSVG::HORZ_FILL);
		rect2->setHeight(120);

		drain::VariableMap & status = ctx.getStatusMap();

		drain::image::TreeSVG & text = group.addChild()(svg::TEXT);
		text->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		text->setAlignAnchor(HEADER_RECT);
		//text->setAlignAnchor(RackSVG::BACKGROUND_RECT);
		text->setAlign(AlignSVG::TOP, AlignSVG::CENTER);
		text->setFontSize(25.0);
		text->setText("TOP:", status["NOD"]);

		drain::image::TreeSVG & text2 = group.addChild()(svg::TEXT);
		text2->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		text2->setAlignAnchor(HEADER_RECT);
		text2->setFontSize(15.0);
		text2->setAlign(AlignSVG::BOTTOM, AlignSVG::CENTER);
		text2->setText("BOTTOM:", status["PLC"]);

		drain::image::TreeSVG & text3 = group.addChild()(svg::TEXT);
		text3->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
		text3->setAlignAnchor(HEADER_RECT);
		text3->setFontSize(20.0);
		text3->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT);
		text3->setText("MIDDLE:", status["PLC"]);

		VariableFormatterODIM<std::string> formatter;

		std::string anchorVert(HEADER_RECT);

		for (const auto entry: {"what:time", "what:date", "what:object"}){

			if (status.hasKey(entry)){
				drain::image::TreeSVG & t = group[entry](svg::TEXT);
				t->addClass(PanelConfSVG::ElemClass::SIDE_PANEL);
				t->setFontSize(10);
				if (!anchorVert.empty()){
					t->setAlignAnchorVert(anchorVert);
					anchorVert.clear();
				}
				t->setAlignAnchorHorz(RackSVG::BACKGROUND_RECT);
				t->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
				t->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);

				const std::string & format = RackSVG::guessFormat(entry);
				if (!format.empty()){
					std::stringstream sstr;
					formatter.formatVariable(entry, status[entry], format, sstr);
					t->ctext = sstr.str();
				}
				else {
					t->setText(status[entry]);
				}
				//t->setText(entry, ":", status[entry]);

			}
		}


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
		group->setAlignAnchor(ANCHOR_ELEM);
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

		// rect->addClass(LayoutSVG::FLOAT);

		// rect->setAlign(AlignSVG::OBJECT, AlignBase::HORZ,  AlignBase::MAX);
		// rect->setAlign<AlignSVG::OUTSIDE>(AlignSVG::OBJECT, AlignBase::HORZ,  AlignBase::MAX);
		// rect->setAlign<AlignSVG::OUTSIDE>(AlignSVG::RIGHT);
		// rect["basename"](drain::image::svg::TITLE) = "test";
		/*
		rect->setAlign(AlignSVG2::ORIG, AlignSVG2::HORZ,  AlignSVG2::MID);
		rect->setAlign(AlignSVG2::REF, AlignSVG2::VERT,  AlignSVG2::MAX);
		rect->setAlign(AlignSVG2::REF, AlignSVG2::HORZ,  AlignSVG2::MIN);
		*/

		typedef drain::image::AlignSVG::Owner   AlOwner;
		typedef drain::image::AlignBase::Pos   Pos;

		const drain::EnumDict<Pos>::dict_t & dict = drain::EnumDict<Pos>::dict;


		//const std::list<Pos> pos = {AlignBase::MAX, AlignBase::MIN, AlignBase::MID};
		// for (const drain::image::LayoutSVG::Axis & ax: {AlignAxis::HORZ, AlignAxis::VERT}){
		for (const Pos & posVert: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ //pos){ // {AlignBase::MID} pos

			char pv = dict.getKey(posVert)[2];

			for (const Pos & posHorz: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ // pos

				char ph = dict.getKey(posHorz)[2];

				for (const Pos & posVertRef: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ // {AlignBase::MID}){

					char rv = dict.getKey(posVertRef)[2];

					for (const Pos & posHorzRef: {AlignBase::MIN, AlignBase::MID, AlignBase::MAX}){ //pos){

						char rh = dict.getKey(posHorzRef)[2];

						//const std::string label = drain::StringBuilder<'-'>(posHorzRef, posVertRef, posHorz, posVert, '-', ph, pv, rh, rv);
						const std::string label = drain::StringBuilder<'-'>(ph, pv, rh, rv);

						drain::image::TreeSVG & text = group[label + "text"](svg::TEXT);
						text->setId(label+"_T");
						text->getBoundingBox().setArea(60,30);
						text->setAlign(AlOwner::ANCHOR, AlignBase::HORZ, posHorzRef);
						text->setAlign(AlOwner::ANCHOR, AlignBase::VERT, posVertRef);
						text->setAlign(AlOwner::OBJECT, AlignBase::HORZ, posHorz);
						text->setAlign(AlOwner::OBJECT, AlignBase::VERT, posVert);
						text->setText(label);

						drain::image::TreeSVG & textBox = group[label](svg::RECT);
						textBox->setId(label+"_R");
						textBox->getBoundingBox().setArea(60,30);
						//textBox->set("mika", textBox->getAlignStr()); // textBox->set("mika", textBox->getAlignStr());
						textBox->setStyle("fill", "green");
						textBox->setStyle("opacity", 0.15);
						textBox->setStyle("stroke-width", "2px");
						textBox->setStyle("stroke", "black");
						textBox->setAlign(AlOwner::ANCHOR, AlignBase::HORZ, posHorzRef);
						textBox->setAlign(AlOwner::ANCHOR, AlignBase::VERT, posVertRef);
						textBox->setAlign(AlOwner::OBJECT, AlignBase::HORZ, posHorz);
						textBox->setAlign(AlOwner::OBJECT, AlignBase::VERT, posVert);
						//textBox->addClass(LayoutSVG::FLOAT);

						//text->addClass(LayoutSVG::FLOAT);
						// drain::image::TreeSVG & textSpan = text["tspan"](svg::TSPAN);
						// textSpan->setText(text->getAlignStr());

					}
				}
			}
		}


	}

};


GraphicsModule::GraphicsModule(){ // : CommandSection("science"){

	// const drain::Flagger::ivalue_t section = drain::Static::get<GraphicsSection>().index;

	// const ScienceModule & mod = drain::Static::get<ScienceModule>();

	// drain::CommandBank & cmdBank = drain::getCommandBank();

	// drain::BeanRefCommand<FreezingLevel> freezingLevel(RainRateOp::freezingLevel);
	// cmdBank.addExternal(freezingLevel).section = section;
	// const drain::bank_section_t IMAGES = drain::Static::get<drain::HiddenSection>().index;
	const drain::bank_section_t HIDDEN = drain::Static::get<drain::HiddenSection>().index;

	install<CmdLinkImage>(); //
	install<CmdLayout>();  // Could be "CmdMainAlign", but syntax is so different. (HORZ,INCR etc)
	install<CmdAlign>();
	install<CmdFontSizes>();
	install<CmdGroupTitle>();
	install<CmdInclude>();
	install<CmdTitle>();
	// install<CmdGroupTitle>().section = HIDDEN; // under construction
	install<CmdPanel>();
	install<CmdPanelFoo>().section = HIDDEN; // addSection(i);
	install<CmdPanelTest>().section = HIDDEN;  // addSection(i);
	// install<CmdImageTitle>(); consider
	install<CmdStyle>();
	install<CmdTitleHeights>();

};



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
	title->ctext = label + " (experimental) ";

	//title->setType(NodeSVG:);
	drain::image::TreeSVG & comment = imageElem["comment"];
	comment->setComment("label:" + label);

	// comment->setType(NodeXML::COMM)

}
*/



} // namespace rack
