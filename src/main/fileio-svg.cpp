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


//  Wrapper. Unused, future option
template <typename E>
class ClassLabelXML : public std::string {
public:

	ClassLabelXML(const E & e): std::string(drain::EnumDict<E>::dict.getKey(e)) {
	}

	template <typename ...T>
	ClassLabelXML(const T... args) : std::string(args...){
	}

};



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

	if (!NodeSVG::fileInfo.checkPath(s)){ // .svg
		mout.fail("suspicious extension for SVG file: ", s);
		mout.advice("extensionRegexp: ", NodeSVG::fileInfo.extensionRegexp);
	}

	drain::Output ofstr(s);
	mout.note("writing SVG file: '", s, "");
	// ofstr << svg;
	NodeSVG::toStream(ofstr, svg);


}
*/

struct GraphicsSection : public drain::CommandSection {

	inline	GraphicsSection(): CommandSection("graphics"){
	};

};

class CmdLinkImage : public drain::SimpleCommand<std::string> {

public:

	CmdLinkImage() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		// drain::image::TreeSVG & group = CmdBaseSVG::getCurrentGroup(ctx);

		// drain::image::TreeSVG & img =
		RackSVG::addImage(ctx, drain::FilePath(this->value), {0,0});

	}

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
			drain::sprinter(drain::EnumDict<AlignBase::Axis>::dict.getKeys(), {"|"}).str() + ':'+
			drain::sprinter(drain::EnumDict<AlignBase::Pos>::dict.getKeys(), {"|"}).str()
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
				/*
				if (drain::EnumDict<AlignSVG::Topol>::setValue(key, align.topol)){
					// ok
				}
				else if (drain::EnumDict<AlignBase::Axis>::setValue(key, align.axis)){
					// ok
				}
				else if (drain::EnumDict<AlignBase::Pos>::setValue(key, align.pos)){
					// ok
				}
				else if (drain::EnumDict<Alignment<> >::setValue(key, (Alignment<> &)align)){ // Dangerous
					// ok
				}
				else {
					mout.error("Unknown alignment key: '", key, "' (all args: ", value, ")");
				}
				*/
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

		drain::Frame2D<double> frame = {400,500};

		drain::image::TreeSVG & group = RackSVG::getCurrentGroup(ctx)[value](NodeSVG::GROUP);
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



		drain::image::TreeSVG & rect = group[ANCHOR_ELEM](NodeSVG::RECT); // +EXT!
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

						drain::image::TreeSVG & text = group[label + "text"](NodeSVG::TEXT);
						text->setId(label+"_T");
						text->getBoundingBox().setArea(60,30);
						text->setAlign(AlOwner::ANCHOR, AlignBase::HORZ, posHorzRef);
						text->setAlign(AlOwner::ANCHOR, AlignBase::VERT, posVertRef);
						text->setAlign(AlOwner::OBJECT, AlignBase::HORZ, posHorz);
						text->setAlign(AlOwner::OBJECT, AlignBase::VERT, posVert);
						text->setText(label);

						drain::image::TreeSVG & textBox = group[label](NodeSVG::RECT);
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
						// drain::image::TreeSVG & textSpan = text["tspan"](NodeSVG::TSPAN);
						// textSpan->setText(text->getAlignStr());

					}
				}
			}
		}


	}

};

class CmdMainTitle : public drain::SimpleCommand<std::string> {

public:

	CmdMainTitle() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "Set main title") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {

		RackContext & ctx = getContext<RackContext>();

		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		drain::image::TreeSVG & headerGroup = RackSVG::getMain(ctx);
		// drain::image::TreeSVG & headerGroup = RackSVG::getCurrentGroup(ctx);

		// TreeSVG & headerGroup = mainGroup["headerGroup"](svg::GROUP);
		// TreeSVG & headerRect = headerGroup["headerRect"](svg::RECT);
		//headerGroup->setAlignAnchor("headerRect2");

		TreeSVG & rectTitle = headerGroup["headerRect"](NodeSVG::RECT); // +EXT!
		// rectTitle->set("name","headerRec_2");
		// rectTitle->set("height", 60);
		rectTitle->setHeight(70);
		rectTitle->setStyle("fill", "gray");
		rectTitle->setStyle("opacity", 0.5);
		rectTitle->setStyle({{"stroke","black"}, {"stroke-dasharray",{2,5}}, {"stroke-width","2px"}});
		rectTitle->setId("textRect2");
		//rectTitle->getBoundingBox().setArea(80,40);
		//rectTitle->setAlign(AlignSVG::INSIDE, AlignSVG::TOP);
		//rectTitle->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);


		rectTitle->setAlign(AlignSVG::Owner::OBJECT, AlignBase::HORZ, AlignBase::Pos::FILL);
		if (ctx.alignVert.topol == AlignSVG::UNDEFINED_TOPOL){
			ctx.alignVert.set(AlignSVG::OUTSIDE);
		}
		if (ctx.alignVert.pos == AlignBase::UNDEFINED_POS){
			ctx.alignVert.set(AlignBase::MAX);
		}
		rectTitle->setAlign(ctx.alignVert, ctx.alignVert.topol); //, AlignSVG::OUTSIDE);
		ctx.alignVert.reset();
		/*
		if (ctx.alignVert.isSet()){
			if (ctx.alignVert.topol){
				rectTitle->setAlign(ctx.alignVert); //, AlignSVG::OUTSIDE);
			}
		}
		rectTitle->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
		*/

		drain::image::TreeSVG & mainTitleText = headerGroup["GENERAL"](NodeSVG::TEXT);
		mainTitleText->setText(value);
		mainTitleText->setAlign(AlignSVG::MIDDLE);
		mainTitleText->setAlign(AlignSVG::CENTER);
		mainTitleText->addClass(LayoutSVG::FLOAT); // could be AlignSVG
		mainTitleText->getBoundingBox().setHeight(16);
		mainTitleText->setStyle("font-size", 20);

		//TreeSVG & timeHeader = group["TIME"](svg::TEXT);
		drain::image::TreeSVG & text1 = headerGroup["TIME"](NodeSVG::TEXT);
		text1->setText(value);
		// text1->setAlign(AlignSVG::LEFT);
		text1->setAlign(AlignSVG::BOTTOM);
		text1->setAlign(AlignSVG::TOP);
		text1->setWidth(8);
		text1->setHeight(20);
		text1->setText(value+"+1");
		text1->addClass(LayoutSVG::FLOAT);

		drain::image::TreeSVG & text2 = headerGroup["LOCATION"](NodeSVG::TEXT);
		text2->setText(value);
		text2->setAlign(AlignSVG::RIGHT);
		text2->setAlign(AlignSVG::TOP);
		text2->setWidth(8);
		text2->setHeight(20);
		text2->setText(value+"+2");
		text2->addClass(LayoutSVG::FLOAT);

	}
};

class CmdPanel : public drain::SimpleCommand<std::string> {

public:

	CmdPanel() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "layout") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {

		// ClassLabelXML<drain::image::AlignSVG> label1(drain::image::AlignSVG::PANEL);
		// ClassLabelXML<drain::image::AlignSVG> label2("PANEL");

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::Frame2D<double> frame = {150,480};

		drain::image::TreeSVG & group = RackSVG::getCurrentGroup(ctx)[value](NodeSVG::GROUP);
		group->setId(value);
		group->addClass(LayoutSVG::FLOAT);

		// rectGroup->addClass(drain::image::LayoutSVG::ALIG NED);
		const std::string ANCHOR_ELEM("main");
		//group->setAlignAnchorVert(MAIN_ELEM);
		group->setAlignAnchorHorz(ANCHOR_ELEM);


		if (ctx.alignHorz.topol != AlignSVG::UNDEFINED_TOPOL){
			group->setAlign(AlignBase::HORZ, ctx.alignHorz.pos, ctx.alignHorz.topol);  // simplify
			mout.unimplemented<LOG_NOTICE>("Set: ", ctx.alignHorz, " -> ", group->getAlignStr());
			//ctx.alignHorz.topol  = AlignSVG::UNDEFINED_TOPOL;
		}
		else {
			group->setAlign(AlignSVG::RIGHT, AlignSVG::OUTSIDE); // AlignSVG::LEFT);
		}

		if (ctx.alignVert.topol != AlignSVG::UNDEFINED_TOPOL){
			group->setAlign(AlignBase::VERT, ctx.alignVert.pos, ctx.alignVert.topol); // simplify
			mout.unimplemented<LOG_NOTICE>("Set: ", ctx.alignVert, " -> ", group->getAlignStr());
			// ctx.alignVert.topol  = AlignSVG::UNDEFINED_TOPOL;
		}
		else {
			group->setAlign(AlignSVG::TOP, AlignSVG::INSIDE); // AlignSVG::BOTTOM);
		}

		drain::image::TreeSVG & rect = group[ANCHOR_ELEM](NodeSVG::RECT); // +EXT!
		rect->set("width", frame.width);
		//rect->set("width", 10); //margin!
		rect->set("height", frame.height);
		rect->set("label", ANCHOR_ELEM);
		rect->setStyle("fill", "yellow");
		rect->setStyle("opacity", 0.5);
		rect->setId("textRect");

		drain::image::TreeSVG & rectTitle = group["title"](NodeSVG::RECT); // +EXT!
		// rectTitle->set("width", 50);
		rectTitle->set("height", 60);
		rectTitle->setStyle("fill", "green");
		rectTitle->setStyle("opacity", 0.5);
		rectTitle->setId("textRect");
		rectTitle->setAlign(AlignSVG::TOP, AlignSVG::INSIDE);
		rectTitle->setAlign(AlignSVG::Owner::OBJECT, AlignBase::HORZ, AlignBase::Pos::FILL);
		//rectTitle->setAlign(AlignSVG::HORZ_FILL);

		drain::image::TreeSVG & rectV = group["title2"](NodeSVG::RECT); // +EXT!
		rectV->set("width", 25);
		rectV->setStyle("fill", "red");
		rectV->setStyle("opacity", 0.5);
		rectV->setId("textV");
		rectV->setAlign(AlignSVG::LEFT, AlignSVG::INSIDE);
		rectV->setAlign(AlignSVG::Owner::OBJECT, AlignBase::VERT, AlignBase::Pos::FILL);

		/*
		drain::image::TreeSVG & textGroup = group["text-group"](NodeSVG::GROUP);
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
			// drain::image::TreeSVG & text = group[s + "_text"](NodeSVG::TEXT);
			drain::image::TreeSVG & text = group.addChild()(NodeSVG::TEXT);
			//text->setId(drain::StringBuilder<'_'>("textline", ++index));
			text->setId("textline", index);
			text->setText(s);
			text->setWidth(10); // Margin + row height! TODO / FIX

			rect->setHeight(3*index);

			// Set horz alignment for every element
			if (horzPos != AlignBase::Pos::UNDEFINED_POS){
				text->setAlign(ctx.alignHorz, AlignSVG::Topol::INSIDE);
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
		mout.reject<LOG_NOTICE>("Main align:", ctx.alignHorz, ", ", ctx.alignVert);

		ctx.alignHorz.topol  = AlignSVG::UNDEFINED_TOPOL;
		ctx.alignVert.topol  = AlignSVG::UNDEFINED_TOPOL;


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

		drain::image::TreeSVG & style = RackSVG::getStyle(ctx);

		style["TH"].data = {{"fill", "black"}, {"stroke","TH..."} };

		// Haaste 1:
		// myRect -> setStyle JOS se on toteutettu myRect.elem["_STYLE"].attributes(...)

		// Haaste 2: (huom. UNDEFINED elem)
		style["TD"] = {};
		style["TD"] = {{"fill", "black"}, {"stroke","TD..."} };
		style["TD"] = "fill:yellow;text-anchor:panchor";

		// FIX: attribuuteista heti seis!
		style = {{"fill", "black"}, {"stroke","STYLE..."} };

		/**  Haaste:
		 *
		 */


		std::string cssSelector, cssConf;
		if (drain::StringTools::split2(value, cssSelector, cssConf, '=')){
			style->setStyle(cssSelector, cssConf+"; setStyle:1");
			style->set(cssSelector, cssConf+"; attrib:1");
			style[cssSelector] = cssConf+"; elemOper:1"; // Assings string to CTEXT !? problem!
			// style[cssSelector].data = {{"koe", "black"}};      // OK but parallel..
			//drain::TreeUtils::dump(ctx.svgTrack);
		}
		else {
			mout.error("Missing '=' in assigment");
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
	install<CmdPanel>(); // .section = HIDDEN; // addSection(i);
	install<CmdPanelTest>().section = HIDDEN; // addSection(i);
	install<CmdMainTitle>();
	install<CmdStyle>();

};



} // namespace rack
