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


namespace drain
{

namespace image
{

struct BaseGDAL {

	// Small
	enum tag_t {
		UNDEFINED=drain::NodeXML<>::UNDEFINED,
		CTEXT=drain::NodeXML<>::CTEXT,
		// Small indiced reserved for base XML
		ROOT=10,
		ITEM=11,
		USER=12}; // , OFFSET, SCALE, UNITS}; // check CTEXT, maybe implement in XML

};

// https://www.awaresystems.be/imaging/tiff/ti fftags/gdal_metadata.html
class NodeGDAL: public BaseGDAL, public drain::NodeXML<BaseGDAL::tag_t> {
public:

	//enum type { UNDEFINED, ROOT, ITEM, USER }; // check CTEXT, maybe implement in XML

	/// Constructor
	NodeGDAL(const tag_t & t = BaseGDAL::USER);

	/// Copy constructor
	NodeGDAL(const NodeGDAL & node);

	// void setGDAL(const std::string & name, const drain::Variable & ctext, int sample, const std::string & role = "");
	//void setGDAL(const drain::Variable & ctext, int sample=0, const std::string & role = "");

	// void setGDAL(const std::string & name, const drain::Variable & ctext);

	template <class T>
	inline
	NodeGDAL & operator=(const T & x){
		setText(x);
		return *this;
	}

	/*
	// Set user attribute
	void setText(const drain::Variable & value);

	// Set user attribute
	void setText(const std::string & value);
	*/

	virtual
	void setType(const tag_t & t);



	/// Standard GDAL attribute
	int sample;

	/// Standard GDAL attribute
	std::string role;

	// Multi-purpose key
	// std::string value;
	std::string name;

protected:

};




/*

inline
std::ostream & operator<<(std::ostream &ostr, const TreeGDAL & tree){
	//return drain::NodeXML::toStream(ostr, tree);
	//return drain::NodeXML<>::docToStream(ostr, tree);
	// return drain::NodeXML<>::toStream(ostr, tree);
	return TreeGDAL::node_data_t::toStream(ostr, tree);
}
*/

} // image::

} // drain::

#endif


