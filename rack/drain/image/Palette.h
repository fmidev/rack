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
#ifndef DRAIN_PALETTE
#define DRAIN_PALETTE

#include "Geometry.h"
#include "File.h"
#include "TreeSVG.h"

#include "util/JSONtree.h"


namespace drain
{

namespace image
{

class PaletteEntry  {

public:

	/// Intensity type
	typedef double value_t;

	/// Color vector type
	typedef std::vector<value_t> vect_t;


	/// Default constructor
	PaletteEntry();

	/// Copy constructor
	PaletteEntry(const PaletteEntry & entry);

	void checkAlpha();

	/// Index or threshold value. Must be signed, as images may generally have negative values.
	double value;


	/// Colors, or more generally, channel values
	//  as three or four element vector: red, green, blue and optional alpha.
	vect_t color;

	value_t alpha;


	/// Unique label (latent)
	//std::string id;

	/// Description appearing in legends
	std::string label;


	/// Suggests hiding the entry in legends. Does not affect colouring of images.
	/**
	 *   When true, indicates that this entry is less significant and can be excluded in legends
	 *   created by
	 *
	 */
	bool hidden;

	/**
	 *  \param ostr - output stream
	 *  \param separator  - char after palette index
	 *  \param separator2 - char after each color intensity value
	 *
	 */
	std::ostream & toOStream(std::ostream &ostr, char separator='\t', char separator2=0) const;

	drain::ReferenceMap map;

protected:


	void init();


};


inline
std::ostream & operator<<(std::ostream &ostr, const PaletteEntry & e){
	return e.toOStream(ostr);
}


class Palette : public std::map<double,PaletteEntry > {

public:


	void reset();

	/// Loads a palette from text file
	/**
	 *   \param flexible - if true, try also directory appended with ./palette, basename palette-<string> and extensions txt,json
	 */
	void load(const std::string & filename, bool flexible = false);

	/// Loads a palette from text file
	void loadTXT(std::ifstream & ifstr);


	/// Loads a palette from text file
	void loadJSON(std::ifstream & ifstr);

	void write(const std::string & filename);

	void exportTXT(std::ostream & ostr, char separator='\t', char separator2=0) const;

	void exportJSON(drain::JSON::tree_t & json) const;

	/// Returns a legend as an SVG graphic.
	void exportSVGLegend(TreeSVG & svg, bool up = true) const;

	/// Creates a palette from json object
	/**
	 *  \param json - JSON structure combining optional \c metadata and compulsory \c entries section.
	 */
	// void convertJSON(const drain::JSON::tree_t & json);

	const ChannelGeometry & getChannels() const {
		update();
		return channels;
	}


	/// Name of the palette. In reading files, the first comment line (without the prefix '#') is copied to this. Legend shows the title on top.
	std::string title;

	/// Some kind of identity key for the palette, typically filename to avoid reloading
	// std::string id; ?

	/// Certain intensities (before scaling with gain and offset) may require special treatment.
	typedef std::map<std::string,PaletteEntry> spec_t;
	spec_t specialCodes; // To be read from palette

	/// Extend palette to contain n entries ("colors") by adding intermediate entries ("colors")
	void refine(size_t n=256);

protected:

	void update() const;

	mutable
	ChannelGeometry channels;

	//void skipLine(std::ifstream &ifstr) const;

	/// Creates a palette from json object
	void importJSON(const drain::JSON::tree_t & json, int depth);

};


inline
std::ostream & operator<<(std::ostream &ostr, const Palette & p){
	p.exportTXT(ostr);
	return ostr;
}

} // image::

} // drain::

#endif
