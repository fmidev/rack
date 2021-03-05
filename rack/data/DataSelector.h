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
			unsigned int count = 1000,
			double elangleMin = -90.0, double elangleMax = 90.0);

	DataSelector(const std::string & parameters = "");

	/// Inits pathmatcher
	// DataSelector(ODIMPathElem::group_t e, ODIMPathElem::group_t e2=ODIMPathElem::ROOT, ODIMPathElem::group_t e3=ODIMPathElem::ROOT);

	template<typename ... T>
	DataSelector(const ODIMPathElem & elem, const T &... rest): BeanLike(__FUNCTION__){
		init();
		pathMatcher.setElems(elem, rest...);
		updateBean();
	}

	// Either this or previous is unneeded?
	template<typename ... T>
	DataSelector(ODIMPathElem::group_t e, const T &... rest): BeanLike(__FUNCTION__){
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
	mutable // called by updateBean()
	drain::RegExp quantityRegExp;

	/// Quality quantity, like QIND or CLASS
	mutable
	drain::RegExp qualityRegExp;


	// std::string  groupStr; // converted to 'groups' with update.




public:


	/// Regular expression of accepted paths, for example ".*/data$". Deprecated
	/**
	 *  \deprecated Use \c dataset and \c data parameters instead
	 */
	//mutable
	std::string path; // temporary!

	mutable
	ODIMPathMatcher pathMatcher;

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

	/// Select paths based on path (chain) matching, as well as on quantity and elevation matching when applicable.
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param pathContainer - container for paths to be found
	 *  \param path - search path, root by default
	 */
	template <class T>
	bool getPaths(const Hi5Tree & src, T & pathContainer) const;
	//bool getPaths(const Hi5Tree & src, T & pathContainer, const ODIMPath & path = ODIMPath()) const;

	//template <class T>
	//bool getPathsDS(const Hi5Tree & src, T & pathContainer) const;

	template <class T>
	bool getPathsDQ(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const;


	/// Returns the first path encountered with selector attributes and given groupFilter .
	/**
	 *
	 *  \param src - HDF5 data structure
	 *  \param path - resulting path, if found; othewise intact
	 */
	bool getPath3(const Hi5Tree & src, ODIMPath & path) const;


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
	bool addPath3(std::list<ODIMPath> & l, const drain::FlexVariableMap & props, const ODIMPath &path){
		//PolarODIM odim;
		//odim.updateFromCastableMap(props);
		l.push_back(path);
		return false;// ??
	}

	/// Add path to a map, using elevation angle as index. (Assumes polar data)
	/**
	 *  Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	 *
	 *  \param m - container for paths
	 *  \param props - metadata containing \c where:elangle
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
	 *  \param props - metadata containing \c what:startdate + \c what:starttime
	 */
	static inline
	bool addPath3(std::map<std::string,ODIMPath> & m, const drain::FlexVariableMap & props, const ODIMPath &path){
		PolarODIM odim;
		odim.updateFromCastableMap(props);
		m[odim.startdate + odim.starttime] = path;
		return false;// ??
	}

};

// TODO: redesign with
// getPaths_DS() level 1
// getPaths_DQ() level 2+
template <class T>
bool DataSelector::getPaths(const Hi5Tree & src, T & pathContainer) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	//if (path.empty())
	//	mout.debug2() << "matcher: " << pathMatcher << mout.endl;

	// Current search point
	const Hi5Tree & s = src;

	//, const ODIMPath & path

	// ODIM struct is needed to communicate keys for maps (elangle or quantity) to addPathT
	//PolarODIM odim;

	/// For dataset indices
	std::set<ODIMPathElem::index_t> dataSetIndices;


	//const bool quantityRequired = quantityRegExp.isSet() || qualityRegExp.isSet();

	//bool quantityFound = false;

	for (Hi5Tree::const_iterator it = s.begin(); it != s.end(); ++it) {

		//bool quantityOk = !quantityRequired; //

		const ODIMPathElem & currentElem = it->first;
		mout.debug3() << "currentElem='" << currentElem << "'" << mout.endl;

		// can be moved inside DATASET scope if no other groups included in path (in future?)
		const drain::image::Image & data    = it->second.data.dataSet; // for ODIM
		const drain::FlexVariableMap & props = data.getProperties();

		ODIMPath path;
		path << currentElem;


		// Check ELANGLE (in datasets) or quantity (in data/quality)
		if (currentElem.is(ODIMPathElem::DATASET)){

			if (dataSetIndices.size() >= count){
				mout.debug2() << "dataset count " << dataSetIndices.size() << ">=" << count << ") already completed for " << currentElem << mout.endl;
				continue;
			}


			if (props.hasKey("where:elangle")){
				if (!elangle.contains(props["where:elangle"])){
					mout.debug() << "outside elangle range" << mout.endl;
					continue;
				}
			}

			if (getPathsDQ(src, pathContainer, path)){
				// Add this data set path
				if (pathMatcher.match(path)){
					mout.debug2() << " path matches (subtree OK) " << path << mout;
					addPath3(pathContainer, props, path);
				}
				dataSetIndices.insert(currentElem.index);
			}

			//continue;
		}
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::DATA)){
			if (currentElem.is(ODIMPathElem::DATA)){
				mout.warn() << " DATA_GROUP '" << currentElem << "' directly under root"  << mout;
			}
			if (getPathsDQ(src, pathContainer, ODIMPath())){
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

	return true; //...
}


template <class T>
bool DataSelector::getPathsDQ(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const {

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
			quantityFound |=  getPathsDQ(src, pathContainer, p);

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
			mout.warn() << " skipping" << path << " / " << currentElem << mout;
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

/*
template <class T>
bool DataSelector::getPaths(const Hi5Tree & src, T & pathContainer, const ODIMPath & path) const {

	drain::Logger mout(__FUNCTION__, __FILE__);

	if (path.empty())
		mout.debug2() << "matcher: " << pathMatcher << mout.endl;

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
		mout.debug3() << "currentElem='" << currentElem << "'" << mout.endl;

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
				mout.debug2() << "count (" << dataSetIndices.size() << ">=" << count << ") already completed for " << currentElem << mout.endl;
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
							mout.debug3() << "matching QUALITY quantity  [" << quantity << "], skipping" << mout.endl;
							quantityOk    = true;
							quantityFound = true;
						}
					}
					else if (quantityRegExp.test(quantity)){
						mout.debug2() << "OK quantity matches: " << quantity << ": " << path << '|' << currentElem << mout.endl;
						quantityOk    = true;
						quantityFound = true;
					}
					else {
						mout.debug3() << "unmatching DATA quantity  [" <<  quantity << "], skipping" << mout.endl;
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
				quantityOk = getPaths(src, pathContainer, p);
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
				quantityOk = getPaths(src, pathContainer, p); // NEW 2021/02 check
				// getPaths(src, pathContainer, p);
			}

		}
		else if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			//mout.warn() << "descending D/Q :" << currentElem << mout.endl;
			if (quantityOk && pathMatcher.match(p)){
				addPath3(pathContainer, v, p);
			}
			getPaths(src, pathContainer, p);
		}
		else {
			/// Recursion: traverse descendants. Note: only quantities may be checked (and zero paths returned)
			if (pathMatcher.match(p)){
				addPath3(pathContainer, v, p);
			}
			getPaths(src, pathContainer, p); // note: original "root" src
			quantityOk = true; //?
		}

		if (quantityOk){
			const ODIMPathElem & e = path.front();
			dataSetIndices.insert(e.getIndex());
			//mout.warn() << "datasets: " << drain::StringTools::join(dataSetIndices,'*') << '=' <<dataSetIndices.size() << ':' <<  p.front().getIndex() << '|' << p << mout.endl;
		}

	}
	//mout.warn() << "quantityFound " << p << " -> ..."<< mout.endl;

	return (quantityFound || !quantityRequired);
}
*/


inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters() << ", matcher=" << selector.pathMatcher;
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
