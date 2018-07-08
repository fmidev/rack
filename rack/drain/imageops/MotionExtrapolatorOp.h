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
#ifndef MOTION_OP_H_
#define MOTION_OP_H_


#include <math.h>

#include "ImageOp.h"

namespace drain
{

namespace image
{


/**! Take a motion field and an image as input, and outputs an extrapolated image.
 * 
 *   T  = motion vector class (typically double)
 *   T2 = input image class (typically unsigned char).
 *
 *
 */
//template <class T=unsigned char,class T2=unsigned char>
class MotionExtrapolatorOp : public ImageOp
{
public:


	MotionExtrapolatorOp(const std::string &p="1,0") : ImageOp(__FUNCTION__,
			"Extrapolates an image using a motion field in injection.",
			"scale,offset",p){};
	 //Applies recursive corrections as post-interpolator. ,rw,rh,rl,rd

	/**
	 *
	 */
	void process(const ImageT<T> &motion,ImageT<T2> &dst) const {
		process(motion,dst,dst);
	};

	/**
	 *
	 */
	void process(const ImageT<T> &motion,ImageT<T2> &src,ImageT<T2> &dst) const {

		const int width  = src.getWidth();
		const int height = src.getHeight();

		AccumulationArray<T> cumulator(width, height);

		const float scale  = this->getParameter("scale",1.0f);
		const float offset = this->getParameter("offset",0.0f);

		const ImageT<T> &u = motion.getChannel(0);
		const ImageT<T> &v = motion.getChannel(1);

		CoordinateHandler c(width,height);
		Point2D<int> p;
		unsigned int a;

		dst.setGeometry(width,height,1,1);

		// WEIGHTED
		//if (motion.getChannelCount()>2){
		if ((motion.getChannelCount()>2) && (src.hasAlphaChannel())){
			cumulator.setMethod(AccumulationArray<T>::WAVG,1.0,1.0);
			std::cerr << "Motion Extrap: weighted\n";
			const ImageT<T> &wMotion = motion.getChannel(motion.getChannelCount()-1);
			const ImageT<T2> &wData = src.getAlphaChannel();

			if (drain::Debug > 3){
				ImageT<T2> tmp;
				ScaleOp<T,T2>(1.0,0).process(wMotion,tmp); File::write(tmp,"motion-w0.png");
			}
			//const Image<T> &w = motion.getChannel(2);  // TODO KORJAA MYÖHEMMIN!
			for (int j = 0; j < height; ++j) {
				for (int i = 0; i < width; ++i) {
					a = src.address(i,j);
					p.setLocation(i+scale*u.at(a)+offset,j+scale*v.at(a)+offset);
					// Still inside image?
					if (c.handle(p) == CoordinateHandler::UNCHANGED){
						cumulator.add(p.x,p.y,src.at(a),wData.at(a));
						//cumulator.add(p.x,p.y,src.at(a),0.15*wData.at(a)+0.85*wMotion.at(a));  // TODO 10 pois
						// if (w.at(a)>1.0) std::cerr << p << ':' << w.at(a) << '\n';
						//dst.at(a) = src.at(p.x,p.y);
						//dst.at(i,j,1) = w.at(a);
					}
				}
			}
		}
		// UNWEIGHTED
		else {
			cumulator.setMethod(AccumulationArray<T>::AVG);
			for (int j = 0; j < height; ++j) {
				for (int i = 0; i < width; ++i) {
					a = src.address(i,j);
					p.setLocation(i+scale*u.at(a)+offset,j+scale*v.at(a)+offset);
					// Still inside image?
					if (c.handle(p) == CoordinateHandler::UNCHANGED){  // TODO ::REVERSIBLE
						cumulator.add(p.x,p.y,src.at(a),10.0);
					}
					//cumulator.add(p.x,p.y,src.at(a),1.0);
				}
			}
		}

		cumulator.extractTo(dst,"dw");
		//dst.setChannelCount(1,1);


		if (drain::Debug > 0){
			std::cerr << this->name;
			std::cerr << " offset=" << offset;
			std::cerr << " scale=" << scale;
			dst.debug();
			std::cerr << '\n';
		}
	};

};

}
}


#endif /*MotionExtrapolatorOP_H_*/
