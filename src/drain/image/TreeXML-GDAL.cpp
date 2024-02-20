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

#include "TreeXML-GDAL.h"


//#include "drain/util/Log.h"
#include "drain/util/StringBuilder.h"
#include "drain/image/AccumulatorGeo.h"


namespace drain
{

namespace image
{


template <>
std::map<NodeGDAL::tag_t,std::string> NodeXML<BaseGDAL::tag_t>::tags = {
	{drain::image::BaseGDAL::UNDEFINED,	"UNDEFINED"},
	{drain::image::BaseGDAL::ROOT,	    "GDALMetadata"},
	{drain::image::BaseGDAL::ITEM,	    "Item"},
	{drain::image::BaseGDAL::USER,	    "Item"},
//	{drain::image::BaseGDAL::OFFSET,	"Item"},
//	{drain::image::BaseGDAL::SCALE,	    "Item"},
//	{drain::image::BaseGDAL::UNITS,	    "Item"},
};

NodeGDAL::NodeGDAL(const tag_t &  t){
	setType(t);
	//this->id = -1;
	//this->name = "~";
}

NodeGDAL::NodeGDAL(const NodeGDAL & node){
	// this->id = -1;
	setType(node.getType());
	// tag = node.tag;
	sample = 0;
	copyStruct(node, node, *this);
}


void NodeGDAL::setType(const tag_t & t){

	type = t;

	if (t == ROOT){
		//tag = "GDALMetadata";
	}
	else if (t == ITEM){
		// tag = "Item";
		link("sample", sample = 0);
		link("role",   role   = "");
		link("name",   name   = "");
	}
	else { // USER
		//tag = "Item";
		// link("name", name);
		// link("role",   role = "");
	}

}

/*
void NodeGDAL::setGDAL(const std::string & name, const drain::Variable & ctext, int sample, const std::string & role){

	setType(ITEM);

	this->name   = name;
	this->ctext  = ctext.toStr();
	this->sample = sample;

	if (role.empty()){
		this->role   = name;
		drain::StringTools::lowerCase(this->role);
	}
	else
		this->role   = role;


}
*/

/*
for (drain::ReferenceMap::const_iterator it = this->begin(); it != this->end(); it++){
	std::cerr << tag << '=' << it->first << ':' << it->second << '\n';
};
*/

/*
void NodeGDAL::setGDAL(const std::string & name, const drain::Variable & ctext){
	setType(ITEM);
	this->name   = name;
	this->ctext  = ctext.toStr();

}
*/
//this->sample = sample;
//this->role   = role;
// drain::StringTools::upperCase(this->name);

/*
for (drain::ReferenceMap::const_iterator it = this->begin(); it != this->end(); it++){
	std::cerr << tag << '=' << it->first << ':' << it->second << '\n';
};
*/

/*
void NodeGDAL::setText(const drain::Variable & value){
	this->ctext  = value.toStr();
}

void NodeGDAL::setText(const std::string & text){
	this->ctext  = text;
}
*/



} // image::





} // drain::
