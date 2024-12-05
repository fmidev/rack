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
//#include <stddef.h>

#include <string>

#include <drain/prog/Command.h>
#include <drain/prog/CommandInstaller.h>
#include <drain/prog/CommandBank.h>

#include "resources.h"

#include "fileio-svg.h"

#include "graphics.h"


namespace rack {

//typedef drain::image::AlignSVG alignSvg;

int MetaDataPrunerSVG::visitPrefix(TreeSVG & tree, const TreeSVG::path_t & path){
	// std::cerr << __FUNCTION__ << ':' << path << std::endl;
	return 0;
}



int MetaDataPrunerSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & current = tree(path);

	//if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){ // At least METADATA must be skipped...
	if (current->getType() != svg::GROUP){ // At least METADATA must be skipped...
		return 0;
	}

	/// Statistics: computer count for each (key,value> pair.
	typedef std::map<std::string, unsigned short> variableStat_t;
	variableStat_t stat;
	/// Number of children having (any) metadata.
	int count = 0;

	/// Iterate children and their attributes: check which attributes (key and value) are shared by all the children.
	for (auto & entry: current.getChildren()){
		TreeSVG & child = entry.second;
		if (child.hasChild("metadata") && !child->hasClass("legend")){ // or has "data"?
			++count;
			TreeSVG & childMetadata = entry.second["metadata"](svg::METADATA);
			for (const auto & attr: childMetadata->getAttributes()){
				// tehty jo metadata->set(attr.first, attr.second);
				std::string s = drain::StringBuilder<>(attr.first,'=',attr.second);
				++stat[s];
			}
		}
	}


	if (count > 0){

		TreeSVG & metadata = current["metadata"](svg::METADATA);
		metadata->addClass("md_shared");

		TreeSVG & debugSharedBase = current["shared"](svg::DESC);
		debugSharedBase->set("type", "SHARED");
		// TreeSVG & debugShared = debugSharedBase["cmt"](svg::COMMENT);
		// debugShared->ctext = "SHARED: ";

		if (mout.isLevel(LOG_DEBUG)){
			TreeSVG & debugAll = current["description"](svg::DESC);
			debugAll->set("COUNT", count);
			debugAll->ctext = "All";
		}

		/*
		TreeSVG & debugExplicit = current["rejected"](svg::DESC);
		debugExplicit->addClass("EXPLICIT");
		debugAll->ctext += drain::sprinter(stat).str();
		*/

		// metadata->getAttributes().clear();
		mout.pending<LOG_DEBUG>("pruning: ", drain::sprinter(stat), path.str());

		for (const auto & e: stat){

			mout.pending<LOG_DEBUG>('\t', e.first, ':', e.second);

			// std::cerr << "\t vector " << e.first << ' ' << e.second << std::endl;
			std::string key, value;
			drain::StringTools::split2(e.first, key, value, '=');
			if (e.second == count){

				mout.accept<LOG_DEBUG>('\t', e.first, ' ', path.str());

				debugSharedBase->ctext += ' ';
				debugSharedBase->ctext += e.first;
				// debugShared->set(key, value);

				metadata->set(key, value); // NOTE: becoming strings (consider type dict?)

				for (auto & entry: current.getChildren()){
					TreeSVG & child = entry.second;
					if (child.hasChild("metadata")){
						TreeSVG & childMetadata = entry.second["metadata"](svg::METADATA);
						childMetadata -> remove(key);
						childMetadata -> addClass("md_pruned");
					}
				}

			}
			else {
				mout.reject<LOG_DEBUG>('\t', e.first, ' ', path.str());
				// debugExplicit->ctext += e.first;
			}
		}

	}

	return 0;

}


/**
 *  \param frame - IMAGE or RECT inside which the text will be aligned
 */
TreeSVG & getTextElem(const TreeSVG & frame, TreeSVG & current, const std::string key){


	/*
	std::string name = current->get("name", "unknown-image");
	if (timeClass.has(key)){
		name += "_TIME";
	}
	if (locationClass.has(key)){
		name += "_LOC";
	}
	*/


	TreeSVG & text = current[key+"_title"];

	// Temporary (until aligned)
	const int x = frame->get("x", 0);
	const int y = frame->get("y", 0);

	if (text -> isUndefined()){
		text -> setType(svg::TEXT);
		//text->set("ref", current["image"]->getId());
		//text->set("ref", 0);
		text->set("x", x + 2);
		text->set("y", y + 20);
		// drain::image::TreeUtilsSVG::markAligned(frame, text);
		// text->addClass(svgAlign::FLOAT); // "imageTitle" !
		// text->set("ref", frame->getId());
	}

	// TODO: align conf for TIME and LOCATION from svgConf
	/*
	if (timeClass.has(key)){
		text->addClass("TIME",  "BOTTOM", CmdBaseSVG::LEFT); // CmdBaseSVG::FLOAT,
		text->set("y", y + 40); // temporary
	}

	if (locationClass.has(key)){
		text->addClass("LOCATION", "TOP", "RIGHT"); // CmdBaseSVG::FLOAT,
		text->set("y", y + 60); // temporary
	}
	*/

	return text;
}



int TitleCreatorSVG::visitPostfix(TreeSVG & tree, const TreeSVG::path_t & path){


	TreeSVG & current = tree(path);

	if (current->getType() != svg::GROUP){ // At least METADATA must be skipped...
		return 0;
	}

	/*
	if (!((current->getType()==svg::GROUP) || (current->getType()==svg::IMAGE))){
		return 0;
	}
	*/


	if (!current.hasChild("metadata")){
		return 0;
	}

	drain::Logger mout(__FILE__, __FUNCTION__);


	TreeSVG & metadata = current["metadata"];


	if (!metadata->getAttributes().empty()){

		if (!current->classList.has(RackSVG::IMAGE_FRAME)){
			TreeSVG & desc = current["debug"](svg::DESC);
			desc->ctext = drain::sprinter(metadata->getAttributes(), drain::Sprinter::plainLayout).str();
			desc->addClass("md_debug_noimg");
			return 0;
		}


		/*
		TreeSVG & text = current["bottom-right"](svg::TEXT);
		text->addClass("FLOAT BOTTOM imageTitle debug");
		text->setText(metadata->getAttributes());
		*/
		VariableFormatterODIM<std::string> formatter; // (No instance properties used, but inheritance/overriding)

		// mout.attention("handle: ", current.data);

		// Note: these are "subtitles", not the main title

		for (const auto & attr: metadata->getAttributes()){
			// consider str replace

			const bool IS_TIME     = (ODIM::timeKeys.count(attr.first)>0) || (ODIM::dateKeys.count(attr.first)>0);
			const bool IS_LOCATION =  ODIM::locationKeys.count(attr.first)>0;

			std::string key("title");
			if (IS_TIME){
				key += "-time";
			}

			if (IS_LOCATION){
				key += "-location";
			}

			TreeSVG & text = getTextElem(current["image"], current, key);
			// Drop function call and embed directly
			// tsvg::markAligned(frame, text);
			text->addClass("imageTitle");

			if (IS_TIME){
				text->addClass(RackSVG::TIME); // ,  alignSvg::BOTTOM, alignSvg::LEFT); // CmdBaseSVG::FLOAT,
				text->setAlignInside(LayoutSVG::Axis::VERT, AlignSVG::MAX); // = BOTTOM
				text->setAlignInside(LayoutSVG::Axis::HORZ, AlignSVG::MIN); // = LEFT
				// text->set("y", y + 40); // temporary
				/* TODO:
				std::stringstream sstr;
				formatVariable2(attr.second, ":2:3", sstr);
				text->ctext = sstr.str();
				*/
			}

			if (IS_LOCATION){
				text->addClass(RackSVG::LOCATION); // , alignSvg::TOP, alignSvg::RIGHT); // CmdBaseSVG::FLOAT,
				text->setAlignInside(LayoutSVG::Axis::VERT, AlignSVG::MAX); // = BOTTOM
				text->setAlignInside(LayoutSVG::Axis::HORZ, AlignSVG::MAX); // = RIGHT

				// text->set("y", y + 60); // temporary
			}


			TreeSVG & tspan = text[attr.first](svg::TSPAN);
			tspan->addClass(attr.first); // allows user-specified style
			std::string v, format;
			drain::StringTools::split2(attr.second.toStr(), v, format, '|');

			// mout.attention("handle: ", attr.first, " ", v, " + ", format);

			if (format.empty()){
				tspan->ctext = v;
			}
			else {
				//mout.attention("handle: ", attr.first, " ", v, " + ", format);
				std::stringstream sstr;
				//VariableFormatterODIM<NodeSVG::map_t::value_t>::formatVariable(attr.first, v, format, sstr);
				formatter.formatVariable(attr.first, v, format, sstr);
				// drain::VariableFormatter<NodeSVG::map_t::value_t>::formatValue(v, format, sstr);
				tspan->ctext = sstr.str();
				tspan->ctext += "?";
			}
			//tspan->ctext = attr.second.toStr();
			tspan->ctext += "&#160;"; //'_';
			//entry.second->set(key, "*"); // TODO: remove attribute
		}
	}
	else {
		mout.debug("title skipped, metadata empty under: ", path);
		// TreeSVG & text = current["mark"](svg::TEXT);
		// text->setText(path.str() + " empty metadata...");
		return 0;
	}

	return 0;

}



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

template <typename E>
class ClassLabelXML : public std::string {
public:

	ClassLabelXML(const E & e): std::string(drain::EnumDict<E>::dict.getKey(e)) {
	}


	template <typename ...T>
	ClassLabelXML(const T... args) : std::string(args...){
	}



};


class CmdSuperPanel : public drain::SimpleCommand<std::string> {

public:

	CmdSuperPanel() : drain::SimpleCommand<std::string>(__FUNCTION__, "SVG test product", "layout") {
		//getParameters().link("level", level = 5);
	}

	void exec() const {

		// ClassLabelXML<drain::image::AlignSVG> label1(drain::image::AlignSVG::PANEL);
		// ClassLabelXML<drain::image::AlignSVG> label2("PANEL");

		RackContext & ctx = getContext<RackContext>();
		drain::Logger mout(ctx.log, __FUNCTION__, getName());

		drain::Frame2D<double> frame = {200,400};

		drain::image::TreeSVG & rectGroup = RackSVG::getCurrentGroup(ctx)[value](NodeSVG::GROUP);
		// rectGroup->addClass(drain::image::AlignSVG::PANEL); // needed?
		const std::string ANCHOR1("rekku");
		rectGroup->setAlignAnchor(ANCHOR1);

		drain::image::TreeSVG & rect = rectGroup[ANCHOR1](NodeSVG::RECT); // +EXT!
		rect->set("width", frame.width);
		rect->set("height", frame.height);
		rect->set("label", ANCHOR1);
		// rect->addClass(AlignSVG::ANCHOR); // maybe good olf
		//image->addClass(CmdBaseSVG::FLOAT);
		rect->setStyle("fill", "red");
		// rect["basename"](drain::image::svg::TITLE) = "test";

		/*
		rect->setAlign(AlignSVG2::ORIG, AlignSVG2::HORZ,  AlignSVG2::MID);
		rect->setAlign(AlignSVG2::REF, AlignSVG2::VERT,  AlignSVG2::MAX);
		rect->setAlign(AlignSVG2::REF, AlignSVG2::HORZ,  AlignSVG2::MIN);
		*/

		//drain::image::TreeSVG & textGroup = rect["labelX"](NodeSVG::GROUP);
		//const std::string ANCHOR = "rect";


		typedef drain::image::LayoutSVG::Axis  AlignAxis;
		typedef drain::image::AlignSVG::Owner   AlignPos;
		typedef drain::image::AlignSVG::value_t Align;

		for (const drain::image::LayoutSVG::Axis & ax: {AlignAxis::HORZ, AlignAxis::VERT}){

			const std::string & axStr = drain::EnumDict<AlignAxis>::dict.getKey(ax);

			for (const AlignSVG::value_t & v: {Align::MAX, Align::MID, Align::MIN}){

				const std::string & vStr = drain::EnumDict<Align>::dict.getKey(v);

				for (const drain::image::LayoutSVG::Axis & axRef: {AlignAxis::HORZ, AlignAxis::VERT}){

					const std::string & axRefStr = drain::EnumDict<AlignAxis>::dict.getKey(axRef);

					for (const AlignSVG::value_t & vRef: {Align::MAX, Align::MID, Align::MIN}){

						const std::string & vRefStr = drain::EnumDict<Align>::dict.getKey(vRef);


						const std::string label = drain::StringBuilder<'-'>(axRefStr, vRefStr, axStr, vStr);

						/*
						drain::image::TreeSVG & textGroup = rectGroup[label](NodeSVG::GROUP);
						const std::string ANCHOR = "rect";
						// textGroup->addClass(alSvg::PANEL, alSvg::RELATIVE); // RELATIVE?
						textGroup->setAlignAnchor(ANCHOR);
						*/

						drain::image::TreeSVG & textBox = rectGroup[label](NodeSVG::RECT);
						textBox->setAlign(AlignPos::REF,  axRef, vRef);
						textBox->setAlign(AlignPos::OBJ, ax, v);
						textBox->getBoundingBox().setArea(50,150);
						textBox->setStyle("fill", "green");
						textBox->setStyle("opacity", 0.25);
						textBox->setStyle("stroke", "darkblue");


						drain::image::TreeSVG & text = rectGroup[label + "text"](NodeSVG::TEXT);
						text->setAlign(AlignPos::OBJ, ax, v);
						text->setAlign(AlignPos::REF,  ax, v);
						/*
								text->setAlign(AlignPos::ORIG, AlignAxis::HORZ, Align::MID);
								text->setAlign(AlignPos::ORIG, AlignAxis::VERT, Align::MID);
								text->setAlign(AlignPos::REF,  AlignAxis::HORZ, Align::MID);
								text->setAlign(AlignPos::REF,  AlignAxis::VERT, Align::MID);
						 */
						// text->setAlign(AlignPos::REF,  ax, v);

						drain::image::TreeSVG & textSpan = text["tspan"](NodeSVG::TSPAN);
						textSpan->setText(label);

					}
				}
			}
		}


		/*
		rect->addClass(drain::image::AlignSVG::ANCHOR);

		typedef drain::image::AlignSVG alSvg;

		for (const auto & rHorz: {alSvg::REF_LEFT, alSvg::REF_CENTER, alSvg::REF_RIGHT}){

			const std::string & clsRH = drain::EnumDict<alSvg>::dict.getKey(rHorz);

			for (const auto & vert: {alSvg::REF_TOP, alSvg::REF_MIDDLE, alSvg::REF_BOTTOM}){

				const std::string & clsV = drain::EnumDict<alSvg>::dict.getKey(vert);

				const std::string label = drain::StringBuilder<'-'>(clsRH, clsV);

				drain::image::TreeSVG & textGroup = rectGroup[label](NodeSVG::GROUP);
				textGroup->addClass(alSvg::PANEL, alSvg::RELATIVE); // RELATIVE?

				drain::image::TreeSVG & textBox = textGroup["rect"](NodeSVG::RECT);
				textBox->addClass(alSvg::ANCHOR); // , rHorz, vert); // CONFLICT! SHOULD BE PANEL
				textBox->getBoundingBox().setArea(240,40);
				textBox->setStyle("fill", "green");
				textBox->setStyle("opacity", 0.25);

				drain::image::TreeSVG & text = textGroup["text"](NodeSVG::TEXT);
				text->addClass(alSvg::RELATIVE, rHorz, vert);

				drain::image::TreeSVG & textSpan = text["tspan"](NodeSVG::TSPAN);
				textSpan->setText(label);

			}

		}
		*/

		/*


		drain::image::TreeSVG & group = RackSVG::getCurrentGroup(ctx);

		const drain::Frame2D<double> frame = {240,250};

		drain::image::TreeSVG & rectGroup = group["rg"](NodeSVG::GROUP);
		rectGroup->setId();

		drain::image::TreeSVG & rect = rectGroup["rect"](NodeSVG::RECT); // +EXT!
		rect->set("x", 250);
		rect->set("y", 300);
		rect->set("width", frame.width);
		rect->set("height", frame.height);
		rect->setStyle("fill", "red");
		rect->setId();
		// rect->addClass("SPESSU");
		// rect["basename"](drain::image::svg::TITLE) = "test";

		typedef drain::image::AlignSVG alSvg;

		for (const auto & rHorz: {alSvg::REF_LEFT, alSvg::REF_CENTER, alSvg::REF_RIGHT}){
			const std::string & clsRH = drain::EnumDict<alSvg>::dict.getKey(rHorz);

			for (const auto & vert: {alSvg::TOP, alSvg::MIDDLE, alSvg::BOTTOM}){
				const std::string & clsV = drain::EnumDict<alSvg>::dict.getKey(vert);

				const std::string label = drain::StringBuilder<'-'>(clsRH, clsV);

				drain::image::TreeSVG & text = rectGroup[label](NodeSVG::TEXT);
				// rect->setId();
				drain::image::TreeUtilsSVG::markAligned(rect, text, AlignSVG::ALIGN, clsRH, clsV);  // Future ext AlignSVG::ALIGN
				text->removeClass("FLOAT"); // yes... experimental

				drain::image::TreeSVG & textSpan = text["tspan"](NodeSVG::TSPAN);
				textSpan->setText(label);

			}

		}

		// drain::image::TreeUtilsSVG::alignNEW(rectGroup); // SPESSUcat

		// text->addClass(svgAlign::_CENTER, svgAlign::MIDDLE);
		// text->addClass(drain::image::AlignSVG::CENTER, drain::image::AlignSVG::MIDDLE);


		//text.addChild();
		*/

	}

};

GraphicsModule::GraphicsModule(){ // : CommandSection("science"){

	//const drain::Flagger::ivalue_t section = drain::Static::get<GraphicsSection>().index;

	//const ScienceModule & mod = drain::Static::get<ScienceModule>();

	//drain::CommandBank & cmdBank = drain::getCommandBank();

	// drain::BeanRefCommand<FreezingLevel> freezingLevel(RainRateOp::freezingLevel);
	// cmdBank.addExternal(freezingLevel).section = section;
	install<CmdLinkImage>(); // "cmdname"
	install<CmdSuperPanel>();

};


} // namespace rack

