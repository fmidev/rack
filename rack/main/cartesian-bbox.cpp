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



#include "drain/util/Fuzzy.h"

#include "drain/image/File.h"

#include "drain/imageops/DistanceTransformFillOp.h"
//#include "drain/imageops/RecursiveRepairerOp.h"


#include "data/DataCoder.h"
#include "hi5/Hi5.h"
//#include "hi5/Hi5Read.h"

#include "radar/Coordinates.h"
#include "radar/Composite.h"
//#include "radar/Extractor.h"


#include "cartesian-bbox.h"  // for cmdFormat called by


namespace rack {

void CartesianBBox::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	RackResources & resources = getResources();

	resources.composite.setBoundingBox(resources.bbox);
	/*
	drain::Point2D<double> & ll = resources.bbox.lowerLeft;
	drain::Point2D<double> & ur = resources.bbox.upperRight;

	if (isMetric(ll.x, 180.0) && isMetric(ll.y, 90.0) && isMetric(ur.x, 180.0) && isMetric(ur.y, 90.0)){

		mout.note() << "experimental: setting metric bbox: " << resources.bbox << mout.endl;

		if (!resources.composite.projectionIsSet()){
			mout.error() << "projection must be set prior to setting metric bbox (" << resources.bbox << ")" << mout.endl;
			return;
		}

		if (resources.composite.projR2M.isLongLat()){
			mout.error() << "trying to set metric bbox (" << resources.bbox << ") on long-lat proj: ";
			mout         << resources.composite.getProjection() << mout.endl;
			return;
		}

		resources.composite.setBoundingBoxM(resources.bbox);
		resources.bbox.set(resources.composite.getBoundingBoxD());
		mout.note() << "bbox in degrees: " << resources.bbox << mout.endl;
		return;
	}

	resources.composite.setBoundingBoxD(resources.bbox);
	*/
}

void CartesianBBoxTest::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__); // = getResources().mout;

	RackResources & resources = getResources();

	if ( ! resources.composite.bboxIsSet() ){
		// error() ?
		mout.warn() << "Bounding box undefined, skipping." << mout.endl;
		return;
	}

	if ( ! resources.composite.projectionIsSet() ){
		// Maybe ok
		mout.note() << "Projection undefined." << mout.endl;
	}

	if ( ! resources.composite.geometryIsSet() ){
		// Maybe ok
		mout.note() << "Composite array geometry undefined" << mout.endl;
	}

	// There may be no data, don't use Data<PolarSrc> etc here.
	Hi5Tree & p = (*resources.currentPolarHi5);

	// Derive range
	drain::VariableMap & attributes = p["where"].data.attributes;
	double lon = attributes.get("lon", 0.0);
	double lat = attributes.get("lat", 0.0);
	//mout.warn() << attributes << mout.endl;
	//
	const drain::VariableMap & a = p.hasChild("dataset1") ? p["dataset1"]["where"].data.attributes : attributes;
	//mout.warn() << (int)p.hasChild("dataset1") << mout.endl;
	//mout.warn() << a << mout.endl;

	/*
	 * There may be no data, don't use Data<PolarSrc> etc here.
	 */
	const double range = a.get("nbins",0.0) * a.get("rscale",0.0)  +  a.get("rstart",0.0);

	mout.debug() << ": lon=" << lon << ", lat=" << lat << ", range=" << range << mout.endl;

	// TODO: str than aeqd?
	RadarProj pRadarToComposite;
	pRadarToComposite.setSiteLocationDeg(lon, lat);
	pRadarToComposite.setProjectionDst(resources.composite.getProjection());

	drain::Rectangle<double> bboxM;
	//resources.composite.determineBoundingBoxM(pRadarToComposite, odimIn.rscale*odimIn.geometry.width + odimIn.rscale/2.0 + odimIn.rstart, bboxM);
	if (range > 0.0) {
		pRadarToComposite.determineBoundingBoxM(range, bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
		//resources.composite.determineBoundingBoxM(pRadarToComposite, range, bboxM);
	}
	else {
		mout.warn() << "could not derive range, using 250km "<< mout.endl;
		pRadarToComposite.determineBoundingBoxM(250000, bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxM.upperRight.x, bboxM.upperRight.y);
		//resources.composite.determineBoundingBoxM(pRadarToComposite, 250000, bboxM);
	}

	// New: also store
	drain::Rectangle<double> bboxD;
	int i,j;

	resources.composite.m2deg(bboxM.lowerLeft.x, bboxM.lowerLeft.y, bboxD.lowerLeft.x, bboxD.lowerLeft.y);
	attributes["LL_lon"] = bboxD.lowerLeft.x;
	attributes["LL_lat"] = bboxD.lowerLeft.y;

	resources.composite.m2pix(bboxM.lowerLeft.x, bboxM.lowerLeft.y, i, j);
	attributes["LL_i"] = i;
	attributes["LL_j"] = j;

	resources.composite.m2deg(bboxM.upperRight.x, bboxM.upperRight.y, bboxD.upperRight.x, bboxD.upperRight.y);
	attributes["UR_lon"] = bboxD.upperRight.x;
	attributes["UR_lat"] = bboxD.upperRight.y;

	resources.composite.m2pix(bboxM.upperRight.x, bboxM.upperRight.y, i, j);
	attributes["UR_i"] = i;
	attributes["UR_j"] = j;

	//dataExtentD.extend();

	//mout.warn() << "comp"  << resources.composite.getBoundingBoxM() << mout.endl;
	//mout.warn() << "radar" << bboxM << mout.endl;

	overlap = bboxM.isOverLapping(resources.composite.getBoundingBoxM());

	mout.debug() << "overlap:" << static_cast<int>(overlap) << mout.endl;

	mout.debug() << "bboxD:" << bboxD << mout.endl;

	//cBBox.bbox.updateDataExtent(bD);
	//cBBox.
	resources.bbox.extend(bboxD);  // extern
	mout.debug() << "combined bbox:" << resources.bbox << mout.endl;

	if (!overlap)
		resources.errorFlags.set(RackResources::INPUT_ERROR);

	if (value > 1){
		exit(overlap ? 0 : value);
	}
}


void CartesianBBoxTile::exec() const {

	drain::Logger mout(__FUNCTION__, __FILE__);
	// TODO PROJ CHECK! => error, warn?

	Composite & composite = getResources().composite;

	/// Use composite's bbox as a starting point.
	drain::Rectangle<double> bboxTile = composite.getBoundingBoxD();

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
		mout.error() << "negative-valued area in a bbox: " << bboxTile << mout.endl;
	}

	// Finally, redefine the composite scope.
	composite.setGeometry(static_cast<unsigned int>(i2-i), static_cast<unsigned int>(j-j2));
	composite.setBoundingBoxD(bboxTile);

}







}  // namespace rack::



// Rack
