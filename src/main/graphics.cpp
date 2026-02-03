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

#include <drain/util/JavaScript.h>
#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/TreeXML.h>
#include <drain/util/TreeHTML.h>
#include <drain/image/FilePng.h>
#include <drain/image/TreeSVG.h>
#include <drain/image/TreeElemUtilsSVG.h>
#include <drain/image/TreeLayoutSVG.h>
#include <drain/image/TreeUtilsSVG.h>
#include <main/graphics-panel.h>

// #include <js/koe.h>


//#include "radar/PolarSector.h"
//#include "fileio-svg.h"

#include "graphics-panel.h"

#include "graphics-radar.h"
#include "graphics-wireframe.h"

#include "graphics.h"




namespace rack {

/*
//const drain::Range<double> azmRad(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);
template <int D,typename T,size_t N>
inline
void convert(const drain::TupleBase<T,N> &src, drain::TupleBase<T,N> & dst){
	typename drain::TupleBase<T,N>::const_iterator sit = src.begin();
	typename drain::TupleBase<T,N>::const_iterator dit = dst.begin();
	while (sit != src.end()){
		*dit = D*(*sit);
		++sit;
		++dit;
	}
}
*/

struct GraphicsSection : public drain::CommandSection {

	inline	GraphicsSection(): CommandSection("graphics"){

		// const char* koe = R"(JS Mika
		/*
		std::vector<double> v(33*33, 1.2345); // = {0.01, 1.0, 1212122.0};

		std::string code;
		drain::JavaScript::createArray(drain::Outlet(code), "i32", drain::JavaScript::Int32, v);
		std::cerr << code << std::endl;
		std::cerr << koe << std::endl;

		drain::super_test();
		*/
	};

};





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
		const std::string ANCHOR_ELEM("myRect"); // not RackSVG::MAIN
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
			// style[cssSelector]->getAttributes().setValues(cssConf, ':', ';');
			mout.pending<LOG_WARNING>("New style str: ", cssSelector, " = ", cssConf);

			drain::XML::map_t m;
			/*
			m.setValues(cssConf, ':', ';'); // FIX map assignment error here
			mout.accept<LOG_WARNING>("Own m: ", m);

			m.clear();
			m.importEntries<true>(cssConf, ':', ';');
			mout.accept<LOG_WARNING>("IMP m: ", m);
			*/
			m.clear();
			drain::MapTools::setValues(m, cssConf, ';', ':');
			mout.accept<LOG_WARNING>("Now m: ", drain::sprinter(m));

			style[cssSelector](svg::STYLE_SELECT);
			//style[cssSelector]->getAttributes().setValues(cssConf, ':', ';');
			//style[cssSelector]->set();
			drain::MapTools::setValues(style[cssSelector]->getMap(), cssConf, ';', ':');

			mout.attention("New style: ", cssSelector, " <- ", drain::sprinter(style[cssSelector]->getAttributes()));

			//style[cssSelector]->setStyle(cssConf); // TODO fix -> ->getAttributes().setValues(cssConf, ':', ';');

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

		ConfSVG & conf = ctx.svgPanelConf;

		drain::Frame2D<double> frame = {160,300};

		drain::image::TreeSVG & group = RackSVG::getCurrentAlignedGroup(ctx)[RackSVG::SIDE_PANEL](svg::GROUP);
		group->setId(value);
		group->addClass(RackSVG::SIDE_PANEL);
		//group->setAlignAnchor(RackSVG::BACKGROUND_RECT);

		RackSVG::consumeAlignRequest(ctx, group);


		drain::image::TreeSVG & rect = group[RackSVG::BACKGROUND_RECT](svg::RECT); // +EXT!
		rect->addClass(RackSVG::ElemClass::SIDE_PANEL);
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
			rect2->addClass(RackSVG::ElemClass::SIDE_PANEL);
			rect2->setMyAlignAnchor(RackSVG::BACKGROUND_RECT);
			rect2->setAlign(AlignSVG::TOP, AlignSVG::HORZ_FILL);
			rect2->setHeight(120);

			drain::image::TreeSVG & text = group.addChild()(svg::TEXT);
			text->setMyAlignAnchor(HEADER_RECT);
			text->setAlign(AlignSVG::TOP, AlignSVG::CENTER);
			text->addClass(RackSVG::ElemClass::SIDE_PANEL);
			text->setFontSize(conf.fontSizes[1], conf.boxHeights[1]);
			text->setStyle("fill", "lightblue");
			text->setText(status["PLC"]);

			drain::image::TreeSVG & text2 = group.addChild()(svg::TEXT);
			text2->setAlign(AlignSVG::BOTTOM, AlignSVG::OUTSIDE);
			text2->setAlign(AlignSVG::CENTER);
			text2->addClass(RackSVG::ElemClass::SIDE_PANEL);
			text2->setFontSize(conf.fontSizes[0], conf.boxHeights[0]);
			text2->setText(status["NOD"]);

			std::stringstream sstr;

			drain::image::TreeSVG & date = group["date"](svg::TEXT); //addTextElem(group, "date");
			date->setMyAlignAnchor(HEADER_RECT);
			// date->setAlign(AlignSVG::MIDDLE, AlignSVG::RIGHT); // CENTER);
			date->setAlign(AlignSVG::BOTTOM, AlignSVG::INSIDE);
			date->setAlign(AlignSVG::CENTER);
			date->addClass(RackSVG::ElemClass::TIME);
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
				tkey->addClass(RackSVG::ElemClass::SIDE_PANEL);
				tkey->setFontSize(conf.fontSizes[2], conf.boxHeights[1]);
				tkey->setStyle("fill", "darkgray");
				tkey->setText(key);

				drain::image::TreeSVG & tval = group[tkey->getId()+"Value"](svg::TEXT);
				tval->setMyAlignAnchor<AlignBase::HORZ>(RackSVG::BACKGROUND_RECT);
				tval->setAlign(AlignSVG::RIGHT, AlignSVG::INSIDE);
				tval->setMyAlignAnchor<AlignBase::VERT>(key);
				tval->setAlign(AlignSVG::BOTTOM, AlignSVG::INSIDE);
				tval->addClass(RackSVG::ElemClass::SIDE_PANEL);
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
		// t->addClass(RackSVG::ElemClass::SIDE_PANEL);
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

/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
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

	NodeSVG::Elem<tag_t::RECT> rect(subGroup["rectangle"]);
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
		TreeLayoutSVG::addStackLayout(ctx.svgTrack, ctx.mainOrientation, ctx.mainDirection); // AlignBase::Axis::HORZ, LayoutSVG::Direction::INCR);
		mout.attention("Aligning: ", ctx.svgTrack.data);
		TreeLayoutSVG::superAlign(ctx.svgTrack); //, AlignBase::Axis::HORZ, LayoutSVG::Direction::INCR);

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

/*
class CmdLogo : public drain::BasicCommand { // drain::SimpleCommand<std::string> {

public:

	CmdLogo() : drain::BasicCommand(__FUNCTION__, "SVG test product") {
		getParameters().link("name",   name, "Name (label) of the logo");
		getParameters().link("variant",  variant, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::EnumDict<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
	}

	CmdLogo(const CmdLogo & cmd) : drain::BasicCommand(cmd) {
		getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	std::string variant = "black";
	std::string name  = "fmi-fi";


	void exec() const override {

		using namespace drain::image;

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

	}

};
*/

class CmdCoords : public drain::BasicCommand, CmdPolarBase { // drain::SimpleCommand<std::string> {

public:

	CmdCoords() : drain::BasicCommand(__FUNCTION__, "SVG test product") {
		// getParameters().link("name",   name, "label");
		// getParameters().link("panel",  panel, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::EnumDict<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
	}

	CmdCoords(const CmdCoords & cmd) : drain::BasicCommand(cmd) {
		// getParameters().copyStruct(cmd.getParameters(), cmd, *this);
	}

	void exec() const override {

		using namespace drain::image;
		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

		RadarSVG radarSVG;

		// drain::image::TreeSVG & overlayGroup = prepareGeoGroup(ctx, radarSVG);

		// const drain::Rectangle<double> &bbox = radarSVG.geoFrame.getBoundingBoxNat();
		//const drain::Point2D<double> & UR = bbox.upperRight;

		const int width  = radarSVG.geoFrame.getFrameWidth();
		const int height = radarSVG.geoFrame.getFrameHeight();

		drain::Point2D<double> UL,LL, UR, LR;

		/*
		radarSVG.geoFrame.pix2LLdeg(drain::Point2D<int>(0,0), UL);
		radarSVG.geoFrame.pix2deg(drain::Point2D<int>(width,0), UR);
		radarSVG.geoFrame.pix2deg(drain::Point2D<int>(0,height), LL);
		radarSVG.geoFrame.pix2deg(drain::Point2D<int>(width,height), LR);
		const drain::Point2D<double> & LL = bbox.lowerLeft;
		*/

		/*
		radarSVG.geoFrame.pix2LLdeg(0,-1,           UL.x, UL.y);
		radarSVG.geoFrame.pix2LLdeg(width, -1,      UR.x, UR.y);
		radarSVG.geoFrame.pix2LLdeg(0,height-1,     LL.x, LL.y);
		radarSVG.geoFrame.pix2LLdeg(width,height-1, LR.x, LR.y);
		*/


		/*
		const double lonSpan = UR.x - LL.x;
		const double latSpan = UR.y - LL.y; // kind of negative
		double cw = 1.0/static_cast<double>(width);
		double ch = 1.0/static_cast<double>(height);
		drain::Point2D<int> point;
		int & i = point.x;
		int & j = point.y;
		*/

		drain::image::ImageT<unsigned char> coords(width,height,3);
		drain::image::Channel & lonChannel = coords.getChannel(0);
		drain::image::Channel & latChannel = coords.getChannel(1);

		double lon, lat;

		drain::Range<double> latRange(90,-90);
		drain::Range<double> lonRange(90,-90);

		for (int j = 0; j < height; ++j) {
			radarSVG.geoFrame.pix2LLdeg(0,j-1, lon,lat);
			lonRange.insert(lon);
			latRange.insert(lat);
			radarSVG.geoFrame.pix2LLdeg(width,j-1, lon,lat);
			lonRange.insert(lon);
			latRange.insert(lat);
		}

		for (int i = 0; i < width; ++i) {
			radarSVG.geoFrame.pix2LLdeg(i,-1, lon,lat);
			lonRange.insert(lon);
			latRange.insert(lat);
			radarSVG.geoFrame.pix2LLdeg(i,height-1, lon,lat);
			lonRange.insert(lon);
			latRange.insert(lat);
		}


		const double maxValue = drain::Type::call<drain::typeMax, double>(coords.getType());
		const double wLon = maxValue / lonRange.width();
		const double wLat = maxValue / latRange.width();

		mout.attention(DRAIN_LOG(lonRange), " resolution: ", 1.0/wLon, " deg");
		mout.attention(DRAIN_LOG(latRange), " resolution: ", 1.0/wLat, " deg");


		size_t address;

		for (int j = 0; j < height; ++j) {
			// WY = ch*static_cast<double>(j);
			// wy = 1.0-WY;
			for (int i = 0; i < width; ++i) {
				// WX = cw*static_cast<double>(i);
				// wx = (1.0-WX);
				radarSVG.geoFrame.pix2LLdeg(i,j-1, lon,lat);
				address = coords.address(i,j);
				lonChannel.put(address, wLon * (lon-lonRange.min));
				latChannel.put(address, wLat * (lat-latRange.min));
			}
		}

		// const std::string & clsNameBase = getName();

		drain::image::FilePng::write(coords, "coords.png");
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

	DRAIN_CMD_INSTALL(Cmd, RadarDot)();
	DRAIN_CMD_INSTALL(Cmd, RadarGrid)();
	DRAIN_CMD_INSTALL(Cmd, RadarSector)();
	DRAIN_CMD_INSTALL(Cmd, RadarLabel)();
	// DRAIN_CMD_INSTALL(Cmd, RadarMarker)();
	DRAIN_CMD_INSTALL(Cmd, RadarRay)();
	linkRelatedCommands(RadarDot, RadarGrid, RadarRay, RadarSector, RadarLabel); // RadarMarker,


	install<CmdAlignTest>();
	install<CmdDebug>();
	//install<CmdLogo>().section = HIDDEN;
	install<CmdCoords>().section = HIDDEN;
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

//DRAIN_ENUM_DICT(CmdRadarMarker::MARKER);
// drain::EnumDict<orientation_enum>::dict
// template <>
