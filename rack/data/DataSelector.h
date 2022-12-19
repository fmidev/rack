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

#ifndef DATASELECTOR_H_
#define DATASELECTOR_H_

#include <set>
#include <list>
#include <map>

#include "drain/util/BeanLike.h"
#include "drain/util/Range.h"
#include "drain/util/ReferenceMap.h"
#include "drain/util/RegExp.h"

#include "drain/util/Variable.h"
#include "ODIM.h"
#include "PolarODIM.h" // elangle

#include "ODIMPathMatcher.h"


namespace rack {



/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *  \see rack::CmdSelect
 */
class DataSelector : public drain::BeanLike {
public:


	// TODO: string => ODIMPath
	DataSelector(const std::string & path, const std::string & quantity,
			unsigned int count = 1000, drain::Range<double> elangle = {-90.0, 90.0}, int dualPRF = 0);
			// double elangleMin = -90.0, double elangleMax = 90.0);

	DataSelector(const std::string & parameters = "");

	/// Inits pathmatcher
	// DataSelector(ODIMPathElem::group_t e, ODIMPathElem::group_t e2=ODIMPathElem::ROOT, ODIMPathElem::group_t e3=ODIMPathElem::ROOT);

	template<typename ... T>
	DataSelector(const ODIMPathElem & elem, const T &... rest): BeanLike(__FUNCTION__), orderFlags(orderDict,':') {
		init();
		pathMatcher.setElems(elem, rest...);
		updateBean();
	}

	// Either this or previous is unneeded?
	template<typename ... T>
	DataSelector(ODIMPathElem::group_t e, const T &... rest): BeanLike(__FUNCTION__), orderFlags(orderDict,':') {
		init();
		pathMatcher.setElems(e, rest...);
		updateBean();
	}


	DataSelector(const DataSelector & selector);

	virtual ~DataSelector();



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


	inline
	void setQuantity(const std::string & quantity){
		this->quantity = quantity;
		this->updateBean();
	}

	/// Regular expression of accepted paths, for example ".*/data$". Deprecated
	/**
	 *  \deprecated Use \c dataset and \c data parameters instead
	 */
	//mutable
	std::string path; // temporary!

	mutable
	ODIMPathMatcher pathMatcher;

	/// Regular expression of accepted PolarODIM what::quantity, for example "DBZ.?" .
	// TODO: protect
	std::string quantity;

	// Path criteria
	// drain::Range<unsigned short> dataset;
	// drain::Range<unsigned short> data;
	/// The (minimum) index of the key in the list of matching keys.
	//  unsigned int index;

	/// The maximum length of the list of matching keys.
	unsigned int count;

	/// The minimum and maximum elevation angle (applicable with volume scan data only).
	drain::Range<double> elangle;

	std::string  order;

	static
	const drain::Flagger::dict_t orderDict;

// Debugging...
// protected:

	typedef enum {DEFAULT=0,MIN=1,MAX=2,DATA=4,TIME=8,ELANGLE=16} orderEnum;

	mutable
	drain::Flagger orderFlags; //(0, orderDict, ':');

	/// Reject or accept VRAD(VH)
	int dualPRF;

protected:


	/// Data quantity (excluding quality data, like QIND or CLASS)
	mutable // called by updateBean()
	drain::RegExp quantityRegExp;

	/// Quality quantity, like QIND or CLASS
	mutable
	drain::RegExp qualityRegExp;



public:




	/// Convert path and quantity strings to pathMatcher and quantity regexps, respectively.
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
	 */
	virtual
	void deriveParameters(const std::string & parameters, bool clear=true);// , ODIMPathElem::group_t defaultGroups = (ODIMPathElem::DATA | ODIMPathElem::QUALITY)); //, char assignmentSymbol='=', char separatorSymbol=0);



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
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param pathContainer - container for paths to be found
	 *
	 *  Calls getSubPaths
	template <class T>
	bool getPathsOLD(const Hi5Tree & src, T & pathContainer) const;
	 */

	void getPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer) const;
	//bool getPaths(const Hi5Tree & src, T & pathContainer, const ODIMPath & path = ODIMPath()) const;

	/// Collect top-level paths, ie. \c /dataset paths and directly rooted \c /what \c /where and \c /how paths
	/**
	 *  \param pathContainer – list or map (ordered by timestamp or angle)
	 *  \param LIMIT_COUNT – if true, use 'count' to limit number of datasets.
	 */
	template <class T>
	void getMainPaths(const Hi5Tree & src, T & pathContainer, bool LIMIT_COUNT=true) const;

	/** Checks dataset elevation angles. Applicable only with measurement volume data.
	 *
	 */
	void getPathsByElangle(const Hi5Tree & src, std::map<double,ODIMPath>    & paths) const;

	/** Checks dataset elevation start times. Applicable only with measurement volume data.
	 *
	 */
	void getPathsByTime(const  Hi5Tree  &  src, std::map<std::string,ODIMPath> & paths) const;


protected:

	/// Continue path matching started with getMainPaths()
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param pathContainer - container for paths to be found
	 *  \param path - search path, dataset<n> by default
	 */
	template <class T>
	bool getSubPaths(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const;

public:

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


	/// Searches the children of child.getType(), or g if given, and stores the one with largest index.
	static
	bool getLastChild(const Hi5Tree & tree, ODIMPathElem & child); //, (ODIMPathElem::group_t g =  ODIMPathElem::NONE);

	/// Searches children of given type, returns a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getNewChild(const Hi5Tree & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax=0xff);

	/// Derive a child with index one greater than the largest index encountered.
	/**
	 *  \param tree - parent structure
	 *  \param child - child to be set index = last+1, hence 1 if none found.
	 *  \return - true if child's index was incremented (ie.)
	 */
	static
	bool getNextChild(const Hi5Tree & tree, ODIMPathElem & child);

	//static 	bool getNextDescendant(Hi5Tree & tree, (ODIMPathElem::group_t g, ODIMPath & path);

	/// Searches children of given type, returns a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getChildren(const Hi5Tree & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups);



	/// Sets parameters in predefined order or sets specified parameters. (Python style calling alternatives.)
	//  *   - \c index - integer value, only changing the \c index member. ???
	/**
	 *  \param parameters - the parameters to be changed, using some of the syntaxes:
	 *   - \c path,quantity,index,count>,elangleMin,elangleMax (complete or partial list of parameter values, in this order)
	 *   - \c parameter1=value1,parameter2=value2,parameter3=value3   (specific assignments as a list)
	 *   The regular expressions should not contain comma (,).
	 *   \param specific - if true, the specific assignments are supported
	 */



protected:


	/// Sets the default values and sets references.
	void init();

	/// Collects paths to a list.
	/**
	 *  \param l - container for paths
	 *  \param props - unused (in this variant, for lists)
	 */
	static inline
	void addPath3(std::list<ODIMPath> & l, const drain::FlexVariableMap & props, const ODIMPath &path){
		//PolarODIM odim;
		//odim.updateFromCastableMap(props);
		l.push_back(path);
	}

	/// Add path to a map, using elevation angle as index. (Assumes polar data)
	/**
	 *  Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	 *
	 *  \param m - container for paths
	 *  \param props - metadata containing \c where:elangle
	 */
	static inline
	void addPath3(std::map<double,ODIMPath> & m, const drain::FlexVariableMap & props, const ODIMPath &path){
		PolarODIM odim(ODIMPathElem::DATASET); // minimise references - only where:elangle needed
		odim.updateFromCastableMap(props);  // where:elangle
		m[odim.elangle] = path;
	}

	/// Add path to a map, using timestamp (\c what:startdate + \c what:starttime ) as index. (Assumes polar data)
	/**
	 *  \param m - container for paths
	 *  \param props - metadata containing \c what:startdate + \c what:starttime
	 */
	static inline
	void addPath3(std::map<std::string,ODIMPath> & m, const drain::FlexVariableMap & props, const ODIMPath &path){
		PolarODIM odim(ODIMPathElem::DATASET); // minimise references - only what:startdate and what:starttime
		odim.updateFromCastableMap(props); // what:startdate &  what:starttime
		m[odim.startdate + odim.starttime] = path;
	}

	/*
	template <typename C>
	static inline
	void addPath4(std::list<ODIMPath> & l, const C & key, const ODIMPath &path){
		l.push_back(path);
	}

	static inline
	void addPath4(std::map<double,ODIMPath> & m, double key, const ODIMPath &path){
		m[key] = path;
	}

	static inline
	void addPath4(std::map<std::string,ODIMPath> & m, const std::string & key, const ODIMPath &path){
		m[key] = path;
	}
	*/


	/**
	 *  \param pathContainer – std::map<double,ODIMPath> or std::map<std::string,ODIMPath> for ELANGLE and TIME, respectively.
	 *
	 *  \param MAX_KEYS: use keys with greatest values: latest TIME or highest ELANGLE
	 */
	template <class M>
	void pruneMap(M & pathContainer, bool MIN_KEYS = false) const ;

};


template <class M>
void DataSelector::pruneMap(M & pathContainer, bool MAX_KEYS) const {

	// Number of paths kept.
	unsigned int n = count <= pathContainer.size() ? count : pathContainer.size();

	typename M::iterator it = pathContainer.begin();
	if (!MAX_KEYS){
		// Jump over n keys.
		std::advance(it, n);
		pathContainer.erase(it, pathContainer.end());
	}
	else {
		// Jump towards end, so that n keys remain
		std::advance(it, pathContainer.size()-n);
		pathContainer.erase(pathContainer.begin(), it);
	}

}

/*
template <class T>
bool DataSelector::getPathsOLD(const Hi5Tree & src, T & pathContainer) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (orderFlags.isSet(TIME)){
		mout.special(__FUNCTION__, ':', orderFlags);
		std::map<std::string,ODIMPath> m;
		getPathsByTime(src, m);
		pruneMap(m, orderFlags.isSet(MAX));
		//return map2list(m, pathContainer, false);
		for (const auto & entry: m){
			//mout.special(entry);
			mout.special(entry.first, '\t', entry.second);
			addPath4(pathContainer, entry.first, entry.second);
		}
	}
	else if (orderFlags.isSet(ELANGLE)){
		mout.special(__FUNCTION__, ':', orderFlags);
		std::map<double,ODIMPath> m;
		getPathsByElangle(src, m);
		pruneMap(m, orderFlags.isSet(MAX));
		for (const auto & entry: m){
			mout.special(entry.first, '\t', entry.second);
			addPath4(pathContainer, entry.first, entry.second);
		}
	}
	else {
		getMainPaths(src, pathContainer);
	}

	return true;
}
*/



// TODO: redesign with
// getPaths_DS() level 1
// getPaths_DQ() level 2+
template <class T>
void DataSelector::getMainPaths(const Hi5Tree & src, T & pathContainer, bool LIMIT_COUNT) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	// Current search point
	const Hi5Tree & s = src;

	/// For book keeping of traversed datasets
	std::set<ODIMPathElem::index_t> dataSetIndices;

	for (Hi5Tree::const_iterator it = s.begin(); it != s.end(); ++it) {

		const ODIMPathElem & currentElem = it->first;
		mout.debug3() << "currentElem='" << currentElem << "'" << mout.endl;

		// can be moved inside DATASET scope if no other groups included in path (in future?)
		const drain::image::Image & data    = it->second.data.dataSet; // for ODIM
		const drain::FlexVariableMap & props = data.getProperties();

		ODIMPath path;
		path << currentElem;


		// Check ELANGLE (in datasets) or quantity (in data/quality)
		if (currentElem.is(ODIMPathElem::DATASET)){

			if ((LIMIT_COUNT) && (dataSetIndices.size() >= count)){
				mout.debug2() << "dataset count " << dataSetIndices.size() << ">=" << count << ") already completed for " << currentElem << mout.endl;
				continue;
			}

			if (props.hasKey("where:elangle")){
				double e = props["where:elangle"];
				if (!elangle.contains(e)){
					mout.debug("elangle ",e," outside range ",elangle);
					continue;
				}
			}

			if (getSubPaths(src, pathContainer, path)){
				// Add this data set path
				if (pathMatcher.match(path)){
					mout.debug2() << " path matches (subtree OK) " << path << mout;
					addPath3(pathContainer, props, path);
				}
				dataSetIndices.insert(currentElem.index);
			}

			//continue;
		}
		//else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::DATA)){
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){ // 2021/04

			if (currentElem.is(ODIMPathElem::DATA)){
				mout.warn() << " DATA_GROUP '" << currentElem << "' directly under root"  << mout;
			}

			if (getSubPaths(src, pathContainer, ODIMPath())){
				// added qualityX ...
			}

		}
		else if (currentElem.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS)){
			// Consider policy..
			//mout.warn() << " skipping ATTRIBUTE_GROUP: /" << currentElem << mout;
			if (pathMatcher.match(path)){
				addPath3(pathContainer, props, path);
			}
			// addPath3(pathContainer, v, path); // ?
		}
		else {
			mout.warn() << " skipping odd group: /" << currentElem << mout;
		}

	}

	//return true; //...
}


template <class T>
bool DataSelector::getSubPaths(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	const bool quantityRequired = quantityRegExp.isSet() || qualityRegExp.isSet();

	bool groupsOK = false || !quantityRequired; // :-)

	const Hi5Tree & s = src(path);

	for (Hi5Tree::const_iterator it = s.begin(); it != s.end(); ++it) {

		bool quantityFound = false || !quantityRequired; // :-)

		//bool quantityOk = !quantityRequired; // Always ok if not required...
		const ODIMPathElem & currentElem = it->first;
		mout.debug3() << "currentElem='" << currentElem << "'" << mout.endl;
		const drain::image::Image & data    = it->second.data.dataSet; // for ODIM
		const drain::FlexVariableMap & props = data.getProperties();

		ODIMPath p(path);
		p << currentElem; // also for attrib groups

		if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){

			if (quantityRequired){



				if (props.hasKey("what:quantity")){

					const std::string quantity = props["what:quantity"].toStr();

					if (qualityRegExp.isSet()){ // NOTE: at least, do not test quantity after this!
						//quantityOk = false;
						if (currentElem.is(ODIMPathElem::QUALITY) && qualityRegExp.test(quantity)){
							mout.note() << "QUALITY quantity matches: [" << quantity << "]" << mout.endl;
							//quantityOk    = true;
							quantityFound = true;
						}
					}
					else if (quantityRegExp.test(quantity)){
						mout.debug2() << "quantity matches: [" << quantity << "]: " << path << '|' << currentElem << mout.endl;
						//quantityOk    = true;
						quantityFound = true;
					}
					else {
						mout.debug3() << "unmatching DATA quantity  [" <<  quantity << "], skipping" << mout.endl;
						//NO continue; have to descend for quality (QIND) below!
						//quantityOk = false;
						// continue;
						// recursion continues below
					}

					if (quantityFound && (dualPRF != 0)){
						double lowPRF   = props.get("how:lowprf",  0.0);
						double hightPRF = props.get("how:highprf", lowPRF);
						bool IS_DUAL_PRF = (lowPRF != hightPRF);
						if (dualPRF > 0){
							if (!IS_DUAL_PRF){
								mout.experimental() << "dualPRF required, rejecting [" << quantity << "] at " << p << mout;
								quantityFound = false;
							}
						}
						else {
							if (IS_DUAL_PRF){
								mout.experimental() << "dualPRF rejected:  [" << quantity << "] at " << p << mout;
								quantityFound = false;
							}
						}
					}

				}
				else {

					if (currentElem.is(ODIMPathElem::DATA))
						mout.warn();
					else
						mout.info();
					mout << "quantity missing in (image) metadata of " << path << '/' << currentElem << mout.endl;

					// continue?
				}

			}


			if (quantityFound && pathMatcher.match(p)){
				addPath3(pathContainer, props, p);
			}

			// Recursion, possibly finding quality quantity
			quantityFound |=  getSubPaths(src, pathContainer, p);

		}
		else if (currentElem.belongsTo(ODIMPathElem::ATTRIBUTE_GROUPS | ODIMPathElem::ARRAY)){
			// mout.warn() << " ATTRIBUTE_GROUP: " << path << " / " << currentElem << mout;
			if (pathMatcher.match(p)){
				mout.debug3() << " adding" << path << " / " << currentElem << mout;
				addPath3(pathContainer, props, p);
			}
		}
		else if (currentElem.is(ODIMPathElem::DATASET)){
			mout.warn() << " NESTING DATASET_GROUP: " << path << " / " << currentElem << mout;
			// addPath3(pathContainer, v, path); // ?
		}
		else {
			mout.debug() << " skipping special: " << path << " / " << currentElem << mout;
		}
		/*
		   else if (currentElem.is(ODIMPathElem::ARRAY)){
			if (pathMatcher.match(p)){
				mout.debug3() << " adding" << path << " / " << currentElem << mout;
				addPath3(pathContainer, props, p);
			}
		  }

		 */
		groupsOK |= quantityFound;
	}

	return groupsOK;
}



inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters() << ", matcher=" << selector.pathMatcher;
	ostr << ", orderFlags=" << selector.orderFlags;
	return ostr;
}


// Experimental
class DatasetSelector : public DataSelector {

public:

	DatasetSelector() : DataSelector(ODIMPathElem::DATASET){
		drain::Logger mout(__FUNCTION__, __FILE__);
		parameters.delink("path");
		mout.info() << "experimental: not re-setting DATASET" << mout.endl;
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

#endif /* DATASELECTOR_H_ */
