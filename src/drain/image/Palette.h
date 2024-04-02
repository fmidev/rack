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

#include <drain/UniTuple.h>
#include "ImageFile.h"
#include "Geometry.h"
#include "TreeSVG.h"

#include "drain/util/BeanLike.h"
// #include "drain/util/Dictionary.h" // temporary ?
#include "drain/util/JSON.h"
#include "drain/util/Dictionary.h"
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

	static
	const SprinterLayout cppLayout2;

	/// Basic constructor.
	inline
	Palette() : ImageCodeMap<PaletteEntry>(), refinement(0) {
	};

	/// Constructor supporting initialisation.
	/**
	 *   Having Variable as key type allows mixing numeric codes and alphabetic (special) codes.
	 *
	 */
	Palette(std::initializer_list<std::pair<Variable, PaletteEntry> > inits);

	/** Given code (class index or similar marker), returns the (threshold) value, ie. index of the palette entry.
	 *
	 *  Argument \code is matched against palette entries with tests in the following order:
	 *
	 *  # exact string match (if \c lenient=true , testing stops here )
	 *  # \c palette.code starts   with \c code
	 *  # \c palette.code end      with \c code
	 *  # \c palette.code contains with \c code
	 *
	 *  If a matching code is not found, std::runtime_exeption is thrown.
	 *
	 * \param code
	 * \param lenient - if false, only exact string match tested, else substring match accepted
	 */
	value_type & getEntryByCode(const std::string & code, bool lenient=true);

	/// Shorthand invoking fgetEntryByCode for retrieving the code
	inline
	key_type getValueByCode(const std::string & code, bool lenient=true){
		value_type & entry = getEntryByCode(code, lenient);
		return entry.first;
	}

	void reset();

	/// Add one color. Deprecating; brace-enclosed initialization more preferable.
	void addEntry(double min, double red=0.0, double green=0.0, double blue=0.0, const std::string &id="", const std::string & label =""); // alpha?

	/// Loads a palette from text file
	/**
	 *   \param flexible - if true, try also directory appended with ./palette, basename palette-<string> and extensions txt,json
	 */
	void load(const std::string & filename, bool flexible = false);

	/// Loads a palette from a text file
	void loadTXT(std::istream & ifstr);

	/// Loads a palette from a json file. Under revision; support may be dropped later.
	void loadJSON(std::istream & ifstr);

	void write(const std::string & filename) const;

	void exportTXT(std::ostream & ostr, char separator='\t', char separator2=0) const;

	//void exportJSON(drain::JSONtree::tree_t & json) const;
	void exportJSON(drain::JSONtree2 & json) const;

	// Export formatted
	void exportFMT(std::ostream & ostr, const std::string & format) const;

	/// Returns a legend as an SVG graphic.
	void exportSVGLegend(TreeSVG & svg, bool up = true) const;

	/// Export palette such
	/**
	 * void exportCPP(std::ostream & ostr, bool flat=true) const;
	 *
	 */

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

	// typedef drain::Dictionary<key_t, std::string> dict_t;

	/// void updateDictionary();

	/// Request for future refinement, ie. refine() call upon next load().
	/** It is upto applications to use this.
	 *
	 */
	size_t refinement;

	// Metadata - for example, the file path of a loaded palette.
	std::string comment;

protected:

	/// Container for special codes ??
	// dict_t dictionary; // deprecating!


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




class PaletteMap : public std::map<std::string,drain::image::Palette> {

public:

	inline
	Palette & get(const std::string & key){
		if (aliases.hasKey(key)){
			return (*this)[aliases.getValue(key)];
		}
		else {
			return (*this)[key];
		}
	}

	inline
	void addAlias(const std::string & key, const std::string & aliasKey){
		aliases.add(aliasKey,key);
	}

// protected:

	drain::Dictionary<std::string, std::string> aliases;
	// std::map<std::string,std::string> aliasMap;

};

} // image::

template <>
std::ostream & drain::Sprinter::toStream(std::ostream & ostr, const drain::image::Palette & map, const drain::SprinterLayout & layout);

} // drain::


#endif
