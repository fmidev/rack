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
#ifndef DRAIN_FILE_GDAL_XML
#define DRAIN_FILE_GDAL_XML

#include "drain/util/Flags.h"
#include "drain/util/TreeXML.h"
#include <drain/util/EnumFlags.h>


namespace drain
{

namespace image
{

struct GDAL {

	// Small
	enum tag_t {
		// Small indices reserved for basic XML
		UNDEFINED=drain::NodeXML<>::UNDEFINED,
		CTEXT=drain::NodeXML<>::CTEXT,
		// GDAL
		ROOT=10,
		ITEM=11,
		USER=12}; // , OFFSET, SCALE, UNITS}; // check CTEXT, maybe implement in XML

};

// https://www.awaresystems.be/imaging/tiff/ti fftags/gdal_metadata.html
class NodeGDAL: public GDAL, public drain::NodeXML<GDAL::tag_t> {
public:

	/// Constructor
	NodeGDAL(const tag_t & t = GDAL::ITEM);

	/// Copy constructor
	NodeGDAL(const NodeGDAL & node);

	// void setGDAL(const std::string & name, const drain::Variable & ctext, int sample, const std::string & role = "");
	// void setGDAL(const drain::Variable & ctext, int sample=0, const std::string & role = "");
	// void setGDAL(const std::string & name, const drain::Variable & ctext);

	template <class T>
	inline
	NodeGDAL & operator=(const T & x){
		setText(x);
		return *this;
	}

	virtual
	void handleType(const tag_t & t) override final;


	// Multi-purpose key
	std::string name;

	/// Standard GDAL attribute
	std::string sample;  // string, to allow empty (unset) value

	/// Standard GDAL attribute
	std::string role;

};

//typedef NodeGDAL::xml_tree_t TreeGDAL;
typedef drain::UnorderedMultiTree<image::NodeGDAL> TreeGDAL;

} // image::

/// Write tree using XML notation.
/**
 *   This function does not write the XML header containing document type info.
 *   \see NodeXML::toStream()
 *   \see NodeXML::docToStream()
 */
inline
std::ostream & operator<<(std::ostream & ostr, const image::TreeGDAL & tree){
	return image::TreeGDAL::node_data_t::toStream(ostr, tree);
}

template <>
const EnumDict<image::NodeGDAL::tag_t>::dict_t EnumDict<image::NodeGDAL::tag_t>::dict;

DRAIN_TYPENAME(image::NodeGDAL);

DRAIN_TYPENAME(image::NodeGDAL::tag_t);

/*
template <>
inline
const char* TypeName<image::NodeGDAL>::get(){
	return "XML-GDAL";
}

template <>
inline
const char* TypeName<image::GDAL::tag_t>::get(){
	return "XML-GDAL2";
}
*/

/// Note: this overrides path based addressing of descendants
template <>
template <>
inline
image::TreeGDAL & image::TreeGDAL::operator()(const image::GDAL::tag_t & type){
	this->data.setType(type);
	return *this;
}

/*
SUPRESSED. Semantics become unclear, esp. if calling: const version of x& = tree("name") which returns a tree.


/// Note: this overrides path based addressing of descendants
template <>
template <>
inline
image::TreeGDAL & image::TreeGDAL::operator()(const std::string & name){
	this->data.name = name;
	return *this;
}
*/




} // drain::



#endif


