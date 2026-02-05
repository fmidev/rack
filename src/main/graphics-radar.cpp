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


#include <drain/util/SelectorXML.h>
#include <drain/image/LayoutSVG.h>
#include <drain/util/EnumUtils.h>

#include "graphics-radar.h"

DRAIN_ENUM_DICT(rack::Graphic::GRAPHIC) = {
		DRAIN_ENUM_ENTRY(rack::Graphic, VECTOR_OVERLAY),
		DRAIN_ENUM_ENTRY(rack::Graphic, HIGHLIGHT),
		DRAIN_ENUM_ENTRY(rack::Graphic, GRID),
		DRAIN_ENUM_ENTRY(rack::Graphic, DOT),
		DRAIN_ENUM_ENTRY(rack::Graphic, LABEL),
		DRAIN_ENUM_ENTRY(rack::Graphic, RAY),
		DRAIN_ENUM_ENTRY(rack::Graphic, SECTOR),
		DRAIN_ENUM_ENTRY(rack::Graphic, ANNULUS),
		DRAIN_ENUM_ENTRY(rack::Graphic, CIRCLE),
};

/*
DRAIN_ENUM_DICT(rack::RadarSVG::StyleClasses) = {
		DRAIN_ENUM_ENTRY(rack::RadarSVG::StyleClasses, VECTOR_OVERLAY),
		DRAIN_ENUM_ENTRY(rack::RadarSVG::StyleClasses, GRID),
		DRAIN_ENUM_ENTRY(rack::RadarSVG::StyleClasses, HIGHLIGHT),
};
*/

namespace rack {

using namespace drain;


drain::image::TreeSVG & Graphic::getGraphicStyle(drain::image::TreeSVG & svgDoc){

	// Consider areas or frames: MAIN_FRAME, GROUP_FRAME, IMAGE_FRAME

	drain::Logger mout(__FILE__, __FUNCTION__);

	using namespace drain;
	using namespace drain::image;

	static
	const std::string OVERLAY_STYLE = "OVERLAY_STYLE"; // TODO enum

	if (!svgDoc.hasChild(OVERLAY_STYLE)){
		// Ensure element position at the beginning of the doc.
		svgDoc.prependChild(OVERLAY_STYLE);
	}

	TreeSVG & style = svgDoc[OVERLAY_STYLE];


	if (style->isUndefined()){

		// mout.debug("initializing style");

		style->setType(svg::STYLE);

		typedef SelectXML<svg::tag_t> Select;
		// These could be
		/*
		SelectXML<svg::tag_t>("polygon");
		SelectXML<svg::tag_t>(ClassXML("GRID"));
		SelectXML<svg::tag_t>(ClassXML(GRID));
		SelectXML<svg::tag_t> select;
		select.set("polygon");
		*/

		style[Select(GRID)] = {
				//style[drain::SelectXML<svg::tag_t>(drain::ClassXML("GRID")).str()] = {
				{"stroke", "white"},
				//{"stroke", "gray"},
				{"stroke-width", 3.0},
				{"fill", "none"}, // debug
				//{"fill", "blue"}, // debug
				//{"fill-opacity", 0.35},
		};

		/*
		style[SelectorXMLcls("SECTOR")] = {
				{"fill", "none"},
				{"stroke", "green"},
				{"stroke-width", 12.0},
				{"opacity", 0.65}
		};
		*/

		style[Select(svg::TEXT, GRID)] = {
				{"text-anchor", "middle"},
				{"font-size", "smaller"}, // ctx.svgPanelConf.fontSizes[1]
				// {"font-size", 20.0},
				{"paint-order", "stroke"},
				{"stroke", "black"},
				{"stroke-opacity", "0.5"},
				{"stroke-width", "0.3em"},
				{"stroke-linejoin", "round"},
				{"fill", "white"}, // debug
				{"fill-opacity", "1"},
		};

		style[Select(svg::TEXT, HIGHLIGHT, PseudoClassCSS::hover)] = {
				//{"display", "block"},
				{"font-size", "larger"},
				{"opacity", 1.0},
				// {"fill", "red"},
				//{"stroke-width", 3.0},
				//{"stroke-color", "orange"},
		};

		//style[SelectorXML(image::svg::PATH, ".HIGHLIGHT", ":hover")] = {
		style[Select(svg::PATH, HIGHLIGHT, PseudoClassCSS::hover)] = {
				//{"display", "block"},
				//{"font-size", "larger"},
				//{"fill", "red"},
				{"stroke-width", 3.0},
				{"opacity", 1.0},
				//{"stroke-color", "orange"},
		};
	}

	return style;
}


drain::image::TreeSVG & RadarSVG::getOverlayGroup(drain::image::TreeSVG & group){

	drain::Logger mout(__FILE__, __FUNCTION__);

	TreeSVG & overlayGroup = group[Graphic::VECTOR_OVERLAY]; // (svg::GROUP);

	//if (!group.hasChild(VECTOR_OVERLAY)){
	// if (!geoGroup->hasClass(VECTOR_OVERLAY)){
	if (overlayGroup->isUndefined()){ //  && overlayGroup.hasChildren()

		using namespace drain::image;
		overlayGroup->setType(svg::GROUP);
		overlayGroup->addClass(Graphic::VECTOR_OVERLAY);
		//overlayGroup->addClass(StyleClasses::GRID);
		overlayGroup->addClass(LayoutSVG::NEUTRAL);
		overlayGroup->setAlign(drain::image::AlignSVG::HORZ_FILL, drain::image::AlignSVG::VERT_FILL);
		// Override with: RackSVG::consumeAlignRequest(ctx, geoGroup);
	}

	return overlayGroup;
}

void RadarSVG::updateRadarConf(const drain::VariableMap & where) {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// Todo: also support fully cartesian input (without single-site metadata)
	// drain::Point2D<double> location(where["lon"], where["lat"]);
	const double lon = where.get("lon", 0.0);
	const double lat = where.get("lat", 0.0);

	if ((lon != 0.0) && (lat != 0.0)){
		radarProj.setSiteLocationDeg(lon, lat);
	}
	else {
		mout.fail<LOG_DEBUG+1>("could not set site location (", lon, ',', lat, ")");
		// warn?
	}

	// NOTE: typically these are not in polar data. Only in projected (ie cartesian)
	/*
	const int epsg = where.get("epsg", 0); // non-standard
	if (epsg){
		mout.attention("EPSG found: ", epsg);
		// geoFrame.setProjectionEPSG(epsg);
		radarProj.setProjectionDst(epsg);
	}
	else {
		const std::string projdef = where.get("projdef", ""); // // otherwise gets "null"
		if (!projdef.empty()){
			radarProj.setProjectionDst(projdef); // Cartesian!
		}
		else {
			// radarProj
		}
	}
	*/

	/*
	int range =  where.get("rstart", 0.0) + where.get("rscale", 0.0)*where.get("nbins", 0.0);
	if (range > 0){
		maxRange = range;
		mout.accept<LOG_WARNING>("setting maxRange=", maxRange);
	}
	else {
		mout.fail<LOG_DEBUG+1>("could not set maxRange");
		mout.fail<LOG_WARNING>(where);
	}
	*/
	/*
	// PolarODIM odim;
	// rstart + rscale*static_cast<double>(area.width);
	// double radius = 250000.0;
	double maxRange =  where.get("rstart", 0.0) + where.get("rscale", 0.0)*where.get("nbins", 0.0);
	drain::Rectangle<double> bbox;
	radarProj.determineBoundingBoxM(maxRange, bbox); // M = "native"
	mout.special("BBOX (250km) of the last input:", bbox);
	// mout.special("GeoFrame BBOX: ", geoFrame);
	mout.special("GeoFrame BBOX: ", geoFrame.getBoundingBoxNat());
	*/
}

// Composite has ReferenceMap, Hi5Tree has VariableMap (drain::image::properties)

void RadarSVG::updateCartesianConf(const drain::VariableMap & where) {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// Todo: also support fully cartesian input (without single-site metadata)
	// radarProj.setSiteLocationDeg(where["lon"], where["lat"]);

	const int epsg = where.get("epsg", 0); // non-standard
	if (epsg){
		mout.attention("EPSG found: ", epsg);
		geoFrame.setProjectionEPSG(epsg);
		radarProj.setProjectionDst(epsg);
	}
	else {
		const std::string projdef = where.get("projdef", ""); // otherwise gets "null"
		geoFrame.setProjection(projdef);
		radarProj.setProjectionDst(projdef); // TEST
	}
	geoFrame.setBoundingBoxD(where["LL_lon"], where["LL_lat"], where["UR_lon"], where["UR_lat"]);
	geoFrame.setGeometry(where["xsize"], where["ysize"]);
	// mout.pending<LOG_WARNING>("now slowly: bbox: -> ", geoFrame.getBoundingBoxDeg(), " -> NATIVE: ", geoFrame.getBoundingBoxNat());

}


void RadarSVG::updateCartesianConf(const Composite & comp) {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// Todo: also support fully Cartesian input (without single-site metadata)
	// radarProj.setSiteLocationDeg(where["lon"], where["lat"]);

	const int epsg = comp.getProj().getDst().getEPSG(); // non-standard
	if (epsg){
		mout.attention("EPSG found: ", epsg);
		geoFrame.setProjectionEPSG(epsg);
		radarProj.setProjectionDst(epsg); // TEST
	}
	else {
		// const std::string projdef = comp.getProjection(); // otherwise gets "null"
		geoFrame.setProjection(comp.getProjStr());
		radarProj.setProjectionDst(comp.getProjStr()); // TEST
		//mout.reject<LOG_WARNING>("No EPSG found, using: ", comp.getProjection());
	}

	// mout.pending<LOG_WARNING>(DRAIN_LOG(comp.projGeo2Native));

	// const drain::Rectangle<double> & b = comp.getBoundingBoxDeg();
	// geoFrame.setBoundingBoxD(b.lowerLeft.x, b.lowerLeft.y, b.upperRight.x, b.upperRight.y);
	// geoFrame.setBoundingBoxD(comp.getBoundingBoxDeg());
	/*
	mout.pending<LOG_WARNING>(DRAIN_LOG(comp.getProj()));
	mout.pending<LOG_WARNING>(DRAIN_LOG(comp.getProj().isSet()));
	mout.pending<LOG_WARNING>(DRAIN_LOG(comp.getProj().isLongLat()));
	*/
	// geoFrame.setBoundingBoxD(comp.getBoundingBoxDeg());
	// mout.reject<LOG_WARNING>(DRAIN_LOG(geoFrame.getBoundingBoxDeg()));
	// mout.reject<LOG_WARNING>(DRAIN_LOG(geoFrame.getBoundingBoxNat()));
	geoFrame.setBoundingBoxNat(comp.getBoundingBoxNat());
	geoFrame.setGeometry(comp.getFrameWidth(), comp.getFrameHeight());
	// mout.accept<LOG_WARNING>(DRAIN_LOG(geoFrame.getBoundingBoxDeg()));
	// mout.accept<LOG_WARNING>(DRAIN_LOG(geoFrame.getBoundingBoxNat()));
	//geoFrame.setGeometry(comp.getFrameWidth(), comp.getFrameHeight());
	// mout.pending<LOG_WARNING>("now slowly: ", comp.getBoundingBoxDeg(), " = ", geoFrame.getBoundingBoxDeg(), " -> NATIVE: ", geoFrame.getBoundingBoxNat());

}


void RadarSVG::getCubicBezierConf(CubicBezierConf & conf, double angleStartR, double angleEndR) const {
	//conf.sectorCount = n;
	//conf.delta = 2.0*M_PI / static_cast<double>(n);
	conf.delta = angleEndR-angleStartR;
	const double k = 4.0/3.0 * ::tan(conf.delta/4.0);
	conf.radialCoeff   = ::sqrt(1.0 + k*k);
	conf.angularOffset = ::atan(k);
}

/// Single command to draw arc
/**
 *  \param drain::Point2D<int> & imgPoint - starting point, should be "already" compatible with (radiusM, azimutthStartR)
 */
void RadarSVG::cubicBezierTo(drain::svgPATH & elem, double radiusM, double azimuthStartR, double azimuthEndR) const {

	double delta = (azimuthEndR - azimuthStartR);

	if (delta > 2.0*M_PI){
		const double rounds = ::floor(delta/2.0*M_PI);
		azimuthEndR -= rounds*2.0*M_PI;
	}
	else if (delta < -2.0*M_PI){
		const double rounds = ::floor(-delta/2.0*M_PI);
		azimuthEndR += rounds*2.0*M_PI;
	}

	if (::abs(azimuthEndR - azimuthStartR) > conf.delta){// (M_PI_4)
		// split by two
		drain::Logger mout(__FILE__, __FUNCTION__);
		mout.debug("delta=", (azimuthEndR - azimuthStartR), " splitting...");
		double azimuthCenterR = (azimuthStartR + azimuthEndR)/2.0;
		mout.debug("=> ", azimuthStartR, '\t', azimuthCenterR);
		mout.debug("=> ", azimuthCenterR, '\t', azimuthEndR);
		// std::cerr << "=> " << azimuthCenterR << '\t' << azimuthEndR << "\n";
		// exit(1);
		cubicBezierTo(elem, radiusM, azimuthStartR,  azimuthCenterR);
		cubicBezierTo(elem, radiusM, azimuthCenterR, azimuthEndR);
		return;
	}

	drain::Point2D<int> imgPoint;
	convert(radiusM, azimuthEndR, imgPoint);
	cubicBezierTo(elem, imgPoint, radiusM, azimuthStartR, azimuthEndR);

}


/**
 *  \param drain::Point2D<int> & imgPoint - starting point, should be "already" compatible with (radiusM, azimutthStartR)
 */
void RadarSVG::cubicBezierTo(drain::svgPATH & elem, drain::Point2D<int> & imgPoint, double radiusM, double azimuthStartR, double azimuthEndR) const {

	// std::cerr << "Start " << azimuthStartR  << '\t' << azimuthEndR << "\n";

	CubicBezierConf conf;
	getCubicBezierConf(conf, azimuthStartR, azimuthEndR);

	const double radiusCtrl = radiusM * conf.radialCoeff;
	double azimuthCtrl = 0.0;

	// Geographic coordinates [metric (or degrees?]
	drain::Point2D<double> radarPointCtrl;

	// Final image coordinates [pix]
	drain::Point2D<int> imgPointCtrl1, imgPointCtrl2;

	azimuthCtrl = azimuthStartR + conf.angularOffset;
	radarPointCtrl.set(radiusCtrl*::sin(azimuthCtrl), radiusCtrl*::cos(azimuthCtrl));
	radarGeoToCompositeImage(radarPointCtrl, imgPointCtrl1);

	azimuthCtrl = azimuthEndR - conf.angularOffset;
	radarPointCtrl.set(radiusCtrl*::sin(azimuthCtrl), radiusCtrl*::cos(azimuthCtrl));
	radarGeoToCompositeImage(radarPointCtrl, imgPointCtrl2);

	elem.absolute<drain::svgPATH::CURVE_C>(imgPointCtrl1, imgPointCtrl2, imgPoint);

	convert(radiusM, azimuthEndR, imgPoint);

}

void RadarSVG::drawSector(drain::svgPATH & bezierElem, const drain::Range<double> & radius, const drain::Range<double> & azimuthR) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (azimuthR.width() > 0.0){
		mout.info("Drawing sector");
		// Scope for destructor. Or flush?
		//drain::svgPATH bezierElem(curve);
		moveTo(bezierElem, radius.max, azimuthR.min);
		cubicBezierTo(bezierElem, radius.max, azimuthR.min, azimuthR.max);
		lineTo(bezierElem, radius.min, azimuthR.max);
		if (radius.min > 0.0){
			// Draw inner arc
			cubicBezierTo(bezierElem, radius.min, azimuthR.max, azimuthR.min);
		}
		//lineTo(bezierElem, radius.max, azimuthR.min);
		close(bezierElem);
	}
	else {
		// Annulus
		// drain::svgPATH bezierElem(curve);
		moveTo(bezierElem, radius.max, 0.0);
		cubicBezierTo(bezierElem, radius.max, 0.0, +2.0*M_PI);
		if (radius.min > 0.0){
			mout.info("Drawing annulus");
			moveTo(bezierElem, radius.min, 0.0);
			// lineTo(bezierElem, radius.min, azimuthR.max);
			cubicBezierTo(bezierElem, radius.min, +2.0*M_PI, 0.0);
			// lineTo(bezierElem, radius.max, azimuthR.min);
		}
		else {
			mout.info("Drawing circle");
		}
		close(bezierElem);
	}
}

} // rack


