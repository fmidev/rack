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
		// mout.note(DRAIN_LOG(whereCart));
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

	/*
	mout.attention(DRAIN_LOG(radarSVG.geoFrame.getEPSG()));
	mout.attention(DRAIN_LOG(radarSVG.radarProj.getSrc().getEPSG()));
	mout.attention(DRAIN_LOG(radarSVG.radarProj.getDst().getEPSG()));
	*/

	// TODO: should these be in shared level? Also Overlays should be for each vertical "stack".
	if (radarSVG.geoFrame.getEPSG()){
		overlayGroup->set("data-epsg", radarSVG.geoFrame.getEPSG());
	}

	if (radarSVG.geoFrame.isLongLat()){
		overlayGroup->set("data-bbox", drain::sprinter(radarSVG.geoFrame.getBoundingBoxDeg().tuple(), drain::Sprinter::plainLayout).str());

	}
	else {
		std::vector<int> geoBBOX;
		radarSVG.geoFrame.getBoundingBoxNat().toSequence(geoBBOX);
		overlayGroup->set("data-bbox", drain::sprinter(geoBBOX, drain::Sprinter::plainLayout).str());
	}



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
				{"fill", "white"},
				{"stroke", "black"},
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

		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse("${NOD} ${what:startdate|%Y/%m/%d} ${what:starttime|%H:%M:%S}", true); // convert escaped
		// mout.special(DRAIN_LOG(statusFormatter));

		const std::string info = statusFormatter.toStr(ctx.getUpdatedStatusMap(), 0, RackContext::variableFormatter); // XXX
		drain::image::TreeSVG & title = curve[svg::TITLE](svg::TITLE);
		title->setText(info);

		if (MASK){
			// Note: mask is full 100% range.
			drain::image::TreeSVG & localMask = overlay[svg::MASK];
			{
				// Private scope, to call bezierElem destructor.
				drain::svgPATH elem(localMask);
				radarSVG.drawSector(elem, {0, radarSVG.radarProj.getRange()});
			}
			// Copy this localMask to shared mask...
			const int w = radarSVG.geoFrame.getFrameWidth();
			const int h = radarSVG.geoFrame.getFrameHeight();
			MaskerSVG::createMask(ctx.svgTrack, overlayGroup, w, h, localMask.data);
			// ... and "delete" the object.
			localMask->setType(svg::COMMENT);
		}


	}

};


void CmdRadarLabel::exec() const  {

	using namespace drain::image;

	RackContext & ctx = getContext<RackContext>();
	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__);

	RadarSVG radarSVG;

	TreeSVG & overlayGroup = getOverlayGroup(ctx, radarSVG);
	TreeSVG & overlay = getOverlay(overlayGroup);


	if (!label.empty()){


		TreeSVG & style = drain::UtilsXML::ensureStyle(ctx.svgTrack, cls, {
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

		//
		// mout.attention(DRAIN_LOG(style->getStyle()));
		// drain::TreeUtils::dump(style, std::cout); //  DataTools::treeToStream); // CmdOutputTree::dataToStream);
		//	drain::TreeUtils::dump(style["font-size"], std::cout); // , DataTools::treeToStream);
		//mout.attention(style["font-size"]);

		drain::UtilsXML::ensureStyle(ctx.svgTrack, "DEBUG", {
				//{"font-size", "12"},
				{"fill", "white"},
				{"fill-opacity", 0.1},
				{"stroke", "red"},  // replace these with image-title etc soft transit
				{"stroke-width", "2"},
		});


		// TODO: group for all (font size etc)
		drain::Point2D<int> imgPoint;
		radarSVG.convert(0.0, 0.0, imgPoint); // radar center (radius=0, azm=0)

		TreeSVG & labelAnchor = overlay["labelAnchor"];
		labelAnchor->addClass(LayoutSVG::GroupType::FIXED);
		labelAnchor->addClass("DEBUG");
		//labelAnchor->setMyAlignAnchor("munDOT");
		if (false){
			drain::svgCIRCLE circle(labelAnchor);
			circle.cx = imgPoint.x;
			circle.cy = imgPoint.y;
			circle.r = 10.0;
			// svgPATH has no "natural" origin...
			// drain::svgPATH bezierElem(labelAnchor);
			// radarSVG.drawCircle(bezierElem, {0.0,15000.0});
		}
		else {
			// drain::svgRECT rect(overlayGroup[RackSVG::BACKGROUND_RECT]);
			drain::svgRECT rect(labelAnchor);
			// rect.node.addClass(RackSVG::BACKGROUND_RECT);
			// mout.attention(DRAIN_LOG(imgPoint));
			rect.width  = 5.0;
			rect.height = 10.0;
			rect.x = imgPoint.x - 10.0;
			rect.y = imgPoint.y -  5.0;
		}

		drain::StringMapper statusFormatter(RackContext::variableMapper);
		statusFormatter.parse(label, true); // convert escaped
		// mout.special(DRAIN_LOG(statusFormatter));

		const std::string formattedLabel = statusFormatter.toStr(ctx.getUpdatedStatusMap(), 0, RackContext::variableFormatter); // XXX
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

					drain::image::TreeSVG & text = overlay.addChild()(drain::image::svg::TEXT);

					text->setFontSize(fontSize, (15*fontSize)/10);
					text->setText(part);
					// text->setFrame(fontSize*2, fontSize);
					text->addClass(this->cls);
					text->addClass(LayoutSVG::GroupType::NEUTRAL);

					text->setMyAlignAnchor<AlignBase::Axis::HORZ>("labelAnchor");
					text->setMyAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS); // For the first element, this is "labelAnchor"
					text->setAlign(alignHorz);
					text->setAlign(alignVert);
					alignVert.set(AlignSVG::TOP, MutualAlign::INSIDE);

					text[svg::TITLE](svg::TITLE)->setText(part);
				}
				alignHorz.pos = AlignBase::Pos::MAX;
			}

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





} // namespace rack

