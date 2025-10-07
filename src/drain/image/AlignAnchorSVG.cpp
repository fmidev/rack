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
 
/*
 * TreeSVG.cpp
 *
 *  Created on: Jun 24, 2012
 *      Author: mpeura
 */

#include "AlignAnchorSVG.h"


namespace drain {

DRAIN_ENUM_DICT(drain::image::AnchorElem::Anchor) = {
		{"", drain::image::AnchorElem::DEFAULT},
		{"@PREVIOUS", drain::image::AnchorElem::PREVIOUS},
		{"@COMPOUND", drain::image::AnchorElem::EXTENSIVE},
		{"@NONE", drain::image::AnchorElem::NONE},
};

/*
DRAIN_ENUM_DICT(drain::image::AnchorElem::Anchor) = {
		{"", drain::image::AnchorElem::DEFAULT},
		{"@PREVIOUS", drain::image::AnchorElem::PREVIOUS},
		{"@COMPOUND", drain::image::AnchorElem::EXTENSIVE},
		{"@NONE", drain::image::AnchorElem::NONE},
};
*/

namespace image {

void AnchorElem::set(const std::string & s){
	assign(s);
}

// inline?
void AnchorElem::set(const AnchorElem::Anchor & anchor){
	set(drain::EnumDict<AnchorElem::Anchor>::getKey(anchor));
}

/*
void AlignAnchorSVG::updateAlignStr(){

	std::stringstream sstr;
	confToStream(sstr);
	alignStr = sstr.str();

}
*/

}  // image::

}  // drain::


