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
// #include <drain/image/TreeElemUtilsSVG.h>
#include <drain/image/TreeUtilsSVG.h>

//#include "graphics.h"
#include "graphics-panel.h"
#include "graphics-interactive.h"


#include <drain/image/GeoFrame.h>

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

#include "js/image_coord_tracker.h"

drain::image::TreeSVG & addCoordMonitor(drain::image::TreeSVG & textObject, const std::string & cls){

	textObject->setType(svg::TEXT); // ensure
	textObject->setAlign(AlignSVG::RIGHT);
	textObject->setFontSize(15,18);
	//textObject->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);
	textObject->addClass(RackSVG::ElemClass::SELECTOR);

	drain::image::TreeSVG & coordDisplay = textObject[cls](svg::TSPAN);
	coordDisplay->addClass(cls);
	//coordDisplay->setText("");
	return coordDisplay;
	// return imagePanelGroup;
}

/// Designed for CmdRect, could be multi-purpose
TreeSVG & CmdRect::prepare(RackContext & ctx, RadarSVG & radarSVG) const {

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	drain::image::TreeSVG & alignedGroup = RackSVG::getCurrentAlignedGroup(ctx);

	TreeSVG & imagePanel = alignedGroup[ctx.currentImagePanel];
	if (imagePanel->isUndefined()){
		mout.warn("currentImagePanel created but undefined");
	}

	if (!imagePanel.hasChild(svg::IMAGE)){
		mout.warn("currentImagePanel created contains no image");
	}

	drain::image::TreeSVG & mouseGroup = imagePanel[RackSVG::ElemClass::MOUSE];
	mouseGroup.addChild()->setComment("Mouse tracker modified by ", getName(), ' ', getLastParameters());

	drain::image::NodeSVG::map_t & attr = mouseGroup->getAttributes();

	std::vector<double> bboxNat;
	attr["data-bbox"].toSequence(bboxNat, ',');
	if (bboxNat.empty()){
		mout.warn("Could not attach coordinate monitor for imagePanel '", imagePanel->getId(), "' - missing 'data-bbox' attribute ");
	}
	else if (bboxNat.size() != 4){
		mout.warn("Image panel #", imagePanel->getId(), ": odd size of elements in 'data-bbox' = ", attr["data-bbox"], " => ", drain::sprinter(bboxNat));
	}

	mout.special("Attaching coordinate monitor to imagePanel: ", imagePanel->getId());
	// mouseGroup[svg::TITLE](svg::TITLE)->setText("Mouse tracker by ", getName(), ' ', getLastParameters());


	int EPSG = attr["data-epsg"];
	if (EPSG > 0){
		mout.info("Projection: using EPSG=", EPSG);
		radarSVG.geoFrame.setProjectionEPSG(EPSG);
	}
	else {
		mout.hint("Projection: EPSG not detected, consider to set/unset fixedAEQD");
		std::string projdef = ctx.getStatus("where:projdef", !fixedAEQD);
		if (projdef.empty()){
			mout.warn(drain::sprinter(attr));
			mout.warn("No valid 'epsg' attribute value, and 'where:projdef' empty.");
		}
		radarSVG.geoFrame.setProjection(projdef);
		mout.info("Projection: EPSG not detected, using projdef; ", projdef);
		mouseGroup.addChild()->setComment("Projection: EPSG not detected, using projdef='", projdef,"'");
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

void GeoFrame_convert(const drain::image::GeoFrame & geo, const std::vector<double> &bb, drain::Unit unit, drain::Rectangle<double> &bboxNat, drain::Rectangle<int> &bboxImg){

	drain::Logger mout(__FILE__, __FUNCTION__);

	switch (unit) {
	case drain::Unit::DEGREE:
		if (geo.isLongLat()){
			// Metric coordinates undefined.
			geo.deg2pix(bb[0], bb[1], bboxImg.lowerLeft.x, bboxImg.lowerLeft.y);
			geo.deg2pix(bb[2], bb[3], bboxImg.upperRight.x, bboxImg.upperRight.y);
			// monitorMove->setText(bbox);
			// This could be enough also for METRIC projection, but...
		}
		else {
			// ... here we get the METRIC BBOX without extra computation:
			geo.deg2m(bb[0], bb[1], bboxNat.lowerLeft.x, bboxNat.lowerLeft.y);
			geo.deg2m(bb[2], bb[3], bboxNat.upperRight.x, bboxNat.upperRight.y);
			geo.m2pix(bboxNat.lowerLeft, bboxImg.lowerLeft);
			geo.m2pix(bboxNat.upperRight, bboxImg.upperRight);
			// monitorMove->setText(bb, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "m)");
		}
		break;
	case drain::Unit::METRE:
		if (!geo.isLongLat()){
			geo.m2pix(bb[0], bb[1], bboxImg.lowerLeft.x, bboxImg.lowerLeft.y);
			geo.m2pix(bb[2], bb[3], bboxImg.upperRight.x, bboxImg.upperRight.y);
			geo.m2deg(bb[0], bb[1], bboxNat.lowerLeft.x,  bboxNat.lowerLeft.y);
			geo.m2deg(bb[2], bb[3], bboxNat.upperRight.x, bboxNat.upperRight.y);
			// monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "deg)");
		}
		else {
			mout.error("Cannot set metric BBOX (", drain::sprinter(bb), ") for long-lat projection: ", geo.getProjStr());
		}
		break;
	case drain::Unit::UNDEFINED:
		// consder: develop drain::BBox::isMetric(bb[0], bb[1])
		unit = drain::Unit::PIXEL;
		// no break;
	case drain::Unit::PIXEL:
		bboxImg.set(bb[0], bb[1], bb[2], bb[3]);
		break;
	default:
		mout.error("unknown unit: ", unit); // or trust false above?
		break;
	}
	mout.attention(DRAIN_LOG(bboxImg));

}

drain::Unit GeoFrame_convert(const drain::image::GeoFrame & geo, const std::string & bbox, drain::Rectangle<double> &bboxNat, drain::Rectangle<int> &bboxImg){

	drain::Logger mout(__FILE__, __FUNCTION__);

	std::vector<double> bb;
	drain::StringTools::split(bbox, bb, ',');

	drain::Unit unit = drain::Units::extract<drain::Unit>(bbox);
	if (unit == drain::Unit::UNDEFINED){
		unit = drain::Unit::PIXEL;
	}
	// mout.attention(drain::sprinter(bb), ' ', DRAIN_LOG(unit));
	GeoFrame_convert(geo, bb, unit, bboxNat, bboxImg);
	return unit;
}



void CmdRect::exec() const {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	if (ctx.currentImagePanel.empty()){
		mout.advice("write PNG file before calling ", getName());
		mout.warn("currentImagePanel empty");
		return;
	}

	// Modify SVG header
	TreeSVG & myJS = drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_coord_tracker");
	myJS = image_coord_tracker;

	TreeSVG & onloadJS = RackSVG::getOnLoadScript(ctx);
	onloadJS["image_coord_tracker"] = drain::StringBuilder<>("image_coord_tracker();"); // Original reason for this?


	RadarSVG radarSVG;
	TreeSVG & imagePanel = prepare(ctx, radarSVG);

	drain::image::TreeSVG & mouseGroup = imagePanel[RackSVG::ElemClass::MOUSE];

	/*
	TreeSVG & monitorStyle = drain::UtilsXML::ensureStyle(ctx.svgTrack, "MONITOR", {
			{"fill", "white"},
			{"stroke", "none"},
			{"font-size", "14px"},
	});
	*/

	drain::image::TreeSVG & visualGroup = mouseGroup[RackSVG::ElemClass::SELECTOR](svg::GROUP);
	visualGroup->addClass(LayoutSVG::FIXED); // check
	visualGroup->addClass(RackSVG::ElemClass::MONITOR); // handles visualGroup->setStyle("visibility", ...);
	visualGroup.addChild()->setComment("Visualisation of the selection");
	// Reserve slot (implemented below).
	drain::image::TreeSVG & reserved = visualGroup[RackSVG::ElemClass::SELECTOR](svg::RECT);
	reserved->setLocation(10,20);
	reserved->setFrame(30,40);

	TreeSVG & coordMoveDisplay = visualGroup[RackSVG::ElemClass::MONITOR];
	coordMoveDisplay->addClass(RackSVG::ElemClass::SELECTOR);

	TreeSVG & monitorMove = addCoordMonitor(coordMoveDisplay, "MONITOR_MOVE");
	monitorMove->setText('.');

	/* This does not work
	coordMoveDisplay->setMyAlignAnchor(AnchorElem::PREVIOUS);
	coordMoveDisplay->setAlign(AlignSVG::BOTTOM);
	coordMoveDisplay->setAlign(AlignSVG::LEFT);
	// coordMoveDisplay->setAlign(AlignSVG::TOP, AlignSVG::LEFT);
	*/
	coordMoveDisplay->resetAlign();
	coordMoveDisplay->addClass(LayoutSVG::FIXED);
	coordMoveDisplay->setLocation(10,20);

	// mout.attention(DRAIN_LOG(this->bbox));

	/*
	std::vector<double> bb;
	drain::StringTools::split(this->bbox, bb, ',');
	*/
	// mout.attention(drain::sprinter(bb));


	// MAIN
	// if (bb.size() == 4){
	// Draw rectangle (RECT)
	/*
	drain::Unit unit = drain::Units::extract<drain::Unit>(this->bbox);
	mout.attention(drain::sprinter(bb), ' ', DRAIN_LOG(unit));
	*/
	drain::Rectangle<double> bboxNat;
	drain::Rectangle<int> bboxImg;

	drain::Unit unit = radarSVG.geoFrame.convert(this->bbox, bboxNat, bboxImg);

	if (bboxImg.empty()){
		mout.warn("Bounding box empty: ", bbox, " -> ", bboxImg);
	}

	switch (unit) {
	case drain::Unit::DEGREE:

		if (radarSVG.geoFrame.isLongLat()){
			// Metric coordinates undefined.
			/// radarSVG.geoFrame.deg2pix(bb[0], bb[1], bboxImg.lowerLeft.x, bboxImg.lowerLeft.y);
			/// radarSVG.geoFrame.deg2pix(bb[2], bb[3], bboxImg.upperRight.x, bboxImg.upperRight.y);
			monitorMove->setText(this->bbox);
			// This could be enough also for METRIC projection, but...
		}
		else {
			// ... here we get the METRIC BBOX without extra computation:
			/// radarSVG.geoFrame.deg2m(bb[0], bb[1], bboxNat.lowerLeft.x, bboxNat.lowerLeft.y);
			/// radarSVG.geoFrame.deg2m(bb[2], bb[3], bboxNat.upperRight.x, bboxNat.upperRight.y);
			/// radarSVG.geoFrame.m2pix(bboxNat.lowerLeft, bboxImg.lowerLeft);
			/// radarSVG.geoFrame.m2pix(bboxNat.upperRight, bboxImg.upperRight);
			monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "m)");
		}
		break;
	case drain::Unit::METRE:
		if (!radarSVG.geoFrame.isLongLat()){
			/// radarSVG.geoFrame.m2pix(bb[0], bb[1], bboxImg.lowerLeft.x, bboxImg.lowerLeft.y);
			/// radarSVG.geoFrame.m2pix(bb[2], bb[3], bboxImg.upperRight.x, bboxImg.upperRight.y);
			/// radarSVG.geoFrame.m2deg(bb[0], bb[1], bboxNat.lowerLeft.x,  bboxNat.lowerLeft.y);
			/// radarSVG.geoFrame.m2deg(bb[2], bb[3], bboxNat.upperRight.x, bboxNat.upperRight.y);
			monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "deg)");
		}
		else {
			mout.error("Cannot set metric BBOX (", this->bbox, ") for long-lat projection: ", radarSVG.geoFrame.getProjStr());
		}
		break;
	case drain::Unit::UNDEFINED:
		// consder: develop drain::BBox::isMetric(bb[0], bb[1])
		// unit = drain::Unit::PIXEL;
		// no break;
	case drain::Unit::PIXEL:
		// bboxImg.set(bb[0], bb[1], bb[2], bb[3]);
		break;
	default:
		mout.error("unknown unit: ", this->bbox); // or trust false above?
		break;
	}
	mout.attention(DRAIN_LOG(bboxImg));


	// TODO: gRectangle -> RECTANGLE (?)
	drain::image::TreeSVG & selectRect = visualGroup[RackSVG::ElemClass::SELECTOR](svg::RECT);
	selectRect->addClass(RackSVG::ElemClass::SELECTOR, LayoutSVG::FIXED);
	// drain::UtilsXML::ensureStyle(ctx.svgTrack, RackSVG::ElemClass::SELECTOR, {
	selectRect->setLocation(std::min(bboxImg.lowerLeft.x, bboxImg.upperRight.x), std::min(bboxImg.lowerLeft.y, bboxImg.upperRight.y));
	selectRect->setFrame(::abs(bboxImg.getWidth()), ::abs(bboxImg.getHeight()));

	TreeSVG & coordSpanDisplay = visualGroup["MONITOR_BOX"];
	coordSpanDisplay->addClass("MONITOR_BOX"); // From JS
	coordSpanDisplay->addClass(RackSVG::ElemClass::SELECTOR);
	coordSpanDisplay->setMyAlignAnchor<AlignBase::Axis::VERT>(drain::image::AnchorElem::PREVIOUS);
	coordSpanDisplay->setAlign(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);

	TreeSVG & monitorDown = addCoordMonitor(coordSpanDisplay, "MONITOR_DOWN");
	coordSpanDisplay.addChild("COMMA")(svg::TSPAN) = ",";
	TreeSVG & monitorUp   = addCoordMonitor(coordSpanDisplay, "MONITOR_UP");
	monitorDown->setText(bboxImg.lowerLeft);
	monitorUp->setText(bboxImg.upperRight, " px");

	// }


	// Experimental
	RackSVG::addJavaScripsDef(ctx, "MONITOR2_BOX", "cls");

	/// Create the actual another plane (RECT) to receive mouse events
	drain::image::TreeSVG & coordTracker = mouseGroup[RackSVG::ElemClass::MOUSE_TRACKER](svg::RECT);
	coordTracker->addClass(LayoutSVG::FIXED);
	coordTracker->setFrame(radarSVG.geoFrame.getFrameWidth(), radarSVG.geoFrame.getFrameHeight());
	coordTracker->set("data-resolution", resolution.tuple());
	coordTracker->addClass(RackSVG::ElemClass::MOUSE_TRACKER);
	coordTracker->setStyle("fill", "yellow"); // TODO: transparent tracker
	coordTracker->setStyle("opacity", 0.1);


	// If mask...
	if (MASK && false){
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		//drain::image::TreeSVG & mask =
		MaskerSVG::createMask(ctx.svgTrack, mouseGroup, w, h, coordTracker);
	}


}


void CmdCoords::exec() const {

	using namespace drain::image;
	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// Modify SVG header
	// drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "add_mouse_listeners") = add_mouse_listeners;
	TreeSVG & myJS = drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_coord_tracker");
	//myJS.addChild() = image_coord_tracker;
	myJS = image_coord_tracker;
	//myJS.addChild() = "var KOE=1.0;";

	TreeSVG & onloadJS = RackSVG::getOnLoadScript(ctx);
	onloadJS["image_coord_tracker"] = drain::StringBuilder<>("image_coord_tracker();");


	RadarSVG radarSVG;
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

	drain::image::TreeSVG & coordDisplay = imagePanelGroup["MONITOR"];
	addCoordMonitor(coordDisplay, "MONITOR_MOVE");
	drain::ClassXML clsTest;
	clsTest = "kks";
	coordDisplay->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordDisplay->setAlign(AlignSVG::TOP);
	// coordDisplay->setAlign(AlignSVG::RIGHT);
	drain::image::TreeSVG & coordSpanDisplay = imagePanelGroup["MONITOR_RECT"];
	coordSpanDisplay->setMyAlignAnchor<AlignBase::Axis::VERT>(drain::image::AnchorElem::PREVIOUS);
	coordSpanDisplay->setAlign(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
	addCoordMonitor(coordSpanDisplay, "MONITOR_DOWN");
	coordSpanDisplay.addChild("COMMA")(svg::TSPAN) = ",";
	addCoordMonitor(coordSpanDisplay, "MONITOR_UP");

	drain::image::TreeSVG & selectRect = imagePanelGroup[RackSVG::ElemClass::SELECTOR](svg::RECT);
	selectRect->addClass(RackSVG::ElemClass::SELECTOR, LayoutSVG::FIXED);
	/*
	drain::UtilsXML::ensureStyle(ctx.svgTrack, RackSVG::ElemClass::SELECTOR, {
			{"fill", "none"},
			{"stroke", "darkgreen"},
			{"stroke-width", 2.0},
			// {"opacity", 0.5}
	});
	*/
	selectRect->setLocation(200,100);
	selectRect->setFrame(400,200);

	/// Create yet another plane (RECT) to receive mouse events
	drain::image::TreeSVG & coordTracker = imagePanelGroup[RackSVG::ElemClass::MOUSE_TRACKER](svg::RECT);
	coordTracker->setMyAlignAnchor(svg::IMAGE);
	coordTracker->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	/// Todo: move/share/generalize
	coordTracker->set("data-bbox", bbox);
	coordTracker->set("data-epsg", overlayGroup->get("data-epsg",""));
	coordTracker->set("data-resolution", resolution.tuple());
	coordTracker->addClass(RackSVG::ElemClass::MOUSE_TRACKER);

	coordTracker->setStyle("fill", "white"); // TODO: transparent tracker
	// coordTracker->setStyle("stroke", "green");
	coordTracker->setStyle("opacity", 0.0001);

	// If mask...
	if (MASK && false){
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		//drain::image::TreeSVG & mask =
		MaskerSVG::createMask(ctx.svgTrack, imagePanelGroup, w, h, coordTracker);
	}


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

	if (bits = 8){
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<int>(*it) - bias;
			*rit = (value>>8) & 0xff;
			*git =  value     & 0xff;
			++rit;
			++git;
		}
	}
	else if (bits = 16){
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

	const Image & data = ctx.getCurrentGrayImage();

	if (data.isEmpty()){
		mout.warn("Could not find data for image (getCurrentGrayImage() failed)");
		return;
	}
	// mout.attention(data);

	/// Ensure the script is available.
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_value_tracker") = image_value_tracker;

	/// Add listeners upon document loading
	TreeSVG & onloadJS = RackSVG::getOnLoadScript(ctx);
	onloadJS["image_value_tracker"] = "image_value_tracker();";


	drain::image::TreeSVG & imagePanelGroup = RackSVG::getImagePanelGroup(ctx);
	// Add marker for 'image_value_tracker()'
	imagePanelGroup->addClass("MOUSE_VALUE");

	drain::image::TreeSVG & dataImageElem = imagePanelGroup[RackSVG::ElemClass::DATA_ARRAY](svg::IMAGE);
	dataImageElem->addClass("MOUSE_VALUE_DATA");  // RackSVG::ElemClass::MOUSE_TRACKER consider shared tracker plane!
	dataImageElem->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	dataImageElem->addClass(RackSVG::ElemClass::DATA_ARRAY);
	drain::UtilsXML::ensureStyle(ctx.svgTrack, RackSVG::ElemClass::DATA_ARRAY, {
			{"opacity", 0.0},  // some browsers disable mouse listener, if fully invisible?
	});
	addGeoData(data, dataImageElem);


	drain::image::TreeSVG & coordMonitor = imagePanelGroup["MOUSE_COORD"](svg::TEXT);
	coordMonitor->setId();
	coordMonitor->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordMonitor->setAlign(AlignSVG::RIGHT, AlignSVG::BOTTOM);
	coordMonitor->addClass("COORD_MONITOR");
	coordMonitor->addClass(RackSVG::ElemClass::IMAGE_TITLE); // , RackSVG::ElemClass::TIME); // check
	coordMonitor->setTextSafe("(x,y)");
	coordMonitor->setFontSize(15,20);

	drain::image::TreeSVG & valueMonitor = imagePanelGroup["MOUSE_VALUE"](svg::TEXT);
	valueMonitor->setId();
	valueMonitor->setMyAlignAnchor("MOUSE_COORD");
	valueMonitor->setAlign(AlignSVG::RIGHT);
	valueMonitor->setAlign(AlignSVG::TOP, MutualAlign::OUTSIDE);
	valueMonitor->addClass("VALUE_MONITOR");
	// Associate with graphical "location" style
	valueMonitor->addClass(RackSVG::ElemClass::IMAGE_TITLE); // , RackSVG::ElemClass::LOCATION);
	valueMonitor->setTextSafe("(value)");
	valueMonitor->setFontSize(15,20);

	/*
	TreeSVG & coordElem = group[RackSVG::ElemClass::LOCATION](svg::TEXT);
		locationHeader->addClass(elemClass, RackSVG::ElemClass::LOCATION);
		locationHeader->addClass(LayoutSVG::NEUTRAL); // testing LayoutSVG::NEUTRAL
		locationHeader->setMyAlignAnchor(anchor);
	*/


	// ???


	// Construct actual data and save it.

	const std::string filenameFinal = ctx.getFormattedStatus(std::string("${outputPrefix}")+filename);
	mout.special(DRAIN_LOG(filenameFinal));
	dataImageElem->setUrl(filenameFinal);

	drain::image::ImageT<uint8_t> dataImage;


	dataImage.setGeometry(data.getWidth(), data.getHeight(), 3);
	drain::image::Channel & red   = dataImage.getChannel(0);
	drain::image::Channel & green = dataImage.getChannel(1);
	//drain::image::Channel & blue  = dataImage.getChannel(2);

	const std::type_info & type = data.getType();

	// const int bias = std::numeric_limits<T>::min();
	const int bias = drain::Type::call<drain::typeMin, int>(type);
	const int bits = 8 * drain::Type::call<drain::sizeGetter>(type);
	// const int bits = 8 * sizeof(T);
	int value;
	mout.attention(DRAIN_LOG(bits), ' ', DRAIN_LOG(bias));

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

	return;

	//drain::image::FilePng::write(data, ctx.outputPrefix + "gray.png");

	/*

	if (type == typeid(unsigned char)){

		typedef unsigned char T;
		const T bias = std::numeric_limits<T>::min();
		T value;
		drain::image::Channel::iterator rit = red.begin();
		drain::image::Channel::iterator git = green.begin();
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<T>(*it) - bias;
			*rit =  value     & 0xff;
			// *git = (value>>8) & 0xff;
			++rit;
			// ++git;
		}

		drain::image::FilePng::write(dataImage, filenameFinal);

	}
	else if (type == typeid(unsigned short int)){
		drain::image::Channel::iterator rit = red.begin();
		drain::image::Channel::iterator git = green.begin();
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			*rit = (static_cast<unsigned short int>(*it))    & 0xff;
			*git = (static_cast<unsigned short int>(*it)>>8) & 0xff;
			++rit;
			++git;
		}

		drain::image::FilePng::write(dataImage, filenameFinal);

	}
	else if (type == typeid(signed short int)){

		mout.unimplemented<LOG_WARNING>("unscaled handling of signed type:", drain::Type::call<drain::simpleName>(type));
		typedef signed short int T;
		const T bias = std::numeric_limits<T>::min();
		T value;
		drain::image::Channel::iterator rit = red.begin();
		drain::image::Channel::iterator git = green.begin();
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			value = static_cast<T>(*it) - bias;
			*rit =  value     & 0xff;
			*git = (value>>8) & 0xff;
			++rit;
			++git;
		}

		drain::image::FilePng::write(dataImage, filenameFinal);

	}
	else {
		mout.unimplemented<LOG_ERR>("type:", drain::Type::call<drain::simpleName>(type));
	}
	*/


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
	drain::image::TreeSVG & imagePanelGroup = RackSVG::getImagePanelGroup(ctx);
	drain::image::TreeSVG & img = imagePanelGroup[svg::IMAGE];

	/// Ensure that the script is available.
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "base64ToArrayLE") = base64ToArrayLE;

	// TEST 1: float vector
	const std::vector<float> floatVector(11*11, 1.2345);
	test(img, floatVector, mout);

	// TEST 2: short unsigned vector
	const int N = 64;
	std::vector<uint16_t> uint16Vector(N*N);
	for (int j=0; j<N; ++j){
		for (int i=0; i<N; ++i){
			uint16Vector[j*N+i] = j*N+i;
		}
	}
	test(img, uint16Vector, mout);


	TreeSVG & onloadJS = RackSVG::getOnLoadScript(ctx);
	// onloadJS["set_image_value_tracker"] = "set_image_value_tracker();";
	onloadJS["demo_base64"] = "demo_base64();";


}


} // namespace rack

