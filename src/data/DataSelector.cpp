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

#include "drain/util/Type.h"
#include "drain/util/RegExp.h"

#include "hi5/Hi5.h"
#include "DataSelector.h"


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

// using namespace hi5;


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



void DataSelector::updateBean() const {

	drain::Logger mout(__FUNCTION__, getName());

	if (!path.empty()){

		mout.debug("Assigning (string) path='", path, "'");

		pathMatcher.set(path);
		mout.debug("Assigned pathMatcher: ", path, " => ", pathMatcher);

		if (!pathMatcher.empty()){
			if (pathMatcher.back().empty()){ // "backroot" = > appears as trailing slash '/'
				pathMatcher.pop_back();
				mout.note() << "stripped trailing slash '/' => " << pathMatcher << mout.endl;
			}
		}
		else {
			mout.warn("path matcher still empty after assigning path='", path, "'");
		}

		//path.clear();
	}
	//mout.warn() << "pathMatcher >> '" << pathMatcher << "'" << mout.endl;

	//path = pathMatcher; // Note: this (over)simplifies data|quality: to data|quality (discards explicit index)
	//mout.warn() << "-----> path >> '" << path << "'" << mout.endl;

	quantityRegExp.clear();
	qualityRegExp.clear();

	std::vector<std::string> s;

	drain::StringTools::split(quantity, s, ":");  // experimental
	if (s.size() == 2){
		// Compare with --qualityQuantity
		mout.deprecating() << "in future, slash '/' may replace colon ':' in args like " << quantity << mout.endl;
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
			mout.warn() << "could not parse quantity='" << quantity << "', should be <quantity> or [<quantity>]:<quality>" << mout.endl;
			break;
	}

	// mout.special() << "quantity [" << quantity <<"], " << "pathMatcher: " << path << " => " << pathMatcher   << mout.endl;


	if (pathMatcher.empty() && !quantity.empty()){

		if (! pathMatcher.front().is(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			pathMatcher.set(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			mout.info() << "quantity [" << quantity <<"] requested, completing path condition: " << pathMatcher << mout.endl;
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
		mout.debug() << "Completed pathMatcher: " << pathMatcher << mout.endl;
	}
	else {
		if (pathMatcher.back().empty()){ // "backroot"
			pathMatcher.pop_back();
			mout.note() << "stripped trailing slash (rootlike empty elem): "  << pathMatcher << mout.endl;
		}

		if (! pathMatcher.back().belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (! pathMatcher.back().is(ODIMPathElem::DATASET)){
				mout.warn() << "Resetting pathMatcher with suspicious tail: " << pathMatcher << mout.endl;
				pathMatcher.clear();
			}
			pathMatcher.push_back(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			mout.note() << "Completed pathMatcher: " << pathMatcher << mout.endl;
		}
	}

	if (quantity.empty()){
		mout.info() << "quantity [" << quantity <<"] unset: " << *this << mout.endl;
	}

}



// Resets, set given parameters and derives missing parameters.
void DataSelector::deriveParameters(const std::string & parameters, bool clear){ //, ODIMPathElem::group_t defaultGroups){

	// drain::Logger mout(__FUNCTION__, __FILE__);

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

	drain::Logger mout(__FUNCTION__, __FILE__);

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
}



void DataSelector::getPathsByElangle(const Hi5Tree & src, std::map<double,ODIMPath> & paths) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (order.criterion == DataOrder::TIME){
		mout.warn("map keys sorted by ELANGLE (double), yet DataOrder::TIME requested");
	}

	getMainPaths(src, paths, false);
	// pruneMap(paths, order.operation);
	//	mout.attention("remaining: ", drain::sprinter(paths));
}

void DataSelector::getPathsByTime(const Hi5Tree & src, std::map<std::string,ODIMPath> & paths) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (order.criterion == DataOrder::ELANGLE){
		mout.warn("map keys sorted by TIME (string), yet DataOrder::ELANGLE requested");
	}

	getMainPaths(src, paths, false);
	//pruneMap(paths, order.operation);
	//mout.attention("remaining: ", drain::sprinter(paths));
}

/*
void DataSelector::pruneMap(std::list<ODIMPath> & pathContainer, DataOrder::Oper oper) const { // default DataOrder::Oper::MIN

	// Number of paths kept.
	unsigned int n = count <= pathContainer.size() ? count : pathContainer.size();

	drain::Logger mout(__FUNCTION__, __FILE__);

	auto it = pathContainer.begin();
	if (oper == DataOrder::Oper::MIN){
		// Jump over n keys.
		std::advance(it, n);
		for (auto it2 = it; it2 != pathContainer.end(); ++it2){
			mout.attention("deleting ", *it2);
		}
		pathContainer.erase(it, pathContainer.end());
	}
	else if (oper == DataOrder::Oper::MAX){
		// Jump towards end, so that n keys remain
		std::advance(it, pathContainer.size()-n);
		for (auto it2 = pathContainer.begin(); it2 != it; ++it2){
			mout.attention("deleting ", *it2);
		}
		pathContainer.erase(pathContainer.begin(), it);
	}
	else {
		throw std::runtime_error("Order oper neither MIN nor MAX: ");
	}

}
*/

bool DataSelector::getLastChild(const Hi5Tree & tree, ODIMPathElem & child){ //, (ODIMPathElem::group_t g

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (!ODIMPathElem::isIndexed(child.getType())){
		mout.warn() << ": index requested for unindexed path element '" << child << "'" << mout.endl;
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

bool DataSelector::getNewChild(const Hi5Tree & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax){

	drain::Logger mout(__FUNCTION__, __FILE__);

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




bool DataSelector::getNextChild(const Hi5Tree & tree, ODIMPathElem & child){

	drain::Logger mout(__FUNCTION__, __FILE__);

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

bool DataSelector::getPath(const Hi5Tree & src, ODIMPath & path) const {
	std::list<ODIMPath> pathContainer;
	getPaths(src, pathContainer);
	if (pathContainer.empty()){
		return false;
	}
	else {
		/*
		for (ODIMPath & p: pathContainer){
			std::cerr << __FUNCTION__ << ':' << p << '\n';
		}
		*/
		path = pathContainer.front();
		return true;
	}
}

bool DataSelector::getLastPath(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t group) const {

	drain::Logger mout(__FUNCTION__, getName());

	if (count > 1){
		mout .debug3() << "count=" << count << ", but only 1 path will be used " << mout.endl;
	}

	if (true){
		mout.warn() << "check group " << group << " vs back() of path: " << path << mout.endl;
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
			mout .debug3() << "found " << paths.size() << " paths , ";
			mout << " using only one: '" << path << "'" << mout.endl;
		}

		path = paths.back();
		if (path.empty()){
			mout .debug3() << "empty path found";
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

// Todo: rename... getChildren by quantity? Also, WHAT + "quantity" needed?
bool DataSelector::getChildren(const Hi5Tree & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups){

	//for (Hi5Tree::const_iterator it = tree.begin(); it != tree.end(); ++it){
	for (const auto & entry: tree){

		// const ODIMPathElem & elem = it->first;
		if (entry.first.belongsTo(groups)){
			//children[tree[entry.first].data.dataSet.properties["what:quantity"]] = entry.first;
			children[entry.second.data.dataSet.properties["what:quantity"]] = entry.first;
		}

	}
	return !children.empty();
}

void DataSelector::swapData(Hi5Tree & src,const ODIMPathElem &srcElem, Hi5Tree & dst){

	//RackResources & resources = getResources();
	// RackContext & ctx = getResources().getContext<RackContext>();

	drain::Logger mout(__FUNCTION__, __FILE__);
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

	drain::Logger mout(__FUNCTION__, __FILE__);

	ODIMPathElem dstElem(groupType, 1);
	DataSelector::getNextChild(dst, dstElem);
	mout.debug("Swapping: ... dst:'", dstElem, "' group type: ", groupType, " note: odim?"); // see quality comb..
	// Create empty dstRoot[path] and swap it...
	dst[dstElem].swap(srcGroup);
	//DataTools::updateInternalAttributes(ctx.polarInputHi5);
}



}  // rack::
