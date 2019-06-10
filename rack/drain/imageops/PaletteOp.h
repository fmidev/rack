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


#include "image/File.h"
#include "image/TreeSVG.h"

#include "util/JSONtree.h"


#include "ImageOp.h"
#include "CopyOp.h"


namespace drain
{

namespace image
{

class PaletteEntry : public std::vector<double> {

public:

	inline
	PaletteEntry() : hidden(false){
	}

	/// Description appearing in legends
	std::string label;

	/// Suggests hiding the entry in legends. Does not affect colouring of images.
	/**
	 *   When true, indicates that this entry is less significant and can be excluded in legends
	 *   created by
	 *
	 */
	bool hidden;

};

class Palette : public std::map<double,PaletteEntry > {

public:

	void reset();

	/// Loads a palette from text file
	void load(const std::string &filename);

	/// Loads a palette from text file
	void loadTXT(std::ifstream & ifstr);


	/// Loads a palette from text file
	void loadJSON(std::ifstream & ifstr);

	/// Creates a palette from json object
	/**
	 *  \param json - JSON structure combining optional \c metadata and compulsory \c entries section.
	 */
	// void convertJSON(const drain::JSON::tree_t & json);

	inline
	bool hasAlpha() const { return _hasAlpha; };

	/// Returns a legend as an SVG graphic.
	void getLegend(TreeSVG & svg, bool up = true) const;

	/// Name of the palette. In reading files, the first comment line (without the prefix '#') is copied to this. Legend shows the title on top.
	std::string title;

	/// Certain intensities (before scaling with gain and offset) may require special treatment.
	std::map<std::string,PaletteEntry > specialCodes; // To be read from palette

	/// Extend palette to contain n entries ("colors") by adding intermediate entries ("colors")
	void refine(size_t n=256);

protected:
	bool _hasAlpha;
	unsigned int colorCount;
	//void skipLine(std::ifstream &ifstr) const;

	/// Creates a palette from json object
	void convertJSON(const drain::JSON::tree_t & json, int depth);

};

inline
std::ostream & operator<<(std::ostream &ostr, const PaletteEntry & e){
	for (PaletteEntry::const_iterator it = e.begin(); it != e.end(); ++it){
		ostr << '\t' << *it;
	}
	return ostr;
}

inline
std::ostream & operator<<(std::ostream &ostr, const Palette & p){

	ostr << "Palette '" << p.title << "'\n";
	for (std::map<std::string,PaletteEntry >::const_iterator it = p.specialCodes.begin(); it != p.specialCodes.end(); ++it){
		ostr << '#' << it->first << ':' << it->second << '\n';
	}
	for (Palette::const_iterator it = p.begin(); it != p.end(); ++it){
		ostr << it->first << ':' << it->second << '\n';
	}
	return ostr;
}

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table.
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  -
 */
class PaletteOp : public ImageOp
{
public:

	PaletteOp() : ImageOp(__FUNCTION__,"Applies colour palette to an image") {
		this->parameters.reference("scale", scale = 1.0);
		this->parameters.reference("offset", offset = 0.0);
	};

	PaletteOp(const Palette & palette) : ImageOp(__FUNCTION__,"Applies colour palette to an image") {
		this->parameters.reference("scale", scale = 1.0);
		this->parameters.reference("offset", offset = 0.0);
		setPalette(palette);
	};


	PaletteOp(const std::string & filename = "") : ImageOp(__FUNCTION__,"Applies colour palette to an image") {
			this->parameters.reference("scale", scale = 1.0);
			this->parameters.reference("offset", offset = 0.0);
			palette.load(filename);
		};

	virtual ~PaletteOp(){};

	void process(const ImageFrame &src,Image &dst) const ;

	virtual
	void help(std::ostream & ostr = std::cout) const;

	/// Creates an internal palette by taking the colors on the diagonal from lower left to upper right corner.
	/*! Maximally 256 colors.
	 *
	 */
	//void setPalette(const ImageFrame &palette) const; //,unsigned int maxColors = 256);
	void setPalette(const Palette &palette); //,unsigned int maxColors = 256);


	/// Creates a gray palette ie. "identity mapping" from gray (x) to rgb (x,x,x).
	// TODO T 256, T2 32768
	void setGrayPalette(unsigned int iChannels=3,unsigned int aChannels=0,float brightness=0.0,float contrast=1.0) const;


	/// Prescale intensities with scale*i + offset.
	double scale;

	/// Prescale intensities with scale*i + offset.
	double offset;

	void setSpecialCode(const std::string code, double f);

	// protect:
	/// Intensity mappings set by user, originally with std::string keys in Palette.
	std::map<double,PaletteEntry > specialCodes;


protected:

	void initialize() const {};

	void makeCompatible(const ImageFrame &src, Image &dst) const;

	//mutable Image paletteImage;
	Palette palette;

};



}

}

#endif /*PALETTEOP_H_*/

// Drain
