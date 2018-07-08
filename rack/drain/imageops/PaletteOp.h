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
#ifndef PALETTEOP_H_
#define PALETTEOP_H_


#include "image/File.h"
#include "image/TreeSVG.h"

#include "ImageOp.h"
#include "CopyOp.h"


namespace drain
{

namespace image
{

class PaletteEntry : public std::vector<double> {
    public: //re 
	std::string label;
};

class Palette : public std::map<double,PaletteEntry > {
// public std::map<double,std::vector<double> > {

public:

	//typedef std::map<double,std::vector<double> >::iterator iterator;

	void load(const std::string &filename);

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
	unsigned int _colors;
	//void skipLine(std::ifstream &ifstr) const;

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
