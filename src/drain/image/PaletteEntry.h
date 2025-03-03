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
#ifndef DRAIN_PALETTE_ENTRY
#define DRAIN_PALETTE_ENTRY

#include <drain/UniTuple.h>
#include <iomanip>

#include "ImageFile.h"
#include "Geometry.h"
#include "TreeSVG.h"

//#include "drain/util/BeanLike.h"
#include "drain/util/VariableMap.h"

#include "drain/util/Dictionary.h" // temporary ?
#include "drain/util/JSON.h"
#include "Legend.h"


namespace drain
{

/// BeanLike-like, based on FlexMap
class FavaBean {
public:

	inline
	FavaBean(){};

	inline
	FavaBean(const FavaBean & b){};

	typedef FlexVariableMap map_t;

	inline
	const map_t & getParameters() const { return parameters; };

	inline
	map_t & getParameters() { return parameters; };


	const std::string name;  // todo separate (Beanlet)

	const std::string description; // todo separate (Beanlet)

	//ReferenceMap parameters;  // todo separate (Beanlet)
	map_t parameters;  // todo separate (Beanlet)?

};

namespace image
{




class PalEntry { //  : public ABC { //: public drain::BeanLike {


public:


	/// Threshold value (lowest value to be associated with entry (color and alpha)
	typedef double value_t;

	/// Color vector type
	//typedef drain::UniTuple<value_t,3> color_t;
	typedef std::vector<value_t> color_vect_t;
	typedef drain::FlexVariableMap map_t;

	PalEntry(const color_vect_t & color={}, value_t alpha=1.0, const std::string & label = "");

	PalEntry(const std::initializer_list<drain::Variable::init_pair_t > & args);

//protected:

	//ReferenceMap parameters;  // todo separate (Beanlet)
	map_t parameters;  // todo separate (Beanlet)?

	//color_t color;
	drain::FlexibleVariable & color; // "Inverse" linkage to parameters.
	value_t alpha = 1.0;

	std::string label;

	inline
	PalEntry & operator=(const PalEntry & b);


	inline
	const map_t & getParameters() const { return parameters; };

	inline
	map_t & getParameters() { return parameters; };


	const std::string name;  // todo separate (Beanlet)

	const std::string description; // todo separate (Beanlet)

protected:

	//ReferenceMap parameters;  // todo separate (Beanlet)

	void init();

};




/// Container for color, transparency (alpha) and textual id and description. Does not contain intensity thresholds (intervals).
/**
 *  The color is stored as 8 bit value [0.0 ,255.0].
 *  Alpha is stored (currenly) as a float: [0.0, 1.0].
 *
 *
 */
class PaletteEntry { // : public BeanLike { // public FavaBean { //

public:

	/// Intensity type
	typedef double value_t;

	// UUSI
	typedef std::vector<value_t> color_vect_t;
	typedef drain::FlexVariableMap map_t;

	/// Color vector type
	typedef FlexibleVariable color_t; // NEW
	// typedef UniTuple<value_t,3> color_t;  // UNITUPLE
	// typedef std::vector<value_t> color_t; // VECT

	/// Default constructor
	PaletteEntry();

	/// Copy constructor
	PaletteEntry(const PaletteEntry & entry);

	///
	/**
	 *  This constructor supports Palette initialization of form:
	 *  \code
	 * 	  palette = {
             { 10, {{"label","Timing problem"},{"color",{240,240,240}},{"code","tech.err.time"},{"alpha",255}} },
             { 13, {{"label","Unclassified"},{"color",{144,144,144}},{"code","tech.class.unclass"},{"alpha",255}} },
             { 64, {{"label","Precipitation"},{"color",{80,208,80}},{"code","precip"},{"alpha",255}} },
             { 72, {{"label","Rain"},{"color",{32,248,96}},{"code","precip.widespread"},{"alpha",255}} },
             // ...
	 *  \endcode
	 *
	 */
	PaletteEntry(const char * code, const char * label, const color_vect_t & color, value_t alpha=255.0, bool hidden=false);

	///
	/**
	   This constructor supports Palette initialization with following entry syntax:
	   \code
	  	  palette = {
  	  	  	  {"nodata", {"nodata", "Data unavailable", {255.0,255.0,255.0}} },
  	  	  	  {"undetect", {"undetect", "Measurement failed", {0.0,0.0,0.0}} },
  	  	  	  {     -32.0, {"noise", "Noise", {0.0,0.0,0.0}} },
  	  	  	  {      24.0, {"precip.rain.moderate", "Moderate rain", {255.0,150.0,50.0}} },
	         // ...
	   \endcode
	 */
	PaletteEntry(const char * label, const color_vect_t & color, value_t alpha=255.0, bool hidden=false);

	///
	/**
	 *  This constructor supports Palette initialization of form:
 	 	\code
	  	  palette = {
  	  	  	  {"nodata", {"Data unavailable", {255.0,255.0,255.0}} },
  	  	  	  {"undetect", {"Measurement failed", {0.0,0.0,0.0}} },
  	  	  	  {     -32.0, {"Noise", {0.0,0.0,0.0}} },
  	  	  	  {      24.0, {"precip.moderate", "Moderate", {255.0,150.0,50.0}} },
	         // ...
	    \endcode
	 *
	 */
	PaletteEntry(const color_vect_t & color, value_t alpha=255.0, bool hidden=false);

	/// Constructor supporting Palette initialization using "{key,value}" form.
	/**
	 *
	 *  \code
	 * 	  palette = {
             { 10, {{"label","Timing problem"},{"color",{240,240,240}},{"code","tech.err.time"},{"alpha",255}} },
             { 13, {{"label","Unclassified"},{"color",{144,144,144}},{"code","tech.class.unclass"},{"alpha",255}} },
             { 64, {{"label","Precipitation"},{"color",{80,208,80}},{"code","precip"},{"alpha",255}} },
             { 72, {{"label","Rain"},{"color",{32,248,96}},{"code","precip.widespread"},{"alpha",255}} },
             // ...
	 *  \endcode
	 *
	 */
	PaletteEntry(const std::initializer_list<Variable::init_pair_t > & args);

	/// Special dummy constructor for reading a title or other metadata related to Palette.
	/**
	 *  This constructor supports Palette initialization of form:
	 *  \code
	 * 	  palette = {
             { "title", {"Radar reflectivity"} }
             // ...
	 *  \endcode
	 *
	 */
	//PaletteEntry(const std::string & label);
	PaletteEntry(const char * label);

	// UUSI
	map_t parameters;

	/// Technical identifier (optional).
	std::string code;

	/// Short description for legends
	std::string label;

	/// Index or threshold value. Must be signed, as image data may generally have negative values.
	// double value;
	// Experimental
	FlexibleVariable & color;

	/// Colors, or more generally, channel values
	// color_t color;

	/// Transparency
	value_t alpha;


	/// Suggests hiding the entry in legends. Does not affect colouring of images.
	/**
	 *   When true, indicates that this entry is less significant and can be skipped
	 *   when rendering legends, for example.
	 *
	 */
	bool hidden;

	// inline
	PaletteEntry & operator=(const PaletteEntry & b);


	inline
	const map_t & getParameters() const { return parameters; };

	inline
	map_t & getParameters() { return parameters; };

	/**
	 *  \param ostr - output stream
	 *  \param separator  - char after palette index
	 *  \param separator2 - char after each color intensity value
	 *
	 */
	//	std::ostream & toOStream(std::ostream &ostr, char separator='\t', char separator2=0) const;
	std::ostream & toStream(std::ostream &ostr, char separator='\t') const;

	void checkAlpha();


	/// Returns the color without leading marker (like "0x").
	void getHexColor(std::ostream & ostr) const;

	/// Write colour in hexadecimal notation, with an optional prefix (often "#").
	inline
	void getHexColor(std::string & str, const std::string & prefix = "") const {
		std::stringstream sstr;
		sstr << prefix;
		getHexColor(sstr);
		str = sstr.str();
	}

	//template <class T>
	/*
	PaletteEntry & operator=(const drain::VariableMap & m){
		setParameters(m);
		return *this;
	};
	*/

protected:

	void init();

};



inline
std::ostream & operator<<(std::ostream &ostr, const PaletteEntry & entry){
	return entry.toStream(ostr);
}


} // image::

template <>
std::ostream & drain::Sprinter::toStream(std::ostream & ostr, const drain::image::PaletteEntry & entry, const drain::SprinterLayout & layout);


} // drain::

/*
template <>
inline
std::ostream & drain::Sprinter::toStream(std::ostream & ostr, const drain::image::PaletteEntry::color_t & color, const drain::SprinterLayout & layout){

	ostr << layout.arrayChars.prefix; // << "xx";
	ostr << std::fixed << std::setprecision(1); // ensure ".0"
	drain::Sprinter::toStream(ostr, color[0], layout);
	ostr << layout.arrayChars.separator;
	drain::Sprinter::toStream(ostr, color[1], layout);
	ostr << layout.arrayChars.separator;
	drain::Sprinter::toStream(ostr, color[2], layout);
	ostr << layout.arrayChars.suffix;
	return ostr;
}
*/


#endif
