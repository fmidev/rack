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

#include "util/Rectangle.h"
#include "AccumulatorGeo.h"



// using namespace std;


namespace drain
{

namespace image
{


GeoFrame::GeoFrame(unsigned int frameWidth,unsigned int frameHeight) :
				xScale(1), yScale(1) //, debug(false)
{

	projR2M.setProjectionSrc("+proj=latlong +ellps=WGS84 +datum=WGS84");
	projR2M.setProjectionDst("+proj=utm +zone=35 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"); /// todo neutralize
	setGeometry(frameWidth,frameHeight);
	setBoundingBoxD(0,0,0,0);

}




void GeoFrame::setGeometry(unsigned int w,unsigned int h)
{
		//this->width  = width;
		//this->height = height;
		// if (w < 1) w = 1;
		// if (h < 1) h = 1;

		frameWidth = w;
		frameHeight = h;

		updateScaling();
}




void GeoFrame::setBoundingBoxR(double lonLL,double latLL,double lonUR,double latUR)
{
	extentR.xLowerLeft  = lonLL;
	extentR.yLowerLeft  = latLL;
	extentR.xUpperRight = lonUR;
	extentR.yUpperRight = latUR;

	static const double R2D = 180.0/M_PI;
	extentD.xLowerLeft  = lonLL*R2D;
	extentD.yLowerLeft  = latLL*R2D;
	extentD.xUpperRight = lonUR*R2D;
	extentD.yUpperRight = latUR*R2D;

	updateScaling();

}

void GeoFrame::setBoundingBoxM(double xLL,double yLL,double xUR,double yUR)
{
	double lonLL, latLL, lonUR, latUR;
	projR2M.projectInv(xLL, yLL, lonLL, latLL);
	projR2M.projectInv(xUR, yUR, lonUR, latUR);
	setBoundingBoxR(lonLL,latLL,lonUR,latUR);
}


void GeoFrame::cropWithM(double xLL, double yLL, double xUR, double yUR) {

	MonitorSource mout("GeoFrame", __FUNCTION__);

	Rectangle<int> frame(0, 0, getFrameWidth(), getFrameHeight());

	Rectangle<int> cropper;
	m2pix(xLL,yLL, cropper.xLowerLeft,  cropper.yUpperRight); // j swapped  "upside down"
	m2pix(xUR,yUR, cropper.xUpperRight, cropper.yLowerLeft);  // j swapped  "upside down"
	// mout.warn() << " frame:" << frame <<  " crop0:" << cropper;

	cropper.xLowerLeft--;
	cropper.yLowerLeft--;
	cropper.xUpperRight++;
	cropper.yUpperRight++;
	frame.crop(cropper);
	// mout.warn()  << " crop:" << cropper << " => frame:" << frame << mout.endl;

	pix2LLm(frame.xLowerLeft,  frame.yUpperRight,  xLL, yLL); // j swapped  "upside down"
	pix2LLm(frame.xUpperRight, frame.yLowerLeft,   xUR, yUR); // j swapped  "upside down"

	/// Reset area and rescale
	setBoundingBoxM(xLL, yLL, xUR, yUR);
	setGeometry(frame.xUpperRight-frame.xLowerLeft, frame.yUpperRight-frame.yLowerLeft);  // +1 +1

}



void GeoFrame::setProjection(const std::string &s){
	projR2M.setProjectionDst(s);
	updateScaling();
}


/// Notice: changed!
void GeoFrame::updateScaling()
{
	projR2M.projectFwd(extentR.xLowerLeft,  extentR.yLowerLeft,  extentM.xLowerLeft,  extentM.yLowerLeft);
	projR2M.projectFwd(extentR.xUpperRight, extentR.yUpperRight, extentM.xUpperRight, extentM.yUpperRight);

	xScale = (extentM.xUpperRight - extentM.xLowerLeft) / getFrameWidth();
	yScale = (extentM.yUpperRight - extentM.yLowerLeft) / getFrameHeight();

}



void GeoFrame::updateDataExtent(const drain::Rectangle<double> & inputExtentD)
{

	if (dataExtentD.getArea() == 0){
		dataExtentD = inputExtentD;
	}
	else {
		// std::cerr << "Extending:" << dataExtentD << std::endl;
		dataExtentD.extend(inputExtentD);
		// std::cerr << "    =>    " << dataExtentD << std::endl;
	}
}


std::ostream &operator<<(std::ostream &ostr, const drain::image::AccumulatorGeo & acc){
	ostr << "CumulatorGeo (frame " << acc.getFrameWidth() << 'x' << acc.getFrameHeight() << ") \n";
	ostr << (const drain::image::AccumulationArray &)acc << '\n';
	ostr << "   allocated (" << acc.getWidth() << 'x' << acc.getHeight() << ") \n";
	//ostr << "   resolution, metres: " << acc.getScaleX() << 'x' << acc.getScaleY() << "\n";
	ostr << "   coord system: " << acc.getCoordinateSystem() << '\n';
	ostr << "   proj:  " << acc.getProjection() << '\n';

	ostr << "   scope, metres:  [" << acc.getBoundingBoxM() << "]\n";
	ostr << "   scope, radians: [" << acc.getBoundingBoxR() << "]\n";
	ostr << "   scope, degrees: [" << acc.getBoundingBoxD() << "]\n";

	return ostr;
}


} // namespace image
} // namespace drain



// Drain
