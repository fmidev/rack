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

PalEntry::PalEntry(const color_vect_t & color, value_t alpha, const std::string & label) : color(parameters["color"] = color), alpha(alpha), label(label){
	init();
}


PalEntry::PalEntry(const std::initializer_list<drain::Variable::init_pair_t > & args) :
		color(parameters["color"])
{
	init();
	drain::SmartMapTools::setValues(parameters, args);
	// parameters(args);
};

void PalEntry::init(){
	// parameters.link("color", c);
	// color.setType(typeid(double)); // is needed? Ok, file read needs "assumption"
	parameters.link("alpha", alpha); // OR embedded in color?
	parameters.link("label", label); // OR embedded in color?
}


PalEntry & PalEntry::operator=(const PalEntry & b){
	color = b.color;
	alpha = b.alpha;
	label = b.label;
	return *this;
}


/*
void PaletteEntry::init(){
	// parameters.link("color", c);
	// color.setType(typeid(double)); // is needed? Ok, file read needs "assumption"
	parameters.link("alpha", alpha); // OR embedded in color?
	parameters.link("label", label); // OR embedded in color?
}
*/


PaletteEntry & PaletteEntry::operator=(const PaletteEntry & b){
	code   = b.code;
	label  = b.label;
	color  = b.color;
	alpha  = b.alpha;
	hidden = b.hidden;
	return *this;
}

/// Colorizes an image of 1 channel to an image of N channels by using a palette image as a lookup table. 
/*! Treats an RGB truecolor image of N pixels as as a palette of N colors.
 *  -
 *  - 
 */





PaletteEntry::PaletteEntry():
						// BeanLike(__FUNCTION__),
				color(parameters["color"]),
				// color(3, 0.0), // VECT
				alpha(255.0),
				hidden(false){
	init();
}

PaletteEntry::PaletteEntry(const PaletteEntry & entry):
// BeanLike(__FUNCTION__),
// color(3, 0.0), // VECT
		code(entry.code),
		label(entry.label),
		color(parameters["color"] = entry.color),
		alpha(entry.alpha),
		hidden(entry.hidden){
	init();
	// parameters.importMap(entry.getParameters());
	// color = b.color;
	// color = entry.color;
}

PaletteEntry::PaletteEntry(const char * code, const char * label, const color_vect_t & color, value_t alpha, bool hidden) :
				// BeanLike(__FUNCTION__),
				code(code),
				label(label),
				color(parameters["color"] = color),
				// color(color),
				alpha(alpha),
				hidden(hidden) {
	init();
	// std::cerr << __FUNCTION__ << '|' << code << '=' << label << *this << "# " << parameters << '\n';
}

PaletteEntry::PaletteEntry(const char * label, const color_vect_t & color, value_t alpha, bool hidden) :
				// BeanLike(__FUNCTION__),
				code(label),
				label(label),
				color(parameters["color"] = color),
				// color(color),
				alpha(alpha),
				hidden(hidden) {
	init();
}

PaletteEntry::PaletteEntry(const color_vect_t & color, value_t alpha, bool hidden) :
		// BeanLike(__FUNCTION__),
		color(parameters["color"] = color),
		// color(color),
		alpha(alpha),
		hidden(hidden) {
	init();
}


PaletteEntry::PaletteEntry(const std::initializer_list<Variable::init_pair_t > & args) :
		// BeanLike(__FUNCTION__),
		// color(3, 0.0),  // VECT
		color(parameters["color"]),
		alpha(255.0),
		hidden(false) {
	init();
	drain::SmartMapTools::setValues(parameters, args);
	/*
	for (auto entry: args){
		std::cout << entry.first << " == " << entry.second << std::endl;
	}*/
	// parameters = args;
	// setParameters(args);
}

PaletteEntry::PaletteEntry(const char * label):
						// BeanLike(__FUNCTION__),
				label(label),
					// color(3, 0.0),  // VECT
				color(parameters["color"]),
				alpha(255.0),
				hidden(true) {
	// Why no init() ? because hidden?
	parameters.link("label", this->label);
}



void PaletteEntry::init(){
	// color.resize(1, 0); VECT
	// parameters.link("color", color); // UNITUPLE
	// parameters.link("color", color); SKIP for FlexibleVariable
	// parameters["color"] << 128.0 << 0.0 << 255.0; // FLEX ?
	parameters.link("code", code);
	parameters.link("label", label);
	parameters.link("alpha", alpha); //  = 255.0
	parameters.link("hidden", hidden); // =false
}


// Alpha check
void PaletteEntry::checkAlpha(){

	//const size_t s = color.size();
	const size_t s = color.getElementCount();

	switch (s){
		case 4:
		case 2:
			// alpha = color[s-1];
			alpha = color.get<double>(s-1);
			// NOVECT color.resize(s-1);
			break;
		default:
			break;
	}

}

void PaletteEntry::getHexColor(std::ostream & ostr) const {

	ostr << std::hex;
	for (color_t::const_iterator it = color.begin(); it!=color.end(); ++it){
		ostr.width(2);
		ostr.fill('0');
		ostr << std::hex << static_cast<int>(*it);
	}
	ostr << std::dec;

}

// std::ostream & PaletteEntry::toOStream(std::ostream &ostr, char separator, char separator2) const{
std::ostream & PaletteEntry::toStream(std::ostream &ostr, char separator) const{


		std::ios_base::fmtflags format(ostr.flags());
		std::streamsize prec = ostr.precision();
		if (Type::call<drain::typeIsFloat>(color.getType())){
			ostr << std::fixed << std::setprecision(1);
			ostr.precision(1);
		}

		char sep = 0;
		for (color_t::const_iterator it=color.begin(); it != color.end(); ++it){
			if (sep){
				ostr << sep;
			}
			else {
				sep = separator;
			}
			ostr << static_cast<double>(*it); // prevent uchars appearing as characters
		}

		// alpha (optional)
		if (alpha < 255.0){ // NOTE: can be 255 in alpha image also...
			if (sep){
				ostr << separator;
			}
			ostr << alpha;
		}

		ostr.flags(format);
		//ostr.unsetf(std::fixed);
		// ostr << std::setprecision(16);
		ostr.precision(prec);
		return ostr;
}

} // image::

} // drain::


template <>
std::ostream & drain::Sprinter::toStream(std::ostream & ostr, const drain::image::PaletteEntry & entry, const drain::SprinterLayout & layout){

	const bool KEYS = ( (layout.keyChars[0]!=0) || (layout.keyChars[1]!=0) || (layout.keyChars[2]!=0) );

	const bool LABEL  = (!entry.label.empty());
	const bool CODE   = ((!entry.code.empty()) && (entry.code != entry.label)); // rethink
	const bool ALPHA  = (entry.alpha != 255.0);
	const bool HIDDEN = (entry.hidden);


	//ostr << std::fixed << std::setprecision(1);  // SPOILED m/dBZ scale


	if (KEYS){

		std::list<std::string> keys = {"label", "color"}; // "label","color"

		if (CODE)
			keys.push_back("code");

		if (ALPHA)
			keys.push_back("alpha");

		if (HIDDEN)
			keys.push_back("hidden");

		drain::Sprinter::mapToStream(ostr, entry.getParameters().getMap(), layout, keys);
	}
	else {
		// Plain (ordered) layout
		ostr << layout.arrayChars.prefix;

		if (CODE){
			drain::Sprinter::toStream(ostr, entry.code, layout);
			ostr << layout.arrayChars.separator << ' '; // <-- yes, additional space
		}

		if (CODE || LABEL){
			drain::Sprinter::toStream(ostr, entry.label, layout);
			ostr << layout.arrayChars.separator << ' '; // <-- yes, additional space
		}

		{
			ostr << layout.arrayChars.prefix;
			char sep = 0;
			for (drain::image::PaletteEntry::value_t v: entry.color){
				if (sep)
					ostr << sep; // << ' '; // <-- yes, additional space
				else
					sep = layout.arrayChars.separator;
				drain::Sprinter::toStream(ostr, v, layout);
			}
			ostr << layout.arrayChars.suffix;
		}

		if (ALPHA || HIDDEN){
			ostr << layout.arrayChars.separator << ' '; // <-- yes, additional space
			drain::Sprinter::toStream(ostr, entry.alpha, layout);
			if (HIDDEN){
				ostr << layout.arrayChars.separator << ' ';
				drain::Sprinter::toStream(ostr, entry.hidden, layout);
			}
		}
		// ostr << ' '; // <-- yes, additional space
		ostr << layout.arrayChars.suffix;

	}


	//return drain::JSON::treeToStream(ostr, tree, layout);
	//return drain::Sprinter::treeToStream(ostr, tree, layout);
	return ostr;
}

// Drain
