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
//#include <drain/util/TreeXML.h>
//#include <drain/util/TreeHTML.h>
#include <drain/image/FilePng.h>
//#include <drain/image/TreeSVG.h>
#include <drain/image/TreeElemUtilsSVG.h>
//#include <drain/image/TreeLayoutSVG.h>
#include <drain/image/TreeUtilsSVG.h>



// #include <js/koe.h>


//#include "radar/PolarSector.h"
#include "graphics.h"
#include "graphics-panel.h"
#include "graphics-radar.h"
#include "graphics-overlay.h"


namespace drain {


}


namespace rack {
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
		//radarSVG.deriveMaxRange(srcPolar);
		radarSVG.radarProj.deriveMaxRange(srcPolar);
		//mout.debug(DRAIN_LOG(radarSVG.maxRange));
	}
	else {
		mout.warn("No polar data read, cannot focus on a specific radar");
		// return;
	}

	Graphic::getGraphicStyle(ctx.svgTrack);

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




	// overlayGroup->addClass(GRID); // add RADAR?
	overlayGroup->set(DATA_ID, srcDsc); // Rack necessity.
	// New. For mouse coords: // drain::Sprinter::plainLayout

	overlayGroup->set("data-bbox", drain::sprinter(radarSVG.geoFrame.getBoundingBoxNat().tuple(), drain::Sprinter::plainLayout).str());
	// DOES not work overlayGroup->set("data-epsg", radarSVG.geoFrame.getEPSG());

	// overlayGroup->addClass("COORDS");
	overlayGroup[svg::DESC](svg::DESC)->setText(srcDsc); // General SVG comment
	// mout.accept<LOG_WARNING>(DRAIN_LOG(radarSVG.geoFrame));
	// mout.reject<LOG_WARNING>(DRAIN_LOG(radarSVG.radarProj));

	return overlayGroup;

};

drain::image::TreeSVG & CmdPolarBase::getOverlay(drain::image::TreeSVG & overlayGroup, const std::string & label) const {

	//std::string srcLabel = overlayGroup->getAttributes().get("data-latest", "unknown");
	std::string srcLabel = overlayGroup->get(DATA_ID, "unknown");
	const std::string & key = label.empty() ? srcLabel : label;
	drain::image::TreeSVG & overlay = overlayGroup[key](drain::image::svg::GROUP); // (drain::image::svg::GROUP);
	overlay->addClass(GRID);
	overlay->set("data-src", srcLabel); // just info
	overlay.addChild()->setComment(getName(), ' ', getParameters(), " getOverLay");

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


void CmdPolarBase::addGeoData(const drain::image::Image & imageData, drain::image::NodeSVG & node){

	CartesianODIM odim(ODIMPathElem::DATA|ODIMPathElem::ROOT); // Encoding + quantity + geo
	odim.copyFrom(imageData);
	// ODIM odim(ODIMPathElem::DATA); // Encoding + quantity
	//mout.attention(DRAIN_LOG(myOdim));

	node.set("data-epsg", odim.epsg);
	node.set("data-bbox-deg", odim.bboxD.toStr(','));
	node.set("data-bbox-nat", imageData.getProperties().get("where:BBOX_native","?"));

	node.set("data-quantity", odim.quantity);
	node.set("data-encoding",
			drain::StringBuilder<','>(odim.scaling.scale, odim.scaling.offset, odim.nodata, odim.undetect).str()); // todo: UNDETECT, NODATA

}


} // rack


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
	resolveDistance(radiusMetres, {0.0,0.0}, dist, radarSVG.radarProj.getRange());


	if (dist.range.max > 0.0){ // earlier!

		// drain::image::TreeUtilsSVG\n
		drain::UtilsXML::ensureStyle(ctx.svgTrack, cls, {
				{"fill", "red"},
				{"stroke", "white"},
				{"stroke-width", 2.0},
				// {"opacity", 0.5}
		});

		// overlay.addChild()->setComment(getName(), ' ', getParameters());
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
				radarSVG.drawSector(elem, {0, radarSVG.radarProj.getRange()});
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
	// overlay.addChild()->setComment(getName(), ' ', getParameters());

	drain::SteppedRange<double> dist(0.0, 0.0, 1.0); // (distanceMetres.range); // double -> int
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

	// const drain::Range<double> azmRad(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);

	overlay.addChild()->setComment("Circles/arcs: ", azm.range.tuple(), " - ", azmRad.tuple());

	for (int i=dist.range.min + dist.step; i<=dist.range.max; i += dist.step){

		drain::image::TreeSVG & g = overlay.addChild()(svg::GROUP);
		g->addClass(Graphic::HIGHLIGHT);
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
		text->addClass(Graphic::GRID); // on top of image, background gray blur applied
		text->addClass(Graphic::HIGHLIGHT);
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

	// overlay.addChild()->setComment(getName(), ' ', getParameters());

	drain::image::TreeSVG & curve = overlay[getName()+getLastParameters()](drain::image::svg::PATH);
	curve -> addClass(cls); // SECTOR
	// drain::image::TreeUtilsSVG\n
	drain::UtilsXML::ensureStyle(ctx.svgTrack, cls, { // SECTOR
			{"fill", "none"},
			// {"stroke", "rgb(160,255,160)"},
			{"stroke-width", 5.0},
			// {"opacity", 0.65}
	});

	drain::SteppedRange<double> dist(0,0,1.0); // (distanceMetres.range); // double -> int
	resolveDistance(radiusMetres, ctx.polarSelector.radius, dist, radarSVG.radarProj.getRange());

	drain::SteppedRange<double> azm(0.0, 0.0, 0.0); // (distanceMetres.range); // double -> int
	resolveAzimuthRange(azimuthDegrees, ctx.polarSelector.azimuth, azm);
	const drain::Range<double> azmR(azm.range.min * drain::DEG2RAD, azm.range.max * drain::DEG2RAD);

	{
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

	if (MASK){
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
	// overlay.addChild()->setComment(getName(), ' ', getParameters());

	drain::image::TreeSVG & curve = overlay[getName()](drain::image::svg::PATH);
	curve->addClass(cls); // SECTOR
	// drain::image::TreeUtilsSVG\n
	drain::UtilsXML::ensureStyle(ctx.svgTrack, cls, { // SECTOR
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

	// const std::string & clsNameBase = getName();


	if (!label.empty()){

		using namespace drain::image;

		// const std::string clsNameLabel = clsNameBase+"_Label";
		// const std::string & clsNameLabel = clsNameBase;

		//// drain::image::TreeUtilsSVG\n drain::UtilsXML::ensureStyle(ctx.svgTrack, cls, {
		drain::UtilsXML::ensureStyle(ctx.svgTrack, cls, {
				//{"font-size", "12"},
				{"fill", "red"},
				{"stroke", "white"},  // replace these with image-title etc soft transit
				{"stroke-width", "0.2"},
		});

		// overlay.addChild()->setComment(clsNameLabel);

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

#include "js/add_mouse_listeners.h"

#include "js/image_coord_tracker.h"


void CmdCoords::exec() const {

	using namespace drain::image;
	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	// GENERALIZE (start)
	// Raise to shared (static) name. Any code to be called upon HTML/SVG page load will be added here.
	/*
	const std::string onload_fnc_name = "rack_onload";
	TreeSVG & onloadJS = drain::UtilsXML::ensureJavaScriptFunction(ctx.svgTrack, onload_fnc_name)[svg::JAVASCRIPT_SCOPE](svg::JAVASCRIPT_SCOPE);
	ctx.svgTrack->set("onload", onload_fnc_name+"()"); // perhaps repeated
	*/

	TreeSVG & onloadJS = RackSVG::getOnLoadScript(ctx);


	RadarSVG radarSVG;
	drain::image::TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG); // ensure BBOX + track class
	overlayGroup->setId(); // visible
	std::string bbox = overlayGroup->get("data-bbox","");
	if (!bbox.empty()){
		mout.special("Attaching coordinate monitor to overlaygroup #", overlayGroup->getId());
	}
	else {
		mout.warn("Could not attach coordinate monitor - overlaygroup #", overlayGroup->getId(), "has no 'data-bbox' attribute ");
	}


	drain::image::TreeSVG & imagePanelGroup = RackSVG::getImagePanelGroup(ctx);
	imagePanelGroup->addClass(RackSVG::ElemClass::MOUSE); //  COORD_TRACKER
	// -> track IMAGE_BORDER
	// -> write to COORDS

	//drain::image::TreeSVG & coordCatcher = imagePanelGroup[RackSVG::ElemClass::BACKGROUND_RECT](svg::RECT);
	//coordCatcher->addClass(RackSVG::ElemClass::BACKGROUND_RECT);
	drain::image::TreeSVG & coordCatcher = imagePanelGroup[RackSVG::ElemClass::IMAGE_BORDER](svg::RECT);
	// Mark for coord monitoring
	//coordCatcher->addClass("COORDS");
	coordCatcher->setStyle("fill","red");
	coordCatcher->setStyle("fill-opacity", 0.5);
	coordCatcher->setAlign(AlignSVG::HORZ_FILL, AlignSVG::VERT_FILL);
	coordCatcher->set("data-bbox", bbox);

	drain::image::TreeSVG & coordDisplay = imagePanelGroup[RackSVG::ElemClass::COORD_TRACKER](svg::TEXT);
	coordDisplay->setId();
	coordDisplay->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordDisplay->setAlign(AlignSVG::RIGHT, AlignSVG::TOP);
	coordDisplay->addClass(RackSVG::ElemClass::MONITOR);
	// Associate with graphical "location" style
	coordDisplay->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);
	coordDisplay->setTextSafe("(lon,lat)");
	coordDisplay->setFontSize(15,20);

	/*
	drain::UtilsXML::ensureStyle(ctx.svgTrack, RackSVG::ElemClass::DATA_ARRAY, {
			{"fill", "red"},
			{"opacity", 0.5},
	});
	*/

	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "add_mouse_listeners") = add_mouse_listeners;
	drain::UtilsXML::getHeaderObject(ctx.svgTrack, svg::SCRIPT, "image_coord_tracker") = image_coord_tracker;

	//
	//TreeSVG & myJS = onloadJS[getName()](svg::JAVASCRIPT_SCOPE);
	onloadJS["add_coord_tracker"] = drain::StringBuilder<>("add_coord_tracker();");



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
	dataImageElem->addClass("MOUSE_VALUE_DATA");  // RackSVG::ElemClass::COORD_TRACKER consider shared tracker plane!
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
	valueMonitor->setAlign(AlignSVG::TOP, AlignSVG::OUTSIDE);
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

//#include "js/base64ToFloat32ArrayLE.h"
#include "js/base64ToArrayLE.h"


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

/*
	void exec2() const  {

		RadarSVG radarSVG;

		// drain::image::TreeSVG & overlayGroup = prepareGeoGroup(ctx, radarSVG);

		// const drain::Rectangle<double> &bbox = radarSVG.geoFrame.getBoundingBoxNat();
		//const drain::Point2D<double> & UR = bbox.upperRight;

		const int width  = radarSVG.geoFrame.getFrameWidth();
		const int height = radarSVG.geoFrame.getFrameHeight();

		drain::Point2D<double> UL,LL, UR, LR;


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

		// drain::image::FilePng::write(coords, "coords.png");
	}

};
*/


} // namespace rack

//DRAIN_ENUM_DICT(CmdRadarMarker::MARKER);
// drain::Enum<orientation_enum>::dict
// template <>
