/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */

//#include <stdexcept>


#include "RunLengthOp.h"


namespace drain
{

namespace image
{


void RunLengthHorzOp::traverseChannel(const Channel &src, Channel &dst) const {

	drain::Logger mout(getImgLog(), name, __FUNCTION__);

	int hResult = 0;

	const int width  = src.getWidth();
	const int height = src.getHeight();
	const CoordinateHandler2D handler(width, height, src.getCoordinatePolicy());

	Point2D<int> p;

	int pos;
	int length;
	int lengthTag;
	const int lengthMax = dst.getMax<int>();

	// Use may give 'threshold' as a relative [0.0,1.0] or absolute value.
	// => Convert to unambiguous thresholdAbs.
	const double thresholdAbs = src.getScaling().inv(threshold);

	for (int j=0; j<height; j++){

		pos = 0;  // pos will never decrease
		length = 0;

		while (pos<width){

			/// SCAN OVER EMPTY AREA, MARKING IT ZERO
			pos = pos+length; // jump over last segment
			while (pos < width){
				p.setLocation(pos,j);
				hResult = handler.handle(p); // Point(p,width,height);
				if (hResult && CoordinateHandler2D::IRREVERSIBLE)
					break;
				if (src.get<double>(p) >= thresholdAbs)
					break;
				dst.put(p,0);
				pos++;
			}

			/// Segment encountered; calculate length  (note max _span_ = w)
			for (length=0; length<width; length++){
				p.setLocation(pos+length,j);
				hResult = handler.handle(p);
				if (hResult && CoordinateHandler2D::IRREVERSIBLE)
					break;
				if (src.get<double>(p) < thresholdAbs)
					break;
			}
			// mark
			lengthTag = std::min(length,lengthMax);
			for (int i=0; i<length; i++){
				p.setLocation(pos+i,j);
				handler.handle(p);
				dst.put(p,lengthTag);
			}
			//pos = pos+length;

		}
	}

}


/// Computes lengths of horizontal or vertical segments.
/**
\code
drainage image.png  --runLengthHorz 128  -o out.png
\endcode

To detect line segments longer than 255 the user should use 16-bit result image, \c --type \c 16 .
Further, \c --scale command may be needed to make segment lengths visible in 16-bit scale.
 */
//void RunLengthVertOp::process(const ImageFrame &src, ImageFrame &dst) const {
void RunLengthVertOp::traverseChannel(const Channel &src, Channel &dst) const {


	drain::Logger mout(getImgLog(), name, __FUNCTION__);

	const int width  = src.getWidth();
	const int height = src.getHeight();
	const CoordinateHandler2D handler(width, height, src.getCoordinatePolicy());

	Point2D<int> p;

	int hResult = 0;

	int pos; // pos will never decrease
	int length;
	int lengthTag;
	const int lengthMax = dst.getMax<int>();

	// Use may give 'threshold' as a relative [0.0,1.0] or absolute value.
	// => Convert to unambiguous thresholdAbs.
	const double thresholdAbs = src.getScaling().inv(threshold);

	mout.debug(2) << "lengthMax=" << lengthMax << mout.endl;

	for (int i=0; i<width; i++){

		pos = 0;
		length = 0;

		while (pos<height){

			// skip
			pos = pos+length;
			while (pos < height){
				p.setLocation(i,pos);
				hResult = handler.handle(p); // Point(p,width,height);
				if (hResult && CoordinateHandler2D::IRREVERSIBLE)
					break;
				if (src.get<double>(p) >= thresholdAbs)
					break;
				dst.put(p,0);
				pos++;
			}

			// calculat e  (note max _span_ = w)
			for (length=0; length<height; length++){
				p.setLocation(i,pos+length);
				hResult = handler.handle(p); // Point(p,width,height);
				if (hResult && CoordinateHandler2D::IRREVERSIBLE)
					break;
				if (src.get<double>(p) < thresholdAbs)
					break;
			}
			// mark
			lengthTag = std::min(length,lengthMax);
			for (int j=0; j<length; j++){
				p.setLocation(i,pos+j);
				handler.handle(p); // Point(p,width,height);  // handled ok above
				dst.put(p,lengthTag);
			}
			//pos = pos+length;  // WARNING - UNNEEDED?

		}
	}

}



}
}
