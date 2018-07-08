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
#ifndef MOTIONIllustrator_OP_H_
#define MOTIONIllustrator_OP_H_


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
template <class T=unsigned char,class T2=unsigned char>
class MotionIllustratorOp : public ImageOp<T,T2>
{
public:


	MotionIllustratorOp(const std::string &p="10,10,1.0,1.0,0.0") : ImageOp<T,T2>("MotionIllustratorOp",
			"Illustrates an image using a motion field in injection.",
			"m,n,scale,contrast,brightness",p){};

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

		const int m = this->getParameter("m",20);
		const int n = this->getParameter("n",20);

		const float scale  = this->getParameter("scale",1.0f);

		// Background image
		const float contrast  = this->getParameter("contrast",1.0f);
		const float brightness = this->getParameter("brightness",0.0f);
		PaletteOp<T2,T2> op;
		op.setGrayPalette(3,0,brightness,contrast);
		op.process(src,dst);

		const ImageT<T> &u = motion.getChannel(0);
		const ImageT<T> &v = motion.getChannel(1);
		const ImageT<T> &w = motion.getChannel(motion.getChannelCount()-1);

		const bool weighted = (motion.getChannelCount()>2);

		CoordinateHandler c(width,height);
		Point2D<int> p;
		unsigned int a;
		int i2,j2,magn; //,l;
		const int iOffset = width/m/2;
		const int jOffset = height/n/2;
		float u2,v2,w2;



		for (int j = 0; j < n; ++j) {
			for (int i = 0; i < m; ++i) {

				//std::cerr << "spot " << i << '\t' << j << '\n';
				i2 = (width*i)/m  + iOffset;
				j2 = (height*j)/n + jOffset;
				a = src.address(i2,j2);
				dst.at(a) = 128;
				dst.at(i2+1,j2+0) = 128;
				dst.at(i2+0,j2+1) = 128;
				dst.at(i2-1,j2+0) = 128;
				dst.at(i2+0,j2-1) = 128;
				//dst.at(i2,j2,1) = 255;

				u2 = u.at(a);
				v2 = v.at(a);
				magn = static_cast<int>(scale * sqrt(u2*u2 + v2*v2));
				for (int k=0; k<magn; k++){
					p.setLocation(i2 + (u2*k)/magn,j2 + (v2*k)/magn);
					//if ((i==5)&&(j==5))std::cerr << p << '\n';
					// Still inside image?
					//if (true){ //
					if (c.handle(p) == CoordinateHandler::UNCHANGED){
						w2 = weighted ? 255-(255*16)/(16+w.at(a)) : 128;
						dst.at(p.x,p.y,0) = static_cast<T2>(127+w2/2);
						dst.at(p.x,p.y,1) = static_cast<T2>(w2);
						dst.at(p.x,p.y,2) = static_cast<T2>(127+w2/2);
					}
				}
			}
		}


	};

};

}
}


#endif /*MotionIllustratorOP_H_*/
