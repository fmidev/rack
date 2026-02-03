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
#include <main/graphics-overlay.h>

// #include <js/koe.h>


//#include "radar/PolarSector.h"
#include "graphics-panel.h"
#include "graphics-radar.h"




namespace rack {

//const drain::Range<double> azmRad(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);
/*
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





const std::string CmdPolarBase::DATA_ID = "data-latest";

/**
 *
 *  circle=max RADAR_CIRCLE
 *  dot=0.0    RADAR_DOT
 *  label=${NOD}  RADAR_LABEL
 *
 *
 */


/**
 *  \param shared - if false, create private object ("layer") for each radar; else use common.
 */
drain::image::TreeSVG & CmdPolarBase::getOverlayGroup(RackContext & ctx, RadarSVG & radarSVG){ // , bool prepend=false){

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	const Hi5Tree & srcPolar = ctx.getHi5(RackContext::POLAR|RackContext::CURRENT); // , RackContext::CURRENT
	const Hi5Tree & srcCurr  = ctx.getHi5(RackContext::CURRENT); // this works well with -c (--cCreate)

	if (!srcPolar.empty()){
		//radarSVG.maxRange = DataTools::getMaxRange(srcPolar);
		radarSVG.deriveMaxRange(srcPolar);
		//mout.debug(DRAIN_LOG(radarSVG.maxRange));
	}
	else {
		mout.warn("No polar data read, cannot focus on a specific radar");
		// return;
	}

	RadarSVG::getOverlayStyle(ctx.svgTrack);

	drain::image::TreeSVG & group    = RackSVG::getCurrentAlignedGroup(ctx);
	drain::image::TreeSVG & overlayGroup = RadarSVG::getOverlayGroup(group);
	RackSVG::consumeAlignRequest(ctx, overlayGroup);

	const drain::VariableMap & wherePolar = srcPolar[ODIMPathElem::WHERE].data.attributes;

	radarSVG.updateRadarConf(wherePolar); // lon, lat

	std::string srcDsc = "unknown_composite";

	const drain::Variable & object = srcCurr[ODIMPathElem::WHAT].data.attributes["object"];
	if ((object == "SCAN") || (object == "PVOL")){
		// mout.note("Polar coordinates"); // Cartesian not "found", ie not created this.
		// mout.warn("Current object is not projected (Cartesian) data, cannot focus on a specific radar");
		srcDsc = srcCurr[ODIMPathElem::WHAT].data.attributes["source"].toStr();
		// overlayGroup->set("data-latest", srcCurr[ODIMPathElem::WHAT].data.attributes["source"]);
		// overlayGroup[svg::DESC](svg::DESC)->setText();
	}

	// mout.note("Cartesian");
	const Hi5Tree & srcCartesian = ctx.getHi5(RackContext::CARTESIAN);  // ctx.getHi5(RackContext::CURRENT | RackContext::CARTESIAN);

	if (!srcCartesian.empty()){ // HDF5 has been either loaded or extracted.
		mout.note("Using Cartesian product (meta data)");
		//mout.note(DRAIN_LOG(srcCartesion));
		const drain::VariableMap & whereCart = srcCurr[ODIMPathElem::WHERE].data.attributes;
		radarSVG.updateCartesianConf(whereCart);
		// Redesign multi-frame mosaick...
		// overlayGroup->set("data-latest",
		srcDsc = srcCurr[ODIMPathElem::HOW].data.attributes["nodes"].toStr();
	}
	else {
		Composite & composite = ctx.getComposite(RackContext::Hi5Role::PRIVATE | RackContext::Hi5Role::SHARED); // SHARED?
		mout.debug("Cartesian product not (yet) created, but using specification: ", DRAIN_LOG(composite));
		radarSVG.updateCartesianConf(composite);
	}

	overlayGroup->set(DATA_ID, srcDsc); // Rack necessity.
	overlayGroup[svg::DESC](svg::DESC)->setText(srcDsc); // General SVG comment
	// mout.accept<LOG_WARNING>(DRAIN_LOG(radarSVG.geoFrame));
	// mout.reject<LOG_WARNING>(DRAIN_LOG(radarSVG.radarProj));

	return overlayGroup;

};

drain::image::TreeSVG & CmdPolarBase::getOverlay(drain::image::TreeSVG & overlayGroup, const std::string & label){

	//std::string srcLabel = overlayGroup->getAttributes().get("data-latest", "unknown");
	std::string srcLabel = overlayGroup->get(DATA_ID, "unknown");
	const std::string & key = label.empty() ? srcLabel : label;
	drain::image::TreeSVG & overlay = overlayGroup[key](drain::image::svg::GROUP); // (drain::image::svg::GROUP);
	overlay->set("data-src", srcLabel); // just info
	return overlay;

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

	mout.accept<LOG_NOTICE>(DRAIN_LOG(dist));
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



} // rack


DRAIN_ENUM_DICT(rack::CmdPolarBase::GRAPHIC) = {
		DRAIN_ENUM_ENTRY(rack::CmdPolarBase, CIRCLE),
		DRAIN_ENUM_ENTRY(rack::CmdPolarBase, DOT),
		// {"CIRCLE", CmdRadarGRAPHIC::GRAPHIC::CIRCLE}
};

DRAIN_ENUM_OSTREAM(rack::CmdPolarBase::GRAPHIC);


namespace rack {


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

	RadarSVG radarSVG;
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	drain::image::TreeSVG & overlay = getOverlay(overlayGroup);

	// Always a range, though here only dis.max used
	drain::SteppedRange<double> dist(0.0, 0.0, 30.0);
	// Note: DOT does not use shared polar selection
	resolveDistance(radiusMetres, {0.0,0.0}, dist, radarSVG.getRange());


	if (dist.range.max > 0.0){ // earlier!

		const std::string DOT = "DOT";

		drain::image::TreeUtilsSVG::ensureStyle(ctx.svgTrack, DOT, {
				{"fill", "red"},
				{"stroke", "white"},
				{"stroke-width", 2.0},
				// {"opacity", 0.5}
		});

		overlay.addChild()->setComment(getName(), ' ', getParameters());
		drain::image::TreeSVG & curve = overlay[DOT](drain::image::svg::PATH);
		curve->addClass(DOT);

		{
			// Private scope, to call bezierElem destructor.
			drain::svgPATH bezierElem(curve);
			radarSVG.drawCircle(bezierElem, dist.range);
		}

		if (MASK){
			// Note: mask is full 100% range.
			drain::image::TreeSVG & localMask = overlay[svg::MASK];
			{
				// Private scope, to call bezierElem destructor.
				drain::svgPATH elem(localMask);
				radarSVG.drawSector(elem, {0, radarSVG.getRange()});
			}
			const int w = radarSVG.geoFrame.getFrameWidth();
			const int h = radarSVG.geoFrame.getFrameHeight();
			MaskerSVG::createMask(ctx.svgTrack, overlayGroup, w, h, localMask.data);
			localMask->setType(svg::COMMENT);
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

	RadarSVG radarSVG;
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	drain::image::TreeSVG & overlay = getOverlay(overlayGroup);
	overlay.addChild()->setComment(getName(), ' ', getParameters());

	drain::SteppedRange<double> dist(0.0, 0.0, 1.0); // (distanceMetres.range); // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.getRange());
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

	drain::SteppedRange<double> azm; // (distanceMetres.range); // double -> int
	// drain::SteppedRange<double> azm(0.0, 0.0, 0.0); // (distanceMetres.range); // double -> int
	resolveAzimuthRange(azimuthDegrees, ctx.polarSelector.azimuth, azm);
	const drain::Range<double> azmRad(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);
	if (azm.step < 5){
		azm.step = 15;
		mout.info("too small azimuthal step, assigned  ", DRAIN_LOG(azm.step));
	}


	overlay.addChild()->setComment("Rays: ", dist.range.tuple());
	double angleRad = 0.0;
	for (int j=azm.range.min; j<=azm.range.max; j += azm.step){

		angleRad = drain::DEG2RAD * static_cast<double>(j);

		drain::image::TreeSVG & rayNode = overlay.addChild();
		rayNode->addClass(RadarSVG::HIGHLIGHT);
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

	// const drain::Range<double> azmRad(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);

	overlay.addChild()->setComment("Circles/arcs: ", azm.range.tuple(), " - ", azmRad.tuple());

	for (int i=dist.range.min + dist.step; i<=dist.range.max; i += dist.step){

		drain::image::TreeSVG & g = overlay.addChild()(svg::GROUP);
		g->addClass(RadarSVG::HIGHLIGHT);
		g->addClass(drain::image::LayoutSVG::FIXED); // NEW 2026?

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
		text->addClass(RadarSVG::GRID); // on top of image, background gray blur applied
		text->addClass(RadarSVG::HIGHLIGHT);
		radarSVG.convert(i + offset, azmRad.min, imgPoint);
		text->setLocation(imgPoint);
		text->setText(i/1000); // "km - ", imgPoint.tuple());

	}


	if (MASK){
		drain::image::TreeSVG & localMask = overlay[svg::MASK];
		{
			// Private scope, to call bezierElem destructor.
			drain::svgPATH elem(localMask);
			radarSVG.drawCircle(elem, {0,  dist.range.max});
		}
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		// drain::image::TreeSVG & mask =
		MaskerSVG::createMask(ctx.svgTrack, overlayGroup, w, h, localMask.data);
		// drain::image::TreeSVG & comment = mask.prependChild("")(svg::COMMENT);
		// comment->setText("applied by: ", getName(), ' ', getParameters());
		localMask->setType(svg::COMMENT);
	}


};




void CmdRadarSector::exec() const  {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	RadarSVG radarSVG;
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	drain::image::TreeSVG & overlay = getOverlay(overlayGroup);

	overlay.addChild()->setComment(getName(), ' ', getParameters());

	drain::image::TreeSVG & curve = overlay[getName()](drain::image::svg::PATH);
	curve -> addClass(getName()); // SECTOR
	drain::image::TreeUtilsSVG::ensureStyle(ctx.svgTrack, getName(), { // SECTOR
			{"fill", "none"},
			{"stroke", "rgb(160,255,160)"},
			{"stroke-width", 5.0},
			// {"opacity", 0.65}
	});

	drain::SteppedRange<double> dist; // (distanceMetres.range); // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.getRange());

	drain::SteppedRange<double> azm(0.0, 0.0, 0.0); // (distanceMetres.range); // double -> int
	resolveAzimuthRange(azimuthDegrees, ctx.polarSelector.azimuth, azm);
	const drain::Range<double> azmR(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);

	{
		drain::svgPATH bezierElem(curve);
		radarSVG.drawSector(bezierElem, dist.range, azmR);
	}

	if (MASK){ //  && !curve->hasClass("MASK")){
		const int w = radarSVG.geoFrame.getFrameWidth();
		const int h = radarSVG.geoFrame.getFrameHeight();
		MaskerSVG::createMask(ctx.svgTrack, overlayGroup, w, h, curve.data);
	}

};





void CmdRadarRay::exec() const {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	RadarSVG radarSVG;
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	drain::image::TreeSVG & overlay = getOverlay(overlayGroup);

	overlay.addChild()->setComment(getName(), ' ', getParameters());

	drain::image::TreeSVG & curve = overlay[getName()](drain::image::svg::PATH);
	curve -> addClass(getName()); // SECTOR
	drain::image::TreeUtilsSVG::ensureStyle(ctx.svgTrack, getName(), { // SECTOR
			{"fill", "none"},
			{"stroke", "rgb(160,255,160)"},
			{"stroke-width", 5.0},
			// {"opacity", 0.65}
	});

	drain::SteppedRange<double> dist; // (distanceMetres.range); // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.getRange());

	const double azmR = azimuthDegrees.range.min * drain::DEG2RAD;

	drain::svgPATH rayElem(curve);
	const int step = 10000; // 10 km jumps
	radarSVG.moveTo(rayElem, dist.range.min, azmR);
	for (int i=dist.range.min+step; i<=dist.range.max; i += step){
		i = std::min(i, static_cast<int>(dist.range.max));
		radarSVG.lineTo(rayElem, static_cast<double>(i), azmR);
	}

	// needed?
	if (MASK){
		/*
			const int w = radarSVG.geoFrame.getFrameWidth();
			const int h = radarSVG.geoFrame.getFrameHeight();
			MaskerSVG::createMask(ctx.svgTrack, overlayGroup, w, h, curve.data);
		 */
	}

};



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

void CmdRadarLabel::exec() const  {

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	RadarSVG radarSVG;

	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	drain::image::TreeSVG & overlay = getOverlay(overlayGroup);
	// drain::image::TreeSVG & overlayGroup = prepareGeoGroup(ctx, radarSVG);

	const std::string & clsNameBase = getName();


	if (!label.empty()){

		using namespace drain::image;

		// const std::string clsNameLabel = clsNameBase+"_Label";
		const std::string & clsNameLabel = clsNameBase;

		drain::image::TreeUtilsSVG::ensureStyle(ctx.svgTrack, clsNameLabel, {
				//{"font-size", "12"},
				{"fill", "red"},
				{"stroke", "white"},  // replace these with image-title etc soft transit
				{"stroke-width", "0.2"},
		});

		overlay.addChild()->setComment(clsNameLabel);

		//drain::image::TreeSVG & rect = overlayGroup.addChild()(drain::image::svg::RECT);
		drain::svgRECT rect(overlayGroup[RackSVG::BACKGROUND_RECT]);
		rect.node.addClass(drain::image::LayoutSVG::GroupType::FIXED);
		rect.node.addClass(RackSVG::BACKGROUND_RECT);
		drain::Point2D<int> imgPoint;
		radarSVG.convert(0.0, 0.0, imgPoint); // radar center
		rect.width  = 100.0;
		rect.height = 50.0;
		rect.x = imgPoint.x - 50.0;
		rect.y = imgPoint.y - 25.0;
		//const std::string & rectId = rect.node.getId();

		mout.special("formatting label");
		// drain::StringTools::convertEscaped(label);

		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(label, true); // convert escaped
		const std::string formattedLabel = statusFormatter.toStr(ctx.getStatusMap(), 0, RackContext::variableFormatter); // XXX



		std::list<std::string> lines;
		drain::StringTools::split(formattedLabel, lines,'\n');
		// const double fontSize  = 10.0;
		// const double rowHeight = 1.2 * fontSize;
		// double j = rowHeight * 0.5*(static_cast<double>(lines.size())-1.0);
		for (const std::string & line: lines){
			drain::image::TreeSVG & text = overlay.addChild()(drain::image::svg::TEXT);
			//text->addClass(clsNameLabel, RackSVG::ElemClass::IMAGE);
			text->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);
			text->setText(line);
			// text->setLocation(0.0, j);
			text->setMyAlignAnchor<AlignBase::Axis::HORZ>(RackSVG::BACKGROUND_RECT);
			text->setMyAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS);
			text->setAlign(drain::image::AlignSVG::CENTER);
			text->setAlign(drain::image::AlignSVG::TOP);
			text->setFontSize(ctx.svgPanelConf.fontSizes[1], ctx.svgPanelConf.boxHeights[1]);
			// text->setHeight(10);
			// text->setFontSize(16);
			// j += rowHeight;
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



} // namespace rack

//DRAIN_ENUM_DICT(CmdRadarMarker::MARKER);
// drain::EnumDict<orientation_enum>::dict
// template <>
