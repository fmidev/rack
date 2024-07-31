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

#ifndef RACK_DATASELECTOR
#define RACK_DATASELECTOR

#include <set>
#include <list>
#include <map>
#include <stdexcept>

#include <drain/RegExp.h>
#include <drain/Sprinter.h>
#include <drain/Type.h>
#include <drain/util/BeanLike.h>
#include <drain/util/Range.h>
#include <drain/util/ReferenceMap.h>
//#include "drain/util/Variable.h"

#include "ODIM.h"
#include "ODIMPathTools.h"
#include "ODIMPathMatcher.h"
#include "PolarODIM.h" // elangle
#include "QuantitySelector.h"


namespace rack {


struct DataOrder { //: public drain::BeanLike {

	enum Crit {DATA, ELANGLE, TIME}; // ALTITUDE
	enum Oper {MIN, MAX};

	const char separator = ':';

	typedef drain::EnumFlagger<drain::SingleFlagger<Crit> > CritFlagger;
	CritFlagger criterion;

	typedef drain::EnumFlagger<drain::SingleFlagger<Oper> > OperFlagger;
	OperFlagger operation;

	template<typename ... TT>
	void set(Crit crit, const TT &... args) {
		criterion = crit;
		set(args...);
	};

	template<typename ... TT>
	void set(Oper oper, const TT &... args) {
		operation = oper;
		set(args...);
	};

	/// Expects <crit>[:<oper>]
	inline
	void set(const std::string s) {
		std::string s1, s2;
		drain::StringTools::split2(s, s1, s2, separator);
		criterion.set(s1);
		operation.set(s2);
		set();
	};

	void set(){
		str = criterion.str() + separator + operation.str();
	}

	std::string str;

};


inline
std::ostream & operator<<(std::ostream & ostr, const DataOrder & order){
	ostr << order.str;
	return ostr;
}

/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *  \see rack::CmdSelect
 */  // QuantitySelector,
class DataSelector: public drain::BeanLike  {
public:

	friend class drain::ReferenceMap;

	/// Pulse repetition frequency mode
	enum Prf {SINGLE=1, DOUBLE=2, ANY=3};

	// TODO: string => ODIMPath
	DataSelector(const std::string & path, const std::string & quantity,
			unsigned int count = 1000, drain::Range<double> elangle = {-90.0, 90.0}, int dualPRF = 0,
			drain::Range<int> timespan={0,0});
			// double elangleMin = -90.0, double elangleMax = 90.0);

	DataSelector(const std::string & parameters = "");

	/// Inits pathmatcher
	// DataSelector(ODIMPathElem::group_t e, ODIMPathElem::group_t e2=ODIMPathElem::ROOT, ODIMPathElem::group_t e3=ODIMPathElem::ROOT);

	template<typename ... T>
	DataSelector(const ODIMPathElem & elem, const T &... args): BeanLike(__FUNCTION__){ //, orderFlags(orderDict,':') {
		init();
		//pathMatcher.setElems(elem, rest...);
		pathMatcher.set(elem, args...);
		updateBean();
	}

	// Either this or previous is unneeded?
	template<typename ... T>
	DataSelector(ODIMPathElem::group_t e, const T &... args): BeanLike(__FUNCTION__){ // , orderFlags(orderDict,':') {
		init();
		//pathMatcher.setElems(e, rest...);
		pathMatcher.set(e, args...);
		updateBean();
	}


	DataSelector(const DataSelector & selector);

	virtual ~DataSelector();


	/// Sets parameters in predefined order or sets specified parameters. (Python style calling alternatives.)
	//  *   - \c index - integer value, only changing the \c index member. ???
	/**
	 *  \param parameters - the parameters to be changed, using some of the syntaxes:
	 *   - \c path,quantity,index,count>,elangleMin,elangleMax (complete or partial list of parameter values, in this order)
	 *   - \c parameter1=value1,parameter2=value2,parameter3=value3   (specific assignments as a list)
	 *   The regular expressions should not contain comma (,).
	 *   \param specific - if true, the specific assignments are supported
	 */


	// Experimental. Raise to beanlike?
	/// Set parameters if args not empty then clear args.
	/**
	 *   return - true, if args were non empty and hence, parameters were set.
	 *
	 *   Note that args is always empty after invoking this function.
	 */
	inline
	bool consumeParameters(std::string & args){
		if (args.empty()){
			return false;
		}
		else {
			setParameters(args);
			args.clear();
			return true;
		}
	}

	template<typename ... TT>
	inline
	void setPathMatcher(TT... args){
		pathMatcher.set(args...);
	}

	inline
	const ODIMPathMatcher & getPathMatcher() const {
		return pathMatcher;
	}

	/// "Drop leading slashes", ie. remove leading empty elements.
	inline
	void trimPathMatcher(){
		return pathMatcher.trimHead(true);
	}

	/// Sets basic quantities and quality quantities. These sets are separated by '/'.
	void setQuantities(const std::string & s); // , const std::string & separators = ","); // todo: rename (here only) quantities?

	void setQuantityRegExp(const std::string & s); // todo: rename (here only) quantities?

	inline
	bool quantityIsSet() const {
		return quantitySelector.isSet() || qualitySelector.isSet();
	}

	/// Retrieve quantity list and regular expression, if defined.
	/**
	 *
	 *
	 */
	inline
	const std::string & getQuantity() const {
		return quantities;
	}

	inline
	const QuantitySelector & getQuantitySelector() const {
		return quantitySelector;
	}

	inline
	const QuantitySelector & getQualitySelector() const {
		return qualitySelector;
	}

	inline
	void setMaxCount(unsigned int i){
		count = i;
	}

	inline
	unsigned int getMaxCount() const {
		return count;
	}


	inline
	void setPrf(const std::string & s){
		this->prf = s;
		this->prfSelector.set(s);
	}

	inline
	void setPrf(Prf prf){
		this->prfSelector.set(prf);
		this->prf = this->prfSelector.str();
		//this->updateBean();
	}

	template<typename ... TT>
	inline
	void setOrder(const TT &... args) {
		this->order.set(args...);
	}

	inline
	const DataOrder & getOrder() const {
		return order;
	}


	/// Collect paths with all the criteria: path, elevation(range), PRF, quantity...
	/**
	 *
	 *
	 */
	void selectPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer) const;


	// TODO add filter, allowing ODIMPathElem::group_t == QUALITY
	static
	void getTimeMap(const Hi5Tree & srcRoot, ODIMPathElemMap & m);


	/// Updates member objects with their corresponding variable values .
	/**
	 *   Converts path and quantity strings to pathMatcher and quantity regexps, respectively.
	 *
	 */
	virtual
	void updateBean() const;

	/// In path, ensure trailing DATA or QUANTITY element.
	void ensureDataGroup();

	/// Restore default values.
	/**
	 *   The values set maximally accepting ie. all the groups are returned with getPaths() call.
	 */
	void reset(); // bool flexible = true "inclusive"


	/// Sets given parameters and implicitly determines missing parameters
	//  TODO: perhaps semantics unclear. Consider separating to updateParameters/ deriveImplicitParameters etc.
	/**
	 *   \param parameters - string containing parameters, like "dataset=1:3,quantity=DBZH"
	 *   \param clear      - first reset to default state
	 *   // groups set, if not given in \c parameters
	virtual
	void deriveParameters(const std::string & parameters, bool clear=true);// , ODIMPathElem::group_t defaultGroups = (ODIMPathElem::DATA | ODIMPathElem::QUALITY)); //, char assignmentSymbol='=', char separatorSymbol=0);
	 */



	/// Retrieves paths using current selection criteria and an additional group selector.
	/**
	 *   Selects paths down to \c dataset and \c data group level. Uses metadata of \c what , \c where and \c how but
	 *   does not include them in the result (container).
	 *
	 *   Each retrieved path starts with \b root \b element (ODIMPathElem::ROOT), corresponding to empty string ("").
	 *
	 *   \tparam T - container class supporting addPathT(), e.g. std::set, std::list or std::vector.
	 *   \param src - the data structure searched for paths
	 *   \param container - container for found paths
	 *   \param groupFilter - selector composed of id's (ODIMPathElem::DATASET etc ) of groups added in the container,
	 *   typically adjusted by calling functions (not by the end user).
	 *
	 *   The full tree structure will be searched for; \c groupFilter does not affect the traversal.
	 *   When retrieving quality groups, \c groupFilter should countain ODIMPathElem::QUALITY.
	 *
	 *   \see deriveParameters().
	 */

	/// Select paths based on path matching, as well as on quantity and elevation matching when applicable.
	void getPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer) const;


	/// Returns the first path encountered with selector attributes and given groupFilter .
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param path - resulting path, if found; othewise intact
	 */
	bool getPath(const Hi5Tree & src, ODIMPath & path) const;



	/// Retrieves paths using current selection criteria and an additional group selector.
	/// Returns the first path encountered with selector attributes and given groupFilter .

	/// Returns the last path encountered with selector attributes and given groupFilter .
	bool getLastPath(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t group = ODIMPathElem::DATA ) const;

	/// Returns the a path with index one greater than the retrieved last path.
	bool getNextPath(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t group = ODIMPathElem::DATA ) const;




	/// Swap branches such that dst gets a /dataset or /data with a new index.
	/**
	 *  Contents of \c src[srcElem] will be swapped with \c dst[dstElem] such that dstElem has an index greater than any index prior to the operation.
	 *
	 *  After the operation \c src[srcElem] contains an empty object that was created under \c dst for swapping.
	 *
	 *  Stylistic note: semantically this should belong to DataTools, but essentially calls getNextChild() hence is kept here.
	 *  Also, DataTools stays independent from DataSelector.
	 */
	static
	void swapData(Hi5Tree & src,const ODIMPathElem &srcElem, Hi5Tree & dst);

	/// Like swapData(Hi5Tree & src,const ODIMPathElem &srcElem, Hi5Tree & dst), but src already at the level.
	static
	void swapData(Hi5Tree & srcGroup, Hi5Tree & dst, ODIMPathElem::group_t groupType);


protected:


	/// Regular expression of accepted paths, for example ".*/data$". Deprecated
	/**
	 *  \deprecated Use \c dataset and \c data parameters instead
	 */
	//mutable
	std::string path; // temporary!

	mutable
	ODIMPathMatcher pathMatcher;

	/// Comma-separated list of conventional quantities, optionally followed by '/', and quality quantities.
	mutable
	std::string quantities;

	mutable
	QuantitySelector quantitySelector;

	mutable
	QuantitySelector qualitySelector;


	/// The maximum length of the list of matching keys.
	unsigned int count;


	/// The minimum and maximum elevation angle (applicable with volume scan data only).
	drain::Range<double> elangle;

	mutable DataOrder order;

	/// Reject or accept VRAD(VH)
	std::string prf;

	mutable
	drain::EnumFlagger<drain::SingleFlagger<Prf> > prfSelector;

	/// Time in seconds, compared to nominal time
	drain::Range<int> timespan;

	/// Continue path matching started with getMainPaths()
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param pathContainer - container for paths to be found
	 *  \param path - search path, dataset<n> by default
	template <class T>
	bool getSubPathsFOO(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const;
	 */

	void updateQuantities() const; // todo: rename (here only) quantities?
	// void updateQuantities(const std::string & separators = ",") const ;

	/// Sets the default values and sets references.
	void init();


	/// Collect paths (only) with criteria: path, elevation(range), PRF, quantity.
	/**
	 *
	 *  \return true, if contained something accepted by tests
	 */
	bool collectPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer, const ODIMPath & basepath = ODIMPath()) const;

	/// Use #DataOrder::criterion \c DATA , \c TIME or \c ELANGLE and #DataOrder::order \c MIN or MAX to sort paths.
	void prunePaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer) const;



};

std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector);



// Experimental
class DatasetSelector : public DataSelector {

public:

	DatasetSelector() : DataSelector(ODIMPathElem::DATASET){
		drain::Logger mout(__FILE__, __FUNCTION__);
		parameters.delink("path");
		mout.info("experimental: not re-setting DATASET" );
		//pathMatcher.setElems(ODIMPathElem::DATASET);
	}

};

// Experimental
/*
class ImageSelector : public DataSelector {

public:

	ImageSelector() : DataSelector(ODIMPathElem::DATA | ODIMPathElem::QUALITY) {
		parameters.link("path", dummy);
		//parameters.delink("path");
		// pathMatcher.setElems(ODIMPathElem::DATASET);
	}


	// Convert path and quantity strings to pathMatcher and quantity regexps, respectively.
	virtual
	void updateBean();

private:

	std::string dummy;

};
*/

} // rack::

namespace drain {
	DRAIN_TYPENAME(rack::DataSelector);
}

#endif /* DATASELECTOR_H_ */
