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

//
#include <drain/image/MouseXML.h>
#include "data/SourceODIM.h"
// #include "resources-svg.h"
#include "graphics-imagepanel.h"
#include "graphics-radar.h"


using namespace drain;
using namespace drain::image;

namespace rack {



/* unused
drain::image::TreeSVG & ImagePanel::getSourceSpecificGroup(const std::string & source) const {

	if (!source.empty()){
		drain::image::TreeSVG & subGroup = imagePanelGroup[source](svg::GROUP);
		subGroup->setAttribute("data-source", source);
		return subGroup;
	}
	else {
		drain::Logger(__FILE__, __FUNCTION__).warn("Argument ('source') empty");
		std::string s;
		drain::image::TreeSVG::generateKey(imagePanelGroup, s);
		// Should not be empty...
		return getSourceSpecificGroup(s);
	}

	// return panel[statusMap.get("what:source", "unknown-source")](svg::GROUP);
}
*/


} // rack::

