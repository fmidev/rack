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

#include <drain/util/Base64.h>
#include <drain/util/Output.h>
#include <drain/util/StringMapper.h>
#include <drain/util/Units.h>

#include <drain/image/FilePng.h>
#include <drain/image/TreeUtilsSVG.h>
#include <drain/image/GeoFrame.h>
#include <drain/image/MouseXML.h>

#include "graphics-base.h"
#include "graphics-panel.h"
#include "graphics-imagepanel.h"
#include "graphics-interactive.h"

namespace drain {


// DRAIN_XML_NODE_SWAP(drain::image::TreeSVG);

}

namespace rack {



// const std::string CmdPolarBase::DATA_ID = "data-latest";


// const std::string CmdPolarSector::SECTOR("SECTOR");

/**
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 */

/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
 */



void CmdDot::exec() const  {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FUNCTION__, getName());

	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG);

	drain::image::TreeSVG & overlayGroup = ctx.getImagePanelGroup();

	ImagePanel superPanel(overlayGroup);
	drain::image::TreeSVG & overlay = superPanel.getOverlayGroup();

	//overlay.addChild()(svg::COMMENT)->setText(getName(), ' ', getParameters(), " imageCoords=", imageCoords);

	drain::Point2D<double> coordsDeg;

	if (drain::BBox::isMetric(coords)){
		radarSVG.geoFrame.m2deg(coords, coordsDeg);
		overlay.addChild()->setComment(getName(), ' ', getParameters(), " coordsDeg=", coordsDeg);
	}
	else {
		coordsDeg = coords;
	}

	//mout.attention(DRAIN_LOG(imageCoords));
	mout.info(DRAIN_LOG(coordsDeg));

	// Essentially drawing a "radar" there, so here its coordinates...
	radarSVG.radarProj.setSiteLocationDeg(coordsDeg.x, coordsDeg.y);

	// const std::string SPOT = "SPOT";
	//drain::image::TreeUtilsSVG
	drain::UtilsXML::ensureStyle(ctx.svgTrack, SPOT, {
			{"fill", "green"},
			{"stroke", "white"},
			{"stroke-width", 2.0},
			// {"opacity", 0.5}
	});

	// overlay.addChild()->setComment(getName(), ' ', getParameters());
	drain::image::TreeSVG & curve = overlay[SPOT](drain::image::svg::PATH);
	curve->addClass(SPOT);
	if (!id.empty()){
		curve->setId(id);
	}

	drain::svgPATH svgElem(curve);
	radarSVG.drawCircle(svgElem, radiusMetres.range); // km

	if (!style.empty()){
		curve->setStyle(style);
	}

	// radarSVG.geoFrame.p

}


/*
void CmdDotTest::exec() const  {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FUNCTION__, getName());

	RadarSVG radarSVG;

	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	// NoTE: this is  not radar specific
	drain::image::TreeSVG & overlay = getOverlay(overlayGroup);
	//overlay.addChild()(svg::COMMENT)->setText(getName(), ' ', getParameters(), " imageCoords=", imageCoords);

	drain::Point2D<double> coordsDeg;
	// drain::Point2D<int> imageCoords;
	if (drain::BBox::isMetric(coords)){
		radarSVG.geoFrame.m2deg(coords, coordsDeg);
		//radarSVG.geoFrame.m2pix(coords, imageCoords);
		overlay.addChild()(svg::COMMENT)->setText(getName(), ' ', getParameters(), " coordsDeg=", coordsDeg);
	}
	else {
		coordsDeg = coords;
		//radarSVG.geoFrame.deg2pix(coords, imageCoords);
	}
	//mout.attention(DRAIN_LOG(imageCoords));
	mout.attention(DRAIN_LOG(coordsDeg));
	//overlay.addChild()(svg::COMMENT)->setText(getName(), ' ', getParameters(), " coordsDeg=", coordsDeg);

	radarSVG.radarProj.setSiteLocationDeg(coordsDeg.x, coordsDeg.y);

	// const std::string SPOT = "SPOT";
	//drain::image::TreeUtilsSVG
	drain::UtilsXML::ensureStyle(ctx.svgTrack, SPOT, {
			{"fill", "green"},
			{"stroke", "white"},
			{"stroke-width", 2.0},
			// {"opacity", 0.5}
	});

	// overlay.addChild()->setComment(getName(), ' ', getParameters());
	drain::image::TreeSVG & curve = overlay[SPOT](drain::image::svg::PATH);
	curve->addClass(SPOT);
	if (!id.empty()){
		curve->setId(id);
	}

	drain::svgPATH svgElem(curve);
	radarSVG.drawCircle(svgElem, radiusMetres.range); // km

	if (!style.empty()){
		curve->setStyle(style);
	}

	// radarSVG.geoFrame.p

}
*/




/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
 */





/*
class CmdLogo : public drain::BasicCommand { // drain::SimpleCommand<std::string> {

public:

	CmdLogo() : drain::BasicCommand(__FUNCTION__, "SVG test product") {
		getParameters().link("name",   name, "Name (label) of the logo");
		getParameters().link("variant",  variant, "label");
		// getParameters().link("anchor", myAnchor, drain::sprinter(drain::Enum<drain::image::AnchorElem::Anchor>::dict.getKeys(), "|", "<>").str());
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

// #include "js/add_mouse_listeners.h"
#include "js/coord_handler.h"
#include "js/image_coord_tracker.h"


//drain::image::TreeSVG & addCoordMonitor(drain::image::TreeSVG & textObject, const std::string & cls){
drain::image::TreeSVG & addCoordMonitor(drain::image::TreeSVG & textObject, MouseXML::ElemClass cls){

	textObject->setType(svg::TEXT); // ensure
	textObject->setFontSize(15,18);
	textObject->addClass(RackSVG::ElemClass::SELECTOR);
	textObject->addClass(LayoutSVG::NEUTRAL);
	// textObject->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);
	// textObject->setAlign(AlignSVG::RIGHT);

	drain::image::TreeSVG & coordDisplay = textObject[cls](svg::TSPAN);
	coordDisplay->addClass(cls);
	return coordDisplay;

}

// Under construction...

template <typename N, typename ...TT>
static
drain::UnorderedMultiTree<N> & ensureJSFunctionScope(drain::UnorderedMultiTree<N> & root,
		const std::string & name, const TT & ...args){

	typedef drain::UnorderedMultiTree<N> T;
	T & scriptElem = drain::UtilsXML::getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT);

	if (!scriptElem.hasChild(name)){
		T & jsFunction = scriptElem[name];
		jsFunction->setType(N::xml_tag_t::JAVASCRIPT_SCOPE);

		jsFunction->setText("var ", name, " = function(", drain::StringBuilder<','>(args...), ')');
		// T & jsFunctionScope = jsFunction.addChild();
		// T & jsFunctionScope = jsFunction[N::xml_tag_t::JAVASCRIPT_SCOPE]; // maybe string best?
		// jsFunctionScope->setType(N::xml_tag_t::JAVASCRIPT_SCOPE);
		return jsFunction;
	}
	else {
		return scriptElem[name];
	}
	// T & jsFunction = getHeaderObject(root, T::node_data_t::xml_tag_t::SCRIPT);

}

/**
 *   Future option: other scope them document
 */
/*
void addVisibilitySwitch(NodeSVG & dstElem, NodeSVG & controlElem, const std::string & scope="document",
		const std::string & mouseEventOn="onmouseenter", const std::string & mouseEventOff="onmouseleave"){
	// Ensure id, for dynamic access
	// dst->setId("coordMove", dst->getId());
	const std::string & id = dstElem.setId(); // ->getId();

	if (scope.empty()){
		controlElem.setAttribute(mouseEventOn,  drain::StringBuilder<>("getElementById('", id, "').style.visibility='visible'"));
		controlElem.setAttribute(mouseEventOff, drain::StringBuilder<>("getElementById('", id, "').style.visibility='hidden'"));
	}
	else {
		controlElem.setAttribute(mouseEventOn,  drain::StringBuilder<>(scope, ".getElementById('", id, "').style.visibility='visible'"));
		controlElem.setAttribute(mouseEventOff, drain::StringBuilder<>(scope, ".getElementById('", id, "').style.visibility='hidden'"));
	}

}
*/


/// Designed for CmdRect, could be multi-purpose
/**
 *   Creates the following "standard" groups:
 *   - mouseGroup (path=MOUSE)
 *
 *   Compare with: addGeoData() above.
 *
 *   Move to graphics-interactive.
 */
/*
TreeSVG & InteractiveSVG::getInteractiveOverlay(RackContext & ctx, RadarSVG & radarSVG, bool fixedAEQD) const {

	using namespace drain::image;
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	TreeSVG & adapterGroup = ctx.getCurrentAdapterGroup();

	TreeSVG & imagePanelGroup = adapterGroup[ctx.currentImagePanel];
	if (imagePanelGroup->isUndefined()){
		mout.warn("currentImagePanel created but undefined");
	}

	if (!imagePanelGroup.hasChild(svg::IMAGE)){
		mout.warn("currentImagePanel created contains no image");
	}


	// drain::image::TreeSVG & mouseGroup = imagePanel[RackSVG::ElemClass::MOUSE](svg::GROUP);
	drain::image::TreeSVG & mouseGroup = imagePanelGroup[OverlayMoverSVG::OVERLAY](svg::GROUP);

	mouseGroup->addClass(ClipperSVG::CLIPPED);
	// mouseGroup.addChild()->setComment("Mouse tracker modified by ", getName(), ' ', getLastParameters());

	drain::image::TreeSVG & visualGroup = mouseGroup[RackSVG::ElemClass::SELECTOR](svg::GROUP);
	visualGroup->addClass(MouseXML::ElemClass::MONITOR); // handles visualGroup->setStyle("visibility", ...);
	visualGroup->setDefaultAlignAnchor(RackSVG::ElemClass::BACKGROUND_RECT);
	visualGroup.addChild()->setComment("Visualisation of the selection");


	drain::image::NodeSVG::map_t & attr = mouseGroup->getAttributes();


	mout.special("Attaching coordinate monitor to imagePanel: ", imagePanelGroup->getId());
	// mouseGroup[svg::TITLE](svg::TITLE)->setText("Mouse tracker by ", getName(), ' ', getLastParameters());


	int EPSG = attr["data-epsg"];
	if (EPSG > 0){
		mout.info("Projection: using EPSG=", EPSG);
		radarSVG.geoFrame.setProjectionEPSG(EPSG);
		radarSVG.radarProj.setProjectionDst(EPSG);
	}
	else {
		mout.hint("Projection: EPSG not detected, consider to set/unset fixedAEQD");
		drain::image::NodeSVG::map_t & attrColumn = adapterGroup->getAttributes();

		mout.note(DRAIN_LOG(fixedAEQD));
		std::string projdef;

		auto & sharedProjdef = attrColumn["data-projdef"];
		mout.warn(DRAIN_LOG(sharedProjdef));
		if (fixedAEQD && !sharedProjdef.empty()){
			projdef = sharedProjdef.toStr();
			mout.warn("Now change: using ", projdef);
			mouseGroup.addChild()->setComment(DRAIN_LOG(fixedAEQD), " - using projdef of the first input:");
		}
		else {
			const drain::VariableMap & status = ctx.getStatusMap();
			projdef = status.get("where:projdef", "");
			sharedProjdef = projdef;
		}
		//mout.attention(DRAIN_LOG(projdef));

		if (projdef.empty()){
			mout.warn("No valid 'epsg' attribute value and also 'projdef' empty.");
		}
		else {
			mouseGroup.addChild()->setComment("Projection: EPSG not detected, using projdef='", projdef,"'");
			radarSVG.geoFrame.setProjection(projdef);
			radarSVG.radarProj.setProjectionDst(projdef);
		}

	}

	std::vector<double> bboxNat;
	attr["data-bbox"].toSequence(bboxNat, ',');
	if (bboxNat.empty()){
		mout.warn("Could not attach coordinate monitor for imagePanel '", imagePanelGroup->getId(), "' - missing 'data-bbox' attribute ");
	}
	else if (bboxNat.size() != 4){
		mout.warn("Image panel #", imagePanelGroup->getId(), ": odd size of elements in 'data-bbox' = ", attr["data-bbox"], " => ", drain::sprinter(bboxNat));
	}
	radarSVG.geoFrame.setBoundingBox(bboxNat[0], bboxNat[1], bboxNat[2],bboxNat[3]);

	const drain::image::TreeSVG & image = imagePanelGroup[svg::IMAGE];
	const drain::Frame2D<int> imageGeometry(image->getWidth(), image->getHeight());
	radarSVG.geoFrame.setGeometry(imageGeometry.width, imageGeometry.height);

	if (radarSVG.geoFrame.isDefined()){
		mout.debug(radarSVG.geoFrame);
	}
	else {
		mout.warn("Failed in initializing", DRAIN_LOG(radarSVG.geoFrame));
	}

	drain::image::TreeSVG & bgRect = visualGroup[RackSVG::ElemClass::BACKGROUND_RECT](svg::RECT);
	bgRect->setFrame(radarSVG.geoFrame.getGeometry());
	bgRect->addClass(RackSVG::ElemClass::BACKGROUND_RECT);
	bgRect->addClass(LayoutSVG::FIXED); // check
	// mout.attention(DRAIN_LOG(radarSVG.geoFrame));
	// radarSVG.radarProj.setProjectionDst();

	mouseGroup.addChild()->setComment("Plane with a mouse listener1");
	// NOte: mouseGroup == overlayGroup
	// mouseListenerElem->addClass("MIKA");
	// mouseListenerElem->setStyle("opacity", 0);

	return imagePanelGroup;
}
*/

void CmdRect::exec() const {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	if (ctx.currentImagePanel.empty()){
		mout.advice("write PNG file before calling ", getName());
		mout.warn("currentImagePanel empty");
		return;
	}

	/*
	TreeSVG & monitorStyle = drain::UtilsXML::ensureStyle(ctx.svgTrack, Interactive::ElemClass::MONITOR, {
			{"fill", "white"},
			{"stroke", "none"},
			{"font-size", "14px"},
	})
	*/

	// Modify SVG header. Notice inverse order (for prepend() )
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_coord_tracker") = image_coord_tracker;
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "coord_handler")       = coord_handler;

	ctx.getOnLoadScript()["image_coord_tracker"] = "image_coord_tracker();";
	// RackSVG::getOnLoadScript(ctx)["test"] = "// Test";

	// Experimental
	ctx.addJavaScripsDef("MONITOR2_BOX", "cls");


	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup(); //adapterGroup[ctx.currentImagePanel];
	ImagePanel superPanel(imagePanelGroup);
	// TODO: pack inside ImagePanel
	// imagePanelGroup->addClass("MOUSE_VALUE");

	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG); // undependent?

	drain::image::TreeSVG & visualGroup = superPanel.getOverlayGroup();

	// Reserve slot (implemented below).
	visualGroup.addChild()->setComment("Visualisation of the selection");

	// Reserve slot
	drain::image::TreeSVG & selectRect = visualGroup[RackSVG::ElemClass::SELECTOR](svg::RECT);
	selectRect->addClass(LayoutSVG::FIXED, LayoutSVG::NEUTRAL); // check
	selectRect->setLocation(10,20);
	selectRect->setFrame(30,40);

	TreeSVG & coordMoveText = visualGroup[MouseXML::ElemClass::MONITOR](svg::TEXT);
	coordMoveText->setMyAlignAnchor(RackSVG::ElemClass::BACKGROUND_RECT);
	coordMoveText->setAlign(AlignSVG::BOTTOM, AlignSVG::RIGHT);
	if (cursorCoord){
		coordMoveText->addClass("CURSOR");
	}

	visualGroup.addChild()->setComment("Display mouse coordinates");
	TreeSVG & monitorMove = addCoordMonitor(coordMoveText, MouseXML::ElemClass::MONITOR_MOVE);
	monitorMove->setText('.');

	// "MAIN"
	drain::Rectangle<double> bboxNat;
	drain::Rectangle<int> bboxImg;
	drain::Unit unit = radarSVG.geoFrame.convert(this->bbox, bboxNat, bboxImg);
	mout.experimental(DRAIN_LOG(this->bbox), DRAIN_LOG(unit));
	mout.experimental(DRAIN_LOG(bboxNat));
	mout.experimental(DRAIN_LOG(bboxImg));
	if (bboxImg.empty()){
		mout.warn("Bounding box empty: ", bbox, " -> ", bboxImg);
	}

	switch (unit) {
	case drain::Unit::DEGREE:
		if (radarSVG.geoFrame.isLongLat()){
			monitorMove->setText(this->bbox);
			// This could be enough also for METRIC projection, but...
		}
		else {
			// ... here we get the METRIC BBOX without extra computation:
			monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "m)");
		}
		break;
	case drain::Unit::METRE:
		if (!radarSVG.geoFrame.isLongLat()){
			monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "deg)");
		}
		else {
			mout.error("Cannot set metric BBOX (", this->bbox, ") for long-lat projection: ", radarSVG.geoFrame.getProjStr());
		}
		break;
	case drain::Unit::UNDEFINED:
		// no break;
	case drain::Unit::PIXEL:
		break;
	default:
		mout.error("unknown unit: ", this->bbox); // or trust false above?
		break;
	}
	mout.attention(DRAIN_LOG(bboxImg));


	selectRect->addClass(RackSVG::ElemClass::SELECTOR);
	selectRect->setLocation(std::min(bboxImg.lowerLeft.x, bboxImg.upperRight.x), std::min(bboxImg.lowerLeft.y, bboxImg.upperRight.y));
	selectRect->setFrame(::abs(bboxImg.getWidth()), ::abs(bboxImg.getHeight()));

	TreeSVG & coordSpanDisplay = visualGroup[MouseXML::ElemClass::MONITOR_DRAG];
	coordSpanDisplay->addClass(RackSVG::ElemClass::SELECTOR);
	coordSpanDisplay->addClass(MouseXML::ElemClass::MONITOR_DRAG); // From JS
	coordSpanDisplay->addClass(LayoutSVG::NEUTRAL); // ?

	coordSpanDisplay.addChild()->setComment("Start coordinates");
	TreeSVG & monitorDown = addCoordMonitor(coordSpanDisplay, MouseXML::ElemClass::MONITOR_DOWN);
	coordSpanDisplay.addChild("COMMA")(svg::TSPAN) = ",";
	coordSpanDisplay.addChild()->setComment("End coordinates");
	TreeSVG & monitorUp   = addCoordMonitor(coordSpanDisplay, MouseXML::ElemClass::MONITOR_UP);
	monitorDown->setText(bboxImg.lowerLeft);
	monitorUp->setText(bboxImg.upperRight, " px");

	coordSpanDisplay->resetAlign();
	coordSpanDisplay->setMyAlignAnchor(RackSVG::ElemClass::SELECTOR);
	coordSpanDisplay->setAlign(AlignSVG::TOP, AlignSVG::LEFT);

	// RackSVG::consumeAlignRequest(ctx, coordSpanDisplay); // TODO return bool, if applied

	/// Create the actual another plane (RECT) to receive mouse events
	//drain::image::TreeSVG & mouseListenerElem = RackSVG::getImageBorder(mouseGroup);
	drain::image::TreeSVG & mouseListener = superPanel.getMouseListenerFrame();
	mouseListener->set("data-resolution", resolution.tuple());
	MouseXML::addVisibilitySwitch(coordMoveText, mouseListener);

	// If mask...
	/*
	if (MASK && false){
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		//drain::image::TreeSVG & mask =
		// MaskerSVG::createMask(ctx.svgTrack, mouseGroup, w, h, mouseListenerElem);
	}
	*/


}


void CmdCoords::exec() const {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	if (ctx.currentImagePanel.empty()){
		mout.advice("write PNG file before calling ", getName());
		mout.warn("currentImagePanel empty");
		return;
	}

	// Modify SVG header. Notice inverse order (for prepend() )
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_coord_tracker") = image_coord_tracker;
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "coord_handler")       = coord_handler;

	ctx.getOnLoadScript()["image_coord_tracker"] = "image_coord_tracker();";



	RadarSVG radarSVG;

	/*
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG); // ensure BBOX + track class
	overlayGroup->setId("over", overlayGroup->getId()); // visible
	std::string bbox = overlayGroup->get("data-bbox","");
	if (!bbox.empty()){
		mout.special("Attaching coordinate monitor to overlaygroup #", overlayGroup->getId());
	}
	else {
		mout.warn("Could not attach coordinate monitor - overlaygroup #", overlayGroup->getId(), "has no 'data-bbox' attribute ");
	}


	drain::image::TreeSVG & imagePanelGroup = RackSVG::getImagePanelGroup(ctx);
	imagePanelGroup->addClass(RackSVG::ElemClass::MOUSE); //  COORD_TRACKER
	// imagePanelGroup->setAttribute("data-mouse-plane", overlayGroup->getId());
	// -> track IMAGE_BORDER
	// -> write to COORDS
	// drain::image::TreeSVG & coordCatcher = imagePanelGroup[RackSVG::ElemClass::BACKGROUND_RECT](svg::RECT);
	// coordCatcher->addClass(RackSVG::ElemClass::BACKGROUND_RECT);

	drain::image::TreeSVG & coordDisplay = imagePanelGroup[MouseXML::ElemClass::MONITOR];
	addCoordMonitor(coordDisplay, MouseXML::ElemClass::MONITOR_MOVE);
	drain::ClassXML clsTest;
	clsTest = "kks";
	coordDisplay->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordDisplay->setAlign(AlignSVG::TOP);
	// coordDisplay->setAlign(AlignSVG::RIGHT);
	drain::image::TreeSVG & coordSpanDisplay = imagePanelGroup["MONITOR_RECT"];
	coordSpanDisplay->setMyAlignAnchor<AlignBase::Axis::VERT>(drain::image::AnchorElem::PREVIOUS);
	coordSpanDisplay->setAlign(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
	addCoordMonitor(coordSpanDisplay, MouseXML::ElemClass::MONITOR_DOWN);
	coordSpanDisplay.addChild("COMMA")(svg::TSPAN) = ",";
	addCoordMonitor(coordSpanDisplay, MouseXML::ElemClass::MONITOR_UP);

	drain::image::TreeSVG & selectRect = imagePanelGroup[RackSVG::ElemClass::SELECTOR](svg::RECT);
	selectRect->addClass(RackSVG::ElemClass::SELECTOR, LayoutSVG::FIXED);
	selectRect->setLocation(200,100);
	selectRect->setFrame(400,200);

	/// Create yet another plane (RECT) to receive mouse events
	drain::image::TreeSVG & coordTracker = imagePanelGroup[MouseXML::ElemClass::MOUSE_TRACKER](svg::RECT);
	coordTracker->setMyAlignAnchor(svg::IMAGE);
	coordTracker->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	/// Todo: move/share/generalize
	coordTracker->set("data-bbox", bbox);
	coordTracker->set("data-epsg", overlayGroup->get("data-epsg",""));
	coordTracker->set("data-resolution", resolution.tuple());
	coordTracker->addClass(MouseXML::ElemClass::MOUSE_TRACKER);

	coordTracker->setStyle("fill", "white"); // TODO: transparent tracker
	// coordTracker->setStyle("stroke", "green");
	coordTracker->setStyle("opacity", 0.0001);

	*/

}


#include "js/image_value_tracker.h"

template <typename T>
void encodeToRGBAimage(const Image & data, drain::image::ImageT<uint8_t> & dataImage){

	dataImage.setGeometry(data.getWidth(), data.getHeight(), 3);

	drain::image::Channel & red   = dataImage.getChannel(0);
	drain::image::Channel & green = dataImage.getChannel(1);

	const int bias = std::numeric_limits<T>::min();
	const int bits = 8 * sizeof(T);
	int value;

	drain::image::Channel::iterator rit = red.begin();
	drain::image::Channel::iterator git = green.begin();

	if (bits == 8){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit = (value>>8) & 0xff;
			*git =  value     & 0xff;
			++rit;
			++git;
		}
	}
	else if (bits == 16){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit =  value     & 0xff;
			*git =  0;
			++rit;
			++git;
		}
	}
	else {
		drain::Logger( __FILE__, __FUNCTION__, __LINE__).error("unsupported bit depth:", bits);
	}

}

void CmdData::exec() const {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, getName(), __FUNCTION__, __LINE__);

	drain::UtilsXML::ensureStyle(ctx.svgTrack, RackSVG::ElemClass::DATA_ARRAY, {
			{"opacity", 0.1},  // some browsers disable mouse listener, if fully invisible?
	});

	const Image & data = ctx.getCurrentGrayImage();

	if (data.isEmpty()){
		mout.warn("Could not find data for image (getCurrentGrayImage() failed)");
		return;
	}
	// mout.attention(data);

	//  Modify SVG header. Ensure the script is available.
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_value_tracker") = image_value_tracker;
	ctx.getOnLoadScript()["image_value_tracker"] = "image_value_tracker();";

	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup(); //adapterGroup[ctx.currentImagePanel];

	// NEW
	ImagePanel superPanel(imagePanelGroup);

	// TODO: pack inside ImagePanel
	imagePanelGroup->addClass("MOUSE_VALUE");

	drain::image::TreeSVG & overlay = superPanel.getOverlayGroup();

	drain::image::TreeSVG & coordMonitor = overlay["MOUSE_COORD"](svg::TEXT); // imagePanelGroup
	coordMonitor->setId();
	//coordMonitor->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordMonitor->setMyAlignAnchor(RackSVG::ElemClass::BACKGROUND_RECT);
	coordMonitor->setAlign(AlignSVG::RIGHT, AlignSVG::BOTTOM);
	coordMonitor->addClass("COORD_MONITOR");
	coordMonitor->addClass(RackSVG::ElemClass::IMAGE_TITLE); // , RackSVG::ElemClass::TIME); // check
	coordMonitor->setTextSafe("(x,y)");
	coordMonitor->setFontSize(15,20);

	drain::image::TreeSVG & valueMonitor = overlay["MOUSE_VALUE"](svg::TEXT); // imagePanelGroup
	valueMonitor->setId();
	valueMonitor->setMyAlignAnchor("MOUSE_COORD");
	valueMonitor->setAlign(AlignSVG::RIGHT);
	valueMonitor->setAlign(AlignSVG::TOP, MutualAlign::OUTSIDE);
	valueMonitor->addClass("VALUE_MONITOR");
	// Associate with graphical "location" style
	valueMonitor->addClass(RackSVG::ElemClass::IMAGE_TITLE); // , RackSVG::ElemClass::LOCATION);
	valueMonitor->setTextSafe("-value-");
	valueMonitor->setFontSize(15,20);

	// Formatted status: also \c filename can contain variables.
	const std::string filenameFinal = ctx.getFormattedStatus(std::string("${outputPrefix}")+filename);
	mout.special(DRAIN_LOG(filenameFinal));
	drain::image::TreeSVG & dataImageElem = superPanel.getDataImage(filenameFinal, data.getGeometry().getAreaGeometry());
	addGeoData(data, dataImageElem);
	//dataImageElem->setUrl(filenameFinal);

	// Construct actual data (RGB) and save it.
	const std::type_info & type = data.getType();
	const int bits = 8 * drain::Type::call<drain::sizeGetter>(type);
	const int bias = drain::Type::call<drain::typeMin, int>(type);
	mout.attention(DRAIN_LOG(bits), ' ', DRAIN_LOG(bias));

	drain::image::ImageT<uint8_t> dataImage(data.getWidth(), data.getHeight(), 3);
	drain::image::Channel & red   = dataImage.getChannel(0);
	drain::image::Channel & green = dataImage.getChannel(1);
	//drain::image::Channel & blue  = dataImage.getChannel(2);


	int value;

	drain::image::Channel::iterator rit = red.begin();   // More significant bits
	drain::image::Channel::iterator git = green.begin(); // Less significant bits

	if (bits == 8){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit =  value     & 0xff;
			*git =  0;
			++rit;
			++git;
		}
	}
	else if (bits == 16){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit = (value>>8) & 0xff;
			*git =  value     & 0xff;
			++rit;
			++git;
		}
	}
	else {
		mout.error(__LINE__, "unsupported bit depth:", bits);
	}

	drain::image::FilePng::write(dataImage, filenameFinal);

	// drain::image::TreeSVG & mouseListenerElem =
	superPanel.getMouseListenerFrame();
	// mouseListenerElem->setStyle("fill", "red");       //
	// mouseListenerElem->setStyle("fill-opacity", 0.1); // TODO: transparent tracker

	// return;

}


// ---- Experimental ----

#include "js/base64ToArrayLE.h"


// Important. Keep this as future option.
void CmdTestData::exec() const {

	using namespace drain::image;
	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	RadarSVG radarSVG;
	//drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG); // ensure BBOX + track class
	//drain::image::TreeSVG & imagePanelGroup = RackSVG::getImagePanelGroup(ctx);
	// drain::image::TreeSVG & img = imagePanelGroup[svg::IMAGE];

	/// Ensure that the script is available.
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "base64ToArrayLE") = base64ToArrayLE;

	// TEST 1: float vector
	const std::vector<float> floatVector(11*11, 1.2345);
	//test(img, floatVector, mout);

	// TEST 2: short unsigned vector
	const int N = 64;
	std::vector<uint16_t> uint16Vector(N*N);
	for (int j=0; j<N; ++j){
		for (int i=0; i<N; ++i){
			uint16Vector[j*N+i] = j*N+i;
		}
	}
	// test(img, uint16Vector, mout);


	TreeSVG & onloadJS = ctx.getOnLoadScript();
	// onloadJS["set_image_value_tracker"] = "set_image_value_tracker();";
	onloadJS["demo_base64"] = "demo_base64();";


}


} // namespace rack


/*
TreeSVG & getImagePanelNEW(RackContext & ctx, TreeSVG & imagePanelGroup) {

	using namespace drain::image;
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// TreeSVG & alignedGroup = RackSVG::getCurrentAlignedGroup(ctx);

	if (imagePanelGroup->isUndefined()){
		mout.warn("currentImagePanel created but undefined");
	}

	if (!imagePanelGroup.hasChild(svg::IMAGE)){
		mout.warn("currentImagePanel created contains no image");
	}


	// could also be [MouseXML::ElemClass::MONITOR]
	drain::image::TreeSVG & mouseGroup = imagePanelGroup[RackSVG::ElemClass::MOUSE](svg::GROUP);
	mouseGroup->addClass(ClipperSVG::CLIPPED);
	// mouseGroup.addChild()->setComment("Mouse tracker modified by ", getName(), ' ', getLastParameters());

	drain::image::TreeSVG & visualGroup = mouseGroup[RackSVG::ElemClass::SELECTOR](svg::GROUP);
	visualGroup->addClass(MouseXML::ElemClass::MONITOR); // handles visualGroup->setStyle("visibility", ...);
	visualGroup->setDefaultAlignAnchor(RackSVG::ElemClass::BACKGROUND_RECT);
	visualGroup.addChild()->setComment("Visualisation of the selection");


	// drain::image::NodeSVG::map_t & attr = mouseGroup->getAttributes();
	// mout.special("Attaching coordinate monitor to imagePanel: ", imagePanel->getId());
	// mouseGroup[svg::TITLE](svg::TITLE)->setText("Mouse tracker by ", getName(), ' ', getLastParameters());

	return imagePanelGroup;
}
*/

/*
TreeSVG & activateImagePanel(RackContext & ctx, RadarSVG & radarSVG, bool fixedAEQD) {

	using namespace drain::image;

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	TreeSVG & alignedGroup = RackSVG::getCurrentAdapterGroup(ctx);

	TreeSVG & imagePanel = alignedGroup[ctx.currentImagePanel];
	if (imagePanel->isUndefined()){
		mout.warn("currentImagePanel created but undefined");
	}

	if (!imagePanel.hasChild(svg::IMAGE)){
		mout.warn("currentImagePanel created contains no image");
	}

	//drain::image::NodeSVG::map_t & attr = mouseGroup->getAttributes();
	drain::image::NodeSVG::map_t & attr = imagePanel->getAttributes();

	int EPSG = attr["data-epsg"];
	if (EPSG > 0){
		mout.info("Projection: using EPSG=", EPSG);
		radarSVG.geoFrame.setProjectionEPSG(EPSG);
		radarSVG.radarProj.setProjectionDst(EPSG);
	}
	else {
		mout.hint("Projection: EPSG not detected, consider to set/unset fixedAEQD");
		drain::image::NodeSVG::map_t & attrColumn = alignedGroup->getAttributes();

		mout.note(DRAIN_LOG(fixedAEQD));
		std::string projdef;

		auto & sharedProjdef = attrColumn["data-projdef"];
		mout.warn(DRAIN_LOG(sharedProjdef));
		if (fixedAEQD && !sharedProjdef.empty()){
			projdef = sharedProjdef.toStr();
			mout.warn("Now change: using ", projdef);
			// mouseGroup.addChild()->setComment(DRAIN_LOG(fixedAEQD), " - using projdef of the first input:");
		}
		else {
			const drain::VariableMap & status = ctx.getStatusMap();
			projdef = status.get("where:projdef", "");
			sharedProjdef = projdef;
		}
		//mout.attention(DRAIN_LOG(projdef));

		if (projdef.empty()){
			mout.warn("No valid 'epsg' attribute value and also 'projdef' empty.");
		}
		else {
			// mouseGroup.addChild()->setComment("Projection: EPSG not detected, using projdef='", projdef,"'");
			radarSVG.geoFrame.setProjection(projdef);
			radarSVG.radarProj.setProjectionDst(projdef);
		}

	}

	std::vector<double> bboxNat;
	attr["data-bbox"].toSequence(bboxNat, ',');
	if (bboxNat.empty()){
		mout.warn("Could not attach coordinate monitor for imagePanel '", imagePanel->getId(), "' - missing 'data-bbox' attribute ");
	}
	else if (bboxNat.size() != 4){
		mout.warn("Image panel #", imagePanel->getId(), ": odd size of elements in 'data-bbox' = ", attr["data-bbox"], " => ", drain::sprinter(bboxNat));
	}
	radarSVG.geoFrame.setBoundingBox(bboxNat[0], bboxNat[1], bboxNat[2],bboxNat[3]);

	const drain::image::TreeSVG & image = imagePanel[svg::IMAGE];
	const drain::Frame2D<int> imageGeometry(image->getWidth(), image->getHeight());
	radarSVG.geoFrame.setGeometry(imageGeometry.width, imageGeometry.height);

	if (radarSVG.geoFrame.isDefined()){
		mout.debug(radarSVG.geoFrame);
	}
	else {
		mout.warn("Failed in initializing", DRAIN_LOG(radarSVG.geoFrame));
	}


	return imagePanel;
}
*/

