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



#include <drain/image/ImageFile.h>
#include <drain/util/Fuzzy.h>
#include <drain/util/StatusFlags.h>
#include <drain/imageops/DistanceTransformFillOp.h>
//#include <drain/imageops/RecursiveRepairerOp.h>


#include "data/DataCoder.h"
#include "hi5/Hi5.h"
//#include "hi5/Hi5Read.h"

#include "radar/RadarProj.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"


#include "cartesian-bbox.h"  // for cmdFormat called by


namespace rack {


void CartesianBBoxTest::exec() const {

	RackContext & ctx = getContext<RackContext>();

	drain::Logger mout(ctx.log, __FILE__, __FUNCTION__); // = getResources().mout;


	if ( ! ctx.composite.bboxIsSet() ){
		mout.warn("Bounding box undefined, skipping." );
		ctx.statusFlags.set(drain::StatusFlags::PARAMETER_ERROR);
		return;
	}

	if ( ! ctx.composite.geometryIsSet() ){
		// Maybe ok
		mout.info("Composite array geometry undefined");
	}

	/*
	if ( ! ctx.composite.projectionIsSet() ){
		mout.note("Projection undefined.");
	}
	*/


	// There may be no data, don't use Data<PolarSrc> etc here.
	Hi5Tree & p = ctx.getHi5(RackContext::CURRENT|RackContext::POLAR|RackContext::INPUT);
	// (*ctx.currentPolarHi5);

	// Derive range
	drain::VariableMap & attributes = p[ODIMPathElem::WHERE].data.attributes;
	double lon = attributes.get("lon", 0.0);
	double lat = attributes.get("lat", 0.0);

	//mout.warn(attributes );
	//
	const drain::VariableMap & a = p.hasChild("dataset1") ? p["dataset1"]["where"].data.attributes : attributes;

	/*
	 * There may be no data, don't use Data<PolarSrc> etc here.
	 */
	const double range = a.get("nbins",0.0) * a.get("rscale",0.0)  +  a.get("rstart",0.0);

	mout.debug(": lon=", lon, ", lat=", lat, ", range=", range);

	// TODO: str than aeqd?
	RadarProj pRadarToComposite;
	pRadarToComposite.setSiteLocationDeg(lon, lat);


	if ( ! ctx.composite.projectionIsSet() ){
		// Maybe ok (if latlong?)
		const std::string & s = pRadarToComposite.src.getProjDef();
		mout.note("Projection undefined, using Azimuthal Equidistant: '", s, "'");
		mout.advice("For 'longlat projection', use 'EPSG:4326'");
		ctx.composite.setProjection(s);
	}




	pRadarToComposite.setProjectionDst(ctx.composite.getProjection());

	drain::Rectangle<double> bboxM;
	//ctx.composite.determineBoundingBoxM(pRadarToComposite, odimIn.rscale*odimIn.geometry.width + odimIn.rscale/2.0 + odimIn.rstart, bboxM);
	if (range > 0.0) {
		pRadarToComposite.determineBoundingBoxM(range, bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
		//ctx.composite.determineBoundingBoxM(pRadarToComposite, range, bboxM);
	}
	else {
		mout.warn("could not derive range, using 250km ");
		pRadarToComposite.determineBoundingBoxM(250000, bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
		//ctx.composite.determineBoundingBoxM(pRadarToComposite, 250000, bboxM);
	}

	// New: also store
	drain::Rectangle<double> bboxD;
	int i,j;

	ctx.composite.m2deg(bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxD.lowerLeft.x, bboxD.lowerLeft.y);
	attributes["LL_lon"] = bboxD.lowerLeft.x;
	attributes["LL_lat"] = bboxD.lowerLeft.y;

	ctx.composite.m2pix(bboxM.lowerLeft.x, bboxM.lowerLeft.y, i, j);
	attributes["LL_i"] = i;
	attributes["LL_j"] = j;

	ctx.composite.m2deg(bboxM.upperRight.x, bboxM.upperRight.y, bboxD.upperRight.x, bboxD.upperRight.y);
	attributes["UR_lon"] = bboxD.upperRight.x;
	attributes["UR_lat"] = bboxD.upperRight.y;

	ctx.composite.m2pix(bboxM.upperRight.x, bboxM.upperRight.y, i, j);
	attributes["UR_i"] = i;
	attributes["UR_j"] = j;

	//dataExtentD.extend();

	//mout.warn("comp"  , ctx.composite.getBoundingBoxNat() );
	//mout.warn("radar" , bboxM );

	//bool
	// mutable
	bool overlap = (bboxM.isOverLapping(ctx.composite.getBoundingBoxNat()));

	mout.debug("overlap:" , static_cast<int>(overlap) , ", bboxD: " , bboxD );

	//resources.bbox.extend(bboxD);  // why?
	//mout.debug("combined bbox:" , resources.bbox );

	if (overlap){
		// ctx.statusFlags.set(RackContext::BBOX_STATUS);
		ctx.statusFlags.unset(drain::StatusFlags::INPUT_ERROR);
	}
	else {
		// ctx.statusFlags.unset(RackContext::BBOX_STATUS);
		ctx.statusFlags.set(drain::StatusFlags::INPUT_ERROR);
	}

	if (mode > 1){
		exit(overlap ? 0 : mode);
	}
}


void CartesianBBoxTile::exec() const {

	drain::Logger mout(__FILE__, __FUNCTION__);
	// TODO PROJ CHECK! => error, warn?

	//Composite & composite = getResources().composite;
	RackContext & ctx = getContext<RackContext>();
	Composite & composite = ctx.composite;

	/// Use composite's bbox as a starting point.
	drain::Rectangle<double> bboxTile = composite.getBoundingBoxDeg();

	/// Crop to desired size. (ie. section of the two bboxes)
	bboxTile.crop(bbox);

	// Project to image coordinates.
	int i, j;
	composite.deg2pix(bboxTile.lowerLeft.x, bboxTile.lowerLeft.y, i, j);
	int i2, j2;
	composite.deg2pix(bboxTile.upperRight.x, bboxTile.upperRight.y, i2, j2);

	// Justify the float-valued bbox to image coordinates (integers).
	// First, decrement UR image coords by one:
	--j;
	--i2;
	composite.pix2deg(i, j, bboxTile.lowerLeft.x, bboxTile.lowerLeft.y);
	composite.pix2deg(i2, j2, bboxTile.upperRight.x, bboxTile.upperRight.y);
	if ((bboxTile.getWidth() <= 0.0) || (bboxTile.getHeight() <= 0.0)){
		mout.error("negative-valued area in a bbox: " , bboxTile );
	}

	// Finally, redefine the composite scope.
	composite.setGeometry(static_cast<unsigned int>(i2-i), static_cast<unsigned int>(j-j2));
	composite.setBoundingBoxD(bboxTile);

}







}  // namespace rack::



// Rack
