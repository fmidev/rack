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
#ifndef PALETTEOP_H_
#define PALETTEOP_H_


#include "image/Palette.h"

#include "ImageOp.h"
#include "CopyOp.h"



namespace drain
{

namespace image
{

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table.
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  -
 */
class PaletteOp : public ImageOp
{
public:

	PaletteOp() : ImageOp(__FUNCTION__,"Applies colour palette to an image"), palettePtr(&myPalette) {
		this->parameters.reference("scale", scale = 1.0);
		this->parameters.reference("offset", offset = 0.0);
		specialCodes.title = "Special codes";
	};

	PaletteOp(const Palette & palette) : ImageOp(__FUNCTION__,"Applies colour palette to an image"), palettePtr(&myPalette) {
		this->parameters.reference("scale", scale = 1.0);
		this->parameters.reference("offset", offset = 0.0);
		specialCodes.title = "Special codes";
		setPalette(palette);
	};


	PaletteOp(const std::string & filename = "") : ImageOp(__FUNCTION__,"Applies colour palette to an image"), palettePtr(&myPalette) {
			this->parameters.reference("scale", scale = 1.0);
			this->parameters.reference("offset", offset = 0.0);
			specialCodes.title = "Special codes";
			myPalette.load(filename);
		};

	virtual ~PaletteOp(){};


	//void processOLD(const ImageFrame &src,Image &dst) const ;

	void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;

	virtual
	void help(std::ostream & ostr = std::cout) const;

	/// Creates an internal palette by taking the colors on the diagonal from lower left to upper right corner.
	/*! Maximally 256 colors.
	 *
	 */
	//void setPalette(const ImageFrame &palette) const; //,unsigned int maxColors = 256);
	void setPalette(const Palette &palette); //,unsigned int maxColors = 256);

	inline
	const Palette & getPalette() const {
		return *palettePtr;
	}


	/// Creates a gray palette ie. "identity mapping" from gray (x) to rgb (x,x,x).
	// TODO T 256, T2 32768
	void setGrayPalette(unsigned int iChannels=3,unsigned int aChannels=0,float brightness=0.0,float contrast=1.0) const;


	/// Prescale intensities with scale*i + offset.
	double scale;

	/// Prescale intensities with scale*i + offset.
	double offset;

	void registerSpecialCode(const std::string code, double f);

	// protect:
	/// Intensity mappings set by user, originally with std::string keys in Palette.
	//std::map<double,PaletteEntry > specialCodes;
	Palette specialCodes;


protected:

	void initialize() const {};

	void makeCompatible(const ImageFrame &src, Image &dst) const;

	//mutable Image paletteImage;
	Palette myPalette;
	const Palette *palettePtr;

};



}

}

#endif /*PALETTEOP_H_*/

// Drain
