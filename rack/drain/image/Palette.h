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

#include <drain/image/ImageFile.h>
#include "Geometry.h"
#include "TreeSVG.h"

#include "drain/util/BeanLike.h"
#include "drain/util/Dictionary.h" // temporary ?
#include "drain/util/JSONtree.h"
#include "drain/util/UniTuple.h"

#include "Legend.h"


namespace drain
{


/*   = old
class PaletteEntry2 : public LegendEntry, public BeanLike {
public:

	PaletteEntry2(): BeanLike(__FUNCTION__), colour(3, 0){
		parameters.link("label",  this->label);
		parameters.link("colour", this->colour);
	}

	PaletteEntry2(const PaletteEntry2 & pale): BeanLike(__FUNCTION__), colour(3, 0){
		parameters.link("label",  this->label = pale.label);
		parameters.link("colour", this->colour = pale.colour);
	}

	PaletteEntry2 & operator=(const PaletteEntry2 & pale){
		this->label  = pale.label;
		this->colour = pale.colour;
		return *this;
	}

	std::vector<double> colour;

	inline
	bool empty() const {
		return this->label.empty();
	}

};
*/

/*
template <>
inline
std::ostream & JSONwriter::toStream(const PaletteEntry2 &e, std::ostream &ostr, unsigned short indentation){
	return JSONwriter::mapToStream(e.getParameters(), ostr, indentation);
}
*/

namespace image
{

class PaletteEntry : public LegendEntry, public BeanLike {

public:

	/// Intensity type
	typedef double value_t;

	/// Color vector type
	//typedef std::vector<value_t> color_t;
	typedef UniTuple<value_t,3> color_t;


	/// Default constructor
	PaletteEntry();

	/// Copy constructor
	PaletteEntry(const PaletteEntry & entry);

	void checkAlpha();

	/// Index or threshold value. Must be signed, as images may generally have negative values.
	double value;


	/// Colors, or more generally, channel values
	//  as three or four element vector: red, green, blue and optional alpha.
	color_t color;

	value_t alpha;


	/// Unique label (latent)
	std::string id; // was already obsolete?

	/// Description appearing in legends
	//  std::string label; // Legend


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

	//drain::ReferenceMap map;

	/// Returns the color without leading marker (like "0x").
	void getHexColor(std::ostream & ostr) const;

	inline
	void getHexColor(std::string & str) const {
		std::stringstream sstr;
		getHexColor(sstr);
		str = sstr.str();
	}

protected:


	void init();


};


inline
std::ostream & operator<<(std::ostream &ostr, const PaletteEntry & e){
	return e.toOStream(ostr);
}


// class Palette : public std::map<double,PaletteEntry > {
class Palette : public ImageCodeMap<PaletteEntry> {

public:

	inline
	Palette() : ImageCodeMap<PaletteEntry>(), refinement(0) {
	};

	void reset();

	/// Add one color
	//void addEntry(double min, const std::string &id, double red=0.0, double green=0.0, double blue=0.0); // alpha
	void addEntry(double min, double red=0.0, double green=0.0, double blue=0.0, const std::string &id="", const std::string & label =""); // alpha?

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

	void exportJSON(drain::JSONtree::tree_t & json) const;

	// Export formatted
	void exportFMT(std::ostream & ostr, const std::string & format) const;

	/// Returns a legend as an SVG graphic.
	void exportSVGLegend(TreeSVG & svg, bool up = true) const;

	/// Creates a palette from json object
	/**
	 *  \param json - JSON structure combining optional \c metadata and compulsory \c entries section.
	 */
	// void convertJSON(const drain::JSONtree::tree_t & json);

	const ChannelGeometry & getChannels() const {
		update();
		return channels;
	}

	/// Name of the palette. In reading files, the first comment line (without the prefix '#') is copied to this. Legend shows the title on top.
	std::string title;

	/// Some kind of identity key for the palette, typically filename to avoid reloading
	//  std::string id; ?

	/// Certain intensities (before scaling with gain and offset) may require special treatment.
	typedef std::map<std::string,PaletteEntry> spec_t;
	spec_t specialCodes; // To be read from palette

	/// Extend palette to contain n entries ("colors") by adding intermediate entries ("colors")
	// void refine(size_t n=256);
	void refine(size_t n=0);

	typedef drain::Dictionary2<key_t, std::string> dict_t;

	/// Container for special codes
	dict_t dictionary; // temp?

	///
	void updateDictionary();

	/// Request for future refinement, ie. refine() call upon next load().
	/** It is upto applications to use this.
	 *
	 */
	size_t refinement;

protected:

	void update() const;

	mutable
	ChannelGeometry channels;

	// void skipLine(std::ifstream &ifstr) const;

	/// Creates a palette from json object
	void importJSON(const drain::JSONtree::tree_t & json, int depth);

};


inline
std::ostream & operator<<(std::ostream &ostr, const Palette & p){
	p.exportTXT(ostr);
	return ostr;
}

} // image::

} // drain::

#endif
