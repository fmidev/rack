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

#include "ImageFile.h"
#include "Geometry.h"
#include "TreeSVG.h"

#include "drain/util/BeanLike.h"
#include "drain/util/Dictionary.h" // temporary ?
#include "drain/util/JSON.h"
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


// Why beanlike? Overkill..

class PaletteEntry : public BeanLike {

public:

	/// Intensity type

	typedef double value_t;

	/// Color vector type
	typedef UniTuple<value_t,3> color_t;


	/// Default constructor
	PaletteEntry();

	/// Copy constructor
	PaletteEntry(const PaletteEntry & entry);

	//PaletteEntry(const std::string & label, double value, color_t color, value_t alpha, bool hidden=false);
	PaletteEntry(const std::string & id, const std::string & label, color_t color, value_t alpha, bool hidden=false);

	PaletteEntry(const std::string & label, color_t color, value_t alpha, bool hidden=false);

	void checkAlpha();

	/// Unique label (latent/invisible? for undetected/nodata)
	std::string id; // was already obsolete?

	/// Short description appearing in legends
	std::string label;

	/// Index or threshold value. Must be signed, as image data may generally have negative values.
	// double value;

	/// Colors, or more generally, channel values
	color_t color;

	value_t alpha;


	/// Suggests hiding the entry in legends. Does not affect colouring of images.
	/**
	 *   When true, indicates that this entry is less significant and can be skipped
	 *   when rendering legends, for example.
	 *
	 */
	bool hidden;

	/**
	 *  \param ostr - output stream
	 *  \param separator  - char after palette index
	 *  \param separator2 - char after each color intensity value
	 *
	 */
	//	std::ostream & toOStream(std::ostream &ostr, char separator='\t', char separator2=0) const;
	std::ostream & toStream(std::ostream &ostr, char separator='\t') const;

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
	return e.toStream(ostr);
}


} // image::

} // drain::

#endif
