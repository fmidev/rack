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
 * ProductOp.h
 *
 *  Created on: Mar 7, 2011
 *      Author: mpeura
 */

#ifndef RACK_BASE
#define RACK_BASE "ProductBase"

#include <drain/Log.h>
#include <stdexcept>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <string>

#include <drain/RegExp.h>
#include <drain/util/SmartMap.h>
#include <drain/util/TreeOrdered.h>

#include "hi5/Hi5.h"
#include "data/Data.h"
#include "data/DataSelector.h"
#include "data/ODIM.h"
#include "data/ODIMPath.h"
#include "data/ProductConf.h"

namespace rack {

using namespace drain::image;


/// Base class for radar data processors. Input can be in polar or Cartesian coordinates.
/** Input and output as HDF5 data, which has been converted to internal structure, drain::Tree<NodeH5>.
 *
 *  Input can be
 *  - polar sweeps, volumes or even str polar products
 *  - Cartesian single-radar products or composites.
 *
 *  Basically, there are two kinds of polar processing
 *  - Cumulative: the volume is traversed, each sweep contributing to a single accumulation array, out of which the product layer(s) is extracted.
 *  - Sequential: each sweep generates new layer (/dataset) in the product; typically, the lowest only is applied.
 *
 *  TODO: Raise to RackOp
 *  TODO: Consider common processDataSet for all data types, see ImageOpRacklet::exec()
 *  \tparam TS - source (input) ODIM type
 *  \tparam TD - destination (output) ODIM type
 */
//template <class TS, class TD>
class ProductBase : public ProductConf, public drain::BeanLike {

protected:

	/// Default constructor
	inline
	ProductBase(const std::string &name, const std::string & description) : drain::BeanLike(name, description){
		dataSelector.setPathMatcher(ODIMPathElem::DATASET);
		// dataSelector.updateBean();
	}

	inline
	ProductBase(const ProductBase & product) : ProductConf(product), drain::BeanLike(product){
	}

	inline
	void storeLastArguments(const std::string & p) override {
		lastArguments = p;
	};

	std::string lastArguments;

public:

	inline
	const std::string & getLastArguments() const{
		return lastArguments;
	};


	/// Destructor
	virtual
	~ProductBase(){};


	/// Copies the conditions of another selector.
	inline
	void setDataSelector(const DataSelector & selector){ dataSelector.setParameters(selector.getParameters()); };

	/// Returns the data selector of this operator.
	inline
	const DataSelector & getDataSelector() const { return dataSelector; };

	/// Returns the data selector of this operator.
	inline
	DataSelector & getDataSelector(){ return dataSelector; };

	/// Dumps the help of this operator.
	void help(std::ostream &ostr = std::cout, bool showDescription = true) const;


	/// Returns a map of encoding parameters that can be changed by the user.
	inline
	const drain::ReferenceMap & getAllowedEncoding() const { return allowedEncoding; };


	/// Set encoding parameters for the result: type,gain,offset,undetect,nodata
	/**
	 *  These parameters do not change the "meteorology" of the product, but only its scaling and so on.
	 *  The operator may limit the access to parameters.
	 */
	virtual
	inline
	void setEncodingRequest(const std::string &p) {

		drain::Logger mout(__FILE__, __FUNCTION__); //REPL name+"(ProductOp)", __FUNCTION__);

		try {
			mout.debug2("Checking if these are allowed" );
			allowedEncoding.setValues(p); // may throw?
		}
		catch (std::exception & e) {
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.warn(" unsupported parameters in: '" , p , "', use: " , allowedEncoding.getKeys() );
			return;
		}

		// assign.
		targetEncoding = p;
	}

	// under construction
	void setAllowedEncoding(const std::string & keys);

	/// Modifies encoding. If type is changed, resets scaling first.
	/**
	 *  TODO: Template ODIM
	 *  (Set allowed encoding under construction.)
	 */
	static
	void completeEncoding(ODIM & productODIM, const std::string & targetEncoding);



	/// Sets target quantity and encoding, if unset. If input odim
	/**
	 *   \param productODIM - metadata to be modified
	 *   \param srcODIM - metadata applied; typically that of an product operator or input data.
	 *   \param applyDefaults - use default values in encoding (esp. gain), if still unset at the end of this function call.
	 */
	static
	void applyODIM(ODIM & productODIM, const ODIM & srcODIM, bool applyDefaults = false);


	static
	void setRackVersion(drain::VariableMap & metadata);

protected:

	/// Sets target encoding.
	static
	void setODIMspecials(ODIM & productODIM);

	/// Defines which encoding parameters can be changed by the user from command line.
	/**
	 *   Regulates which parameters are visible and accessible on command line.
	 *   The parameters are set in the constructor by referencing them to this->odim.
	 *   The parameters are listed in help.
	 *
	 *   Assigning parameters that have not been referenced will raise an error.
	 *
	 *   See setEncoding() which carries that task.
	 */
	drain::ReferenceMap allowedEncoding;


};



inline
std::ostream & operator<<(std::ostream & ostr, const rack::ProductBase &op){
	op.help(ostr); // todo: name & params only
	return ostr;
}




}  // namespace rack


#endif // RACK_PRODUCT_O
