/**


    Copyright 2014 - 2015   Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU Lesser Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.


*/


#ifndef RACK_PRODUCT_ADAPTER
#define RACK_PRODUCT_ADAPTER

#include "commands.h"

namespace rack {


/// Wrapper for meteorological products derived from VolumeOp.
/**
 *  \tparam T - product operator, like CappiOp.
 */
template <class T>
class ProductAdapter : public Command {

public:

	ProductAdapter(bool add = true) : Command(), adapterName(std::string("ProductAdapter<")+productOp.getName()+">"){ // adapterName("test")

		static RegExp nameCutter("^(.*)(Op|Functor)$");
		const std::string & key = productOp.getName();
		if (nameCutter.execute(key) == 0){ // Matches
			getRegistry().add(*this, nameCutter.result[1], 0);
			// addEntry(*this, nameCutter.result[1], 0);
		}
		else {
			getRegistry().add(*this, key, 0);
			//addEntry(*this, name, 0);
		}
		//addEntry(*this, productOp.getName(), 0);
	};

	~ProductAdapter(){};

	T productOp;

	const std::string adapterName;

	inline const std::string & getName() const { return productOp.getName(); };


	/// Returns a description for help functions.
	const std::string & getDescription() const {
		if (descriptionFull.empty()){
			std::stringstream sstr;
			sstr << productOp.getDescription() << '\n';
			EncodingODIM odim(productOp.odim);
			sstr << "  # Encoding: "; // << EncodingODIM(productOp.odim);
			for (EncodingODIM::const_iterator it = odim.begin(); it != odim.end(); ++it){
				const std::string & key = it->first;
				const size_t i = key.find(':');
				if (i != std::string::npos){
					//alias(key.substr(i+1), key);
					sstr << key.substr(i+1) << '=' << it->second << ',';
				}
			}
			const std::string k = productOp.getAllowedEncoding().getKeys();
			if (!k.empty())
				sstr << " user modifiable: " << k;
			sstr << '\n';
			//sstr << "  # Selector: " << productOp.dataSelector;
			sstr << "  # Input quantities: " << productOp.dataSelector.quantity;
			descriptionFull = sstr.str();
			//+ "\n# Selector: " + productOp.dataSelector;
		}
		return descriptionFull;
	}


	const ReferenceMap & getParameters() const {
		return productOp.getParameters();
	};


	// NOTE: Potentially, VolumeOp could be generalized to a RackOp, having polar OR Cartesian targets.
	virtual
	inline
	HI5TREE & getTarget() const { //std::cerr << "getTarget() = polarHi5" << std::endl;
		return getResources().polarHi5;
	};

	virtual
	//void run(const VariableMap & parameters = VariableMap()) { rackLet.run(parameters); };
	/**
	 *  Like AnDReLetAdapter, but filters inputHi5 to polarHi5.
	 */
	void run(const std::string & params = "") {

		drain::Logger mout(adapterName+"::"+__FUNCTION__);

		mout.timestamp("BEGIN_PRODUCT");

		//VolumeOp<PolarODIM> & op = productOp;
		productOp.setParameters(params);

		RackResources & resources = getResources();

		if (!resources.select.empty()){
			mout.debug() << "Setting data selector: " << resources.select << mout.endl;
			productOp.dataSelector.setParameters(resources.select);
			mout.debug(1) << "New values: " << productOp.getDataSelector() << mout.endl;
			resources.select.clear();
		}

		if (!resources.targetEncoding.empty()){
			mout.debug() << "Setting target parameters: " << resources.targetEncoding << mout.endl;
			//productOp.getODIM().setValues(cmd.value, true); // nbins, nrays not in EncodingODIM, consider other than that?
			productOp.setEncodingRequest(resources.targetEncoding);
			//mout.debug(1) << "New values: " << productOp.odim  << mout.endl;
			resources.targetEncoding.clear();
		}

		mout.debug() << "Running: " << productOp.getName() << mout.endl;

		//op.filter(getResources().inputHi5, getResources().polarHi5);
		const HI5TREE & src = resources.inputHi5;
		HI5TREE & dst = getTarget();  //For AnDRe ops, src serves also as dst.

		//mout.warn() << dst << mout.endl;
		productOp.processVolume(src, dst);

		DataTools::updateCoordinatePolicy(dst, RackResources::polarLeft);
		DataTools::updateAttributes(dst);
		resources.currentPolarHi5 = & dst; // if cartesian, be careful with this...
		resources.currentHi5      = & dst;

		mout.timestamp("END_PRODUCT");
	};

	/// By default, source and destination objects are separate.
	//  (For AnDRe, source and destination are the same object.)
	/*
	virtual
	inline
	void filter(const HI5TREE &src, HI5TREE &dst) const {
		const VolumeOp & op = productOp;
		op.filter(getResources().inputHi5, getResources().polarHi5);
	}
	*/

	virtual
	inline
	std::ostream & toOstream(std::ostream & ostr) const {
		//const VolumeOp<PolarODIM> & op = productOp;
		ostr << adapterName << ": " << productOp;
		return ostr;
	}

protected:

	mutable std::string descriptionFull;

};


} /* namespace rack */

#endif /* RACK_PRODUCTS */
