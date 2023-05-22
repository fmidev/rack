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

#include "drain/util/Rectangle.h"
#include "AccumulatorGeo.h"



// using namespace std;


namespace drain
{

namespace image
{

/*
const double GeoFrame::DEG2RAD(M_PI/180.0);
const double GeoFrame::RAD2DEG(180.0/M_PI);
const int    GeoFrame::EARTH_RADIUSi(6371000);
const double GeoFrame::EARTH_RADIUS(6371000.0);
*/

/*
static const double DEG2RAD = M_PI/180.0;
static const double RAD2DEG = 180.0/M_PI;
static const int   EARTH_RADIUSi = 6371000;
static const double EARTH_RADIUS = 6371000.0;
*/

GeoFrame::GeoFrame(unsigned int frameWidth,unsigned int frameHeight) :
				xScale(1), yScale(1) //, debug(false)
{

	projGeo2Native.setProjectionSrc("+proj=latlong +ellps=WGS84 +datum=WGS84 +type=crs");
	//projR2M.setProjectionDst("+proj=utm +zone=35 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"); /// todo neutralize
	setGeometry(frameWidth,frameHeight);
	setBoundingBoxD(0,0,0,0);

}


/*
GeoFrame::GeoFrame(const GeoFrame & gf) : projR2M(gf.projR2M), xScale(1), yScale(1) {
	setGeometry(gf.getFrameWidth(), gf.getFrameHeight());
	setBoundingBoxR(gf.getBoundingBoxR());
}
*/



void GeoFrame::setGeometry(unsigned int w,unsigned int h) {

	//frameGeometry.setArea(w, h);
	frameWidth = w;
	frameHeight = h;
	updateScaling();

}

void GeoFrame::setBoundingBox(double lonLL, double latLL, double lonUR, double latUR){

	Logger mout(__FUNCTION__, __FILE__);

	// FIX: 2022/03: from AND to OR
	if (BBox::isMetric({lonLL, latLL}) || BBox::isMetric({lonUR, latUR})){
	// if (isMetric(lonLL, 180.0) || isMetric(latLL, 90.0) || isMetric(lonUR, 180.0) || isMetric(latUR, 90.0)){

		//mout.note() << "experimental: setting metric bbox: " << mout.endl; // << resources.bbox
		//mout.note() << lonLL << ' ' << latLL << ' ' << lonUR << ' ' << latUR << mout.endl;

		if (!projectionIsSet()){
			mout.advice("For best accuracy, set projection before metric bbox");
			//mout.error() << "projection must be set prior to setting metric bbox (" << ")" << mout.endl;
			//return;
		}

		if (projectionIsSet() && projGeo2Native.isLongLat()){
			mout.error("trying to set metric bbox (", ") on long-lat proj: ", getProjection());
			return;
		}

		setBoundingBoxM(lonLL, latLL, lonUR, latUR); // essentially modifies BoxR and BoxD

		mout.special() << "setting metric bbox: " << getBoundingBoxM() << mout.endl; // << resources.bbox

	}
	else {

		mout.special("setting deg bbox: ",lonLL,' ',latLL,' ',lonUR,' ',latUR);
		setBoundingBoxD(lonLL, latLL, lonUR, latUR);

	}


}



/*
void GeoFrame::setBoundingBoxR(double lonLL,double latLL,double lonUR,double latUR) {

	Logger mout(__FUNCTION__, __FILE__);

	bBoxR.set(lonLL, latLL, lonUR, latUR);

	updateBoundingBoxD();
	if (projectionIsSet()){
		updateBoundingBoxM();
	}
	else {
		//mout.warn() = "Projection should be set prior to bounding box";
	}

	updateScaling();

}
*/

void GeoFrame::setBoundingBoxD(double lonLL,double latLL,double lonUR,double latUR) {

	Logger mout(__FUNCTION__, __FILE__);

	bBoxD.set(lonLL, latLL, lonUR, latUR);

	updateBoundingBoxR();
	if (projectionIsSet()){
		updateBoundingBoxM();
		mout.debug("Updated native bbox: ", bBoxNative);
	}
	else {
		//mout.warn() = "Projection should be set prior to bounding box";
	}

	updateScaling();

}

void GeoFrame::setBoundingBoxM(double xLL,double yLL,double xUR,double yUR) {

	if (projGeo2Native.isSet()){
		// TODO if latlon!
		// Proj6: DEGREES
		// Set main BBOX (long, lat in DEGREES)
		projGeo2Native.projectInv(xLL, yLL, bBoxD.lowerLeft.x,  bBoxD.lowerLeft.y);
		projGeo2Native.projectInv(xUR, yUR, bBoxD.upperRight.x, bBoxD.upperRight.y);
		// Rescale also to degrees
		updateBoundingBoxR();
		/*
		// Set main BBOX (long, lat in radians)
		projR2M.projectInv(xLL, yLL, bBoxR.lowerLeft.x,  bBoxR.lowerLeft.y);
		projR2M.projectInv(xUR, yUR, bBoxR.upperRight.x, bBoxR.upperRight.y);
		// Rescale also to degrees
		updateBoundingBoxD();
		*/
	}
	else {
		Logger mout(__FUNCTION__, __FILE__);
		mout.warn("Tryng to set metric BBOX prior to setting projection");
	}

	/// Set exact metric bbox
	bBoxNative.set(xLL, yLL, xUR, yUR);

	updateScaling();
}

void GeoFrame::getCenterPixel(drain::Rectangle<double> & p) const {
	const int im = getFrameWidth()/2;
	const int jm = getFrameHeight()/2;
	pix2deg(im-1, jm-1, p.lowerLeft.x,  p.lowerLeft.y);
	pix2deg(im+1, jm+1, p.upperRight.x, p.upperRight.y);
}

/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
/*
double GeoFrame::getXScale() const {
	if (projR2M.isLongLat()){ // approximate!
		drain::Rectangle<double> p;
		getCenterPixel(p);
		return (p.upperRight.x-p.lowerLeft.x )/2.0 * DEG2RAD * EARTH_RADIUS * cos(DEG2RAD*(p.lowerLeft.y+p.upperRight.y)/2.0);
		//yScale = (pixelDeg.lowerLeft.y -pixelDeg.upperRight.y)/2.0 * DEG2RAD * EARTH_RADIUS;
	}
	else {
		return xScale;
	}
}
*/


/// Return vertical resolution of a pixel in meters (if metric) or degrees (if unprojected, "latlon").
/*
double GeoFrame::getYScale() const {
	if (projR2M.isLongLat()){ // approximate!
		drain::Rectangle<double> p;
		getCenterPixel(p);
		return (p.lowerLeft.y -p.upperRight.y)/2.0 * DEG2RAD * EARTH_RADIUS;
	}
	else {
		return yScale;
	}
}
*/
/// Assuming up-to-date bbox [rad], adjust bbox [deg]
void GeoFrame::updateBoundingBoxD(){
	bBoxD.lowerLeft.x  = RAD2DEG * bBoxR.lowerLeft.x;
	bBoxD.lowerLeft.y  = RAD2DEG * bBoxR.lowerLeft.y;
	bBoxD.upperRight.x = RAD2DEG * bBoxR.upperRight.x;
	bBoxD.upperRight.y = RAD2DEG * bBoxR.upperRight.y;
}

/// Assuming up-to-date bbox [rad], adjust bbox [deg]
void GeoFrame::updateBoundingBoxR(){
	bBoxR.lowerLeft.x  = DEG2RAD * bBoxD.lowerLeft.x;
	bBoxR.lowerLeft.y  = DEG2RAD * bBoxD.lowerLeft.y;
	bBoxR.upperRight.x = DEG2RAD * bBoxD.upperRight.x;
	bBoxR.upperRight.y = DEG2RAD * bBoxD.upperRight.y;
}

/// Given BBox in geo coords [rad], adjust metric bounding box. Do not update xScale or yScale.
void GeoFrame::updateBoundingBoxM(){

	if (projGeo2Native.isSet()){
		projGeo2Native.projectFwd(bBoxD.lowerLeft.x,  bBoxD.lowerLeft.y,  bBoxNative.lowerLeft.x,  bBoxNative.lowerLeft.y);
		projGeo2Native.projectFwd(bBoxD.upperRight.x, bBoxD.upperRight.y, bBoxNative.upperRight.x, bBoxNative.upperRight.y);
		// projR2M.projectFwd(bBoxR.lowerLeft.x,  bBoxR.lowerLeft.y,  bBoxNative.lowerLeft.x,  bBoxNative.lowerLeft.y);
		// projR2M.projectFwd(bBoxR.upperRight.x, bBoxR.upperRight.y, bBoxNative.upperRight.x, bBoxNative.upperRight.y);
	}
	else {
		// drain::Logger mout(__FUNCTION__, __FILE__);
		// mout.debug() << "could not (yet) set metric/native bbox" << mout.endl;
		// warn?
		if (isLongLat()){ // ie. native coords went radial above
			//mout.warn() << "LAN-LON.. could set R native bbox" << mout.endl;
			bBoxNative.assign(bBoxD);
		}
	}

	/*
	if (isLongLat()){ // ie. native coords went radial above
		bBoxNative.lowerLeft.x  *= RAD2DEG;
		bBoxNative.lowerLeft.y  *= RAD2DEG;
		bBoxNative.upperRight.x *= RAD2DEG;
		bBoxNative.upperRight.y *= RAD2DEG;
	}
	*/

}



/*
void GeoFrame::setProjection(const std::string &s){
	// const bool PROJ_SET = projectionIsSet();
	//mout.special() << "(metric) Area: " << bBoxNative.getArea() << mout;
	projGeo2Native.setProjectionDst(s);
}
*/

void GeoFrame::updateProjection(){

	Logger mout(__FUNCTION__, __FILE__);

	const bool METRIC_BBOX = (bBoxNative.getArea() > 40000);

	if (METRIC_BBOX){
		if (isLongLat()){
			/*
			projR2M.projectInv(xLL, yLL, bBoxR.lowerLeft.x,  bBoxR.lowerLeft.y);
			projR2M.projectInv(xUR, yUR, bBoxR.upperRight.x, bBoxR.upperRight.y);
			// Rescale also to degrees
			updateBoundingBoxD();
			*/
			mout.warn("Metric BBox (", bBoxNative, ") incompatible with the requested LongLat projection: '"); //, s, "'");
		}
		else {
			/*
			if (PROJ_SET) {
				// Problem: if proj is changed, which bbox should be used as a basis for adjusting the others?
				//mout.warn() << "Re-adjusting Metric Bounding Box (" << bBoxNative << "), risk of loosing precision" << mout;
				mout.warn("Using previous metric BBox (", bBoxNative, ") as reference");
			}
			*/
			projGeo2Native.projectInv(bBoxNative.lowerLeft.x,  bBoxNative.lowerLeft.y,  bBoxD.lowerLeft.x,  bBoxD.lowerLeft.y);
			projGeo2Native.projectInv(bBoxNative.upperRight.x, bBoxNative.upperRight.y, bBoxD.upperRight.x, bBoxD.upperRight.y);
			updateBoundingBoxR();
			/*
			projR2M.projectInv(bBoxNative.lowerLeft.x,  bBoxNative.lowerLeft.y,  bBoxR.lowerLeft.x,  bBoxR.lowerLeft.y);
			projR2M.projectInv(bBoxNative.upperRight.x, bBoxNative.upperRight.y, bBoxR.upperRight.x, bBoxR.upperRight.y);
			updateBoundingBoxD();
			*/
		}
	}
	else
		updateBoundingBoxM();

	updateScaling();
}


/// Notice: changed! For LatLon, consider approx? See composite
void GeoFrame::updateScaling()
{
	Logger mout(__FUNCTION__, __FILE__);
	// QUICK FIX, TODO redesign
	/*
	if (projR2M.isSet()){
		projR2M.projectFwd(bBoxR.lowerLeft.x,  bBoxR.lowerLeft.y,  bBoxNative.lowerLeft.x,  bBoxNative.lowerLeft.y);
		projR2M.projectFwd(bBoxR.upperRight.x, bBoxR.upperRight.y, bBoxNative.upperRight.x, bBoxNative.upperRight.y);
	}
	*/

	//if (projR2M.isSet() && projR2M.isLongLat()){
	if (false){

		// if (projR2M.isLongLat()){  // approximate!
		drain::Rectangle<double> p;
		getCenterPixel(p);
		xScale = (p.upperRight.x-p.lowerLeft.x )/2.0 * DEG2RAD * EARTH_RADIUS * cos(DEG2RAD*(p.lowerLeft.y+p.upperRight.y)/2.0);
		yScale = (p.lowerLeft.y -p.upperRight.y)/2.0 * DEG2RAD * EARTH_RADIUS;
		// return (p.upperRight.x-p.lowerLeft.x )/2.0 * DEG2RAD * EARTH_RADIUS * cos(DEG2RAD*(p.lowerLeft.y+p.upperRight.y)/2.0);
		// yScale = (pixelDeg.lowerLeft.y -pixelDeg.upperRight.y)/2.0 * DEG2RAD * EARTH_RADIUS;
		//	}
		/*
			else {
			// set metric bbox ?
			projR2M.projectFwd(bBoxR.lowerLeft.x,  bBoxR.lowerLeft.y,  bBoxNative.lowerLeft.x,  bBoxNative.lowerLeft.y);
			projR2M.projectFwd(bBoxR.upperRight.x, bBoxR.upperRight.y, bBoxNative.upperRight.x, bBoxNative.upperRight.y);
			xScale = (bBoxNative.upperRight.x - bBoxNative.lowerLeft.x) / static_cast<double>(getFrameWidth());
			yScale = (bBoxNative.upperRight.y - bBoxNative.lowerLeft.y) / static_cast<double>(getFrameHeight());
		}
		*/

	}
	else {
		if (geometryIsSet()){
			xScale = (bBoxNative.upperRight.x - bBoxNative.lowerLeft.x) / static_cast<double>(getFrameWidth());
			yScale = (bBoxNative.upperRight.y - bBoxNative.lowerLeft.y) / static_cast<double>(getFrameHeight());
		}
		//Logger mout(__FUNCTION__, __FILE__);
		//mout.warn() << "Tryng to update scaling prior to setting projection";
	}

	//mout.warn() << " scaling " << xScale << ',' << yScale << mout.endl;

}


void GeoFrame::cropWithM(double xLL, double yLL, double xUR, double yUR) {

	Logger mout(__FUNCTION__, __FILE__);
	//mout.warn() << "me " << *this << mout.endl;

	Rectangle<int> frame(0, 0, getFrameWidth(), getFrameHeight());

	Rectangle<int> cropper;
	m2pix(xLL,yLL, cropper.lowerLeft.x,  cropper.upperRight.y); // j swapped  "upside down"
	m2pix(xUR,yUR, cropper.upperRight.x, cropper.lowerLeft.y);  // j swapped  "upside down"
	//mout.warn() << " frame:" << frame <<  " crop0:" << cropper << mout.endl;

	// Why both? Perhaps just rounding
	cropper.lowerLeft.x--;
	cropper.lowerLeft.y--;
	cropper.upperRight.x++;
	cropper.upperRight.y++;
	frame.crop(cropper);
	//mout.warn()  << " crop:" << cropper << " => frame:" << frame << mout.endl;
	//mout.warn()  << " fatal:" << (frameHeight-1) << '-' <<  frame.upperRight.y << '*' << yScale << '+' << bBoxNative.lowerLeft.y << mout.endl;

	pix2LLm(frame.lowerLeft.x,  frame.upperRight.y,  xLL, yLL); // j swapped  "upside down"
	pix2LLm(frame.upperRight.x, frame.lowerLeft.y,   xUR, yUR); // j swapped  "upside down"

	// mout.warn()  << xLL << ',' << yLL << '\t' << xUR << ',' << yUR << mout.endl;

	/// Reset area and rescale
	setBoundingBoxM(xLL, yLL, xUR, yUR);
	setGeometry(frame.upperRight.x-frame.lowerLeft.x, frame.upperRight.y-frame.lowerLeft.y);  // +1 +1

}





void GeoFrame::updateDataExtent(const drain::Rectangle<double> & inputExtentD)
{

	if (dataBBoxD.getArea() == 0){
		dataBBoxD  = inputExtentD;
		dataOverlapD = inputExtentD;
	}
	else {
		// std::cerr << "Extending:" << dataExtentD << std::endl;
		dataBBoxD.extend(inputExtentD);
		dataOverlapD.contract(inputExtentD);
		// std::cerr << "    =>    " << dataExtentD << std::endl;
	}
}

std::ostream & GeoFrame::toStream(std::ostream & ostr) const {

	ostr << "frame " << this->getFrameWidth() << 'x' << this->getFrameHeight() << "\n";
	ostr << "   input coords: " << getCoordinateSystem() << '\n';
	// const std::string & projStr = ;
	const short int EPSG = projGeo2Native.getDst().getEPSG();
	if (EPSG)
		ostr << "   projection: epsg(" << EPSG << ")\n";
	else
		ostr << "   projection:" << getProjection() << '\n';

	ostr << "   bbox[nat]:   [" << getBoundingBoxM() << "]\n";
	ostr << "   bbox[rad]:   [" << getBoundingBoxR() << "]\n";
	ostr << "   bbox[deg]:   [" << getBoundingBoxD() << "]\n";
	ostr << "   resolution[m/pix]: (" << xScale << ',' << yScale << ")\n";

	return ostr;

}


} // namespace image
} // namespace drain



