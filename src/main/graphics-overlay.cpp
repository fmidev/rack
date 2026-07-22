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
#include <drain/image/TreeElemUtilsSVG.h>
#include <drain/image/TreeUtilsSVG.h>

#include "graphics.h"
#include "graphics-panel.h"
#include "graphics-radar.h"
#include "graphics-overlay.h"
#include "graphics-imagepanel.h"


namespace drain {


}


namespace rack {

/*
class VectorFrame : protected ImagePanel {
public:

	inline
	VectorFrame(drain::image::TreeSVG & imagePanelGroup) : ImagePanel(imagePanelGroup){

	};

	drain::image::TreeSVG & getVectorGroup() const;


};

drain::image::TreeSVG & VectorFrame::getVectorGroup(const std::string key = "") const {
	drain::image::TreeSVG & overlay = getOverlay();

	if (key.empty()){
		std::string k;
		drain::image::TreeSVG::generateKey(overlay, k);
		return getVectorGroup(k);
	}


};
*/

const std::string CmdPolarBase::DATA_ID = "data-latest";


/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
 */

// New.. Keep this?
void CmdPolarBase::updateRadarSVG(RackContext & ctx, RadarSVG & radarSVG){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	const Hi5Tree & srcPolar = ctx.getHi5(RackContext::POLAR|RackContext::CURRENT); // , RackContext::CURRENT
	const Hi5Tree & srcCurr  = ctx.getHi5(RackContext::CURRENT); // this works well with -c (--cCreate)

	if (!srcPolar.empty()){
		//radarSVG.deriveMaxRange(srcPolar);
		radarSVG.radarProj.deriveMaxRange(srcPolar);
		//mout.debug(DRAIN_LOG(radarSVG.maxRange));
	}
	else {
		mout.warn("No polar data read, cannot focus on a specific radar");
		// return;
	}

	const drain::VariableMap & wherePolar = srcPolar[ODIMPathElem::WHERE].data.attributes;

	radarSVG.updateRadarConf(wherePolar); // lon, lat

	// std::string srcDsc = "unknown_composite";

	const drain::Variable & object = srcCurr[ODIMPathElem::WHAT].data.attributes["object"];
	if ((object == "SCAN") || (object == "PVOL")){
		// mout.note("Polar coordinates"); // Cartesian not "found", ie not created this.
		// mout.warn("Current object is not projected (Cartesian) data, cannot focus on a specific radar");
		// srcDsc = srcCurr[ODIMPathElem::WHAT].data.attributes["source"].toStr();
		radarSVG.source = srcCurr[ODIMPathElem::WHAT].data.attributes["source"].toStr();
	}

	// mout.note("Cartesian");
	const Hi5Tree & srcCartesian = ctx.getHi5(RackContext::CARTESIAN);  // ctx.getHi5(RackContext::CURRENT | RackContext::CARTESIAN);

	if (!srcCartesian.empty()){ // HDF5 has been either loaded or extracted.
		mout.note("Using Cartesian product (meta data)");
		//mout.note(DRAIN_LOG(srcCartesion));
		const drain::VariableMap & whereCart = srcCurr[ODIMPathElem::WHERE].data.attributes;
		// mout.note(DRAIN_LOG(whereCart));
		radarSVG.updateCartesianConf(whereCart);
		// Redesign multi-frame mosaick...
		// overlayGroup->set("data-latest",
		// srcDsc =
		radarSVG.source = srcCurr[ODIMPathElem::HOW].data.attributes["nodes"].toStr();
	}
	else {
		Composite & composite = ctx.getComposite(RackContext::Hi5Role::PRIVATE | RackContext::Hi5Role::SHARED); // SHARED?
		mout.debug("Cartesian product not (yet) created, but using specification: ", DRAIN_LOG(composite));
		radarSVG.updateCartesianConf(composite);
	}

	/*
	mout.attention(DRAIN_LOG(radarSVG.geoFrame.getEPSG()));
	mout.attention(DRAIN_LOG(radarSVG.radarProj.getSrc().getEPSG()));
	mout.attention(DRAIN_LOG(radarSVG.radarProj.getDst().getEPSG()));
	mout.accept<LOG_WARNING>(DRAIN_LOG(radarSVG.geoFrame));
	mout.reject<LOG_WARNING>(DRAIN_LOG(radarSVG.radarProj));
	*/

}



double CmdPolarBase::ensureMetricRange(double maxRange, double range){
	if (range < -1.0){
		return range;
	}
	else if (range > +1.0){
		return range;
	}
	else {
		return range*maxRange;
	}
};

/// Given a distance or a distance range, ensure metric (if relative) and also validate generally.
/**
 *
 */
void CmdPolarBase::resolveDistance(const drain::SteppedRange<double> & ownDist, const drain::SteppedRange<double> & sharedDist, drain::SteppedRange<double> & dist, double maxDistance){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (ownDist.range.width() > 0.0){
		mout.info("using specific range: ", ownDist.range.tuple());
		dist.range.set(ownDist.range.tuple());
		dist.step = ownDist.step;
		// warn if .max == ?
	}
	else if (ownDist.range.max > 0.0){
		mout.info("using max distance of specific range: ", ownDist.range.tuple());
		dist.range.set(0.0, ownDist.range.max);
		dist.step =   ownDist.step;
	}
	else if (sharedDist.range.width() > 0.0){
		mout.info("using polarSelector range: ", sharedDist.range.tuple());
		dist.range.set(sharedDist.range.tuple());
		dist.step =    sharedDist.step;
	}
	else if (sharedDist.range.max > 0.0){
		mout.info("using max distance of polarSelector range: ", sharedDist.range.tuple());
		dist.range.set(0.0, sharedDist.range.max);
		dist.step =  sharedDist.step;
	}
	else {
		mout.info("guessing range (0,max)");
		dist.range.set(0.0, 1.0); // min to max
		//dist.step = 50000; // 50 km
	}



	if (dist.range.min<0){ // Relax this later!
		//mout.unimplemented("negative start");
		// mout.error("negative start of radial range ", dist.range.min, " [metres]");
		//mout.unimplemented<LOG_ERR>("negative start of radial range ", dist.range.min, " [metres]");
		//return;
	}

	if (dist.range.width() < 0){
		if (dist.range.max == 0){
			dist.range.max = dist.range.min;
			dist.range.min = 0;
		}
		else {
			mout.error("illegal (reversed) radial range: ", DRAIN_LOG(dist.range));
			return;
		}
	}

	if (maxDistance > 0.0){
		dist.range.min = ensureMetricRange(maxDistance, dist.range.min);
		dist.range.max = ensureMetricRange(maxDistance, dist.range.max);
	}



	if (dist.step == 0.0){
		if (ownDist.step > 0.0){
			dist.step = ownDist.step;
		}
		else if (sharedDist.step > 0.0){
			dist.step = sharedDist.step;
		}
		else {
			dist.step = 50000; // 50 km
		}
	}

	mout.accept(DRAIN_LOG(dist));
	// rescale max?

};


void CmdPolarBase::resolveAzimuthRange(const drain::SteppedRange<double> & ownAzm, const drain::SteppedRange<double> & sharedAzm, drain::SteppedRange<double> & azm){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (ownAzm.range.width() > 0){
		azm.set(ownAzm.tuple());
	}
	else if (sharedAzm.range.width() > 0.0){
		azm.set(sharedAzm.tuple());
	}
	/* else {
			azm.set(30.0, 0.0, 360.0);
		}
	 */

	if (azm.range.min < -360.0){
		mout.error("illegal start of azimuthal range ", azm.range.min, " [degrees]");
	}
	if (azm.range.max > +720.0){
		mout.error("illegal start of azimuthal range ", azm.range.min, " [degrees]");
	}
	if (azm.range.max < azm.range.min){
		mout.error("illegal azimuthal range ", azm.range.tuple(), " [degrees]");
	}

	mout.accept<LOG_NOTICE>(DRAIN_LOG(azm));
};


/// Designed for CmdData, could be multi-purpose
/**
 *   Compare with: prepare() below...
 */
void CmdPolarBase::addGeoData(const drain::image::Image & imageData, drain::image::NodeSVG & node){

	CartesianODIM odim(ODIMPathElem::DATA|ODIMPathElem::ROOT); // Encoding + quantity + geo
	odim.copyFrom(imageData);
	// ODIM odim(ODIMPathElem::DATA); // Encoding + quantity
	// mout.attention(DRAIN_LOG(myOdim));

	node.set("data-epsg", odim.epsg);
	// Currently unused:
	// node.set("data-bbox-deg", odim.bboxD.toStr(','));
	// Currently unused:
	// node.set("data-bbox-nat", imageData.getProperties().get("where:BBOX_native","?"));

	node.set("data-quantity", odim.quantity);
	node.set("data-encoding",
			drain::StringBuilder<','>(odim.type,odim.scaling.scale, odim.scaling.offset, odim.nodata, odim.undetect).str());

}

// --------------------------------------------------------------------------------------------------------



/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
 */
void CmdRadarDot::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);


	// General
	Graphic::getGraphicStyle(ctx.getSVG()); // needed?
	drain::UtilsXML::ensureStyle(ctx.getSVG(), cls, {
			{"fill", "white"},
			{"stroke", "black"},
			{"stroke-width", 2.0},
			// {"opacity", 0.5}
	});

	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG);
	const drain::Frame2D<int> & geom = radarSVG.geoFrame.getGeometry();

	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup();

	// ctx.getStatusMap().get("what:source", "unknown-source");

	ImagePanel superPanel(imagePanelGroup, geom);
	/*
	imagePanelGroup->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	imagePanelGroup->addClass(LayoutSVG::NEUTRAL);
	imagePanelGroup->addClass(LayoutSVG::INDEPENDENT);
	imagePanelGroup->addClass(ClipperSVG::CLIPPED);
	*/

	TreeSVG & overlayGroup = superPanel.getOverlayGroup();

	overlayGroup->set(DATA_ID, radarSVG.source); // needed? changes for each radar?

	// Always a range, though here only dis.max used
	drain::SteppedRange<double> dist(0.0, 0.0, 30.0);
	// Note: DOT does not use shared polar selection
	resolveDistance(radiusMetres, {0.0,0.0}, dist, radarSVG.radarProj.getRange());

	if (dist.range.max <= 0.0){
		mout.warn(DRAIN_LOG(radiusMetres), " => ", DRAIN_LOG(dist));
		mout.warn("Zero or negative distance range, skipping ", getName());
		return;
	}


	// drain::image::TreeUtilsSVG\n

	TreeSVG & vectGroup = superPanel.getVectorOverlayGroup(radarSVG.source);
	//TreeSVG & vectGroup = superPanel.getSourceSpecificGroup(radarSVG.source);
	vectGroup->addClass(GRAPHIC::GRID);
	// RackSVG::getSourceSpecificGroup(ctx, overlayGroup);

	vectGroup->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	vectGroup.addChild()->setComment(getName(), '[', cls, ']', ' ', getParameters(), " for ", radarSVG.source);
	// vectGroup.addChild()->setComment(getName(), ' ', getParameters());
	drain::image::TreeSVG & curve = vectGroup[DOT](drain::image::svg::PATH);
	curve->addClass(DOT);

	{
		// Private scope, to call bezierElem destructor.
		drain::svgPATH bezierElem(curve);
		radarSVG.drawCircle(bezierElem, dist.range);
	}

	drain::image::TreeSVG & title = curve[svg::TITLE](svg::TITLE);
	title->setText(ctx.getFormattedStatus("${NOD} ${what:startdate|%Y/%m/%d} ${what:starttime|%H:%M:%S}"));

	MaskerSVG::MaskPosition pos = drain::Enum<MaskerSVG::MaskPosition>::dict.getValue(MASK, false);
	if (pos != MaskerSVG::MaskPosition::NONE){
		// if (MASK){
		// Note: mask is full 100% range.
		drain::image::TreeSVG & localMask = vectGroup[svg::MASK];
		{
			// Private scope, to call bezierElem destructor.
			drain::svgPATH elem(localMask);
			radarSVG.drawSector(elem, {0, radarSVG.radarProj.getRange()});
		}
		// Copy this localMask to shared mask...
		// const int w = radarSVG.geoFrame.getFrameWidth();
		// const int h = radarSVG.geoFrame.getFrameHeight();
		MaskerSVG::createMask(ctx.getSVG(), overlayGroup, geom.width, geom.height, localMask.data, pos);
		// ... and "delete" the object.
		localMask->setType(svg::COMMENT);
		localMask->setComment("Original position of MASK:", getName(), getParameters());
	}


};

/*
void CmdRadarDotTest::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	RadarSVG radarSVG;
	getOverlayGroup(ctx, radarSVG);


	drain::SteppedRange<double> dist(0.0, 0.0, 30.0);
	// Note: DOT does not (yet) use shared polar selection
	resolveDistance(radiusMetres, {0.0,0.0}, dist, radarSVG.radarProj.getRange());

	if (dist.range.max <= 0.0){
		mout.warn(DRAIN_LOG(radiusMetres), " => ", DRAIN_LOG(dist));
		mout.warn("Zero or negative distance range, skipping ", getName());
		return;
	}

	TreeSVG & imagePanel = RackSVG::getVectorImagePanelGroup(ctx);

	// For each separate, or?
	TreeSVG & vectGroup = imagePanel[DOT](svg::GROUP);


	// drain::image::TreeUtilsSVG\n
	drain::UtilsXML::ensureStyle(ctx.getSVG(), cls, {
			{"fill", "white"},
			{"stroke", "black"},
			{"stroke-width", 2.0},
			// {"opacity", 0.5}
	});

	TreeSVG & curve = vectGroup.addChild()(drain::image::svg::PATH);
	curve->addClass(DOT);
	{
		// Private scope, to call bezierElem destructor.
		drain::svgPATH bezierElem(curve);
		radarSVG.drawCircle(bezierElem, dist.range);
	}

	drain::image::TreeSVG & title = curve[svg::TITLE](svg::TITLE);
	title->setText(ctx.getFormattedStatus("${NOD} ${what:startdate|%Y/%m/%d} ${what:starttime|%H:%M:%S}"));

	if (MASK){

		// Note: mask is full 100% range. Clipper should handle the graphic part.
		drain::image::TreeSVG & tmpMask = imagePanel[MaskerSVG::COVER];
		{
			// Private scope, to call bezierElem destructor.
			drain::svgPATH elem(tmpMask);
			radarSVG.drawSector(elem, {0, radarSVG.radarProj.getRange()});
		}
		// Copy this localMask to shared mask...
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		MaskerSVG::createMask(ctx.getSVG(), imagePanel, w, h, tmpMask.data);
		// ... and "delete" the object.
		tmpMask->setType(svg::COMMENT);
	}


};
*/


void CmdRadarLabel::exec() const  {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// General
	Graphic::getGraphicStyle(ctx.getSVG());

	TreeSVG & style = drain::UtilsXML::ensureStyle(ctx.getSVG(), cls, {
			{"font-size", "12"},
			{"stroke", "black"},
			{"stroke-width", "0.2"},
			{"stroke-opacity", "0.5"},
			{"stroke-width", "0.3em"},
			{"stroke-linejoin", "round"},
			{"fill", "white"}, // debug
			{"fill-opacity", "1"},
			{"paint-order", "stroke"},
	});

	drain::UtilsXML::ensureStyle(ctx.getSVG(), "DEBUG", { // ?
			//{"font-size", "12"},
			{"fill", "white"},
			{"fill-opacity", 0.1},
			{"stroke", "red"},  // replace these with image-title etc soft transit
			{"stroke-width", "2"},
	});


	/// Step 1: initialize radarSVG
	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG);

	// getOverlayGroup(ctx, radarSVG);
	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup();

	ImagePanel superPanel(imagePanelGroup);

	TreeSVG & vectGroup = superPanel.getOverlayGroup();
	vectGroup->addClass(GRAPHIC::GRID);

	if (label.empty()){
		vectGroup.addChild()->setComment(getName(), '[', cls, ']', " - empty label skipped");
		mout.warn("Empty argument for ", getName());
		return;
	}

	// remove!
	std::string s;
	drain::StringTools::getSafeKey(label, s, "_- ");
	vectGroup.addChild()->setComment(getName(), '[', cls, ']', ' ', s);
	// TreeSVG & vectGroup = imagePanel[source](svg::GROUP);
	// TreeSVG & overlay = imagePanel[LABEL](svg::GROUP);


	// TODO: group for all (font size etc)
	drain::Point2D<int> imgPoint;
	radarSVG.convert(0.0, 0.0, imgPoint); // radar center (radius=0, azm=0)

	const std::string LABEL_ANCHOR = "labelAnchor";

	TreeSVG & labelAnchor = vectGroup[LABEL_ANCHOR];
	labelAnchor->addClass(LayoutSVG::GroupType::FIXED);
	labelAnchor->addClass(LayoutSVG::GroupType::NEUTRAL); // IMPORTANT! Else, other elems of the same group (like DOTS) become translated...
	// labelAnchor->addClass("DEBUG");
	// labelAnchor->setMyAlignAnchor("munDOT");
	/*
		drain::svgCIRCLE circle(labelAnchor);
			circle.cx = imgPoint.x;
			circle.cy = imgPoint.y;
			circle.r = 10.0;
			// svgPATH has no "natural" origin...
			// drain::svgPATH bezierElem(labelAnchor);
			// radarSVG.drawCircle(bezierElem, {0.0,15000.0});
	 */
	{
		drain::svgRECT rect(labelAnchor);
		rect.width  = 5.0;
		rect.height = 10.0;
		rect.x = imgPoint.x - 10.0;
		rect.y = imgPoint.y -  5.0;
	}

	/*
		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(label, true); // convert escaped
		// mout.special(DRAIN_LOG(statusFormatter));

		const std::string formattedLabel = statusFormatter.toStr(ctx.getUpdatedStatusMap(), 0, RackContext::variableFormatter); // XXX
	 */
	const std::string formattedLabel = ctx.getFormattedStatus(label);
	// mout.special(DRAIN_LOG(formattedLabel));

	// int fontSize=10;
	mout.attention(drain::sprinter(style->getAttributes()));
	int fontSize = style->get("font-size", 13);
	mout.special(DRAIN_LOG(fontSize));

	std::list<std::string> lines;
	drain::StringTools::split(formattedLabel, lines,'\n');
	labelAnchor->setLocation(imgPoint.x, imgPoint.y - int(fontSize*lines.size())/2);
	labelAnchor->setFrame(0,0);

	for (std::string & line: lines){

		if (line.empty()){
			// TODO: allow empty line
			continue;
		}

		// AlignSVG::HorzAlign alignHorz = drain::image::AlignSVG::CENTER;
		// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz(AlignBase::Pos::MID); // , MutualAlign::INSIDE);
		// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
		// CompleteAlignment<const AlignBase::Axis, AlignBase::Axis::VERT> alignVert(AlignBase::Pos::MAX, MutualAlign::OUTSIDE);

		CompleteAlignment<AlignBase::Axis, AlignBase::Axis::HORZ> alignHorz(AlignSVG::CENTER, MutualAlign::OUTSIDE);  // , MutualAlign::INSIDE);
		CompleteAlignment<AlignBase::Axis, AlignBase::Axis::VERT> alignVert(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);

		std::list<std::string> parts;
		drain::StringTools::split(line, parts, '|');
		switch (parts.size()) {
		case 1:
			alignHorz.pos = AlignBase::Pos::MID;
			break;
		case 2:
			alignHorz.pos = AlignBase::Pos::MIN;
			break;
		default:
			alignHorz.pos = AlignBase::Pos::MIN;
			mout.warn("Text contained several alignment markers '|'");
			break;
		}

		for (const std::string & part: parts){
			if (!part.empty()){

				drain::image::TreeSVG & text = vectGroup.addChild()(drain::image::svg::TEXT);

				text->setFontSize(fontSize, (15*fontSize)/10);
				text->setText(part);
				// text->setFrame(fontSize*2, fontSize);
				text->addClass(this->cls);
				text->addClass(LayoutSVG::GroupType::NEUTRAL);

				text->setMyAlignAnchor<AlignBase::Axis::HORZ>(LABEL_ANCHOR);
				text->setMyAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS); // For the first element, this is "labelAnchor"
				text->setAlign(alignHorz);
				text->setAlign(alignVert);
				alignVert.set(AlignSVG::TOP, MutualAlign::INSIDE);

				text[svg::TITLE](svg::TITLE)->setText(part);
			}
			alignHorz.pos = AlignBase::Pos::MAX;
		}

	}

};




/**
 *
 *  See also: CmdGrid, CmdCartesianGrid
 */


void CmdRadarGrid::exec() const  {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// General
	Graphic::getGraphicStyle(ctx.getSVG());

	/// Step 1: initialize radarSVG
	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG);
	const drain::Frame2D<int> & geom = radarSVG.geoFrame.getGeometry();

	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup(); // (geom); // (ctx, radarSVG);
	ImagePanel superPanel(imagePanelGroup, geom);

	/// Step 2a: check distance parameter
	drain::SteppedRange<double> dist(0.0, 0.0, 1.0);  // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.radarProj.getRange());
	if (dist.range.span() == 0.0){
		mout.error("empty range: ", radiusMetres.range.tuple(), " => ", dist.range.tuple(), " [metres]");
	}
	if (dist.step < 5000){
		dist.step = 5000;
		mout.info("too small distance step, assigned  ", DRAIN_LOG(dist.step));
	}
	else if (dist.step < 10000){
		mout.suspicious("small distance step: ", DRAIN_LOG(dist.step), ", remember to use metres");
	}
	mout.debug("final range: ", DRAIN_LOG(dist.range), DRAIN_LOG(dist.step));

	/// Step 2b: check azimuth parameter
	drain::SteppedRange<double> azm; // double -> int
	resolveAzimuthRange(azimuthDegrees, ctx.polarSelector.azimuth, azm);
	const drain::Range<double> azmRad(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);
	if (azm.step < 5){
		azm.step = 15;
		mout.info("too small azimuthal step, assigned  ", DRAIN_LOG(azm.step));
	}


	TreeSVG & vectorGroup = superPanel.getVectorOverlayGroup(radarSVG.source);
	vectorGroup->addClass(cls); // GRID
	vectorGroup.addChild()->setComment("Rays: ", dist.range.tuple());

	double angleRad = 0.0;
	for (int j=azm.range.min; j<=azm.range.max; j += azm.step){

		angleRad = drain::DEG2RAD * static_cast<double>(j);

		drain::image::TreeSVG & rayNode = vectorGroup.addChild();
		rayNode->addClass(Graphic::HIGHLIGHT);
		rayNode[svg::TITLE](svg::TITLE)->setText(j, drain::XML::DEGREE); // degree sign "&#176;"

		drain::svgPATH rayElem(rayNode);

		radarSVG.moveTo(rayElem, dist.range.min, angleRad);

		// Drawing resolution here (metres). Note: not same as dist.step
		const int step = 10000; // 10 km jumps
		for (int i=dist.range.min+step; i<=dist.range.max; i += step){
			i = std::min(i, static_cast<int>(dist.range.max));
			radarSVG.lineTo(rayElem, static_cast<double>(i), angleRad);
		}

	}


	vectorGroup.addChild()->setComment("Circles/arcs: ", azm.range.tuple(), " - ", azmRad.tuple());

	// double distMin = dist.range.min;
	if (dist.range.min == 0){
		// Dont draw arc in origin...
		dist.range.min += dist.step;
	}

	for (int i=dist.range.min; i<=dist.range.max; i += dist.step){

		drain::image::TreeSVG & g = vectorGroup.addChild()(svg::GROUP);
		g->addClass(Graphic::HIGHLIGHT); //?
		g->addClass(drain::image::LayoutSVG::FIXED); // NEW 2026?
		g->addClass(Graphic::GRID);

		drain::image::TreeSVG & arcNode = g.addChild();
		drain::svgPATH arcElem(arcNode);
		radarSVG.moveTo(arcElem, i, azmRad.min);
		radarSVG.cubicBezierTo(arcElem, i, azmRad.min, azmRad.max);
		// Tool tip
		arcNode[svg::TITLE](svg::TITLE)->setText(i/1000, " kms");
		// radarSVG.close(arcElem);

		// if (true){ // todo switch?
		g.addChild()->setComment("Write kilometres");
		// double angleRad = drain::DEG2RAD * static_cast<double>(azimuthDegrees.range.min);
		drain::Point2D<int> imgPoint;
		int offset = radiusMetres.step/4;
		drain::image::TreeSVG & text = g[svg::TEXT](svg::TEXT);
		// text->addClass(Graphic::GRID); // on top of image, background gray blur applied
		text->addClass(Graphic::HIGHLIGHT);
		radarSVG.convert(i + offset, azmRad.min, imgPoint);
		text->setLocation(imgPoint);
		text->setText(i/1000); // "km - ", imgPoint.tuple());

	}

	MaskerSVG::MaskPosition pos = drain::Enum<MaskerSVG::MaskPosition>::dict.getValue(MASK, false);
	if (pos != MaskerSVG::MaskPosition::NONE){

		TreeSVG & localMask = imagePanelGroup[MaskerSVG::COVER];
		{
			// Private scope, to call bezierElem destructor.
			drain::svgPATH elem(localMask);
			radarSVG.drawCircle(elem, {0,  dist.range.max});
		}

		MaskerSVG::createMask(ctx.getSVG(), superPanel.getOverlayGroup(), geom.width, geom.height, localMask.data, pos);
		localMask->setType(svg::COMMENT); // "delete"

	}


};




void CmdRadarSector::exec() const  {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// General
	Graphic::getGraphicStyle(ctx.getSVG());

	drain::UtilsXML::ensureStyle(ctx.getSVG(), cls, { // SECTOR
			{"fill", "none"},
			// {"stroke", "rgb(160,255,160)"},
			{"stroke-width", 5.0},
			// {"opacity", 0.65}
	});

	/// Step 1: initialize radarSVG
	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG);


	// const drain::Frame2D<int> & geom = radarSVG.geoFrame.getGeometry();
	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup(); // (geom); // (ctx, radarSVG);
	ImagePanel superPanel(imagePanelGroup, radarSVG.geoFrame.getGeometry());

	TreeSVG & vectorGroup = superPanel.getVectorOverlayGroup(radarSVG.source);
	vectorGroup->addClass(Graphic::GRID, cls); // SECTOR
	// vectorGroup->addClass(Graphic::GRID);
	drain::image::TreeSVG & curve = vectorGroup.addChild()(drain::image::svg::PATH);

	// Main
	drain::SteppedRange<double> dist(0,0,1.0); // (distanceMetres.range); // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.radarProj.getRange());

	drain::SteppedRange<double> azm(0.0, 0.0, 0.0); // (distanceMetres.range); // double -> int
	resolveAzimuthRange(azimuthDegrees, ctx.polarSelector.azimuth, azm);
	const drain::Range<double> azmR(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);

	{
		// Sub scope to ensure writing data (bezier curve points)
		drain::svgPATH svgElem(curve);
		radarSVG.drawSector(svgElem, dist.range, azmR);
	}

	// Note: above svgElem clears element
	// curve->addClass(SECTOR);
	if (azmR.empty()){
		if (dist.range.empty()){
			curve->addClass(CIRCLE);
		}
		else {
			curve->addClass(ANNULUS);
		}
	}

	MaskerSVG::MaskPosition pos = drain::Enum<MaskerSVG::MaskPosition>::dict.getValue(MASK, false);
	if (pos){
		// In this case, the mask shape is equal to the original elshape.
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		MaskerSVG::createMask(ctx.getSVG(), superPanel.getOverlayGroup(), w, h, curve.data, pos);
	}



};





void CmdRadarRay::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// RadarSVG radarSVG;
	// drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	// drain::image::TreeSVG & overlay = getOverlay(overlayGroup);
	// overlay.addChild()->setComment(getName(), ' ', getParameters());
	RadarSVG radarSVG;
	updateRadarSVG(ctx, radarSVG);
	TreeSVG & imagePanelGroup = ctx.getImagePanelGroup(); // (ctx, radarSVG);
	ImagePanel superPanel(imagePanelGroup);
	drain::image::TreeSVG & overlay = superPanel.getOverlayGroup();

	drain::image::TreeSVG & curve = overlay[getName()](drain::image::svg::PATH);
	curve->addClass(cls); // SECTOR
	// drain::image::TreeUtilsSVG\n
	drain::UtilsXML::ensureStyle(ctx.getSVG(), cls, { // SECTOR
			{"fill", "none"},
			{"stroke", "rgb(160,255,160)"},
			{"stroke-width", 5.0},
			// {"opacity", 0.65}
	});

	drain::SteppedRange<double> dist; // (distanceMetres.range); // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.radarProj.getRange());

	const double azmR = azimuthDegrees.range.min * drain::DEG2RAD;

	drain::svgPATH rayElem(curve);
	const int step = 10000; // 10 km jumps
	radarSVG.moveTo(rayElem, dist.range.min, azmR);
	for (int i=dist.range.min+step; i<=dist.range.max; i += step){
		i = std::min(i, static_cast<int>(dist.range.max));
		radarSVG.lineTo(rayElem, static_cast<double>(i), azmR);
	}

	// NOT needed if (MASK){ }

};





} // namespace rack

