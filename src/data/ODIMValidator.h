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
#ifndef ODIM_VALIDATOR
#define ODIM_VALIDATOR

#include <drain/Type.h>
#include <string>
#include <typeinfo>

#include <hdf5.h>

#include <drain/RegExp.h>
#include <drain/VariableLike.h>
#include "drain/util/Dictionary.h"


#include "ODIMPath.h"


namespace rack {

// .*/quality[1-9][0-9]*/what/nodata;Attribute;real;FALSE;


class ODIMNodeValidator {

public:

	typedef drain::Dictionary<H5I_type_t, std::string> h5dict_t;


	ODIMNodeValidator & assign(const std::string & line);

	std::ostream & toOStr(std::ostream & ostr) const;

	drain::RegExp pathRegExp;

	h5dict_t::key_t h5Type;

	//drain::Type basetype;
	drain::Variable basetype; // only for type

	drain::Variable required;

	drain::RegExp valueRegExp;


protected:

	static
	const h5dict_t & getH5TypeDict();

};


inline
std::ostream & operator<<(std::ostream & ostr, const ODIMNodeValidator & v){
	return v.toOStr(ostr);
}

class ODIMValidator : public std::list<ODIMNodeValidator> {
public:

	/// Searches for an entry with matching \c pathRegExp
	/**
	 *  Works like STL find(), but uses regExp for comparison.
	 *  \param path - path to be checked, including root, eg. "/dataset2/data4/what"
	 *  \param h5type - H5I_GROUP, H5I_ATTRIB or H%I_DATASET
	 *  \return iterator pointing to the matching entry, if found; else end().
	 */
	const_iterator validate(const std::string & path, H5I_type_t h5type = H5I_GROUP) const;

};

}  // namespace rack

#endif
