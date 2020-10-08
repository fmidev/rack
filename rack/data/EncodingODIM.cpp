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

#include "drain/util/Log.h"

#include "EncodingODIM.h"

namespace rack {


const std::set<ODIMPathElem> & EncodingODIM::attributeGroups(createAttributeGroups());

const std::set<ODIMPathElem> & EncodingODIM::createAttributeGroups(){

	static std::set<ODIMPathElem> s;
	s.insert(ODIMPathElem(ODIMPathElem::WHAT));
	s.insert(ODIMPathElem(ODIMPathElem::WHERE));
	s.insert(ODIMPathElem(ODIMPathElem::HOW));
	return s;

}

void EncodingODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ODIMPathElem::ROOT){
	}

	if (initialize & ODIMPathElem::DATASET){
	}

	if (initialize & ODIMPathElem::DATA){
		link("what:type", type = "C");
		link("what:gain",   scale = 0.0);
		link("what:offset", offset = 0.0);
		link("what:undetect", undetect = 0.0);
		link("what:nodata", nodata = 0.0);
	}

}


void EncodingODIM::initFromImage(const drain::image::Image & img){  // =""
	init(ODIMPathElem::ALL_LEVELS);
	EncodingODIM::copyFrom(img);
}



void EncodingODIM::clear(){
  for (ReferenceMap::iterator it = begin(); it != end(); ++it)
	  it->second.clear();
}



EncodingODIM & EncodingODIM::setScaling(double gain, double offset){

	if (type.empty())
		type = "C";

	//char typecode = type.at(0);

	if (std::isnan(offset))
		offset = -gain;

	const std::type_info & t = drain::Type::getTypeInfo(type.at(0));

	return setScaling(gain, offset, drain::Type::call<drain::typeMin, double>(t), drain::Type::call<drain::typeMax, double>(t));
	//return setScaling(gain, offset, drain::Type::getMin<double>(typecode), drain::Type::call<drain::typeMax,double>(typecode));

}

EncodingODIM & EncodingODIM::setScaling(double gain, double offset, double undetect, double nodata) {

	if (type.empty())
		type = "C";

	this->scale = gain;
	this->offset = offset;
	this->undetect = undetect;
	this->nodata = nodata;

	return *this;
}


void EncodingODIM::updateLenient(const EncodingODIM & odim){

	if (type.empty())
		type = odim.type;

	if ((scale == 0.0) && (type == odim.type)){
		scale   = odim.scale;
		offset = odim.offset;
		nodata = odim.nodata;
		undetect = odim.undetect;
	}

}

// Rename to grant
void EncodingODIM::grantShortKeys(drain::ReferenceMap & ref) {

	const EncodingODIM::keylist_t & keys = getKeyList();

	for (EncodingODIM::keylist_t::const_iterator kit = keys.begin(); kit != keys.end(); ++kit){
		const size_t i = kit->find(':');  // type?
		if (i != std::string::npos){
			ref.link(kit->substr(i+1), operator[](*kit));
			//alias(key.substr(i+1), key);
		}
	}
}

void EncodingODIM::copyFrom(const drain::image::Image & data){

	drain::Logger mout(__FUNCTION__, __FILE__);

	const drain::FlexVariableMap & m = data.getProperties();

	for (drain::FlexVariableMap::const_iterator it = m.begin(); it != m.end(); ++it){

		const EncodingODIM::iterator oit = find(it->first);

		if (oit != end()){
			const drain::FlexVariable & srcValue = it->second;
			const std::type_info & t = srcValue.getType();
			drain::Castable & dstValue = oit->second;
			// std::cerr << key << " type=" << t.name() << '\n';
			mout.debug(4) << "setting '" << it->first << "'=" << srcValue  << '|' << drain::Type::getTypeChar(t) << mout.endl;

			if (t == typeid(double)){
				//(*this)[key] = (double)srcValue;
				dstValue = static_cast<double>(srcValue);
				//std::cerr << "DOUBLE" << key << "\n\t";
				/*
					std::cerr.precision(20);
					std::cerr << (double)srcValue << "\n\t";
					std::cerr << undetect << "\n\t";
					//std::cerr << (const double &)srcValue << std::endl;
					std::cerr << (const double &)(*this)[key] << std::endl;
				 */
			}
			else if (t == typeid(float))
				dstValue = static_cast<float>(srcValue);
			else
				dstValue = srcValue;
		}
		else {
			mout.debug(2) << "img property '" << it->first << "' not supported by ODIM" << mout.endl;
		}

	}
	//type = std::string("") + drain::Type::getTypeChar(data.getType());
	type = drain::Type::getTypeChar(data.getType());
}


void EncodingODIM::setRange(double min, double max) {

		if (type.empty())
			type = "C";

		//const drain::Type t(type.at(0));
		const drain::Type t(type);

		const double minData = drain::Type::call<drain::typeMin, double>(t); // getMin<double>(type);
		const double maxData = drain::Type::call<drain::typeMax, double>(t); // drain::Type::call<drain::typeMax,double>(t);

		undetect = minData;
		nodata   = maxData;

		if (drain::Type::call<drain::typeIsInteger>(t)){
			scale = (max-min) / static_cast<double>((maxData-1) - (minData+1));
			offset = min - scale*(minData+1);
			//drain::Logger mout("QuantityMap", __FUNCTION__);
		}
		else {
			scale = 1.0;
			offset = 0.0;
		}

		//if (!defaultType)			defaultType = typecode;

}

void EncodingODIM::checkType(Hi5Tree & dst, EncodingODIM & odim){


	for (std::map<std::string,drain::Referencer>::iterator it = odim.begin(); it != odim.end(); ++it){

		const std::string & key = it->first;
		const size_t i  = key.find(':');

		/// Determine if it is ATTRIBUTE at root or PATH:ATTRIBUTE
		//  drain::Variable & attribute = (i==std::string::npos) ? dst.data.attributes[key] : dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
		if (i==std::string::npos){
			std::cerr << "_checkType no ODIM group? " << key << '\n';
		}
		else {

			Hi5Tree & dstGroup = dst[key.substr(0,i)];

			std::cerr << "checkType: " << key << '\n';

			std::map<std::string,drain::Variable>::iterator ait = dstGroup.data.attributes.find(key.substr(i+1));
			if (ait == dstGroup.data.attributes.end())
				continue;

			std::cerr << "  found: " << ait->first << '\n';

			//drain::Variable & attribute = dst[key.substr(0,i)].data.attributes[key.substr(i+1)];
			const std::type_info & t = it->second.getType();
			if (ait->second.getType() != t){
				it->second = ait->second;
				ait->second.setType(t);
				ait->second = it->second; // DOES NOT WORK!
			}
		}
	}

	/*
	for (Hi5Tree::iterator it = dst.begin(); it != dst.end(); ++it) {
		//checkType(it->second, odim); // TODO skip /what /where?
	}
	*/

}

double EncodingODIM::getMin() const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const std::type_info & t = drain::Type::getTypeInfo(type);

	if (t == typeid(void))
		throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

	if (drain::Type::call<drain::typeIsInteger>(t)){
		long int i = drain::Type::call<drain::typeMin, long int>(t);
		double d;
		do { // loop, yet max 2 steps
			d = static_cast<double>(i);
			//mout.warn() << "trying: " << i << " \t-> " << scaleForward(d) << mout.endl;
			++i;
		} while ((d == undetect) || (d == nodata));
		return scaleForward(d);
	}
	else
		return scaleForward( drain::Type::call<drain::typeMin, double>(t) );

}

double EncodingODIM::getMax() const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const std::type_info & t = drain::Type::getTypeInfo(type);

	if (t == typeid(void))
		throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

	if (drain::Type::call<drain::typeIsInteger>(t)){
		long int i = drain::Type::call<drain::typeMax, long int>(t);
		double d;
		do { // loop, yet max 2 steps
			d = static_cast<double>(i);
			// mout.warn() << "trying: " << i << " \t-> " <<  scaleForward(d) << mout.endl;
			--i;
		} while ((d == undetect) || (d == nodata));
		return scaleForward(d);
		/*
			if (static_cast<double>(i) != nodata)  // or undetect ?
				return scaleForward(static_cast<double>(i));
			else
				return scaleForward(static_cast<double>(i-1));
		 */
	}
	else
		return scaleForward( drain::Type::call<drain::typeMax, double>(t) );

}


}  // namespace rack



// Rack
 // REP // REP // REP
