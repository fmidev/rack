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

#include <drain/util/Log.h>

#include "ODIMValidator.h"

namespace rack {

const ODIMValidator::h5dict_t & ODIMValidator::getH5TypeDict(){

	static
	h5dict_t dict;

	if (dict.empty()){
		dict.add(H5I_GROUP, "Group");
		dict.add(H5I_DATASET, "Dataset");
		dict.add(H5I_ATTR, "Attribute");
	}

	return dict;
}


ODIMValidator & ODIMValidator::assign(const std::string & s){

	std::vector<std::string> v;

	drain::StringTools::split(s, v, ';');

	pathRegExp.setExpression(v[0]);

	h5Type = getH5TypeDict().getKey(v[1]);

	//value.reset();
	basetype.setType(typeid(void));
	if (v[2] == "real"){
		basetype.setType(typeid(double));
	}
	else if (v[2] == "integer"){
		basetype.setType(typeid(long int));
	}
	else { // WARN?
		basetype.setType(typeid(std::string));
	}

	required.setType(typeid(bool));
	required = v[3];

	if (v.size() > 4){
		valueRegExp = v[4];
	}
	else {
		valueRegExp = "";
	}
	// std::cout << "  VAL:" << s << " => " << valueRegExp.toStr() << std::endl;
	// .*/quality[1-9][0-9]*/what/nodata;Attribute;real;FALSE;


	return *this;

}

std::ostream & ODIMValidator::toOStr(std::ostream & ostr) const {

	ostr << '@';

	ostr << pathRegExp.toStr() << ';';

	ostr << getH5TypeDict().getValue(h5Type) << ';';

	if (basetype.isString())
		ostr << "string";
	else
		ostr << drain::Type::call<drain::simpleName>(basetype.getType());
	ostr << ';';

	// drain::StringTools::upperCase(required.toStr());
	ostr << required << ';';

	ostr << valueRegExp.toStr() << '$';

	return ostr;

}


}  // namespace rack



