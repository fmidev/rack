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
#ifndef DRAIN_PALETTEOP_H
#define DRAIN_PALETTEOP_H

#include <map>
#include "drain/util/Bank.h"
#include "drain/image/Palette.h"

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
		this->parameters.link("scale", scale = 1.0);
		this->parameters.link("offset", offset = 0.0);
		specialCodes.title = "Special codes";
	};

	PaletteOp(const Palette & palette) : ImageOp(__FUNCTION__,"Applies colour palette to an image"), palettePtr(&myPalette) {
		this->parameters.link("scale", scale = 1.0);
		this->parameters.link("offset", offset = 0.0);
		specialCodes.title = "Special codes";
		setPalette(palette);
	};


	PaletteOp(const std::string & filename = "") : ImageOp(__FUNCTION__,"Applies colour palette to an image"), palettePtr(&myPalette) {
			this->parameters.link("scale", scale = 1.0);
			this->parameters.link("offset", offset = 0.0);
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

	/// NEW. Return a palette (like "DBZH") from Palette::paletteBank if already exists; else try to read in from TXT or JSON file.
	/**
	 *   Note: if not existing, creates an empty palette.
	static
	Palette & ensurePalette(const std::string & key);
	 */

	/// NEW. Return a palette (like "DBZH") from Palette::paletteBank if already exists; else try to read in from TXT or JSON file.
	static
	Palette & getPalette(const std::string & key);

	/// NEW Load palette to Palette::paletteBank by key (like "DBZH") from TXT or JSON file.
	static
	Palette & loadPalette(const std::string & key);

	/// Uses the original coloured palette to create a gray-level palette.
	// static
	// Palette & getGrayPalette(const std::string & key);

	// TODO: Refinement. Or better in actual op exec, with lookup? Or: retrieve a lookup...
	// const Palette & getPalette(const std::string & key) const;


	/// Creates a gray palette ie. "identity mapping" from gray (x) to rgb (x,x,x).
	// TODO T 256, T2 32768
	// void setGrayPalette(unsigned int iChannels=3, unsigned int aChannels=0, float brightness=0.0,float contrast=1.0) const;


	/// Prescale intensities with scale*i + offset.
	double scale;

	/// Prescale intensities with scale*i + offset.
	double offset;

	/// Connect special code (label) to intensity
	/**
	 *   Palette file is independent of storage type and therefore independent of numeric values used for
	 *   special codes. That information is transmitted in image meta data, and connected to the palette with
	 *   this function.
	 *
	 */
	void registerSpecialCode(const std::string & code, double d);

	// protect:
	/// Intensity mappings set by user, originally with std::string keys in Palette.
	Palette specialCodes;

	/*
	static inline
	std::map<std::string,drain::image::Palette> & getPaletteMap(){
		return paletteMap;
	};
	*/
	//typedef	std::map<std::string,drain::image::Palette> palette_map_t;
	typedef	PaletteMap palette_map_t;

	/// Returns a static palette map which is initially empty.
	static
	palette_map_t & getPaletteMap();
	/*
	static
	std::map<std::string,drain::image::Palette> paletteMap;
	*/
protected:


	//void initialize() const {Image & dst};

	void getDstConf(const ImageConf &src, ImageConf &dst) const;
	//void makeCompatible(const ImageFrame &src, Image &dst) const;

	//mutable Image paletteImage;
	Palette myPalette;
	const Palette *palettePtr;

};



}

}

#endif /*PALETTEOP_H_*/

// Drain
