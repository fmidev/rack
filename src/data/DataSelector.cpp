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

#include "drain/util/Type.h"
#include "drain/util/RegExp.h"

#include "hi5/Hi5.h"
#include "DataSelector.h"

#include "ODIMPathTools.h"


namespace rack {


///const drain::FlaggerBase<Crit>::dict_t CritFlagger::dict = {{"DATA", DATA}, {"ELANGLE", ELANGLE}, {"TIME", TIME}};


template <>
const drain::FlaggerDict drain::EnumDict<DataOrder::Crit>::dict = {
		{"DATA",    rack::DataOrder::DATA},
		{"ELANGLE", rack::DataOrder::ELANGLE},
		{"TIME",    rack::DataOrder::TIME}
};

template <>
const drain::FlaggerDict drain::EnumDict<DataOrder::Oper>::dict =  {
		{"MIN", rack::DataOrder::MIN},
		{"MAX", rack::DataOrder::MAX}
};

template <>
const drain::FlaggerDict drain::EnumDict<DataSelector::Prf>::dict =  {
		{"ANY", rack::DatasetSelector::ANY},
		{"SINGLE", rack::DataSelector::SINGLE},
		{"DOUBLE", rack::DataSelector::DOUBLE}
};

/// PRESELECT
/*
 *  Consider replacing props with direct group attribs, like [WHERE].attr["elangle"] and  [WHAT].attr["time"]
 *
 */
void DataSelector::selectPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	collectPaths(src, pathContainer, ODIMPath());

	prunePaths(src, pathContainer);

}

bool DataSelector::collectPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer, const ODIMPath & basepath) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (basepath.empty()){
		if (qualityRegExp.isSet()){
			mout.attention<LOG_DEBUG>(basepath, "qualityRegExp: ", qualityRegExp);
		}
		if (quantityRegExp.isSet()){
			mout.attention<LOG_DEBUG>(basepath, "quantityRegExp: ", quantityRegExp);
		}
	}

	// mout.attention<LOG_INFO>("quantityRegExp: ", quantityRegExp,  ", qualityRegExp: ", qualityRegExp);

	bool result = false;

	for (const auto & entry: src(basepath)) {

		const ODIMPathElem & currentElem = entry.first;
		ODIMPath path(basepath, currentElem);
		//mout.debug3("currentElem='" , currentElem , "'" );

		const drain::image::Image & data    = entry.second.data.image; // for ODIM
		const drain::FlexVariableMap & props = data.getProperties();


		// Check ELANGLE (in datasets) or quantity (in data/quality)
		if (currentElem.is(ODIMPathElem::DATASET)){

			mout.debug2("DATASET = '" ,path , "'" );

			// PRF criterion applies?
			if (selectPRF != ANY){
				double lowPRF   = props.get("how:lowprf",  0.0);
				double hightPRF = props.get("how:highprf", lowPRF);
				if ((lowPRF == hightPRF) == (selectPRF == Prf::SINGLE)){
					mout.accept<LOG_DEBUG>("PRF=", lowPRF, '/', hightPRF, ", required ", selectPRF, ": including " , path);
				}
				else {
					mout.reject<LOG_DEBUG>("PRF=", lowPRF, '/', hightPRF, ", required ", selectPRF, ": excluding " , path);
					continue; // yes, subtrees skipped ok
				}
			}

			/*
			if (!pathMatcher.matchElem(currentElem, true)){
				// pathMatcher does not accept this dataset<N> at all
				mout.reject(currentElem);
				continue;
			}
			*/

			if (props.hasKey("where:elangle")){
				double e = props["where:elangle"];
				if (!elangle.contains(e)){
					mout.reject<LOG_DEBUG>("elangle ",e," outside range ",elangle);
					continue;
				}
			}

			if (collectPaths(src, pathContainer, path)){

				result = true;

				if (pathMatcher.match(path)){
					mout.accept<LOG_DEBUG>("DATASET path ", path, " matches (and subtree OK)");
					pathContainer.push_back(path);
					// addPath(pathContainer, props, path);
				}
				else {
					mout.reject<LOG_DEBUG+1>("DATASET path '", path, "' does not match '", pathMatcher, "' (but subtree OK)");
				}
			}
			else {
				mout.reject<LOG_DEBUG>("DATASET contained no matching groups: ", path );
			}

		}
		//else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::DATA)){
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){ // 2021/04

			bool quantityOK = false;

			const std::string retrievedQuantity = props["what:quantity"].toStr();

			mout.debug2("DATA = '" ,path , "' [", retrievedQuantity, "]");

			// QUANTITY criterion applies?
			if (quantityRegExp.isSet() || qualityRegExp.isSet()){


				if (qualityRegExp.isSet()){ // At least: do not test quantity after this
					//quantityOk = false;
					if (currentElem.is(ODIMPathElem::QUALITY) && qualityRegExp.test(retrievedQuantity)){
						mout.note("QUALITY quantity matches: [", retrievedQuantity, "]: ", basepath, '|', currentElem);
						quantityOK = true;
					}
				}
				else if (quantityRegExp.test(retrievedQuantity)){
					mout.accept<LOG_DEBUG>("quantity matches: [", retrievedQuantity, "]: ", basepath, '|', currentElem);
					quantityOK = true;
				}
				else {
					mout.reject<LOG_DEBUG+1>("unmatching DATA quantity  [" ,  retrievedQuantity , "], skipping" );
					// no continue! Recursion follows (for quality quantity)
				}
			}
			else {
				// No quantity constraint.
				quantityOK = true;
			}

			result |= quantityOK; // = at least one found

			if (quantityOK){
				if (pathMatcher.match(path)){
					mout.accept<LOG_DEBUG>("DATA/QUALITY path matches: ", path,  " [", retrievedQuantity, "]");
					pathContainer.push_back(path);
				}
				else {
					mout.reject<LOG_DEBUG+1>("DATA path '", path, "' does not match '", pathMatcher, "' (but quantity check OK)");
				}
			}

			result |= collectPaths(src, pathContainer, path);

		}
		else if (currentElem.is(ODIMPathElem::ARRAY) || currentElem.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){

			// Does not affect result.
			if (pathMatcher.match(path)){
				pathContainer.push_back(path);
			}
		}
		else if (currentElem.is(ODIMPathElem::LEGEND)){
			mout.debug("skipping LEGEND: /" , currentElem );
		}
		else {
			mout.warn("skipping odd group: /" , currentElem );
		}

	}

	return result;
}




/** Future C++20 option:
template <DataOrder E>
class SuperElemLess {
	inline
	bool operator()(const SuperElem & e1, const SuperElem & e2) const {
	}
}
*/


/// Using \c criterion TIME and \c order (MIN or MAX)
void DataSelector::prunePaths(const Hi5Tree & src, std::list<ODIMPath> & pathList) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (pathList.empty()){
		//mout.warn<LOG_INFO>("Path list empty, returning");
		mout.warn("Path list empty, returning");
		return;
	}

	/// ! TODO! IS this needed! See next. Vector search may be slow, but anyway.
	/// Or this could be updated on the fly.
	/*
	mout.debug("STEP 1: pick the stem groups (DATASETs) from the full path list"); //  fulfilling elangle and time criterion
	std::set<ODIMPathElem> retrieved;
	for (ODIMPath & path: pathList) {

		// Check... (Should not be needed)
		while ((!path.empty()) && path.front().empty()){
			mout.warn("Rooted path '", path, "' stripping leading slash");
			path.pop_front();
		}

		if (path.front().is(ODIMPathElem::DATASET)){
			retrieved.insert(path.front());
		}
		else {

		}

	}
	*/

	// mout.debug("..STEP 2: from all the DATASETs, pick accepted ones to a sortable structure.");
	// for (const auto & entry: src) {
		//const ODIMPathElem & elem = entry.first;

	std::vector<ODIMPathElem2> accepted; // sortable!
	accepted.reserve(src.getChildren().size());
	mout.debug("STEP 1: from the given paths, extract DATASETs to a sortable structure.");

	for (ODIMPath & path: pathList) {

		while ((!path.empty()) && path.front().empty()){
			mout.warn("Rooted path '", path, "' stripping leading slash");
			path.pop_front();
		}

		const ODIMPathElem & elem = path.front();

		if (elem.is(ODIMPathElem::DATASET)){

			//if (retrieved.find(elem) != retrieved.end()){
			// Speedup: vector<bool> found(n, false);  found[elem.getIndex()] == true;
			if (std::find(accepted.begin(), accepted.end(), elem) == accepted.end()){
				const drain::image::Image & data    = src[elem].data.image;
				const drain::FlexVariableMap & props = data.getProperties();
				accepted.push_back(ODIMPathElem2(elem, props.get("where:elangle", 0.0), props.get("what:startdate",""), props.get("what:starttime","")));
			}
			/*
			if (accepted.find(elem) != accepted.end()){
				mout.debug(elem);
				const drain::image::Image & data    = entry.second.data.image;
				const drain::image::Image & data    = src[elem].data.image;
				const drain::FlexVariableMap & props = data.getProperties();
				accepted.push_back(ODIMPathElem2(elem, props.get("where:elangle", 0.0), props.get("what:startdate",""), props.get("what:starttime","")));
			}
			*/
			else {
				mout.debug("not in list of retrieved paths, skipping: ", elem);
				//continue;
			}

		}

	}

	mout.accept<LOG_DEBUG>("DATASETs before sorting and pruning: ", drain::sprinter(accepted));

	const size_t a0 = accepted.size(); // Used for debugging below.

	if (a0 == count){
		mout.attention<LOG_DEBUG>("retrieved count (", a0, ") matches requested count, skipping sorted select (to STEP 4).");
	}
	else {

		mout.debug("STEP 2: sort using criterion ", order.criterion);
		switch (order.criterion.value) {
			case DataOrder::DATA:
				// Already in order
				// std::sort(accepted.begin(), accepted.end(), ODIMPathLess());
				// mout.debug("criterion: DATA path");
				break;
			case DataOrder::TIME:
				std::sort(accepted.begin(), accepted.end(), ODIMPathLessTime());
				// mout.debug("criterion: TIME");
				break;
			case DataOrder::ELANGLE:
				std::sort(accepted.begin(), accepted.end(), ODIMPathLessElangle());
				// mout.debug("criterion: ELANGLE");
				break;
				// case DataOrder::NONE "random" ?
			default:
				mout.error("something went wrong,  order.criterion=", order.criterion);
		}

		// mout.debug("sorted DATASETs: ", drain::sprinter(accepted));
		size_t n = count;
		n = std::min(n, a0);
		mout.debug("STEP 3: save the ", order.operation, ' ', n, " entries (delete others)");

		std::vector<ODIMPathElem2>::iterator it = accepted.begin();
		switch (order.operation.value){ // explicit: Enum value.
		case DataOrder::MIN:
			std::advance(it, n);
			accepted.erase(it, accepted.end());
			break;
		case DataOrder::MAX:
			std::advance(it, accepted.size()-n);
			accepted.erase(accepted.begin(), it);
			break;
		default:
			mout.error("something went wrong,  order.operation=", order.operation);
		}

		mout.accept<LOG_DEBUG>("Final ", accepted.size(), " (out of initial ", a0, ") DATASETs: ", drain::sprinter(accepted)); // without ATTRIBUTES
		if (accepted.size() != count){
			mout.debug("Retrieved count (", accepted.size(), ") is less than requested (", count, ") DATASETs.");
		}
	}

	mout.debug("STEP 4: From the retrieved paths select the ones with accepted DATASET elems");
	std::list<ODIMPath> finalPaths;

	for (const ODIMPath & path: pathList){

		const ODIMPathElem & stem = path.front();

		//  rack --inputSelect '/where|what|dataset2:11,count=3,order=TIME:MAX,quantity=^DBZH$,prf=DOUBLE'

		if (stem.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			if (pathMatcher.match(path)){
				mout.special<LOG_DEBUG>("adding back ATTRIBUTE_GROUP: ", stem);
				finalPaths.push_back(path);
			}
			continue;
		}

		// bool stemOk = false; // debugging
		for (const ODIMPathElem & a: accepted){
			if (stem == a){
				mout.accept<LOG_DEBUG>("Finally accepting '", path, ", due to stem=", stem);
				finalPaths.push_back(path);
				// stemOk = true;
				break;
				//continue;
			}
		}

		/* if (!stemOk){
			mout.reject<LOG_DEBUG>("Excluded '", path, ", in the last stage, stem='", stem, '"');
		}*/

	}

	pathList.swap(finalPaths);

	/*
	mout.attention("final list:");
	for (const ODIMPath & path: pathContainer){
		mout.ok(path);
	}
	*/

}


void DataSelector::getTimeMap(const Hi5Tree & srcRoot, ODIMPathElemMap & m){

	for (const auto & entry: srcRoot) {
		if (entry.first.is(ODIMPathElem::DATASET)){
			const drain::VariableMap & attr = entry.second[ODIMPathElem::WHAT].data.attributes;
			m[attr.get("startdate","") + attr.get("starttime","")] = entry.first;
		}
	}

};

void DataSelector::getQuantityMap(const Hi5Tree & srcDataset, ODIMPathElemMap & m){

	for (const auto & entry: srcDataset) {
		if (entry.first.is(ODIMPathElem::DATA)){
			const drain::VariableMap & attr = entry.second[ODIMPathElem::WHAT].data.attributes;
			m[attr.get("quantity","")] = entry.first;
		}
	}

};


DataSelector::DataSelector(
		const std::string & path,
		const std::string & quantity,
		unsigned int count,
		drain::Range<double> elangle,
		int dualPRF
		) : BeanLike(__FUNCTION__) { //, orderFlags(orderDict) {

	//std::cerr << "DataSelector: " << quantity << " => " << this->quantity << std::endl;

	init();

	this->path = path;
	this->quantity = quantity;
	this->count = count;
	this->elangle = elangle;
	//this->order.str = "";
	this->order.set(DataOrder::DATA, DataOrder::MIN);
	//this->dualPRF = dualPRF;
	this->selectPRF.set(Prf::ANY);

	updateBean();

	drain::Logger mout(__FUNCTION__, getName());
	// mout.special("xx  selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
}


DataSelector::DataSelector(const std::string & parameters) : BeanLike(__FUNCTION__) { //  , orderFlags(orderDict,':') { //, groups(ODIMPathElem::getDictionary(), ':') {

	init();
	drain::Logger mout(__FUNCTION__, getName());
	//mout.special("y1 selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
	//mout.special("y1 selector params='", parameters, "'");

	// this->parameters.setValues(parameters, '=', ',');
	//mout.special("y1 this params='", this->parameters, "'");
	//mout.special("y2 selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
	//updateBean();

	setParameters(parameters); // calls updateBean()

	//mout.special("y3 selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
}


DataSelector::DataSelector(const DataSelector & selector) : BeanLike(selector) { //, orderFlags(orderDict,':'){ //, groups(ODIMPathElem::getDictionary(), ':') {
	init();
	setParameters(selector.getParameters()); // calls updateBean()!
	//this->parameters.copyStruct(selector.getParameters(), selector, *this);
}


DataSelector::~DataSelector() {
}


///
void DataSelector::init() {

	reset();
	pathMatcher.separator.acceptTrailing = true;
	parameters.link("path", path, "[/]dataset<i>[/data<j>|/quality<j>]");
	parameters.link("quantity", quantity, "DBZH|VRAD|RHOHV|...");
	parameters.link("elangle", elangle.tuple(), "min[:max]").fillArray = false;
	parameters.link("count", count);
	//parameters.link("order", order.str, drain::sprinter(orderDict).str()); // TODO:  sprinter(orderDict)
	parameters.link("order", order.str,
				drain::sprinter(drain::EnumDict<DataOrder::Crit>::dict.getKeys()).str() + ':' +
				drain::sprinter(drain::EnumDict<DataOrder::Oper>::dict.getKeys()).str()
				);
	//parameters.link("order", order.str, drain::sprinter(order.getParameters().getKeyList()).str());
	parameters.link("prf", prf, drain::sprinter(drain::EnumDict<DataSelector::Prf>::dict.getKeys()).str());
	// <-- TODO: develop to: enum PRF {"Single",1}, {"Dual",2}

}


void DataSelector::reset() {

	path = "";
	pathMatcher.clear();

	quantity = "";

	//index = 0;
	count = 1000;

	drain::Range<double> e =  {-90.0,+90.0};
	elangle = e; // {-90.0,+90.0}; // unflexible

	//dualPRF = 0;
	selectPRF.set(Prf::ANY);
	prf = selectPRF.str();

	// order.str = "";
	order.set(DataOrder::DATA, DataOrder::MIN);
	// order.criterion.set(DataOrder::DATA);
	// order.operation.set(DataOrder::MIN);
	// orderFlags.value = 0; // needs this... :-(

}

void DataSelector::setQuantity(const std::string & quantity){
	this->quantity = quantity;
	updateQuantity();
}

void DataSelector::updateQuantity() const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	quantityRegExp.clear();
	qualityRegExp.clear();

	std::vector<std::string> s;

	drain::StringTools::split(quantity, s, ":");  // experimental
	if (s.size() == 2){
		// Compare with --qualityQuantity
		mout.deprecating("in future, slash '/' may replace colon ':' in args like " , quantity );
	}
	else {
		drain::StringTools::split(quantity, s, "/");  // experimental
	}

	switch (s.size()) {
		case 2:
			qualityRegExp.setExpression(s[1]);
			// no break
		case 1:
			quantityRegExp.setExpression(s[0]);
			break;
		default:
			mout.warn("could not parse quantity='" , quantity , "', should be <quantity> or [<quantity>]:<quality>" );
			break;
	}

	// dont update bean. Bean updates this.
}

bool DataSelector::testQuantity(const std::string & s) const {
	drain::Logger mout(__FILE__, __FUNCTION__);
	mout.unimplemented("code");
	return false;
}


void DataSelector::updateBean() const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!path.empty()){

		mout.debug("Assigning (string) path='", path, "'");

		pathMatcher.set(path);
		mout.debug("Assigned pathMatcher: ", path, " => ", pathMatcher);

		if (!pathMatcher.empty()){
			if (pathMatcher.back().empty()){ // "backroot" = > appears as trailing slash '/'
				pathMatcher.pop_back();
				mout.note("stripped trailing slash '/' => ", pathMatcher);
			}
		}
		else {
			mout.warn("path matcher still empty after assigning path='", path, "'");
		}

		//path.clear();
	}

	//path = pathMatcher; // Note: this (over)simplifies data|quality: to data|quality (discards explicit index)

	updateQuantity();
	// mout.special("quantity [" , quantity ,"], " , "pathMatcher: " , path , " => " , pathMatcher   );

	if (pathMatcher.empty() && !quantity.empty()){

		if (! pathMatcher.front().is(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			pathMatcher.set(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			mout.info("quantity [" , quantity ,"] requested, completing path condition: " , pathMatcher );
		}
		//path = pathMatcher;
	}

	order.set(order.str);
	selectPRF.set(prf);

}

void DataSelector::ensureDataGroup(){

	drain::Logger mout(__FUNCTION__, getName());

	if (pathMatcher.empty()){
		pathMatcher.set(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
		mout.debug("Completed pathMatcher: " , pathMatcher );
	}
	else {
		if (pathMatcher.back().empty()){ // "backroot"
			pathMatcher.pop_back();
			mout.note("stripped trailing slash (rootlike empty elem): "  , pathMatcher );
		}

		if (! pathMatcher.back().belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (! pathMatcher.back().is(ODIMPathElem::DATASET)){
				mout.warn("Resetting pathMatcher with suspicious tail: " , pathMatcher );
				pathMatcher.clear();
			}
			pathMatcher.push_back(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			mout.note("Completed pathMatcher: " , pathMatcher );
		}
	}

	if (quantity.empty()){
		mout.info("quantity [" , quantity ,"] unset: " , *this );
	}

}



// Resets, set given parameters and derives missing parameters.
void DataSelector::deriveParameters(const std::string & parameters, bool clear){ //, ODIMPathElem::group_t defaultGroups){

	// drain::Logger mout(__FILE__, __FUNCTION__);

	// Consider:
	if (clear){
		reset();
		// groups.value = 0;
		// data = 0; // = 0:0 ~ delete none, unless set below
	}

	BeanLike::setParameters(parameters);

	return; // groups.value;
}


void DataSelector::getPaths(const Hi5Tree & src, std::list<ODIMPath> & pathList) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.experimental<LOG_DEBUG>("revised code: getPaths->selectPaths(): ", __FILE__, ':', __LINE__);
	selectPaths(src, pathList);

	/*
	if (order.criterion == DataOrder::DATA){
		// = default
		getMainPaths(src, pathList);
	}
	else {

		mout.debug2("Special select requested: ", *this);

		if (order.criterion == DataOrder::TIME){
			mout.debug(__FUNCTION__, ':', order.str);
			std::map<std::string,ODIMPath> m;
			getPathsByTime(src, m);
			copyPaths(m, order.operation, pathList);
		}
		else if (order.criterion == DataOrder::ELANGLE){
			mout.debug(__FUNCTION__, ':', order.str);
			std::map<double,ODIMPath> m;
			getPathsByElangle(src, m);
			copyPaths(m, order.operation, pathList);
		}
		else {
			mout.error(std::string(__FILE__), " unimplemented ENUM value for order.criterion: ", order.criterion.str());
		}
	}
	*/
}


/*
void DataSelector::getPathsByElangleFOO(const Hi5Tree & src, std::map<double,ODIMPath> & paths) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (order.criterion == DataOrder::TIME){
		mout.warn("map keys sorted by ELANGLE (double), yet DataOrder::TIME requested");
	}

	getMainPathsFOO(src, paths, false);
}

void DataSelector::getPathsByTimeFOO(const Hi5Tree & src, std::map<std::string,ODIMPath> & paths) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (order.criterion == DataOrder::ELANGLE){
		mout.warn("map keys sorted by TIME (string), yet DataOrder::ELANGLE requested");
	}

	getMainPathsFOO(src, paths, false);
	//pruneMap(paths, order.operation);
	//mout.attention("remaining: ", drain::sprinter(paths));
}
*/


/*
bool DataSelector::getLastChild(const Hi5Tree & tree, ODIMPathElem & child){ //, (ODIMPathElem::group_t g

	drain::Logger mout(__FILE__, __FUNCTION__);

	if (!ODIMPathElem::isIndexed(child.getType())){
		mout.warn(": index requested for unindexed path element '" , child , "'" );
		return false;
	}

	child.index = 0; // needed
	for (Hi5Tree::const_iterator it = tree.begin(); it != tree.end(); ++it){

		if (it->first.getType() == child.getType()){
			child.index = std::max(child.getIndex(), it->first.getIndex());
			mout .debug3() << "considering (" << child << ")" << mout.endl;
		}

	}

	return child.getIndex() > 0;

}
*/

/*
bool DataSelector::getNewChild(const Hi5Tree & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax){

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
*/


/*
bool DataSelector::getNextChild(const Hi5Tree & tree, ODIMPathElem & child){

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
			mout.debug("returning a new child element '", child, "'");
			child.index = 1;
		}
		return false;
	}
}
*/

bool DataSelector::getPath(const Hi5Tree & src, ODIMPath & path) const {

	std::list<ODIMPath> paths;
	//getPaths(src, pathContainer);
	selectPaths(src, paths);
	if (paths.empty()){
		return false;
	}
	else {
		/*
		for (ODIMPath & p: paths){
			std::cerr << __FUNCTION__ << ':' << p << '\n';
		}
		*/

		path = paths.front();
		return true;
	}
}

bool DataSelector::getLastPath(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t group) const {

	drain::Logger mout(__FUNCTION__, getName());

	// TODO: obsolete – warn? /redesign

	if (count > 1){
		mout.debug3("count=", count, ", but only 1 path will be used ");
	}

	if (true){
		mout.warn("check group ", group, " vs back() of path: ", path);
	}

	ODIMPathList paths;
	//getPaths(src, paths, group);
	getPaths(src, paths);
	if (paths.empty()){
		path.clear();  // sure?
		return false;
	}
	else {

		if (paths.size() > 1){
			mout.debug3("found ", paths.size(), " paths, using only one: '", path, "'");
		}

		path = paths.back();
		if (path.empty()){
			mout .debug3() << "empty path found";
			return false;
		}

		ODIMPathElem & elem = path.back();
		if (group != elem.getType()){
			mout.warn("multiple-group filter (", group, "), returning (", elem.getType(), "):  ", path);
		}
		return true;
	}
}

bool DataSelector::getNextPath(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t group) const {

	drain::Logger mout(__FUNCTION__, getName());

	if (!getLastPath(src, path, group)){
		// Empty
		path.set(ODIMPathElem(group, 1)); // todo: check group is valid code (return false, if not)
		return true;
	}
	else {
		ODIMPathElem & elem = path.back();
		if (elem.isIndexed()){
			++elem.index;
			if (group != elem.getType()){
				mout.warn("requested group ", group, ", returning (type=", elem.getType(), "): ", path);
			}
			return true;
		}
		else {
			mout.warn("leaf element '", elem, "' not indexed type");
			return false;
		}
	}
}

// Todo: rename... getChildren by quantity? Also, WHAT + "quantity" needed?
/*
bool DataSelector::getChildren(const Hi5Tree & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups){

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
*/

void DataSelector::swapData(Hi5Tree & src,const ODIMPathElem &srcElem, Hi5Tree & dst){

	//RackResources & resources = getResources();
	// RackContext & ctx = getResources().getContext<RackContext>();

	drain::Logger mout(__FILE__, __FUNCTION__);
	// mout.warn("Swapping!");
	mout.debug("Swapping: src: '", srcElem, "'...");
	swapData(src[srcElem], dst, srcElem.getType());

	/*
	//mout.debug("Swapping subtree of type: ", srcElem.getType());
	ODIMPathElem dstElem(srcElem.getType());
	DataSelector::getNextChild(dst, dstElem);
	mout.attention("Swapping src:", srcElem, ") <=> dst:", dstElem);
	// Create empty dstRoot[path] and swap it...
	dst[dstElem].swap(src[srcElem]);
	*/
}

void DataSelector::swapData(Hi5Tree & srcGroup, Hi5Tree & dst, ODIMPathElem::group_t groupType){

	drain::Logger mout(__FILE__, __FUNCTION__);

	ODIMPathElem dstElem(groupType, 1);
	//DataSelector::getNextChild(dst, dstElem);
	ODIMPathTools::getNextChild(dst, dstElem);
	mout.debug("Swapping: ... dst:'", dstElem, "' group type: ", groupType, " note: odim?"); // see quality comb..
	// Create empty dst[dstElem] and swap it...
	dst[dstElem].swap(srcGroup);
	//DataTools::updateInternalAttributes(ctx.polarInputHi5);
}



}  // rack::
