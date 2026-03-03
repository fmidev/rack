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

   Based on the example code by Niles D. Ritter,
   http://svn.osgeo.org/metacrs/geotiff/trunk/libgeotiff/bin/makegeo.c

 */

#include <drain/StringBuilder.h>
// #include <AccumulatorGeo.h>


#include "TreeXML-GDAL.h"


namespace drain
{


DRAIN_TYPENAME_DEF(image::NodeGDAL);
DRAIN_TYPENAME_DEF(image::NodeGDAL::tag_t);

DRAIN_ENUM_DICT(image::NodeGDAL::tag_t) = {
		{"UNDEFINED",    drain::image::GDAL::UNDEFINED},
		{"GDALMetadata", drain::image::GDAL::ROOT},
		{"Item", drain::image::GDAL::ITEM},
		{"Item", drain::image::GDAL::USER}, // User tag has the same label, but additional attributes.
	//	{"Item", drain::image::GDAL::OFFSET},
	//	{"Item", drain::image::GDAL::SCALE,	    },
	//	{"Item", drain::image::GDAL::UNITS},
};

// TreeGDAL::node_data_t::tag_t;
// template <>
// const NodeXML<image::TreeGDAL::node_data_t::tag_t>::xml_default_elem_map_t NodeXML<image::TreeGDAL::node_data_t::tag_t>::xml_default_elems = {

DRAIN_XML_DEFAULT_ELEMS(image::TreeGDAL) = {
		{image::NodeGDAL::tag_t::ROOT, image::NodeGDAL::tag_t::ITEM},
};

namespace image {


NodeGDAL::NodeGDAL(const tag_t & t) : xml_node_t() {
	setType(t);
}


NodeGDAL::NodeGDAL(const NodeGDAL & node) : xml_node_t() { // , sample(-1)
	XML::xmlAssignNode(*this, node); // good here
}


void NodeGDAL::handleType(){ // const tag_t & t

	clear();
	getAttributes()["name"].link(name);

	switch (getNativeType()){
	case ROOT:
		break;
	case ITEM:
		getAttributes()["role"].link(role = "");
		getAttributes()["name"].link(name); // don't change!  = ""
		break;
	default:
		return;
	}

	/*
	if (t == ROOT){
	}
	else if (t == ITEM){
		//	link("sample", sample = "0"); //  = -1);
		link("role",   role = "");
		link("name",   name); // don't change!  = ""
	}
	else { // USER
		// clear(); // clearLinks could be better?
	}
	*/

	// Logger mout(__FILE__, __FUNCTION__);
	// mout.warn("setType: ", getTag(), " name:", name);

}


} // image::

} // drain::
