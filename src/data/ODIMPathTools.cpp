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

#include <algorithm>
#include <syslog.h>  // levels: LOG_ERROR etc.

//#include <drain/util/Type.h>
//#include <drain/util/RegExp.h>

#include "hi5/Hi5.h"
#include "ODIMPathTools.h"


namespace rack {

bool ODIMPathTools::getLastChild(const Hi5Tree & tree, ODIMPathElem & child, bool create){ //, (ODIMPathElem::group_t g

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!ODIMPathElem::isIndexed(child.getType())){
		mout.warn(": index requested for unindexed path element '" , child , "'" );
		return false;
	}

	child.index = 0; // needed
	//for (Hi5Tree::const_iterator it = tree.begin(); it != tree.end(); ++it){
	for (const auto & entry: tree.getChildren()){

		if (entry.first.getType() == child.getType()){
			child.index = std::max(child.getIndex(), entry.first.getIndex());
			mout.debug3("considering (", child, ")");
		}

	}

	if (create && (child.index == 0)){
		child.index = 1;
	}

	return child.getIndex() > 0;

}

bool ODIMPathTools::getNewChild(const Hi5Tree & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!child.isIndexed()){ // ODIMPathElem::isIndexed(child.getType())){
		mout.warn(": index requested for unindexed path element '" , child , "'" );
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




bool ODIMPathTools::getNextChild(const Hi5Tree & tree, ODIMPathElem & child){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (getLastChild(tree, child)){
		++child.index;
		return true;
	}
	else {
		//if (!ODIMPath::isIndexed(child.getType())){
		if (! child.isIndexed()){
			mout.warn("index requested for unindexed path element '", child, "'");
		}
		else {
			child.index = 1;
			mout.debug("returning a new child element '", child, "'");
		}
		return false;
	}
}



// Todo: rename... getChildren by quantity? Also, WHAT + "quantity" needed?
bool ODIMPathTools::getChildren(const Hi5Tree & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups){

	//for (Hi5Tree::const_iterator it = tree.begin(); it != tree.end(); ++it){
	for (const auto & entry: tree){

		// const ODIMPathElem & elem = it->first;
		if (entry.first.belongsTo(groups)){
			//children[tree[entry.first].data.image.properties["what:quantity"]] = entry.first;
			children[entry.second.data.image.properties["what:quantity"]] = entry.first;
		}

	}
	return !children.empty();
}


}  // rack::
