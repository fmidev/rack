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
#ifndef POLAR_ATTENUATION_H_
#define POLAR_ATTENUATION_H_

//#include "radar.h"
#include <drain/image/ImageOp.h>
//#include "image/ImageOp.h"


namespace rack
{


/// Computes attenuation caused by \em precipitation.
/*!
 *
 *  By Ulrich Blahak (and Isztar Zawadski, Heikki Pohjola, Jarmo Koistinen),
 *  for liquid precipitation (including bright band):
 *  \f[
 *      k(Z) = 1.12\cdot10^{-4} Z_e^{0.62}
 *  \f]
 *  and for snow:
 *  \f[
 *      k(Z) = 1.1\cdot10^{-7} Z_e + 2.1\cdot10^{-5} Z_e^{0.5}
 *  \f]
 *
 * Hence, the general expression for precipitation attenuation is
 * \f[
 *      k(Z) = c Z_e^p + c_2 Z_e^{p_2}.
 * \f]
 * In the code, \c coeff, \c pow,  \c coeff2, and \c pow2
 * refer to \f$c\f$, \f$p\f$, \f$c_2\f$, and \f$p_2\f$, respectively.
 */
/*
template <class T = unsigned char,class T2 = unsigned char>
class PolarAttenuation : public drain::image::ImageOp<T,T2>
{
public:

	PolarAttenuation(const std::string p="1.12e-4,0.62,0,0"){
		this->setInfo("Computes attenuation using cZe^p + c2Ze^p2. Alternatively, set c 'rain' or 'snow'.",
				"c,p,c2,p2",p);
	};

	void filter(const ImageT<T> &src, ImageT<T2> &dst) const ;

};
*/

/*
template <class T,class T2>
void PolarAttenuation<T,T2>::filter(const ImageT<T> &src, ImageT<T2> &dst) const
{
	const std::string & cStr = this->parameters.get("c","1.12e-4");
	double c  = this->parameters.get("c",1.12e-4);
	double p  = this->parameters.get("p",0.62);
	double c2 = this->parameters.get("c2",0);
	double p2 = this->parameters.get("p2",0);

	if (cStr == "rain"){
		c = 1.12E-7;
		p = 0.62;
		c2 = 0;
		p2 = 0;
	}
	else if (cStr == "snow"){
		c = 1.1E-7;
		p = 1;
		c2 = 2.1E-5;
		p2 = 0.5;
	}
	else if (cStr == "demo"){
		c = 1.11e-11;
		p = 0.9;
		c2 = 0.0;
		p2 = 0.0;
	}
	else if (cStr == "demo2"){
		c = 1.12e-03;
		p = 0.12;
		c2 = 0.0;
		p2 = 0.0;
	}

	const unsigned int srcWidth = src.getGeometry().getWidth();
	const unsigned int srcHeight = src.getGeometry().getHeight();
	const unsigned int srcChannels = src.getGeometry().getImageChannelCount();
	dst.setGeometry(srcWidth,srcHeight,srcChannels);

	std::cout << "Attn: << " << src.getGeometry() << "\n";
	std::cout << "Attn: << " << dst.getGeometry() << "\n";

	float binDepth = src.properties.get("BIN_DEPTH",500.0f);

	std::cerr << "Attn:" << c << ','<< p << ','<< c2 << ','<< p2 << '\n';

	// Main loops

	/// Observed data (dBZ) 
	double dbzObs;

	/// Attenuation (Z) in the currenty bin (calculation involves cumulated attenuation)
	double zAttnBin;

	/// Cumulated attenuation (Z)
	double zAttnCumulated=0;

	/// Degree of confidence  Cumulated attenuation (dBZ)
	double confidence;
	//dbzAttnCumulated;

	/// Corrected (Z)
	double zTrue;

	//double dbzTrue;

	for (unsigned int k = 0; k < srcChannels; ++k) {
		for (unsigned int j = 0; j < srcHeight; ++j) {
			zAttnCumulated = 0;
			for (unsigned int i = 0; i < srcWidth; ++i) {
				dbzObs = (src.at(i,j,k)-64)*2;

				/// Actual Z derived from the actual dBZ,
				zTrue = rack::dbzToZ( dbzObs ) + zAttnCumulated;

				/// one-way attenuation per one bin
				/// two-way attenuation per one bin
				// TODO: bin width should be in the exponent?
				zAttnBin = c*pow(zTrue,p) * binDepth * 2.0;
				zAttnCumulated += zAttnBin;

				//confidence = (::radar::zToDbz(zAttnCumulated)+64)*4;  // TODO
				confidence = zAttnCumulated/100.0;  // TODO
				confidence = 255.0/(1+confidence*confidence);
				//dbzTrue = ::radar::zToDbz( zTrue );

				//if (j&3 == 3)	    			dst.at(i,j,k) = static_cast<T2>(255.0-confidence);
				//else
				//dst.at(i,j,k) = static_cast<T2>(drain::radar::zToDbz(drain::radar::dbzToZ( dbzObs ) )/2 + 64);
				//dst.at(i,j,k) = static_cast<T2>(zToDbz(zAttnCumulated));
				dst.at(i,j,k) = static_cast<T2>(confidence);
			}
			//std::cerr << "attn = " << zAttnCumulated << "\n";
		}		
	}


}
*/

} // rack::

#endif /*POLAR_ATTENUATION_H_*/

// Rack
