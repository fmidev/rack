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

/*
  const bool IMAGE_PNG = drain::image::FilePng::fileInfo.checkPath(path);
  const bool IMAGE_PNM = drain::image::FilePnm::fileInfo.checkPath(path);
  const bool IMAGE_TIF = drain::image::FileTIFF::fileInfo.checkPath(path);
  const bool IMAGE_SVG = drain::image::NodeSVG::fileInfo.checkPath(path);
*/
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
#include "graphics.h"
#include "graphics-radar.h"


namespace drain {

template <> // for T (Tree class)
template <> // for K (path elem arg)
bool image::TreeSVG::hasChild(const rack::RackSVG::ElemClass & key) const {
	// std::string(".")+
	return hasChild(EnumDict<rack::RackSVG::ElemClass>::dict.getKey(key, true)); // no error on non-existent dict entry
}


/// Automatic conversion of elem classes to strings.
/**
 *
 */
template <> // for T (Tree class)
template <> // for K (path elem arg)
const image::TreeSVG & image::TreeSVG::operator[](const rack::RackSVG::ElemClass & value) const {
	// std::string(".")+
	return (*this)[EnumDict<rack::RackSVG::ElemClass>::dict.getKey(value, false)];
}


template <> // for T (Tree class)
template <> // for K (path elem arg)
image::TreeSVG & image::TreeSVG::operator[](const rack::RackSVG::ElemClass & key){
	// std::string(".")+
	return (*this)[EnumDict<rack::RackSVG::ElemClass>::dict.getKey(key, false)];
}

/*
template <>
const drain::EnumDict<rack::RackSVG::ElemClass>::dict_t  drain::EnumDict<rack::RackSVG::ElemClass>::dict = {
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};
 */


}

DRAIN_ENUM_DICT(rack::RackSVG::ElemClass) = {
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, NONE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, MAIN_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GROUP_TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_TITLE),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TITLE),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, LOCATION),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, TIME),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, GENERAL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_PANEL),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, IMAGE_BORDER),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, BACKGROUND_RECT),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, BORDER),
		DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SIDE_PANEL),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::ElemClass, SHARED_METADATA),
		// DRAIN_ENUM_ENTRY(rack::RackSVG::TitleClass, IMAGE_SET)
};




namespace rack {

/// Group identifiers for elements which be automatically aligned (stacked horizontally or vertically)


// onst std::string RackSVG::BACKGROUND_RECT = "BACKGROUND_RECT";





void TitleCreatorSVG::formatTitle(TreeSVG & group, const NodeSVG::map_t & attributes) const {

	if (attributes.empty()){
		return;
	}

	drain::Logger mout(__FILE__, __FUNCTION__);

	VariableFormatterODIM<std::string> formatter; // (No instance properties used, but inheritance/overriding)

	for (const auto & attr: attributes){

		std::string key, format;
		drain::StringTools::split2(attr.first, key, format, '|');

		RackSVG::ElemClass elemClass = RackSVG::ElemClass::GENERAL;

		if ((ODIM::timeKeys.count(attr.first)>0) || (ODIM::dateKeys.count(attr.first)>0)){
			elemClass = RackSVG::ElemClass::TIME;
		}
		else if (ODIM::locationKeys.count(attr.first)>0){
			elemClass = RackSVG::ElemClass::LOCATION;
		}

		if (!group.hasChild(elemClass)){
			// getGroupTitle
			RackSVG::addTitleBox(this->svgConf, group, RackSVG::GROUP_TITLE);
			TreeSVG & t = group[elemClass];
			t[svg::COMMENT]->setComment("added: ", attr.first, ' ', attr.second);
		}

		TreeSVG & text  = group[elemClass];
		if (text->isUndefined()){
			mout.suspicious("Expected TEXT elem, but undefined:", NodePrinter(group).str());
			// text->setType(svg::TEXT);
			// text[svg::COMMENT]->setComment("skipped: ", attr.first, ' ', attr.second);
		}

		TreeSVG & tspan = text[attr.first](svg::TSPAN);
		tspan->addClass(attr.first); // allows user-specified style

		if (elemClass == RackSVG::ElemClass::TIME){
			if (format.empty()){
				format = RackSVG::guessFormat(key);
				text->set("data-format", format);
			}
			else {
				text->set("data-format", format); // DEBUG
			}
			// mout.accept<LOG_DEBUG>("TIME text format", format);
		}
		else if (elemClass == RackSVG::ElemClass::LOCATION){
			// text->setAlign(AlignSVG::BOTTOM, AlignSVG::RIGHT);
		}
		else {
			// consider more classes later!
			// text->setAlign(AlignSVG::MIDDLE, AlignSVG::CENTER);
			// text->setText(drain::StringBuilder<'|'>(elemClass, attr.first, attr.second)); //CTXX
		}

		// mout.attention("handle: ", attr.first, " ", v, " + ", format);

		if (format.empty()){
			tspan->setTextSafe(attr.second, ' ');
		}
		else {
			//mout.attention("handle: ", attr.first, " ", v, " + ", format);
			std::stringstream sstr;
			formatter.formatVariable(key, attr.second, format, sstr);
			// tspan->setText(sstr.str(), " ");
			// tspan->setText(sstr.str(), drain::XML::entity_t::NONBREAKABLE_SPACE); // "&#160;",  non-breakable space
			tspan->setTextSafe(sstr.str(), ' '); // "&#160;",  non-breakable space
		}

	}



}


int TitleCreatorSVG::visitPostfix(TreeSVG &root, const TreeSVG::path_t &path){

	drain::Logger mout(__FILE__, __FUNCTION__);
	// Apply to groups only.

	TreeSVG &group = root(path);

	if (!group->typeIs(svg::GROUP)) {
		return 0;
	}

	if (!group.hasChild(svg::METADATA)) {
		mout.reject<LOG_DEBUG>("skipping, group has no METADATA element: ", group.data);
		return 0;
	}

	if (group->hasClass(LayoutSVG::ADAPTER)) {
	// 	return 0;
	}

	const NodeSVG::map_t &attributesPrivate = group[svg::METADATA]->getAttributes();
	const NodeSVG::map_t &attributesShared  = group[MetaDataCollectorSVG::SHARED]->getAttributes();
	// const bool WRITE_PRIVATE_METADATA = !attributesPrivate.empty();

	// bool WRITE_SHARED_METADATA = !attributesShared.empty();

	if (svgConf.groupTitle == "NONE") {
		mout.obsolete("groupTitle 'NONE'");
	}

	// const bool MAIN_AUTO  =  (svgConf.mainTitle == "AUTO");
	// const bool GROUP_AUTO =  (svgConf.groupTitle == "AUTO"); // (svgConf.groupTitleFormatted.substr(0,4) == "AUTO");
	// const bool GROUP_NONE =  (svgConf.groupTitle.empty()); // (svgConf.groupTitleFormatted.substr(0,4) == "NONE");
	// const bool GROUP_USER = !(svgConf.groupTitleFormatted.empty() || GROUP_AUTO || GROUP_NONE);
	// const bool GROUP_USER = !(GROUP_AUTO || GROUP_NONE);

	//if (group->hasClass(RackSVG::ElemClass::MAIN_TITLE)) {
	if (group->hasClass("MAIN")) {
		if (svgConf.mainTitle.empty()){
			group[RackSVG::ElemClass::MAIN_TITLE]->setComment("explicitly empty MAIN_TITLE – skipped");
		}
		else {
			TreeSVG & mainText = RackSVG::addTitleBox(svgConf, group, RackSVG::ElemClass::MAIN_TITLE);
			if (svgConf.mainTitle == "AUTO"){
				formatTitle(group, attributesPrivate);
				formatTitle(group, attributesShared);
			}
			else {
				mainText->setTextSafe(svgConf.mainTitle);
			}
		}
	}
	else if (group->hasClass(LayoutSVG::STACK_LAYOUT)) {
		if (svgConf.groupTitle.empty()){
			group[RackSVG::ElemClass::GROUP_TITLE]->setComment("explicitly empty GROUP_TITLE – skipped");
		}
		else {
			TreeSVG & adapterGroup = RackSVG::getAdapterGroup(group);
			TreeSVG & mainText = RackSVG::addTitleBox(svgConf, adapterGroup, RackSVG::ElemClass::GROUP_TITLE);
			if (svgConf.groupTitle == "AUTO"){
				formatTitle(adapterGroup, attributesPrivate);
				if (svgConf.mainTitle.empty()){
					// Flush the attributes not displayed this far. As AUTO was requested...
					formatTitle(adapterGroup, attributesShared);
				}
			}
			else {
				// group->get("data-title", "");
				mainText->setTextSafe(group->get("data-title", "- - -"));
			}
		}
	}
	else if (group->hasClass(RackSVG::ElemClass::IMAGE_PANEL)) {
		// Add elements directly on the image (corners), skip creating a background rectangle.
		// Use IMAGE element "image" as anchor.
		// RackSVG::appendTitleElements(svgConf, group, "image", RackSVG::ElemClass::IMAGE_TITLE);
		RackSVG::appendTitleElements(svgConf, group, svg::IMAGE, RackSVG::ElemClass::IMAGE_TITLE);
		formatTitle(group, attributesPrivate);
	}
	else {
		// "Neutral groups go here"
		mout.debug("Skipped group:", NodePrinter(group).str());
	}


	/*
	 if (!group.hasChild(svg::METADATA)){
	 mout.attention("group has no METADATA element: ", group.data);
	 return 0;
	 }
	 */
	// Always
	/*
	formatTitle(group, attributesPrivate);
	if (WRITE_SHARED_METADATA && !group->hasClass(RackSVG::ElemClass::IMAGE_PANEL)) {
		formatTitle(group, attributesShared);
	}
	*/
	/*
	 if (WRITE_SHARED_METADATA && group.hasChild(RackSVG::ElemClass::SHARED_METADATA)){ // explicit request: GROUP
	 writeTitles(group, group[RackSVG::ElemClass::SHARED_METADATA]->getAttributes());
	 }
	 */
	return 0;
}


// ........................




void CmdLayout::exec() const  {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FUNCTION__, getName());

	drain::EnumDict<orientation_enum>::setValue(orientation, ctx.mainOrientation);
	drain::EnumDict<direction_enum>::setValue(direction,     ctx.mainDirection);

	// reset
	orientation = drain::EnumDict<orientation_enum>::dict.getKey(orientation_enum::HORZ);
	direction   = drain::EnumDict<direction_enum>::dict.getKey(direction_enum::INCR);

}


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


void CmdAlign::exec() const  {

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
	mout.attention("new anchors: (", ctx.anchorHorz, '+', ctx.anchorVert, ')');

	// Position
	if (!position.empty()){

		std::list<std::string> args;
		drain::StringTools::split(position, args, ':');

		//CompleteAlignment<> align(AlignSVG::Topol::UNDEFINED_TOPOL, AlignBase::Pos::UNDEFINED_POS);
		CompleteAlignment<> align(AlignSVG::Topol::INSIDE, AlignBase::Pos::UNDEFINED_POS);

		for (const std::string & arg: args){

			std::list<std::string> keys;
			drain::StringTools::split(arg, keys, '.');

			//CompleteAlignment<> align(AlignSVG::Topol::UNDEFINED_TOPOL, AlignBase::Pos::UNDEFINED_POS);

			for (const std::string & key: keys){
				align.set(key);
				mout.debug("updated align: ", align, " == topol=", align.topol, ", axis=", align.axis, ", pos=", align.pos);
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
				// Ok, topology (INSIDE or OUTSIDE) was set
				break;
			default:
				mout.advice("use: ", drain::sprinter(drain::EnumDict<AlignBase::Axis>::dict.getKeys(), {"|"}).str());
				mout.advice("use: ", drain::sprinter(drain::EnumDict<AlignSVG::HorzAlign>::dict.getKeys(), {"|"}).str());
				mout.advice("use: ", drain::sprinter(drain::EnumDict<AlignSVG::VertAlign>::dict.getKeys(), {"|"}).str());
				// mout.advice("use: ", drain::sprinter(drain::EnumDict<Alignment<> >::dict.getKeys(), {"|"}).str()); // = HorzAlign + VertAlign
				mout.error("could not determine axis from argument '", arg, "'");
				break;
			}


		}

	}

	// mout.accept<LOG_NOTICE>(ctx.alignHorz.topol, '/',(AlignBase &)ctx.alignHorz);
	// mout.accept<LOG_NOTICE>(ctx.alignVert.topol, '/',(AlignBase &)ctx.alignVert);

}



void CmdFontSizes::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);
	adjust(ctx.svgPanelConf.boxHeights, 0.8);
	mout.accept<LOG_WARNING>("new values: ", ctx.svgPanelConf.fontSizes);

}


void CmdTitleHeights::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	adjust(ctx.svgPanelConf.boxHeights, 0.8);
	mout.accept<LOG_DEBUG>("new BOX  values: ", ctx.svgPanelConf.boxHeights);

	for (size_t i= 0; i<ctx.svgPanelConf.boxHeights.size(); ++i){
		ctx.svgPanelConf.fontSizes[i] = 0.65 * ctx.svgPanelConf.boxHeights[i];
	}
	mout.accept<LOG_DEBUG>("new FONT values: ", ctx.svgPanelConf.fontSizes);

	// CmdFontSizes::updateFontStyles(ctx);
}


void CmdLinkImage::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FUNCTION__, getName());

	const Composite & composite = ctx.getComposite(RackContext::CURRENT|RackContext::PRIVATE|RackContext::SHARED);

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



void CmdGroupTitle::exec() const {
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



void CmdInclude::exec() const  {

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
		ctx.svgPanelConf.svgIncludes.unset(drain::image::FileSVG::IncludePolicy::ON);
	}
	else {
		ctx.svgPanelConf.svgIncludes.set(drain::image::FileSVG::IncludePolicy::ON); // so PNG,SVG,.. need no explicit "ON"
		ctx.svgPanelConf.svgIncludes.set(value);
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




} // rack


