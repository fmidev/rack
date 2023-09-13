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

#include <sstream>
#include <list>

#include "drain/util/FilePath.h"
#include "drain/util/StringMapper.h"
//
#include "drain/util/Output.h"

#include "Palette.h"

namespace drain
{

namespace image
{

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table. 
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  - 
 */


PaletteEntry::PaletteEntry(): BeanLike(__FUNCTION__), //value(0.0),
		alpha(255.0), hidden(false){
	init();
}

PaletteEntry::PaletteEntry(const PaletteEntry & entry): BeanLike(__FUNCTION__), // value(0.0),
		alpha(255.0), hidden(false){
	init();
	parameters.importMap(entry.getParameters());
	color = entry.color;
}

/*
PaletteEntry::PaletteEntry(const std::string & label, double value, color_t color, value_t alpha, bool hidden) :
		BeanLike(__FUNCTION__), label(label), value(value), color(color), alpha(alpha), hidden(hidden) {
	init();
}
*/

PaletteEntry::PaletteEntry(const std::string & id, const std::string & label, color_t color, value_t alpha, bool hidden) :
		BeanLike(__FUNCTION__), id(id), label(label), color(color), alpha(alpha), hidden(hidden) {
	init();
}

PaletteEntry::PaletteEntry(const std::string & label, color_t color, value_t alpha, bool hidden) :
		BeanLike(__FUNCTION__), id(label), label(label), color(color), alpha(alpha), hidden(hidden) {
	init();
}


void PaletteEntry::init(){
	// color.resize(1, 0); NOVECT
	// parameters.link("value", value); //  = 0.0
	// parameters.link("color", color);
	parameters.link("id", id);
	parameters.link("label", label);
	parameters.link("color", color);
	parameters.link("alpha", alpha); //  = 255.0
	parameters.link("hidden", hidden); // =false
}


// Alpha check
void PaletteEntry::checkAlpha(){

	const size_t s = color.size();

	switch (s){
		case 4:
		case 2:
			alpha = color[s-1];
			// NOVECT color.resize(s-1);
			break;
		default:
			break;
	}

}

void PaletteEntry::getHexColor(std::ostream & ostr) const {

	for (color_t::const_iterator it = color.begin(); it!=color.end(); ++it){
		ostr.width(2);
		ostr.fill('0');
		ostr << std::hex << static_cast<int>(*it);
	}

}

// std::ostream & PaletteEntry::toOStream(std::ostream &ostr, char separator, char separator2) const{
std::ostream & PaletteEntry::toStream(std::ostream &ostr, char separator) const{


		//if (!separator2)
		//	separator2 = separator;


		/*
		ostr << "# value from Palette index!" << separator;
		if (std::isnan(value)){
			ostr << '@' << separator;
		}
		else {
			ostr << value << separator;
		}
		*/

		char sep = 0;
		for (color_t::const_iterator it=color.begin(); it != color.end(); ++it){
			if (sep){
				ostr << sep;
			}
			else {
				sep = separator;
			}
			ostr << *it; //  << separator2;
		}

		// alpha (optional)
		if (alpha < 255.0){ // NOTE: can be 255 in alpha image also...
			if (sep){
				ostr << separator;
			}
			ostr << alpha;
		}
		//if (color.size()==4)
		//	if (color[3] != 255.0)
		//		ostr << color[3];

		return ostr;
}



}

}

// Drain
