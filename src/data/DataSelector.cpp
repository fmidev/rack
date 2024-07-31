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

#include <drain/Type.h>
#include <algorithm>
#include <syslog.h>  // levels: LOG_ERROR etc.

#include <drain/RegExp.h>

#include "hi5/Hi5.h"
#include "DataSelector.h"

#include "ODIMPathTools.h"


namespace rack {


///const drain::FlaggerBase<Crit>::dict_t CritFlagger::dict = {{"DATA", DATA}, {"ELANGLE", ELANGLE}, {"TIME", TIME}};

template <>
const drain::FlagResolver::dict_t drain::EnumDict<DataOrder::Crit>::dict = {
		{"DATA",    rack::DataOrder::DATA},
		{"ELANGLE", rack::DataOrder::ELANGLE},
		{"TIME",    rack::DataOrder::TIME}
};

template <>
const drain::FlagResolver::dict_t drain::EnumDict<DataOrder::Oper>::dict =  {
		{"MIN", rack::DataOrder::MIN},
		{"MAX", rack::DataOrder::MAX}
};

template <>
const drain::FlagResolver::dict_t drain::EnumDict<DataSelector::Prf>::dict =  {
		{"ANY", rack::DatasetSelector::ANY},
		{"SINGLE", rack::DataSelector::SINGLE},
		{"DOUBLE", rack::DataSelector::DOUBLE}
};


DataSelector::DataSelector(
		const std::string & path,
		const std::string & quantities,
		unsigned int count,
		drain::Range<double> elangle,
		int dualPRF,
		drain::Range<int> timespan
		) : BeanLike(__FUNCTION__) { //, orderFlags(orderDict) {

	//std::cerr << "DataSelector: " << quantity << " => " << this->quantity << std::endl;

	init();

	this->path = path;
	this->quantities = quantities;
	this->count = count;
	this->elangle = elangle;
	//this->order.str = "";
	this->order.set(DataOrder::DATA, DataOrder::MIN);
	//this->dualPRF = dualPRF;
	this->prfSelector.set(Prf::ANY);


	updateBean();

	drain::Logger mout(__FUNCTION__, getName());
	// mout.special("xx  selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
}


DataSelector::DataSelector(const std::string & parameters) : BeanLike(__FUNCTION__) { //  , orderFlags(orderDict,':') { //, groups(ODIMPathElem::getDictionary(), ':') {

	init();
	drain::Logger mout(__FUNCTION__, getName());
	// mout.special("y1 selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
	// mout.special("y1 selector params='", parameters, "'");
	// this->parameters.setValues(parameters, '=', ',');
	// mout.special("y1 this params='", this->parameters, "'");
	// mout.special("y2 selector orderFlags=", orderFlags, ' ', orderFlags.value, '=', orderFlags.ownValue);
	// updateBean();
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


std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	selector.toStream(ostr);
	// ostr << '{' << selector.getQuantitySelector() << " / " << selector.getQualitySelector() << '}';
	return ostr;
}


///
void DataSelector::init() {

	reset();
	pathMatcher.separator.acceptTrailing = true;
	parameters.link("path", path, "[/]dataset<i>[/data<j>|/quality<j>]");
	parameters.link("quantity", quantities, "DBZH|VRAD|RHOHV|...");
	parameters.link("elangle", elangle.tuple(), "min[:max]").fillArray = false;
	parameters.link("count", count);
	//parameters.link("order", order.str, drain::sprinter(orderDict).str()); // TODO:  sprinter(orderDict)
	parameters.link("order", order.str,
				drain::sprinter(drain::EnumDict<DataOrder::Crit>::dict.getKeys()).str() + ':' +
				drain::sprinter(drain::EnumDict<DataOrder::Oper>::dict.getKeys()).str()
				);
	//parameters.link("order", order.str, drain::sprinter(order.getParameters().getKeyList()).str());
	parameters.link("prf", prf, drain::sprinter(drain::EnumDict<DataSelector::Prf>::dict.getKeys()).str());
	parameters.link("timespan", timespan.tuple(), "range from nominal time [seconds]").fillArray = false;
	// <-- TODO: develop to: enum PRF {"Single",1}, {"Dual",2}

}


void DataSelector::reset() {

	path = "";
	pathMatcher.clear();

	quantities = "";
	quantitySelector.clear();
	qualitySelector.clear();

	//index = 0;
	//count = 1000;
	count = 0xfff;

	static const drain::Range<double> e =  {-90.0,+90.0};
	elangle = e; // {-90.0,+90.0}; // unflexible

	//dualPRF = 0;
	prfSelector.set(Prf::ANY);
	prf = prfSelector.str();

	order.set(DataOrder::DATA, DataOrder::MIN);

	timespan.tuple() = {0,0};

}


/// Updates member objects with their corresponding variable values .
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

	// path = pathMatcher; // Note: this (over)simplifies data|quality: to data|quality (discards explicit index)
	// quantities
	// setQuantities(quantities, ":");
	updateQuantities(); // ":"

	// mout.special("quantities [" , quantities ,"], " , "pathMatcher: " , path , " => " , pathMatcher   );

	if (pathMatcher.empty() && !quantities.empty()){

		if (! pathMatcher.front().is(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			pathMatcher.set(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			mout.info("quantity(es) [" ,quantities ,"] requested, completing path condition: " , pathMatcher );
		}
		//path = pathMatcher;
	}

	order.set(order.str);
	prfSelector.set(prf);

}

void DataSelector::ensureDataGroup(){

	drain::Logger mout(__FUNCTION__, getName());

	const bool QUANTITY_QUALITY = getQuantitySelector().isSet() && getQualitySelector().isSet();

	if (pathMatcher.empty()){
		if (QUANTITY_QUALITY){
			pathMatcher.set(ODIMPathElem::DATA, ODIMPathElem::QUALITY);
		}
		else {
			pathMatcher.set(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
		}
		mout.debug("Completed pathMatcher: " , pathMatcher );
	}
	else {
		pathMatcher.trimTail();
		/*
		if (pathMatcher.back().empty()){ // "backroot"
			pathMatcher.pop_back();
			mout.note("stripped trailing slash (rootlike empty elem): "  , pathMatcher );
		}
		*/

		if (! pathMatcher.back().belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (! pathMatcher.back().is(ODIMPathElem::DATASET)){
				mout.warn("Resetting pathMatcher with suspicious tail: " , pathMatcher );
				pathMatcher.clear();
			}
			if (QUANTITY_QUALITY){
				//pathMatcher.set(ODIMPathElem::DATA, ODIMPathElem::QUALITY);
				pathMatcher.push_back(ODIMPathElem::DATA);
				pathMatcher.push_back(ODIMPathElem::QUALITY);
			}
			else {
				//pathMatcher.set(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
				pathMatcher.push_back(ODIMPathElem::DATA | ODIMPathElem::QUALITY);
			}

			mout.note("Modified pathMatcher: " , pathMatcher );
		}
	}

	path = pathMatcher.str();
	//updateBean();

	if (quantities.empty()){
		mout.info("quantity [" , quantities ,"] unset: " , *this );
	}

}

void DataSelector::setQuantities(const std::string & quantities){ // , const std::string & separators
	this->quantities = quantities;
	//updateQuantities(separators);
	updateQuantities();
}


// Cf with
void DataSelector::updateQuantities() const { // const std::string & separators

	drain::Logger mout(__FILE__,__FUNCTION__);
	// Don't call updateBean(), it calls this...

	// Check if contains separator ':' and regexp(s):
	/*
	size_t pos = quantities.find(':');
	if (pos != std::string::npos){

		if (separators.find(':') != std::string::npos){
			if (quantities.find_first_of("^?*[]()$") != std::string::npos){ //
				mout.warn("RegExp (", quantities.substr(pos, pos+5), "...) a string to be split by separators '", separators, "'");
				mout.advice<LOG_WARNING>("consider --select ... without quantities followed by --selectQuantity q1,q2,q3,...");
				mout.hint<LOG_WARNING>("use ':' to separate quantities and '/' to prefix QUALITY quantities");
				// mout.error("unsupported or ambiguous quality selection: ", quantities);
			}
		}

		if (quantities.substr(pos) == ":QIND"){
			mout.attention<LOG_WARNING>("selection syntax has changed");
			mout.advice<LOG_WARNING>("use ':' to separate quantities and '/' to prefix QUALITY quantities");
			std::string s(quantities);
			s.at(pos) = '/';
			mout.advice<LOG_WARNING>("use ", s, " instead of ", quantities);
		}

	}
	*/


	// Phase 1: split qty vs QUALITY
	//std::vector<std::string> args;
	std::string args, qargs;
	drain::StringTools::split2(quantities, args, qargs, '/'); // ':' general separator; ',' only through --setQuantity because --select uses ',' as separator.
	quantitySelector.setQuantities(args); //, separators);
	//mout.special<LOG_WARNING>("quantities", args, " -> ", quantitySelector);
	qualitySelector.setQuantities(qargs); //, separators);
	// 	mout.special<LOG_WARNING>("qualities ", qargs, " -> ", qualitySelector);

	//this->quantities = quantities;
	//updateQuantities();


	std::stringstream sstr;
	sstr << quantitySelector;
	if (qualitySelector.isSet()){
		sstr << '/' << qualitySelector;
	}
	quantities = sstr.str();

}



void DataSelector::setQuantityRegExp(const std::string & quantities){

	drain::Logger mout(__FILE__,__FUNCTION__);
	//setQuantityRegExp(quantities);
	mout.deprecating<LOG_DEBUG+1>("selection syntax has changed, use setQuantities() ");
	quantitySelector.setQuantity(quantities);
}


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

	if (basepath.empty()){ // = debug start
		mout.attention<LOG_DEBUG>("collectPaths quantities: '", getQuantity(), "'");
		// mout.attention<LOG_DEBUG>("start: quantity:   ", quantitySelector);
		// mout.attention<LOG_DEBUG>("start: quality:    ", qualitySelector);
		if (quantitySelector.isSet()){
			// mout.attention<LOG_DEBUG>("start: quantity: ", quantitySelector);
		}
		if (qualitySelector.isSet()){
			// mout.attention<LOG_DEBUG>("start: quality: ", qualitySelector);
		}
		mout.debug("starting with: ", quantitySelector); //, " re=", quantitySelector.getRegExp());

	}

	// mout.attention<LOG_INFO>("quantityRegExp: ", quantityRegExp,  ", qualityRegExp: ", qualityRegExp);

	bool result = false;

	/*  OLD g++ compiler problem: src(basepath)
	if (!basepath.empty()){
		mout.attention<LOG_INFO>("starting: ", basepath);
		mout.attention<LOG_INFO>("    tree: ", src(basepath));
		drain::TreeUtils::dump(src(basepath));
	}
	*/

	for (const auto & entry: src(basepath)) {

		const ODIMPathElem & currentElem = entry.first;

		//ODIMPath path(basepath, currentElem);
		ODIMPath path(basepath); //, currentElem);
		path.appendElem(currentElem);
		//mout.debug3("currentElem='" , currentElem , "'" );

		mout.pending<LOG_DEBUG+1>("Considering: ", basepath, "//", currentElem);

		const drain::image::Image & data    = entry.second.data.image; // for ODIM
		const drain::FlexVariableMap & props = data.getProperties();

		if (props.empty()){
			mout.attention("props empty at ", basepath, " -> /",  entry.first);
		}

		// Check ELANGLE (in datasets) or quantity (in data/quality)
		if (currentElem.is(ODIMPathElem::DATASET)){

			mout.debug2("DATASET = '" ,path , "'" );
			//mout.debug2("DATASET = '" , props);

			// PRF criterion applies?
			if (prfSelector != ANY){
				double lowPRF   = props.get("how:lowprf",  0.0);
				double highPRF = props.get("how:highprf", lowPRF);
				//if (((lowPRF == highPRF) && (highPRF == 0)) == (prfSelector == Prf::SINGLE)){
				if (((lowPRF == highPRF)) == (prfSelector == Prf::SINGLE)){
					mout.accept<LOG_DEBUG>("PRF=", lowPRF, '/', highPRF, ", required ", prfSelector, ": including " , path);
				}
				else {
					mout.reject<LOG_DEBUG>("PRF=", lowPRF, '/', highPRF, ", required ", prfSelector, ": excluding " , path);
					continue; // yes, subtrees skipped ok
				}
			}
			else {
				mout.pending<LOG_DEBUG>("No PRF constrain (prf=", prfSelector, ")");
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
			else {
				// mout.suspicious<LOG_DEBUG+1>(props);
				mout.fail<LOG_DEBUG>("props contain no where:elangle");
			}

			if (timespan.max > 0){
				if (props.hasKey("what:starttime")){
					drain::Time nominal;
					ODIM::getTime(nominal, props["what:date"], props["what:time"]);

					drain::Time measured;
					ODIM::getTime(measured, props["what:startdate"], props["what:starttime"]);

					int delaySeconds = measured.getTime() - nominal.getTime();

					if (timespan.span() > 0){ // from -300...300 for example
						if (!timespan.contains(delaySeconds)){
							mout.reject<LOG_NOTICE>("delay (", delaySeconds,") too outside timespan ", timespan);
							continue;
						}
					}
					else {
						if (delaySeconds > timespan.max){
							mout.reject<LOG_NOTICE>("delay (", delaySeconds,") larger than timespan.max=", timespan.max, " seconds");
							continue;
						}
					}

				}
				else {
					// DATASET has no time
				}
			}


			//bool checkQualityGroup = !quantitySelector.isSet();
			// mout.pending<LOG_DEBUG>("continuing down from '", basepath, "' + '/", currentElem, "' = '",  path, "'...");

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

			// What about matching this?

		}
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){

			const std::string retrievedQuantity = props["what:quantity"].toStr(); // note: creates entry?
			bool quantityOK = false;
			bool checkSubGroup = false; // Accept subtree traversal

			mout.special<LOG_DEBUG+1>("checking: DATA/QUANTITY [", retrievedQuantity, "] <- ", path);

			if (currentElem.is(ODIMPathElem::DATA)){ // 2021/04
				if (quantitySelector.isSet()){ // 2021/04
					if (quantitySelector.testQuantity(retrievedQuantity)){
						quantityOK = !qualitySelector.isSet();  // think of DBZH/QIND -> plain DBZH should not be accepted
						checkSubGroup   = true;
						mout.attention<LOG_DEBUG>("checked: DATA[", retrievedQuantity, "] at ", path, " quantity=", quantityOK, ", checkSubGroup!");
					}
					else {
						checkSubGroup = !qualitySelector.isSet(); // consider -Q QIND or --select quality=QIND
						mout.special<LOG_DEBUG+1>("DATA: unmatching quantity [", retrievedQuantity, "] at ", path, ", checkSubGroup='", checkSubGroup, "'");
						// mout.reject<LOG_NOTICE>("<",quantitySelector, ">");
						// mout.reject<LOG_NOTICE>("DATA [", retrievedQuantity, "] at ", path);
					}
				}
				else { // problem: -Q QIND -> QIND "is" quantity
					checkSubGroup   = true;  // implies accepting sub-qualities (consider: don't accept quality, unless explicity requested)
					if (!qualitySelector.isSet()){ // any quantity goes
						mout.accept<LOG_DEBUG>("unconstrained DATA[", retrievedQuantity, "] at ", path);
						quantityOK = true;
					}
					else {
						mout.special<LOG_DEBUG>("DATA:  [", retrievedQuantity, "] but Q selector ", qualitySelector);
					}
				}
				//mout.attention<LOG_NOTICE>("DATA :  [", retrievedQuantity, "] at ", path);
			}
			else if (currentElem.is(ODIMPathElem::QUALITY)){ // 2021/04

				if (qualitySelector.isSet()){ // 2021/04
					if (qualitySelector.testQuantity(retrievedQuantity)){
						quantityOK = true;
						checkSubGroup   = true; //needed?
						mout.accept<LOG_DEBUG>("matching QUALITY quantity  [",  retrievedQuantity, "]" );
					}
					else {
						mout.reject<LOG_DEBUG>("unmatching QUALITY quantity  [",  retrievedQuantity, "], skipping" );
					}
				}
				else if (quantitySelector.isSet()){ // yes, defined as "plain" quality, QIND (not DBZH/QIND)
					if (quantitySelector.testQuantity(retrievedQuantity)){
						quantityOK = true;
						checkSubGroup    = true; //needed?
						mout.accept<LOG_DEBUG>("matching quantity [",  retrievedQuantity, "] as QUALITY quantity" );
					}
					else {
						mout.reject<LOG_DEBUG>("unmatching QUALITY quantity  [",  retrievedQuantity, "], skipping" );
					}
				}
				else {
					quantityOK = true;
					checkSubGroup    = true; //needed?
					mout.pending<LOG_DEBUG>("No QUANTITY or QUALITY constraint, accepting [",  retrievedQuantity, "] ", basepath, "//", currentElem);
					//mout.reject<LOG_DEBUG>("explicit QUALITY quantity undefined, skipping", basepath, "//", currentElem);
				}
			}
			else {
				mout.reject<LOG_WARNING>("something went wrong, unexpected group: ", basepath, "//", currentElem, " [", retrievedQuantity, "]");
			}

			result |= quantityOK; // = at least one found

			if (quantityOK){
				//mout.accept<LOG_INFO>("quantity[", retrievedQuantity, "] at ", basepath, "//", currentElem);
				if (pathMatcher.match(path)){
					mout.accept<LOG_DEBUG>("DATA/QUALITY path matches: ", path,  " [", retrievedQuantity, "]");
					pathContainer.push_back(path);
				}
				else {
					mout.reject<LOG_DEBUG+1>("DATA path '", path, "' does not match '", pathMatcher, "' (but quantity check OK)");
					//mout.attention("pathMatcher empty=", pathMatcher.empty());
				}
			}

			if (checkSubGroup){
				if (currentElem.belongsTo(ODIMPathElem::DATA|ODIMPathElem::QUALITY)){
					mout.pending<LOG_DEBUG+1>("DATA/QUALITY group ", path, " -> continuing to subgroups");
				}
				else {
					mout.pending<LOG_DEBUG+1>("continuing to subgroup:" ,  path);
				}
				result |= collectPaths(src, pathContainer, path);
			}

		}
		else if (currentElem.is(ODIMPathElem::ARRAY) || currentElem.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){

			// Does not affect result.
			if (pathMatcher.match(path)){
				mout.accept<LOG_DEBUG>("adding ARRAY / ATTRIBUTE group at: ", path);
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
		mout.warn("Path list empty, returning");
		return;
	}

	if (pathList.size() == count){
		mout.experimental<LOG_DEBUG+1>("count (", count, ") matches already, no need to prune");
		return;
	}


	std::vector<ODIMPathElem2> accepted; // sortable!
	accepted.reserve(src.getChildren().size());
	mout.debug("STEP 1: from the given (", pathList.size(), ") paths, extract DATASETs to a sortable structure.");

	for (ODIMPath & path: pathList) {

		path.trimHead();
		/*
		while ((!path.empty()) && path.front().empty()){
			mout.warn("Rooted path '", path, "' stripping leading slash");
			path.pop_front();
		}
		*/
		mout.debug2("? ", path);

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
				mout.debug3("already accepted DATASET elem: ", elem);
				//continue;
			}

		}
		else {
			// if not attrib?
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
				mout.accept<LOG_DEBUG+1>("Finally accepting '", path, ", due to stem=", stem);
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




// Resets, set given parameters and derives missing parameters.
/*
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
*/


void DataSelector::getPaths(const Hi5Tree & src, std::list<ODIMPath> & pathList) const {

	drain::Logger mout(__FILE__, __FUNCTION__);

	mout.experimental<LOG_DEBUG>("revised code: getPaths->selectPaths(): ", __FILE__, ':', __LINE__);
	selectPaths(src, pathList);
}


bool DataSelector::getPath(const Hi5Tree & src, ODIMPath & path) const {

	std::list<ODIMPath> paths;
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
		// TODO! if several quantities,  traverse list of quantities, give the first match
		drain::Logger mout(__FILE__, __FUNCTION__);

		const QuantitySelector::quantity_list & l = quantitySelector.getList();

		if (!(path.back().is(ODIMPathElem::DATASET) || l.empty())){
			// mout.experimental("could use quantity order here: ", path, " qty:", l.front().value);
			mout.debug("checking quantity once more...");
			for (const QuantityMatcher & m: l){
				for (ODIMPath & p: paths){
					std::string q = src(p).data.image.properties.get("what:quantity", "");
					if (m == q){
						mout.special<LOG_INFO>("found preferred [", q,"] matching '", m, "' at: ", p);
						path = p;
						return true;
					}
				}
			}
		}

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

namespace drain {
	DRAIN_TYPENAME_DEF(rack::DataSelector);
}
