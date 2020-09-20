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
			// unsigned int index=0,
			unsigned int count = 1000,
			double elangleMin = -90.0, double elangleMax = 90.0);

	DataSelector(const std::string & parameters = "");

	/// Inits pathmatcher
	DataSelector(ODIMPathElem::group_t e, ODIMPathElem::group_t e2=ODIMPathElem::ROOT, ODIMPathElem::group_t e3=ODIMPathElem::ROOT);

	DataSelector(const DataSelector & selector);

	virtual ~DataSelector();


	/// Regular expression of accepted PolarODIM what::quantity, for example "DBZ.?" .
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

	//drain::Flags groups;
	//virtual	void setParameters(const std::string & parameters);

protected:

	/// Data quantity (excluding quality data, like QIND or CLASS)
	drain::RegExp quantityRegExp;

	/// Quality quantity, like QIND or CLASS
	drain::RegExp qualityRegExp;


	std::string  groupStr; // converted to 'groups' with update.




public:


	/// Regular expression of accepted paths, for example ".*/data$". Deprecated
	/**
	 *  \deprecated Use \c dataset and \c data parameters instead
	 */
	std::string path; // temporary!

	ODIMPathMatcher pathMatcher;

	/// Convert path and quantity strings to pathMatcher and quantity regexps, respectively.
	virtual
	void update();


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
	// Recommended convenience function.
	/*
	template <class T>
	inline
	void getPaths(const Hi5Tree & src, T & pathContainer, ODIMPathElem::group_t groupFilter) const {
		getPaths(src, pathContainer, drain::RegExp(quantity), groupFilter, ODIMPath());
	}
	*/
	/*
	inline
	void getPaths(const Hi5Tree & src, std::list<ODIMPath> & pathContainer, ODIMPathElem::group_t groupFilter) const {
		getPaths3(src, pathContainer);
	}*/

	/// Select paths based on path (chain) matching, as well as on quantity and elevation matching when applicable.
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param pathContainer - container for paths to be found
	 *  \param groupFilter - deprecating...
	 *  \param path - initial path
	 */
	//void getPaths3(const Hi5Tree & src, std::list<ODIMPath> & pathContainer, ODIMPathElem::group_t groupFilter=ODIMPathElem::ALL_GROUPS , const ODIMPath & path = ODIMPath()) const;
	//void getPaths3(const Hi5Tree & src, std::list<ODIMPath> & pathContainer, const ODIMPath & path = ODIMPath()) const;
	template <class T>
	bool getPaths3(const Hi5Tree & src, T & pathContainer, const ODIMPath & path = ODIMPath()) const;

	/// Returns the first path encountered with selector attributes and given groupFilter .
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param path - resulting path, if found; othewise intact
	 */
	bool getPath3(const Hi5Tree & src, ODIMPath & path) const;

	/*

	template <class T>
	inline
	void getPaths(const Hi5Tree & src, T & pathContainer) const {
		drain::Logger mout(__FUNCTION__, getName());
		mout.note() << "deprecating" <<  mout.endl;
		getPaths3(src, pathContainer);
		if (groups.value > 0){
			getPaths(src, pathContainer, groups.value);
		}
		else {
			drain::Logger mout(__FUNCTION__, getName());
			mout.note() << "groups flag unset, using DATA + DATASET" << mout.endl;
			getPaths(src, pathContainer, (ODIMPathElem::DATA | ODIMPathElem::DATASET));
			//groups = groupStr;
		}
	}
		 */


	/// Retrieves paths using current selection criteria and an additional group selector.
	/*
	 *   \tparam T - container class supporting addPathT(), e.g. std::set, std::list or std::vector.
	 *   \param src - the data structure searched for paths
	 *   \param container - container for found paths
	 *   \param groupFilter - selector (ODIMPath group types) for the lowest groups to be added in container,
	 *   \return - true quantity was found
	 *
	 *   See the simplified version of this function.
	 */
	//template <class T>
	//bool getPaths(const Hi5Tree & src, T & pathContainer, const drain::RegExp & quantityRE, ODIMPathElem::group_t groupFilter, const ODIMPath & path ) const;


	/// Returns the first path encountered with selector attributes and given groupFilter .
	/*
	inline
	bool getPathNEW(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t groupFilter) const {
		drain::Logger mout(__FUNCTION__, __FILE__);
		mout.warn() << "obsolete" << mout.endl;
		return getPath3(src, path);
	}

	/// Returns the first path encountered with selector attributes and given groupFilter .
	inline
	bool getPathNEW(const Hi5Tree & src, ODIMPath & path) const {
		return getPath3(src, path);
	}
    */

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


	/// Traverses down the tree and returns matching paths as a map with elevations as keys.
	/**
	 *   \param m
	 */
	//template <class T>
	/*
	static inline
	void getPathsByElevation(const Hi5Tree &src, const DataSelector & selector, std::map<double, ODIMPath> & m){
		//getPathsT(src, selector, m);
		selector.getPaths(src, m, ODIMPathElem::DATASET); // TODO check
	}
	 */

	/*
	static inline
	void getPathsByQuantity(const Hi5Tree &src, const DataSelector & selector, std::map<std::string, ODIMPath> & m){
		//getPathsT(src, selector, m);
		selector.getPaths(src, m, ODIMPathElem::DATASET); // TODO check
	}
	*/



	/// Temporary fix: try to derive dataset and data indices from path regexp.
	/**
	 *   Variable 'path' will be probably obsolete in future.
	 */
	inline
	void convertRegExpToRanges(const std::string & param){
	}

	/// Temporary fix: try to derive dataset and data indices from deprecated 'path' regexp (and also, quantity, current first param.)
	/**
	 *   Variable 'path' will be probably obsolete in future.
	 */
	inline
	void convertRegExpToRanges(){
		convertRegExpToRanges(this->path);
		//convertRegExpToRanges(this->quantity);
	};

protected:


	/// Sets the default values and sets references.
	void init();

	/// Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	/**
	 *   Applicable for ODIMPathList and std::map<double,std::string>.
	 *   \param container - ODIMPathList and std::map<double,std::string>
	 */
	/*
	template <class T>
	static
	inline
	void getPathsT(const Hi5Tree &src, const DataSelector & selector, T & container){
		getPaths(src, container, selector.path, selector.quantity,
				selector.index, selector.count,
				selector.elangle.min, selector.elangle.max); // min, max
	}
 */

	/// NEW Collects paths to a list.
	/*
	template <class P>
	static
	void addPathT(std::list<P> & l, const PolarODIM & odim, const P & path){ l.push_back(path); }
*/

	/// Collects paths to a set. (unused?)
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	/*
	template <class P>
	static inline
	void addPathT(std::set<P> & l, const PolarODIM & odim, const P & path){
		l.insert(path);
	}
*/

	/// Collects paths by their elevation angle (elangle).
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	template <class P>
	static inline
	void addPathT(std::map<double,P> & m, const PolarODIM & odim, const std::string & path){
		m[odim.elangle] = path;
	}
 */


	/// Collects paths by their quantity, eg. DBZH, VRAD, and RHOHV.
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	template <class P>
	static inline
	void addPathT(std::map<std::string,P> & m, const PolarODIM & odim, const std::string & path){
		m[odim.quantity] = path;
	}

	static inline
	bool addPath3(std::map<double,ODIMPath> & m, const PolarODIM & odim, const ODIMPath &path){
		m[odim.elangle] = path;
		return false;// ??
	}

	static inline
	bool addPath3(std::list<ODIMPath> & l, const PolarODIM & odim, const ODIMPath &path){
		l.push_back(path);
		return false;// ??
	}
*/

	/**
	 *  \param l - container for paths
	 */
	static inline
	bool addPath3(std::list<ODIMPath> & l, const drain::FlexVariableMap & props, const ODIMPath &path){
		PolarODIM odim;
		odim.updateFromCastableMap(props);
		l.push_back(path);
		return false;// ??
	}

	/// Add path to a map, using elevation angle as index. (Assumes polar data)
	/**
	 *  \param l - container for paths
	 */
	static inline
	bool addPath3(std::map<double,ODIMPath> & m, const drain::FlexVariableMap & props, const ODIMPath &path){
		PolarODIM odim;
		odim.updateFromCastableMap(props);
		m[odim.elangle] = path;
		return false;// ??
	}

	/// Add path to a map, using timestamp (\c what:startdate + \c what:starttime ) as index. (Assumes polar data)
	/**
	 *  \param m - container for paths
	 */
	static inline
	bool addPath3(std::map<std::string,ODIMPath> & m, const drain::FlexVariableMap & props, const ODIMPath &path){
		PolarODIM odim;
		odim.updateFromCastableMap(props);
		m[odim.startdate + odim.starttime] = path;
		return false;// ??
	}


	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	template <class P>
	static inline
	bool addPathT(std::list<P> & l, const std::string &path){
		l.push_back(path);
		return false;// ??
	}


	/// Add a std::string. Search ends with the first entry encountered. (?)
	static inline
	bool addPathT(std::string & str, const std::string &path){
		str.assign(path);
		return true;  // ??
	};

};

template <class T>
bool DataSelector::getPaths3(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (path.empty())
		mout.debug(1) << "matcher: " << pathMatcher << mout.endl;

	// Current search point
	const Hi5Tree & s = src(path); // =src, if path empty

	// ODIM struct is needed to communicate keys for maps (elangle or quantity) to addPathT
	PolarODIM odim;

	/// For dataset indices
	std::set<ODIMPathElem::index_t> dataSetIndices;

	const bool quantityRequired = quantityRegExp.isSet() || qualityRegExp.isSet();
	//bool quantityChecked = false;
	bool quantityFound   = false;
	//bool resultOK = true;

	for (Hi5Tree::const_iterator it = s.begin(); it != s.end(); ++it) {

		bool quantityOk = !quantityRequired; //

		const ODIMPathElem & currentElem = it->first;
		mout.debug(1) << "currentElem='" << currentElem << "'" << mout.endl;

		const drain::image::Image & d    = it->second.data.dataSet; // for ODIM
		const drain::FlexVariableMap & v = d.getProperties();

		// Check ELANGLE (in datasets) or quantity (in data/quality)
		if (currentElem.is(ODIMPathElem::DATASET)){ // what about quality?

			if (v.hasKey("where:elangle")){
				if (!elangle.contains(v["where:elangle"])){
					mout.debug() << "outside elangle range" << mout.endl;
					continue;
				}
			}

			if (dataSetIndices.size() >= count){
				mout.debug() << "count (" << dataSetIndices.size() << ">=" << count << ") already completed for " << currentElem << mout.endl;
				continue;
			}

		}
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){

			if (quantityRequired){

				if (v.hasKey("what:quantity")){

					const std::string quantity = v["what:quantity"].toStr();

					if (qualityRegExp.isSet()){ // NOTE: at least, do not test quantity after this!
						//quantityOk = false;
						if (currentElem.is(ODIMPathElem::QUALITY) && qualityRegExp.test(quantity)){
							mout.debug(2) << "matching QUALITY quantity  [" << quantity << "], skipping" << mout.endl;
							quantityOk    = true;
							quantityFound = true;
						}
					}
					else if (quantityRegExp.test(quantity)){
						mout.debug(1) << "OK quantity matches: " << quantity << ": " << path << '|' << currentElem << mout.endl;
						quantityOk    = true;
						quantityFound = true;
					}
					else {
						mout.debug(2) << "unmatching DATA quantity  [" <<  quantity << "], skipping" << mout.endl;
						//NO continue; have to descend for quality (QIND) below!
						//quantityOk = false;
					}

				}
				else {
					if (currentElem.is(ODIMPathElem::DATA))
						mout.warn();
					else
						mout.info();
					mout << "quantity missing in (image) metadata of " << path << '/' << currentElem << mout.endl;
				}
			}
		}
		else {
			//mout.warn() << "NOT testing: " << currentElem << mout.endl;
		}


		ODIMPath p(path);
		p << currentElem;


		if (currentElem.is(ODIMPathElem::DATASET)){

			if (pathMatcher.match(p) && (dataSetIndices.size() < count)){

				typename T::iterator tit = pathContainer.end();
				addPath3(pathContainer, v, p); // accept tentatively, maybe cancelled below

				// RECURSION
				quantityOk = getPaths3(src, pathContainer, p);
				if (quantityOk || !quantityRequired){
					mout.debug() << "yes, was ok: " << p << mout.endl;
				}
				else {
					mout.debug() << "oops, removing: " << p << mout.endl;
					pathContainer.erase(--tit);
				}
				// check over flow
			}
			else {
				// not collecting DATASET's but traversing
				getPaths3(src, pathContainer, p);
			}

		}
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			//mout.warn() << "descending D/Q :" << currentElem << mout.endl;
			if (quantityOk && pathMatcher.match(p)){
				addPath3(pathContainer, v, p);
			}
			getPaths3(src, pathContainer, p);
		}
		else {
			/// Recursion: traverse descendants. Note: only quantities may be checked (and zero paths returned)
			if (pathMatcher.match(p)){
				addPath3(pathContainer, v, p);
			}
			getPaths3(src, pathContainer, p); // note: original "root" src
			quantityOk = true; //?
		}

		if (quantityOk){
			const ODIMPathElem & e = p.front();
			if (pathMatcher.matchElem(e, true)){
				mout.debug(1) << "stem: " << e  << mout.endl;
				dataSetIndices.insert(e.getIndex());
			}
			//mout.warn() << "datasets: " << drain::StringTools::join(dataSetIndices,'*') << '=' <<dataSetIndices.size() << ':' <<  p.front().getIndex() << '|' << p << mout.endl;
		}

	}
	//mout.warn() << "quantityFound " << p << " -> ..."<< mout.endl;

	return (quantityFound || !quantityRequired);
}


inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters() << ", matcher=" << selector.pathMatcher;
	return ostr;
}


// Experimental
class DatasetSelector : public DataSelector {

public:

	DatasetSelector(){
		parameters.dereference("path");
		pathMatcher.setElems(ODIMPathElem::DATASET);
	}

};

} // rack::

#endif /* DATASELECTOR_H_ */
