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


//const std::set<ODIMPathElem> & EncodingODIM::attributeGroups(createAttributeGroups());

const ODIMPathElemSeq & EncodingODIM::attributeGroups(getAttributeGroups());

const ODIMPathElemSeq & EncodingODIM::getAttributeGroups(){
	static ODIMPathElemSeq s = {ODIMPathElem::WHAT, ODIMPathElem::WHERE, ODIMPathElem::HOW};
	return s;
}

const drain::FlaggerDict EncodingODIM::settingDict = {
		{"NONE", NONE},
		{"SCALING", SCALING},
		{"RANGE", RANGE}
};

EncodingODIM::EncodingODIM(const EncodingODIM & odim) : scaling(ownScaling), scalingConst(ownScaling), explicitSettings(odim.explicitSettings){
	init(ODIMPathElem::ALL_LEVELS); //2023/04/24
	//copyStruct(odim, odim, *this); // 2023/04/24
	//keyList = odim.keyList; // Otherways order comes back to default (std::map key order)
	updateFromMap(odim); // importMap can NOT be used because non-EncodingODIM arg will have a larger map
	scaling.physRange.set(odim.scaling.physRange);
};


/// Scale driven encoding for brace inits. RISK: group_t confusion?
EncodingODIM::EncodingODIM(char type, double scale, double offset, double nodata, double undetect, const drain::Range<double> & range) :
	scaling(ownScaling),
	scalingConst(ownScaling),
	explicitSettings(NONE)
	{

	init(ODIMPathElem::ALL_LEVELS); // Note: clears values
	//init(ODIMPathElem::OTHER);

	this->type.assign(1, type);
	setTypeDefaults();

	if (scale == 0.0){
		explicitSettings |= SCALING;
	}
	else {
		this->scaling.set(scale, offset);
	}

	if (!std::isnan(nodata)){
		this->nodata = nodata;
	}

	if (!std::isnan(undetect)){
		this->undetect = undetect;
	}

	if (range.empty()){
		explicitSettings |= RANGE;
	}
	else {
		// Override those of type defaults?
		this->scaling.setPhysicalRange(range); // does not change scaling
	}

};

/// Range-driven encoding for brace inits.
EncodingODIM::EncodingODIM(char type, const drain::Range<double> & range, double scale, double offset, double nodata, double undetect) :
	scaling(ownScaling),
	scalingConst(ownScaling),
	explicitSettings(NONE)
	{

	init(ODIMPathElem::ALL_LEVELS); // Note: clears values
	// init(ODIMPathElem::OTHER);

	this->type.assign(1, type);
	//setTypeDefaults();
	setRange(range.min, range.max); // sets scaling

	if (scale != 0.0){
		explicitSettings |= SCALING;
		this->scaling.set(scale, offset);
	}

	if (!std::isnan(nodata)){
		this->nodata = nodata;
	}

	if (!std::isnan(undetect)){
		this->undetect = undetect;
	}

	//std::cerr << type << " ERROR DANGERED\n";

};

void EncodingODIM::init(group_t initialize){ // ::referenceRootAttrs(){

	if (initialize & ODIMPathElem::ROOT){
	}

	if (initialize & ODIMPathElem::DATASET){
	}

	if (initialize & ODIMPathElem::DATA){
		link("what:type", type = "C");
		link("what:gain",   scaling.scale = 0.0);
		link("what:offset", scaling.offset = 0.0);
		link("what:undetect", undetect = 0.0);
		link("what:nodata", nodata = 0.0);
		// Experimental
		// link("how:physRange", scaling.physRange.tuple());
		// link("how:physRange", scaling.physRange);
	}

	/*
	if (initialize & ODIMPathElem::OTHER){
		link("type", type = "C");
		link("physRange", scaling.physRange.tuple());
		link("gain",   scaling.scale = 0.0);
		link("offset", scaling.offset = 0.0);
		link("undetect", undetect = 0.0);
		link("nodata", nodata = 0.0);
	}
	*/

}


void EncodingODIM::initFromImage(const drain::image::Image & img){  // =""
	init(ODIMPathElem::ALL_LEVELS);
	EncodingODIM::copyFrom(img);
}



void EncodingODIM::clear(){
  for (drain::ReferenceMap::iterator it = begin(); it != end(); ++it)
	  it->second.clear();
  // TODO: for (drain::Referencer & ref: *this){ }
}



EncodingODIM & EncodingODIM::setScaling(double gain, double offset){

	if (gain != 0.0)
		explicitSettings |= SCALING;

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

	if (gain != 0.0)
		explicitSettings |= SCALING;

	if (type.empty())
		type = "C";

	this->scaling.set(gain, offset);
	//this->scaling.scale = gain;
	//this->scaling.offset = offset;
	this->undetect = undetect;
	this->nodata = nodata;

	return *this;
}


void EncodingODIM::updateLenient(const EncodingODIM & odim){

	if (type.empty())
		type = odim.type;

	if ((this->scaling.scale == 0.0) && (type == odim.type)){
		this->scaling.assign(odim.scaling);
		//scale   = odim.scale;
		//offset = odim.offset;
		nodata = odim.nodata;
		undetect = odim.undetect;
	}

}

// Rename to grant
void EncodingODIM::grantShortKeys(drain::ReferenceMap & ref) {

	//const EncodingODIM::keylist_t & keys = getKeyList();

	for (const std::string & key: getKeyList()){
		const size_t i = key.find(':');  // type?
		if (i != std::string::npos){
			ref.link(key.substr(i+1), operator[](key));
		}
	}
}

void EncodingODIM::copyFrom(const drain::image::Image & data){

	drain::Logger mout(__FILE__, __FUNCTION__);

	const drain::FlexVariableMap & m = data.getProperties();

	//for (drain::FlexVariableMap::const_iterator it = m.begin(); it != m.end(); ++it){
	for (const auto & entry: m){

		const EncodingODIM::iterator oit = find(entry.first);

		if (oit != end()){
			const drain::FlexibleVariable & srcValue = entry.second;
			const std::type_info & t = srcValue.getType();
			drain::Castable & dstValue = oit->second;
			// std::cerr << key << " type=" << t.name() << '\n';
			mout.debug3("setting '" , entry.first , "'=" , srcValue  , '|' , drain::Type::getTypeChar(t) );

			if (t == typeid(double)){
				dstValue = static_cast<double>(srcValue);
			}
			else if (t == typeid(float))
				dstValue = static_cast<float>(srcValue);
			else
				dstValue = srcValue;
		}
		else {
			mout.reject<LOG_DEBUG+2>("img property '" , entry.first , "' unsupported by EncodingODIM" );
		}

	}
	type = drain::Type::getTypeChar(data.getType());
	//mout.attention("copyFrom:", data);
}


void EncodingODIM::setRange(double min, double max) {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (min != max)
		explicitSettings |= RANGE;

	if (type.empty())
		type = "C";

	//const drain::Type t(type.at(0));
	const drain::Type t(type);

	const double minData = drain::Type::call<drain::typeMin, double>(t); // getMin<double>(type);
	const double maxData = drain::Type::call<drain::typeMax, double>(t); // drain::Type::call<drain::typeMax,double>(t);

	undetect = minData;
	nodata   = maxData;


	if (drain::Type::call<drain::typeIsInteger>(t)){
		scaling.scale = (max-min) / static_cast<double>((maxData-1) - (minData+1));
		scaling.offset = min - scaling.scale*(minData+1);
		//drain::Logger mout("QuantityMap", __FUNCTION__);
	}
	else {
		scaling.set(1.0, 0.0);
		//scaling.scale = 1.0;
		//scaling.offset = 0.0;
	}

	/*
	if (!scaling.physRange.empty()){
		mout.warn("resetting range: ", scaling.physRange, " -> ", min, ',',  max);
	}
	else {
		//mout.special(" phys range:", min, ',',  max);
	}
	*/

	scaling.physRange.set(min, max);

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

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::type_info & t = drain::Type::getTypeInfo(type);

	if (t == typeid(void))
		throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

	if (drain::Type::call<drain::typeIsInteger>(t)){
		long int i = drain::Type::call<drain::typeMin, long int>(t);
		double d;
		do { // loop, yet max 2 steps
			d = static_cast<double>(i);
			//mout.warn("trying: " , i , " \t-> " , scaleForward(d) );
			++i;
		} while ((d == undetect) || (d == nodata));
		return scaleForward(d);
	}
	else
		return scaleForward( drain::Type::call<drain::typeMin, double>(t) );

}

double EncodingODIM::getMax() const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	const std::type_info & t = drain::Type::getTypeInfo(type);

	if (t == typeid(void))
		throw std::runtime_error(std::string("ODIM")+__FUNCTION__+" type unset");

	if (drain::Type::call<drain::typeIsInteger>(t)){
		long int i = drain::Type::call<drain::typeMax, long int>(t);
		double d;
		do { // loop, yet max 2 steps
			d = static_cast<double>(i);
			// mout.warn("trying: " , i , " \t-> " ,  scaleForward(d) );
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

