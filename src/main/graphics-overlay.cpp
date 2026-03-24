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
		labelAnchor->addClass(LayoutSVG::GroupType::NEUTRAL);
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


			// CompleteAlignment<> alignVert(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);

			// bool newline=true;
			for (const std::string & part: parts){
				if (!part.empty()){

					drain::image::TreeSVG & text = overlay.addChild()(drain::image::svg::TEXT);
					text->setFontSize(fontSize, (15*fontSize)/10);
					text->setText(part);
					// drain::image::TreeSVG & text = overlay.addChild()(svg::RECT);
					// text->setFrame(fontSize*2, fontSize);
					text->addClass(this->cls);
					//text->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);
					text->addClass(LayoutSVG::GroupType::NEUTRAL);
					text->setMyAlignAnchor<AlignBase::Axis::HORZ>("labelAnchor");
					text->setMyAlignAnchor<AlignBase::Axis::VERT>(AnchorElem::PREVIOUS); // For the first element, this is "labelAnchor"

					//text->setAlign(alignHorz, MutualAlign::OUTSIDE);
					text->setAlign(alignHorz);
					text->setAlign(alignVert);
					alignVert.set(AlignSVG::TOP, MutualAlign::INSIDE);
					/*
					if (newline){
						//text->setAlign(alignVert);
						text->setAlign(drain::image::AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
						//text->setAlign(alignVert., drain::image::AlignSVG::OUTSIDE);
						newline = false;
					}
					else {
						text->setAlign(drain::image::AlignSVG::TOP, MutualAlign::INSIDE);
					}
					*/
					// text->setAlign(alignVert);
					// alignVert.set(drain::image::AlignSVG::TOP, drain::image::AlignSVG::INSIDE);
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

drain::image::TreeSVG & addCoordMonitor(drain::image::TreeSVG & imagePanelGroup, const std::string & cls){

	imagePanelGroup->setType(svg::TEXT); // ensure
	imagePanelGroup->setAlign(AlignSVG::RIGHT);
	imagePanelGroup->setFontSize(15,18);
	imagePanelGroup->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);

	drain::image::TreeSVG & coordDisplay = imagePanelGroup[cls](svg::TSPAN);
	coordDisplay->addClass(cls);
	// Associate with graphical "location" style
	//coordDisplay->addClass(RackSVG::ElemClass::IMAGE_TITLE, RackSVG::ElemClass::LOCATION);
	coordDisplay->setTextSafe("*");
	// coordDisplay->setFontSize(15,18);
	// coordDisplay->setAlign(AlignSVG::RIGHT);
	// return coordDisplay;
	return imagePanelGroup;
}

void CmdRect::exec() const {

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


	// REMOVE this!
	drain::image::TreeSVG & coordDisplay = imagePanelGroup["MONITOR"];
	addCoordMonitor(coordDisplay, "MONITOR_MOVE");
	coordDisplay->setMyAlignAnchor(RackSVG::ElemClass::IMAGE_BORDER);
	coordDisplay->setAlign(AlignSVG::TOP);


	// coordDisplay->setAlign(AlignSVG::RIGHT);
	drain::image::TreeSVG & coordSpanDisplay = imagePanelGroup["MONITOR_BOX"];
	coordSpanDisplay->addClass("MONITOR_BOX");
	coordSpanDisplay->setMyAlignAnchor<AlignBase::Axis::VERT>(drain::image::AnchorElem::PREVIOUS);
	coordSpanDisplay->setAlign(AlignSVG::BOTTOM, MutualAlign::OUTSIDE);
	addCoordMonitor(coordSpanDisplay, "MONITOR_DOWN");
	coordSpanDisplay.addChild("COMMA")(svg::TSPAN) = ",";
	addCoordMonitor(coordSpanDisplay, "MONITOR_UP");

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
	drain::Unit unit = drain::Units::extract<drain::Unit>(this->bbox);
	mout.attention(DRAIN_LOG(this->bbox));
	mout.attention(DRAIN_LOG(unit));

	mout.attention("Unit=", unit);

	//drain::image::BBoxSVG finalBBox;
	//drain::UniTuple<double,4> bb;
	// drain::Reference ref(finalBbox.tuple());
	std::vector<double> bb;
	drain::StringTools::split(this->bbox, bb, ',');
	mout.attention(drain::sprinter(bb));


	//drain::BBox finalBBox;
	drain::Rectangle<int> imgBBox;
	// finalBBox.set(bb[0], bb[1], bb[2], bb[3]);

	switch (unit) {
	case drain::Unit::DEGREE:
		radarSVG.geoFrame.deg2pix(bb[0], bb[1], imgBBox.lowerLeft.x, imgBBox.lowerLeft.y);
		radarSVG.geoFrame.deg2pix(bb[2], bb[3], imgBBox.upperRight.x, imgBBox.upperRight.y);
		break;
	case drain::Unit::METRE:
		radarSVG.geoFrame.m2pix(bb[0], bb[1], imgBBox.lowerLeft.x, imgBBox.lowerLeft.y);
		radarSVG.geoFrame.m2pix(bb[2], bb[3], imgBBox.upperRight.x, imgBBox.upperRight.y);
		//finalBBox.set(bb[0], bb[1], bb[2], bb[3]);
		break;
	case drain::Unit::UNDEFINED:
	case drain::Unit::PIXEL:
		imgBBox.set(bb[0], bb[1], bb[2], bb[3]);
		break;
	default:
		mout.error("unknown unit: ", this->bbox); // or trust false above?
		break;
	}
	mout.attention(DRAIN_LOG(imgBBox));


	selectRect->setLocation(std::min(imgBBox.lowerLeft.x, imgBBox.upperRight.x), std::min(imgBBox.lowerLeft.y, imgBBox.upperRight.y));
	// mout.warn(this->bbox.getWidth(),'x', this->bbox.getHeight());
	selectRect->setFrame(::abs(imgBBox.getWidth()), ::abs(imgBBox.getHeight()));

	/* Tästä mallia:
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
	*/



	RackSVG::addJavaScripsDef(ctx, "MONITOR2_BOX", "cls");

	mout.warn(this->bbox);
	mout.warn(drain::sprinter(selectRect.data.getAttributes()));

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

	/*
	drain::image::TreeSVG & myMouseUp = myJS["TEST1"]; // (svg::SCRIPT) <- nyt tajus/handlasi JAVASCRIPT_SCOPE:n
	myMouseUp = "// TEST1;\n";
	drain::image::TreeSVG & myMouseUp2 = myJS["TEST2"](svg::JAVASCRIPT_SCOPE); // (svg::SCRIPT) <- nyt tajus/handlasi JAVASCRIPT_SCOPE:n
	myMouseUp2 = "// fct Here!;\n";
	/// TODO: xml::toStream handler for svg::JAVASCRIPT_SCOPE ?
	//  -> (pre-string: funct{} and explicit newline, or no translation
	myMouseUp2.addChild() = "// TEST2;\n";
	myMouseUp2.addChild() = "// TEST3;\n";
		*/

	/*
	myJS.addChild() = "// Added by Rack \n";
	drain::image::TreeSVG & myMouseUp = myJS["MOUSUO"]; // (svg::SCRIPT) <- nyt tajus/handlasi JAVASCRIPT_SCOPE:n
	myMouseUp->setText("CoordMonitor.prototype.updateUp = function(x,y){}; //", myMouseUp->getNativeType(), "");
	drain::image::TreeSVG & myMouseUpLine = myMouseUp[svg::JAVASCRIPT_SCOPE](svg::JAVASCRIPT_SCOPE);
	myMouseUpLine->setText("// test /\n");
	myMouseUpLine["lapo"] = "console.log(x,y); console.warn(this.group.selectorRect);};\n";
	myMouseUpLine.addChild() = "console.log(x,y); console.warn(this.group.selectorRect);};\n";
	drain::TreeUtils::dump(myJS);
	*/
	// CoordMonitor.prototype.updateDown

	/*
	TreeSVG & myMouseMove = drain::UtilsXML::ensureJavaScriptFunction(ctx.svgTrack, "myFunction","x", "y")[svg::JAVASCRIPT_SCOPE](svg::JAVASCRIPT_SCOPE);
	myMouseMove.addChild() = "x=1.0;";
	myMouseMove.addChild() = "y=2.0;";
	*/
	// TreeSVG & myJs = ctx.svgTrack[]
	// ctx.svgTrack;

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

namespace drain {

}  // namespace drain

//DRAIN_ENUM_DICT(CmdRadarMarker::MARKER);
// drain::Enum<orientation_enum>::dict
// template <>
