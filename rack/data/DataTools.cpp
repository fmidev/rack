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



/*
const drain::Variable & DataTools::getAttribute(const HI5TREE &src, const std::string & path, const std::string & group, const std::string & attributeName){

	const HI5TREE &g = src(path)[group];

	if (g.data.attributes.hasKey(attributeName))
		return g.data.attributes[attributeName];
	else {
		if (path.empty()){
			static drain::Variable dummy;
			return dummy;
		}
		else {
			size_t i = path.rfind('/');
			//if ((i != path.npos) && (i != 0))
			if (i != path.npos)
				return getAttribute(src, path.substr(0,i), group, attributeName);
			else
				return getAttribute(src, "", group, attributeName);
			//else
			//	return dummy;
		}
	}
}
 */

// const drain::image::CoordinatePolicy & policy,
//void DataSelector::_updateAttributes(HI5TREE & src, const drain::VariableMap & attributes){
void DataTools::updateAttributes(HI5TREE & src,  const drain::VariableMap & attributes){

	//drain::Logger mout(drain::monitor, "DataTools::updateAttributes");
	drain::Logger mout(__FILE__, __FUNCTION__);


	drain::VariableMap & a = src.data.dataSet.properties;
	// Copy
	//a.clear();
	a.importMap(attributes);

	const bool HAS_DATA = src.hasChild("data");
	if (HAS_DATA){  // move down?
		const drain::image::Image & img = src["data"].data.dataSet;
		//img.setCoordinatePolicy(policy);
		if (img.typeIsSet())
			a["what:type"] = std::string(1u, drain::Type::getTypeChar(img.getType()));
	}



	// Traverse /what, /where, and /how groups.
	const std::set<std::string> & g = EncodingODIM::attributeGroups;
	std::stringstream sstr;
	for (std::set<std::string>::const_iterator git = g.begin(); git != g.end(); ++git){
		if (src.hasChild(*git)){
			const drain::VariableMap  & groupAttributes = src[*git].data.attributes;
			for(drain::VariableMap::const_iterator it = groupAttributes.begin(); it != groupAttributes.end(); it++){
				sstr.str("");
				sstr << *git << ':' << it->first;
				a[sstr.str()] = it->second;
				// if (it->first == "quantity") mout.warn() << "quantity=" << it->second << mout.endl;
			}
		}
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
		if (g.find(it->first) == g.end())
			updateAttributes(it->second,  a); // policy,
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

	drain::image::Image & data = src.data.dataSet;
	if (!data.isEmpty()){
		data.setCoordinatePolicy(policy);
		//data.setName(path + ':' + data.properties["what:quantity"].toStr());
		data.setName(data.properties["what:quantity"].toStr());
	}

	const std::set<std::string> & g = EncodingODIM::attributeGroups;

	for (HI5TREE::iterator it = src.begin(); it != src.end(); ++it){
		const std::string & key = it->first;
		//if ((key != "what" ) && (key != "where" ) && (key != "how" ))
		if (g.find(key) == g.end())
			updateCoordinatePolicy(it->second, policy);

	}
}



bool DataTools::getNewChild(HI5TREE & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax){

	if (BaseODIM::isIndexed(child.getType())){
		std::cerr << __FILE__ << ':' << __FUNCTION__ << ": index requested for unindexed path element '" << child << "'" << std::endl;
		//throw std::runtime_error("DataTools::getNewChild indexing requested for path element ");
		return false;
	}

	const ODIMPathElem::index_t iMin = std::max(1, child.index);

	for (ODIMPathElem::index_t i = iMin; i<iMax; ++i){
		child.index = i;
		if (!tree.hasChild(child)){
			return true;
		}
	}

	return false;
}


bool DataTools::getLastChild(HI5TREE & tree, ODIMPathElem & child){ //, BaseODIM::group_t g



	if (!BaseODIM::isIndexed(child.getType())){
		std::cerr << __FILE__ << ':' << __FUNCTION__ << ": index requested for unindexed path element '" << child << "'" << std::endl;
		//throw std::runtime_error("DataTools::getNewChild indexing requested for path element ");
		return false;
	}

	ODIMPathElem e;
	child.index = 0; // needed
	for (HI5TREE::const_iterator it = tree.begin(); it != tree.end(); ++it){

		e.set(it->first);

		if (e.getType() == child.getType()){
			//std::cerr << __FUNCTION__ << " consider " << e << '\n';
			child.index = std::max(child.getIndex(), e.getIndex());
			/*if (e.getIndex() > child.getIndex()){
				child.index = e.getIndex();
			}
			*/
		}
	}

	return child.getIndex() > 0;

}


DataSelector2::DataSelector2() : drain::BeanLike("DataSelector2") {

	// For compatibility
	parameters.reference("path", path);

	parameters.reference("elangle", elangle.vect);

	parameters.reference("quantity", quantityStr);
	// quantity.insert("");
	// reset(){...} ?

	parameters.reference("dataset", dataset.vect);
	parameters["dataset"].fillArray = true;

	parameters.reference("data", data.vect);
	parameters["data"].fillArray = true;

}

void DataSelector2::reset(){
	dataset.min = 0;
	dataset.max = 0xffff;
	data.min = 0;
	data.max = 0xffff;
	elangle.min = -90.0;
	elangle.max = +90.0;
}

void DataSelector2::collapse(){
	dataset.max = dataset.min;
	data.max = data.min;
	elangle.max = elangle.min;
}

bool DataSelector2::isValidPath(const ODIMPath & path) const {

	for (ODIMPath::const_iterator it = path.begin(); it != path.end(); ++it){
		switch (it->group) {
			case ODIM::DATASET:
				if (!dataset.isInside(it->index))
					return false;
				break;
			case ODIM::DATA:
				if (!data.isInside(it->index))
					return false;
				break;
			default:
				break;
		}
	}

	return true;

}


bool DataSelector2::isValidData(const drain::ReferenceMap & properties) const {

	if (properties.hasKey("where:elangle"))
		if (!this->elangle.isInside(properties["where:elangle"]))
			return false;

	if (properties.hasKey("what:quantity")){
		drain::StringTools::split(quantityStr, quantitySet, ':');
		if (quantitySet.find(properties["what:quantity"]) == quantitySet.end())
			return false;
	}

	return true;

}

}  // rack::

// Rack
