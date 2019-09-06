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

#include <drain/util/Type.h>

#include "hi5/Hi5.h"
#include "DataTools.h"

namespace rack {

using namespace hi5;


// const drain::image::CoordinatePolicy & policy,
//void DataSelector::_updateAttributes(HI5TREE & src, const drain::VariableMap & attributes){
void DataTools::updateInternalAttributes(HI5TREE & src,  const drain::FlexVariableMap & attributes){

	drain::Logger mout(__FILE__, __FUNCTION__);

	/// Write to "hidden" variables of this node (src).
	drain::FlexVariableMap & a = src.data.dataSet.properties;
	a.importMap(attributes);
	//std::cerr << "MAP now: " << a << "\n\n";

	/// Collect values of succeeding /what, /where, and /how groups .
	const std::set<ODIMPathElem> & g = EncodingODIM::attributeGroups;
	std::stringstream sstr;
	for (std::set<ODIMPathElem>::const_iterator git = g.begin(); git != g.end(); ++git){
		if (src.hasChild(*git)){
			const drain::VariableMap  & groupAttributes = src[*git].data.attributes;
			for(drain::VariableMap::const_iterator it = groupAttributes.begin(); it != groupAttributes.end(); it++){
				sstr.str("");
				sstr << *git << ':' << it->first;
				a[sstr.str()] = it->second;
				// mout.warn() << sstr.str() << '=' << it->second << " ... " << a[sstr.str()] << mout.endl;
				// if (it->first == "quantity") mout.warn() << "quantity=" << it->second << mout.endl;
			}
		}
	}

	const bool HAS_DATA = src.hasChild("data");

	if (HAS_DATA){  // move down?
		const drain::image::Image & img = src["data"].data.dataSet;
		//img.setCoordinatePolicy(policy);
		if (img.typeIsSet())
			a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
	}

	if (HAS_DATA){
		drain::image::Image & img = src["data"].data.dataSet;
		if (img.typeIsSet()){
			const drain::image::ImageScaling & s = img.getScaling();
			img.setScaling(a.get("what:gain", s.scale), a.get("what:offset", s.offset));
		}
	}



	// Traverse children (recursion)

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){
		// mout.note() << "considering " << it->first << mout.endl;
		//it->first.be
		//if (! it->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS))
		if (it->first.belongsTo(ODIMPathElem::DATA_GROUPS))
			//if (g.find(it->first) == g.end())
			mout.warn() << "descending to... " << it->first << mout.endl;
			updateInternalAttributes(it->second,  a); // policy,
	}


	// std::cerr << "### updateAttributes"
}

bool DataTools::removeIfNoSave(HI5TREE & dst){
	if (dst.data.noSave){
		drain::Logger mout("DataTools", __FUNCTION__);
		mout.note() << "// about to resetting noSave struct: " << dst.data << mout.endl;
		/*
		dst.data.attributes.clear();
		dst.data.dataSet.resetGeometry();
		dst.getChildren().clear();
		 */
		return true;
	}
	else
		return false;
}

void DataTools::updateCoordinatePolicy(HI5TREE & src, const drain::image::CoordinatePolicy & policy){

	drain::Logger mout(__FILE__, __FUNCTION__);

	drain::image::Image & data = src.data.dataSet;
	if (!data.isEmpty()){
		data.setCoordinatePolicy(policy);
		//data.setName(path + ':' + data.properties["what:quantity"].toStr());
		data.setName(data.properties["what:quantity"].toStr());
		//mout.warn() << "qty=" << data.getName() << " - " << data.getCoordinatePolicy() << '\n';
	}

	// const std::set<ODIMPathElem> & g = EncodingODIM::attributeGroups;

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){
		//const ODIMPathElem & key = it->first;
		if (!it->first.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			//mout.warn() << "updating " << it->first << '\n';
			updateCoordinatePolicy(it->second, policy);
		}
		else {
		//	mout.warn() << "not updating " << it->first << '\n';
		}
		//if (g.find(key) == g.end()) updateCoordinatePolicy(it->second, policy);
	}
}



}  // rack::

// Rack
 // REP
