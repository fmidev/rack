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

#include "ImageFile.h"
#include "Geometry.h"
#include "TreeSVG.h"

#include "drain/util/BeanLike.h"
#include "drain/util/Dictionary.h" // temporary ?
#include "drain/util/JSON.h"
#include "drain/util/UniTuple.h"

#include "Legend.h"
#include "PaletteEntry.h"


namespace drain
{


namespace image
{


/** A map of RGB colours, with optional transparency (alpha) values, and with Dictionary support.
 *
 */
class Palette : public ImageCodeMap<PaletteEntry> {

public:

	/// Basic constructor.
	inline
	Palette() : ImageCodeMap<PaletteEntry>(), refinement(0) {
	};

	// NEW
	/// Constructor supporting initialisation.
	inline
	Palette(std::initializer_list<std::pair<const double, PaletteEntry> > l): ImageCodeMap<PaletteEntry>(l), refinement(0){
	};


	void reset();

	/// Add one color
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

	//void exportJSON(drain::JSONtree::tree_t & json) const;
	void exportJSON(drain::JSONtree2 & json) const;

	// Export formatted
	void exportFMT(std::ostream & ostr, const std::string & format) const;

	/// Returns a legend as an SVG graphic.
	void exportSVGLegend(TreeSVG & svg, bool up = true) const;

	void exportCPP(std::ostream & ostr) const;

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

	/// Extend palette to contain n entries ("colors") by adding intermediate entries.
	void refine(size_t n=0);

	typedef drain::Dictionary<key_t, std::string> dict_t;

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

	/// Creates a palette from json object
	void importJSON(const drain::JSONtree2 & json);

	mutable
	ChannelGeometry channels;


};


inline
std::ostream & operator<<(std::ostream &ostr, const Palette & p){
	p.exportTXT(ostr);
	return ostr;
}

} // image::

} // drain::

#endif
