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
	textObject->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);

	drain::image::TreeSVG & coordDisplay = textObject[cls](svg::TSPAN);
	coordDisplay->addClass(cls);
	//coordDisplay->setText("");
	return coordDisplay;
	// return imagePanelGroup;
}

void CmdRect::exec() const {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// Modify SVG header
	TreeSVG & myJS = drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_coord_tracker");
	myJS = image_coord_tracker;

	TreeSVG & onloadJS = RackSVG::getOnLoadScript(ctx);
	onloadJS["image_coord_tracker"] = drain::StringBuilder<>("image_coord_tracker();");


	RadarSVG radarSVG;
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG); // ensure BBOX + track class
	overlayGroup->setId(); // visible
	std::string bboxGeo = overlayGroup->get("data-bbox","");
	if (!bboxGeo.empty()){
		mout.special("Attaching coordinate monitor to overlaygroup #", overlayGroup->getId());
	}
	else {
		mout.warn("Could not attach coordinate monitor - overlaygroup #", overlayGroup->getId(), "has no 'data-bbox' attribute ");
	}


	drain::image::TreeSVG & imagePanelGroup = RackSVG::getImagePanelGroup(ctx);
	imagePanelGroup->addClass(RackSVG::ElemClass::MOUSE); //  COORD_TRACKER

	/*
	TreeSVG & monitorStyle = drain::UtilsXML::ensureStyle(ctx.svgTrack, "MONITOR", {
			{"fill", "white"},
			{"stroke", "none"},
			{"font-size", "14px"},
	});
	*/


	// REMOVE this! (why)
	TreeSVG & coordDisplay = imagePanelGroup["MONITOR"];
	coordDisplay->addClass(RackSVG::ElemClass::SELECTOR);
	coordDisplay->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordDisplay->setAlign(AlignSVG::TOP);
	TreeSVG & monitorMove = addCoordMonitor(coordDisplay, "MONITOR_MOVE");
	monitorMove->setText('.');

	mout.attention(DRAIN_LOG(this->bbox));

	std::vector<double> bb;
	drain::StringTools::split(this->bbox, bb, ',');
	mout.attention(drain::sprinter(bb));

	drain::Unit unit = drain::Units::extract<drain::Unit>(this->bbox);
	mout.attention(DRAIN_LOG(unit));
	mout.attention("Unit=", unit);


	if (bb.size() == 4){
		// Draw rectangle (RECT)

		drain::Rectangle<double> bboxNat;
		drain::Rectangle<int> bboxImg;

		switch (unit) {
		case drain::Unit::DEGREE:
			if (radarSVG.geoFrame.isLongLat()){
				// Metric coordinates undefined.
				radarSVG.geoFrame.deg2pix(bb[0], bb[1], bboxImg.lowerLeft.x, bboxImg.lowerLeft.y);
				radarSVG.geoFrame.deg2pix(bb[2], bb[3], bboxImg.upperRight.x, bboxImg.upperRight.y);
				monitorMove->setText(this->bbox);
				// This could be enough also for METRIC projection, but...
			}
			else {
				// ... here we get the METRIC BBOX without extra computation:
				radarSVG.geoFrame.deg2m(bb[0], bb[1], bboxNat.lowerLeft.x, bboxNat.lowerLeft.y);
				radarSVG.geoFrame.deg2m(bb[2], bb[3], bboxNat.upperRight.x, bboxNat.upperRight.y);
				radarSVG.geoFrame.m2pix(bboxNat.lowerLeft, bboxImg.lowerLeft);
				radarSVG.geoFrame.m2pix(bboxNat.upperRight, bboxImg.upperRight);
				monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "m)");
			}
			break;
		case drain::Unit::METRE:
			if (!radarSVG.geoFrame.isLongLat()){
				radarSVG.geoFrame.m2pix(bb[0], bb[1], bboxImg.lowerLeft.x, bboxImg.lowerLeft.y);
				radarSVG.geoFrame.m2pix(bb[2], bb[3], bboxImg.upperRight.x, bboxImg.upperRight.y);
				radarSVG.geoFrame.m2deg(bb[0], bb[1], bboxNat.lowerLeft.x,  bboxNat.lowerLeft.y);
				radarSVG.geoFrame.m2deg(bb[2], bb[3], bboxNat.upperRight.x, bboxNat.upperRight.y);
				monitorMove->setText(this->bbox, " (", bboxNat.lowerLeft, ',', bboxNat.upperRight, "deg)");
			}
			else {
				mout.error("Cannot set metric BBOX (", this->bbox, ") for long-lat projection: ", radarSVG.geoFrame.getProjStr());
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
			mout.error("unknown unit: ", this->bbox); // or trust false above?
			break;
		}
		mout.attention(DRAIN_LOG(bboxImg));


		// TODO: gRectangle -> RECTANGLE
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
		selectRect->setLocation(std::min(bboxImg.lowerLeft.x, bboxImg.upperRight.x), std::min(bboxImg.lowerLeft.y, bboxImg.upperRight.y));
		selectRect->setFrame(::abs(bboxImg.getWidth()), ::abs(bboxImg.getHeight()));

		TreeSVG & coordSpanDisplay = imagePanelGroup["MONITOR_BOX"];
		coordSpanDisplay->addClass("MONITOR_BOX"); // From JS
		coordSpanDisplay->addClass(RackSVG::ElemClass::SELECTOR);
		coordSpanDisplay->setMyAlignAnchor<AlignBase::Axis::VERT>(drain::image::AnchorElem::PREVIOUS);
		coordSpanDisplay->setAlign(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
		TreeSVG & monitorDown = addCoordMonitor(coordSpanDisplay, "MONITOR_DOWN");
		coordSpanDisplay.addChild("COMMA")(svg::TSPAN) = ",";
		TreeSVG & monitorUp   = addCoordMonitor(coordSpanDisplay, "MONITOR_UP");

		monitorDown->setText(bboxImg.lowerLeft);
		monitorUp->setText(bboxImg.upperRight, " px");

	}


	// Experimental
	RackSVG::addJavaScripsDef(ctx, "MONITOR2_BOX", "cls");

	/// Create yet another plane (RECT) to receive mouse events
	drain::image::TreeSVG & coordTracker = imagePanelGroup[RackSVG::ElemClass::MOUSE_TRACKER](svg::RECT);
	coordTracker->setMyAlignAnchor(svg::IMAGE);
	coordTracker->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	/// Todo: move/share/generalize
	coordTracker->set("data-bbox", bboxGeo);
	coordTracker->set("data-epsg", overlayGroup->get("data-epsg",""));
	coordTracker->set("data-resolution", resolution.tuple());
	coordTracker->addClass(RackSVG::ElemClass::MOUSE_TRACKER);
	coordTracker->setStyle("fill", "white"); // TODO: transparent tracker
	coordTracker->setStyle("opacity", 0.0001);

	// If mask...
	if (MASK && false){
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		//drain::image::TreeSVG & mask =
		MaskerSVG::createMask(ctx.svgTrack, imagePanelGroup, w, h, coordTracker);
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

void CmdData::exec() const {

	using namespace drain::image;
	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, getName(), __FUNCTION__, __LINE__);

	const drain::image::Image & data = ctx.getCurrentGrayImage();

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
			{"opacity", 0.15},
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

	drain::image::ImageT<uint8_t> dataImage;
	dataImage.setGeometry(data.getWidth(), data.getHeight(), 3);
	drain::image::Channel & red   = dataImage.getChannel(0);
	drain::image::Channel & green = dataImage.getChannel(1);
	//drain::image::Channel & blue  = dataImage.getChannel(2);

	//drain::image::FilePng::write(data, ctx.outputPrefix + "gray.png");
	const std::string filenameFinal = ctx.getFormattedStatus(std::string("${outputPrefix}")+filename);
	mout.special(DRAIN_LOG(filenameFinal));
	dataImageElem->setUrl(filenameFinal);

	const std::type_info & type = data.getType();
	if (type == typeid(unsigned short)){
		//const size_t s = data.getArea();
		drain::image::Channel::iterator rit = red.begin();
		drain::image::Channel::iterator git = green.begin();
		for (drain::image::Image::const_iterator it=data.begin(); it!=data.end(); ++it){
			*rit = (static_cast<unsigned int>(*it))    & 0xff;
			*git = (static_cast<unsigned int>(*it)>>8) & 0xff;
			++rit;
			++git;
		}

		drain::image::FilePng::write(dataImage, filenameFinal);

	}
	else {
		mout.unimplemented<LOG_ERR>("type:", drain::Type::call<drain::simpleName>(type));
	}



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

