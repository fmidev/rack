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
#include <drain/util/RegExp.h>

#include "hi5/Hi5.h"
#include "DataSelector.h"


namespace rack {

using namespace hi5;

DataSelector::DataSelector(const std::string & path, const std::string & quantity,
		unsigned int index, unsigned int count,
		double elangleMin, double elangleMax) : BeanLike(__FUNCTION__) { //, elangle(2) {

	//std::cerr << "DataSelector: " << quantity << " => " << this->quantity << std::endl;
	init();
	this->path = path;
	this->quantity = quantity;
	this->index = index;
	this->count = count;
	this->elangle.min = elangleMin;
	this->elangle.max = elangleMax;
	this->groups = ODIMPathElem::ALL_GROUPS;
	//std::cerr << 1 << *this << std::endl;
}


DataSelector::DataSelector(const std::string & parameters) : BeanLike(__FUNCTION__){ //, elangle(2) {

	init();
	setParameters(parameters);

}


DataSelector::DataSelector(const DataSelector & selector) : BeanLike(__FUNCTION__){ //, elangle(2) {
	init();
	copy(selector);
}


DataSelector::~DataSelector() {
}

void DataSelector::init() {

	reset();

	parameters.reference("path", path);
	parameters.reference("quantity", quantity);
	parameters.reference("index", index);
	parameters.reference("count", count);

	parameters.reference("elangle", elangle.vect);

	parameters.reference("quantity", quantity);
	// quantity.insert("");
	// reset(){...} ?

	parameters.reference("dataset", dataset.vect);
	parameters["dataset"].fillArray = true;

	parameters.reference("data", data.vect);
	parameters["data"].fillArray = true;

	// Deprecating, use "elangle=min:max" instead
	parameters.reference("elangleMin", elangle.min);
	parameters.reference("elangleMax", elangle.max);

	parameters.reference("groups", groups, "2=DATASET,4=DATA,8=QUALITY,16=WHAT,32=WHERE,64=HOW");


	/*
	 * OLD
	parameters.reference("elangle", elangle);
	parameters.reference("elangleMin", elangle[0]);
	parameters.reference("elangleMax", elangle[1]);
	 */

}

void DataSelector::reset() {

	path = "";
	quantity = "";
	index = 0;
	count = 1000;

	elangle.min = -90.0;
	elangle.max = +90.0;

	dataset.min = 0;
	dataset.max = 0xffff;
	data.min = 0;
	data.max = 0xffff;


}

/*
bool DataSelector::isValidPath(const ODIMPath & path) const {

	for (ODIMPath::const_iterator it = path.begin(); it != path.end(); ++it){
		switch (it->group) {
			case ODIMPathElem::DATASET:
				if (!dataset.isInside(it->index))
					return false;
				break;
			case ODIMPathElem::DATA:
				if (!data.isInside(it->index))
					return false;
				break;
			default:
				break;
		}
	}

	return true;

}
 */

void DataSelector::updatePaths(){

	drain::Logger mout(getName(), __FUNCTION__);

	static drain::RegExp rangeRE("^\\[?([+-]?[[:digit:]]+)(:([+-]?[[:digit:]]+))?\\]?$");

	static drain::RegExp datasetRE("/?dataset([^/]+)(/.*)?$");
	if (datasetRE.execute(path)==0){

		mout.warn() << "'path=" << path << "' (with regexps) is deprecating, use form 'dataset=...' instead" << mout.endl;
		//mout.note() << "dataset result" << drain::StringTools::join(datasetRE.result, '|') << mout.endl;

		const std::string & datasetIndex = datasetRE.result[1];
		//mout.note() << "dataset index string: " << datasetIndex << mout.endl;

		const int i = atoi(datasetIndex.c_str());
		if (i > 0){ // at least starts with a digit
			// setParameter("dataset", i);
			dataset = i; // assign will fill both
			mout.debug(3) << "first guess: dataset=" << dataset << mout.endl;
		}

		if (rangeRE.execute(datasetIndex) == 0){
			mout.debug(1) << "dataset result: " << drain::StringTools::join(rangeRE.result,'|') << mout.endl;
			if (rangeRE.result.size() >= 2){
				dataset = atoi(rangeRE.result[1].c_str());
				if (rangeRE.result.size() >= 4){
					dataset.max = atoi(rangeRE.result[3].c_str());
				}
				mout.note() << "derived: dataset=" << dataset << mout.endl;
			}
			else {
				mout.warn() << rangeRE.result.size() << " elems in regexp result - something went wrong" << mout.endl;
			}
			mout.warn() << "guessed: dataset=" << dataset << mout.endl;
		}
		else {
			//mout.note() << drain::StringTools::join(rangeRE.result,'|') << mout.endl;
			mout.warn() << "guessed: dataset=" << dataset << mout.endl;
		}

		//mout.warn() << "could not derive range from '" << datasetIndex << "'" << mout.endl;

	}

	static drain::RegExp dataRE("/?data([^set/]+)(/.*)?$");
	if (dataRE.execute(path)==0){

		mout.warn() << "'path=" << path << "' (with regexps) is deprecating, use form 'data=...' instead" << mout.endl;
		//mout.note() << "dataset result" << drain::StringTools::join(datasetRE.result, '|') << mout.endl;

		const std::string & dataIndex = dataRE.result[1];
		//mout.note() << "'path=" << dataIndex << "' (with regexps) is deprecating, consider explicit form 'data=i:j'" << mout.endl;

		const int i = atoi(dataIndex.c_str());
		if (i > 0){ // at least starts with a digit
			// setParameter("data", i);
			data = i; // assign will fill both
			mout.debug(3) << "first guess: data=" << data << mout.endl;
		}

		if (rangeRE.execute(dataIndex) == 0){
			mout.debug(1) << "data result: " << drain::StringTools::join(rangeRE.result,'|') << mout.endl;
			if (rangeRE.result.size() >= 2){
				data = atoi(rangeRE.result[1].c_str());
				if (rangeRE.result.size() >= 4){
					data.max = atoi(rangeRE.result[3].c_str());
				}
				mout.note() << "derived: data=" << data << mout.endl;
			}
			else {
				mout.warn() << rangeRE.result.size() << " elems in regexp result - something went wrong" << mout.endl;
			}
			mout.warn() << "guessed: data=" << data << mout.endl;
		}
		else {
			//mout.note() << drain::StringTools::join(rangeRE.result,'|') << mout.endl;
			mout.warn() << "guessed: data=" << data << mout.endl;
		}

	}


}

/*
bool DataSelector::isValidData(const drain::ReferenceMap & properties) const {
	if (properties.hasKey("where:elangle"))
		if (!elangle.isInside(properties["where:elangle"]))
			return false;
	//if (properties.hasKey("what:quantity")){
	return true;
}
 */

ODIMPathElem::group_t DataSelector::resetParameters(const std::string & parameters){

	drain::Logger mout(getName(), __FUNCTION__);

	ODIMPathElem::group_t groupFilter = 0;

	data    = 0; // = 0:0 ~ delete none, unless set below
	//dataset = 0; // = 0:0 ~ traverse all, delete none, unless set below
	//elangle = NAN;

	setParameters(parameters);

	/*
	if (std::isnan(elangle.max)){
		elangle.min = -90;
		elangle.min = +90;
	}

	if (dataset.max == 0){
		dataset.min = 0;
		dataset.max = 0xffff;
	}
	 */

	if (!quantity.empty()){
		if (data.max == 0){
			data.min = 0;
			data.max = 0xffff;
			setParameters(parameters); // data range may be re-adjusted
		}
	}

	if (data.max == 0){
		/// No DATA indices nor quantities specified, so only DATASET should be returned.
		groupFilter = ODIMPathElem::DATASET;
		// Allow still all descendants (for quantity check?)
		data.min = 0;
		data.max = 0xffff;
	}
	else {
		// Note: now all the DATASETs will be traversed but not returned
		// If datasets specified, deletion rules apply to their descendants.
		groupFilter = ODIMPathElem::DATA | ODIMPathElem::QUALITY;
	}
	//(ODIMPathElem::group_t groupFilter = ODIMPathElem::DATA | ODIMPathElem::QUALITY | (data.max?ODIMPathElem::DATA

	mout.debug() << "group mask: " << groupFilter << ", selector now: " << *this << mout.endl;

	/*
	std::list<ODIMPath> paths;
	HI5TREE & dst = *getResources().currentHi5;
	selector.getPathsNEW(dst, paths, groupFilter); // RE2
	 */
	return groupFilter;
}

bool DataSelector::getLastChild(const HI5TREE & tree, ODIMPathElem & child){ //, (ODIMPathElem::group_t g

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!ODIMPathElem::isIndexed(child.getType())){
		mout.warn() << ": index requested for unindexed path element '" << child << "'" << mout.endl;
		return false;
	}

	child.index = 0; // needed
	for (HI5TREE::const_iterator it = tree.begin(); it != tree.end(); ++it){

		if (it->first.getType() == child.getType()){
			child.index = std::max(child.getIndex(), it->first.getIndex());
			mout.debug(3) << "considering (" << child << ")" << mout.endl;
		}

	}

	return child.getIndex() > 0;

}

bool DataSelector::getNewChild(const HI5TREE & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!child.isIndexed()){ // ODIMPathElem::isIndexed(child.getType())){
		mout.warn() << ": index requested for unindexed path element '" << child << "'" << mout.endl;
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




bool DataSelector::getNextChild(const HI5TREE & tree, ODIMPathElem & child){

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (getLastChild(tree, child)){
		++child.index;
		return true;
	}
	else {
		//if (!ODIMPath::isIndexed(child.getType())){
		if (! child.isIndexed()){
			mout.warn() << "index requested for unindexed path element '" << child << "'" << mout.endl;
		}
		else {
			mout.debug() << "returning a new child element '" << child << "'" << mout.endl;
			child.index = 1;
		}
		return false;
	}
}



bool DataSelector::getPathNEW(const HI5TREE & src, ODIMPath & path, ODIMPathElem::group_t groupFilter) const {

	drain::Logger mout(getName(), __FUNCTION__);

	if (count > 1){
		mout.debug(3) << "count=" << count << ", but only 1 path will be used " << mout.endl;
	}

	std::list<ODIMPath> paths;
	getPathsNEW(src, paths, groupFilter);
	if (paths.empty()){
		path.clear();  // sure?
		return false;
	}
	else {
		path = paths.front(); // *paths.begin();
		if (paths.size() > 1){
			mout.debug(3) << "found " << paths.size() << " paths , ";
			mout << " using only one: '" << path << "'" << mout.endl;
		}
		return true;
	}
}

bool DataSelector::getLastPath(const HI5TREE & src, ODIMPath & path, ODIMPathElem::group_t group) const {

	drain::Logger mout(getName(), __FUNCTION__);

	if (count > 1){
		mout.debug(3) << "count=" << count << ", but only 1 path will be used " << mout.endl;
	}

	std::list<ODIMPath> paths;
	getPathsNEW(src, paths, group);
	if (paths.empty()){
		path.clear();  // sure?
		return false;
	}
	else {

		if (paths.size() > 1){
			mout.debug(3) << "found " << paths.size() << " paths , ";
			mout << " using only one: '" << path << "'" << mout.endl;
		}

		path = paths.back();
		if (path.empty()){
			mout.debug(3) << "empty path found";
			return false;
		}

		ODIMPathElem & elem = path.back();
		if (group != elem.getType()){
			mout.warn() << "multiple-group filter (" << group << "), ";
			mout << "returning (" << elem.getType() << "):  " << path << mout.endl;
		}
		return true;
	}
}

bool DataSelector::getNextPath(const HI5TREE & src, ODIMPath & path, ODIMPathElem::group_t group) const {

	drain::Logger mout(getName(), __FUNCTION__);

	if (!getLastPath(src, path, group)){
		// Empty
		path = ODIMPathElem(group, 1); // todo: check group is valid code (return false, if not)
		return true;
	}
	else {
		ODIMPathElem & elem = path.back();
		if (elem.isIndexed()){
			++elem.index;
			if (group != elem.getType()){
				mout.warn() << "requested group " << group << ", ";
				mout << "returning (" << elem.getType() << "): " << path << mout.endl;
			}
			return true;
		}
		else {
			mout.warn() << "leaf element '" << elem << "' not indexed type" << mout.endl;
			return false;
		}
	}
}

bool DataSelector::getChildren(const HI5TREE & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups){

	for (HI5TREE::const_iterator it = tree.begin(); it != tree.end(); ++it){

		const ODIMPathElem & elem = it->first;

		if (elem.belongsTo(groups)){
			children[tree[elem].data.dataSet.properties["what:quantity"]] = elem;
		}

	}
	return !children.empty();
}


// ------ OLD ------------------
/*
bool DataSelector::getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & basePath, int & index){

	drain::Logger mout("DataSelector", __FUNCTION__);
	//drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug(2) << "selector=" << selector << mout.endl;

	std::list<ODIMPath> paths;
	//getPaths(src, selector, paths);
	selector.getPathsNEW(src, paths);

	if (paths.empty()){
		//mout.warn() << "No paths with: "  << selector << mout.endl;
		return false;
	}

	//drain::RegExp r("^(.*[^0-9])([0-9]+)([^0-9]*)$");  // 2nd item is the last numeric substd::string
	// skip leading '/'
	drain::RegExp r("^/?([^/].*[^0-9])([0-9]+)([^0-9]*)$");  // 2nd item is the last numeric substd::string
	index = -1;
	drain::Variable v(0);
	//vField.setType<int>();
	//vField = 0;  //
	for (std::list<ODIMPath>::iterator it = paths.begin(); it != paths.end(); ++it){
		/// std::cerr << "???" << *it << std::endl;
		if (r.execute(*it) != REG_NOMATCH ){
			mout.debug(2) << r.result[1] << '|' << r.result[2] << mout.endl;
			v = r.result[2];
			if (static_cast<int>(v) > index){
				index = v;
				basePath  = r.result[1];
				/// std::cerr << "xxxx" << *it << std::endl;
			}
		}
	}

	mout.debug(2) << "result: " << basePath << mout.endl;

	return (index != -1);

}

bool DataSelector::getLastOrdinalPath(const HI5TREE &src, const std::string & pathRegExp, std::string & path){

	drain::Logger mout("DataSelector", __FUNCTION__);
	//drain::Logger mout(__FILE__, __FUNCTION__);
	//drain::Logger mout(drain::monitor,"DataSelector::getNextOrdinalPath::filter");

	mout.debug(2) << " selector=" << pathRegExp << mout.endl;

	int index = -1;

	DataSelector::getLastOrdinalPath(src, DataSelector(pathRegExp), path, index);

	if (index == -1)
		return false;
	else {
		std::stringstream sstr;
		sstr << path << index;
		path = sstr.str();
		mout.debug(2) << "result: " << path << mout.endl;
		return true;
	}
}


bool DataSelector::getNextOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path){

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.debug(1) << " selector=" << selector << mout.endl;

	int index = -1;

	DataSelector::getLastOrdinalPath(src, selector, path, index);

	if (index == -1)
		return false;
	else {
		std::stringstream sstr;
		sstr << path << ++index;
		path = sstr.str();
		mout.debug(1) << "result: " << path << mout.endl;
		return true;
	}
}
 */

}  // rack::
 // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP
