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

#include <drain/util/BeanLike.h>
#include <drain/util/Range.h>
#include <drain/util/ReferenceMap.h>
#include <drain/util/RegExp.h>

#include <drain/util/Variable.h>
#include "ODIM.h"
#include "PolarODIM.h" // elangle

namespace rack {



/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *    Future version will use:
 *    - data=
 *    - dataset=
 *
 *    Applies drain::RegExp in matching.
 */
class DataSelector : public drain::BeanLike {
public:


	DataSelector(const std::string & path, const std::string & quantity,
			unsigned int index=0, unsigned int count = 1000,
			double elangleMin = -90.0, double elangleMax = 90.0);

	DataSelector(const std::string & parameters = "");

	DataSelector(const DataSelector & selector);

	virtual ~DataSelector();

	/// Regular expression of accepted paths, for example ".*/data$".
	std::string path;

	// Path criteria
	drain::Range<unsigned short> dataset;
	drain::Range<unsigned short> data;
	// This was under consideration
	// mutable drain::RegExp pathRegExp;


	/// Regular expression of accepted PolarODIM what::quantity, for example "DBZ.?" .
	std::string quantity;
	//NEW /? std::string quantityStr;

	/// The (minimum) index of the key in the list of matching keys.
	unsigned int index;

	/// The maximum length of the list of matching keys.
	unsigned int count;

	/// The minimum and maximum elevation angle (applicable with volume scan data only).
	drain::Range<double> elangle;

	ODIMPathElem::group_t groups;

	//bool isValidPath(const ODIMPath & path) const;

	// Data criteria

	/// Check if metadata matches.
	// bool isValidData(const drain::ReferenceMap & properties) const ;

	/// Sets parameters and tries to guess additional filter
	/**
	 *   Tries to guess additional filter for paths returned with getPathsNEW().
	 *
	 *   \return - bitmask a combination of ODIMPathElem::DATASET, ODIMPathElem::DATA, and ODIMPathElem::QUALITY.
	 */
	ODIMPathElem::group_t resetParameters(const std::string & parameters);

	/// Restore default values.
	void reset();

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
	 *   \see resetParameters().
	 */
	// Recommended convenience function.
	template <class T>
	inline
	void getPathsNEW(const HI5TREE & src, T & pathContainer, ODIMPathElem::group_t groupFilter) const {
		getPathsNEW(src, pathContainer, drain::RegExp(quantity), groupFilter, ODIMPath());
	}

	template <class T>
	inline
	void getPathsNEW(const HI5TREE & src, T & pathContainer) const {
		getPathsNEW(src, pathContainer, groups);
	}


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
	template <class T>
	bool getPathsNEW(const HI5TREE & src, T & pathContainer, const drain::RegExp & quantityRE, ODIMPathElem::group_t groupFilter, const ODIMPath & path ) const;
	/*
	bool getPathsNEW(const HI5TREE & src, T & pathContainer, const drain::RegExp & quantityRE, int groupFilter = (ODIMPathElem::DATA|ODIMPathElem::DATASET),
			const ODIMPath & path = ODIMPath(ODIMPathElem()) ) const;
	*/

	/// Returns the first path encountered with selector attributes and given groupFilter .
	bool getPathNEW(const HI5TREE & src, ODIMPath & path, ODIMPathElem::group_t groupFilter = (ODIMPathElem::DATA|ODIMPathElem::DATASET) ) const;

	/// Returns the last path encountered with selector attributes and given groupFilter .
	bool getLastPath(const HI5TREE & src, ODIMPath & path, ODIMPathElem::group_t group = ODIMPathElem::DATA ) const;

	/// Returns the a path with index one greater than the retrieved last path.
	bool getNextPath(const HI5TREE & src, ODIMPath & path, ODIMPathElem::group_t group = ODIMPathElem::DATA ) const;


	/// Searches the children of child.getType(), or g if given, and stores the one with largest index.
	static
	bool getLastChild(const HI5TREE & tree, ODIMPathElem & child); //, (ODIMPathElem::group_t g =  ODIMPathElem::NONE);

	/// Searches children of given type, returns a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getNewChild(const HI5TREE & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax=0xff);

	/// Derive a child with index one greater than the largest index encountered.
	/**
	 *  \param tree - parent structure
	 *  \param child - child to be set index = last+1, hence 1 if none found.
	 *  \return - true if child's index was incremented (ie.)
	 */
	static
	bool getNextChild(const HI5TREE & tree, ODIMPathElem & child);

	//static 	bool getNextDescendant(HI5TREE & tree, (ODIMPathElem::group_t g, ODIMPath & path);

	/// Searches children of given type, returns a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getChildren(const HI5TREE & tree, std::map<std::string,ODIMPathElem> & children, ODIMPathElem::group_t groups);


	/// Temporary fix: try to derive dataset and data indices from path regexp.
	/**
	 *   Variable 'path' will be probably obsolete in future.
	 */
	void updatePaths();

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
	static inline
	void getPathsByElevation(const HI5TREE &src, const DataSelector & selector, std::map<double, ODIMPath> & m){
		//getPathsT(src, selector, m);
		selector.getPathsNEW(src, m, ODIMPathElem::DATASET); // TODO check
	}


	static inline
	void getPathsByQuantity(const HI5TREE &src, const DataSelector & selector, std::map<std::string, ODIMPath> & m){
		//getPathsT(src, selector, m);
		selector.getPathsNEW(src, m, ODIMPathElem::DATASET); // TODO check
	}


	/// Returns data paths, mapped by elevation.
	//static 	void getPaths(const HI5TREE &src, std::map<double,std::string> & path);

	/// Convenience function. Often only o ne path is return in the list, or only the first path is needed.
	/*
	static
	inline
	bool getPath(const HI5TREE &src, const DataSelector & selector, std::string & path){
		std::list<ODIMPath> l;
		getPaths(src, selector, l);
		if (l.empty()){
			path.clear();
			return false;
		}
		else {
			path = *l.begin();
			return true;
		}
	}
	*/



	// TODO ? bool getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path, int & index){

	/// Finds the path that is numerically greatest with respect to the last numeric
	/*
	 *
	 *  \return - true, if a path was found with the given selector.
	 */
	// static	bool getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & basePath, int & index);

	/// A shortcut.
	/*
	 */
	// static bool getLastOrdinalPath(const HI5TREE &src, const std::string & pathRegexp, std::string & path);


	/// Detect the last path accepted by regexp and increment the trailing numeral (eg. data2 => data3) and return the std::string in \c path.
	/**
	 * \param src - hdf tree to be searched
	 * \param pathRegExp - parent path(s) defined as regular expressions, for example:
	 * - \c /dataset[0-9]$
	 * - \c /dataset2/data[0-9]$
	 * - \c data[0-9]$
	 * - \c data2/quality[0-9]$
	 * \param path - std::string in which the result is stored, if found.
	 * \return - true, if a path was found
	 *
	 *  Notice that \c path can be initialized with a default value; if no path is found, the default value will stay intact.
	 */
	/*
	static
	inline
	bool getNextOrdinalPath(const HI5TREE &src, const std::string & pathRegExp, std::string & path){
		DataSelector s;
		s.path = pathRegExp;
		return getNextOrdinalPath(src, s, path);
	}
	*/

	/// A shortcut
	/*
	 *
	 */
	// static 	bool getNextOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path);



protected:

	//drain::ReferenceMap _parameters;

	/// Sets the default values.
	void init();

	/// Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	/**
	 *   Applicable for std::list<ODIMPath> and std::map<double,std::string>.
	 *   \param container - std::list<ODIMPath> and std::map<double,std::string>
	 */
	template <class T>
	static
	inline
	void getPathsT(const HI5TREE &src, const DataSelector & selector, T & container){
		getPaths(src, container, selector.path, selector.quantity,
				selector.index, selector.count,
				selector.elangle.min, selector.elangle.max); // min, max
	}



	/*
	template <class F, class T>
	inline
	void reference(const std::string & key, F & target, const T & value){
		parameters.reference(key, target, value);
	}
	 */

	/*
	template <class F>
	inline
	void reference(const std::string & key, F & target){
		parameters.reference(key, target);
	}
	 */

	/// NEW Collects paths to a list.
	template <class P>
	static
	void addPathT(std::list<P> & l, const PolarODIM & odim, const P & path){ l.push_back(path); }


	/// Collects paths to a list.
	/*
	static
	inline
	void addPathT(std::list<ODIMPath> & l, const PolarODIM & odim, const std::string &path){ l.push_back(path); }; // discards elevation
	 */

	/// Collects paths to a set. (unused?)
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	template <class P>
	static inline
	void addPathT(std::set<P> & l, const PolarODIM & odim, const P & path){
		l.insert(path);
	}
	/*
	static
	inline
	void addPathT(std::set<std::string> & s, const PolarODIM & odim, const std::string &path){ s.insert(path); }; // discards elevation
	 */

	/// Collects paths by their elevation angle (elangle).
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	template <class P>
	static inline
	void addPathT(std::map<double,P> & m, const PolarODIM & odim, const std::string & path){
		m[odim.elangle] = path;
	}


	/// Collects paths by their quantity, eg. DBZH, VRAD, and RHOHV.
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	template <class P>
	static inline
	void addPathT(std::map<std::string,P> & m, const PolarODIM & odim, const std::string & path){
		m[odim.quantity] = path;
	}

	template <class P>
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	static inline
	bool addPathT(std::list<P> & l, const std::string &path){
		l.push_back(path);
		return false;// ??
	} // ??


	/// Add a std::string. Search ends with the first entry encountered. (?)
	static inline
	bool addPathT(std::string & str, const std::string &path){
		str.assign(path);
		return true;  // ??
	};

};


template <class T>
bool DataSelector::getPathsNEW(const HI5TREE &src, T & pathContainer, const drain::RegExp & quantityRE, ODIMPathElem::group_t groupFilter, const ODIMPath & path) const {

	drain::Logger mout(getName(), __FUNCTION__);

	mout.debug(3) << "considering '" << path << "/' -> ..." << mout.endl;

	// ODIM struct is needed to communicate keys for maps (elangle or quantity) to addPathT
	PolarODIM odim;

	if (path.empty())
		mout.debug() << "empty path (ok)"  << mout.endl;
	else if (!src.hasPath(path)){
		mout.warn() << "data structure has no path:"  << path << mout.endl;
		return false;
	}

	// Current search point
	const HI5TREE & s = src(path); // =src, if path empty


	// Selector contains a rule for desired quantity. (This also indicates that quantityRE is set).
	const bool QUANTITY_MATCH_REQUIRED = !quantity.empty();


	// Changed to true, if a descendant contains the desired quantity.
	bool groupMatch = false;

	//const bool IS_ROOT = (path.size() == 1);
	std::set<ODIMPathElem> datasets;

	for (HI5TREE::const_iterator it = s.begin(); it != s.end(); ++it) {

		ODIMPathElem child(it->first);
		mout.debug(3) << "*it='" << it->first << "' (" << it->first.group << "),\t child=" << "'" << child <<"' (" << child.group <<  ") include=" << child.belongsTo(groupFilter) << mout.endl;
		bool quantityMatch = false;


		if (child.is(ODIMPathElem::DATASET)){ // what about quality?
			if (!dataset.isInside(child.index)){
				mout.debug(3) << "dataset " << child.index << " not in [" <<  dataset << "], skipping" << mout.endl;
				continue;
			}
		}
		else if (child.is(ODIMPathElem::DATA)){ // what about quality?
			if (!data.isInside(child.index)){
				mout.debug(3) << "data " << child.index << " not in [" <<  data << "], skipping" << mout.endl;
				continue;
			}
		}

		const hi5::NodeHi5 & node = it->second.data;

		// Currently, there is no switch to exclude/include no-save (temporary) structures.
		if (node.noSave){
			mout.debug(3) << "noSave data, ok: " << it->first << mout.endl;
			//continue;
		}


		// In indexed DATASET/QUALITY/DATA groups, images are the metadata containers
		const drain::image::Image & d = node.dataSet;
		//mout.note() << "reconsidering " << child << ':' << d.properties["what:quantity"] << mout.endl;

		if (child.is(ODIMPathElem::DATASET)){ // what about quality?
			if (d.properties.hasKey("where:elangle")){
				if (!elangle.isInside(d.properties["where:elangle"])){
					mout.debug() << "outside elangle range"<< mout.endl;
					continue;
				}
			}
		}

		//if (child.is(ODIMPathElem::DATA) || (child.is(ODIMPathElem::QUALITY) &&  child.belongsTo(groupFilter)) ){
		if (QUANTITY_MATCH_REQUIRED){

			if (child.is(ODIMPathElem::ARRAY)){
				if (groupFilter & ODIMPathElem::ARRAY){
					mout.warn() << "path " << path << '/' << child << " will not be detected; ARRAY does not support quantity (request=" << quantity << ")" << mout.endl;
				}
			}

			if (child.is(ODIMPathElem::DATA) || child.is(ODIMPathElem::QUALITY) ){

				if (d.properties.hasKey("what:quantity")){

					quantityMatch = quantityRE.test(d.properties["what:quantity"]);
					if (quantityMatch){
						mout.debug() << it->first << ":\t found quantity '" << d.properties["what:quantity"] << "'" << mout.endl;
						groupMatch = true;
					}
					else {
						mout.debug(1) << it->first << ":\t quantity '" << quantity << "' !~ '" << d.properties["what:quantity"] << "'" << mout.endl;
						// allow sub-QIND, so do not: continue;
					}
				}
				else {
					if (child.belongsTo(groupFilter))
						mout.warn() << "testing '" << quantity << "': no what:quantity in " << path << '/' << it->first  << mout.endl;
					// else check  child.is(ODIMPathElem::QUALITY) and QIND?
				}
			}
		}



		ODIMPath p(path);
		p << child;

		/// Recursion: traverse descendants
		//mout.warn() << "descending to " << p << mout.endl;
		T descendantPaths;
		const bool descendantMatch = getPathsNEW(src, descendantPaths, quantityRE, groupFilter, p); // note: original "root" src

		if (descendantMatch)
			groupMatch = true;


		if (child.belongsTo(groupFilter)){ // If quantity match requiredAccept DATASET i

			bool accept = !QUANTITY_MATCH_REQUIRED;
			accept = accept || ((child.is(ODIMPathElem::DATA) || child.is(ODIMPathElem::QUALITY)) && quantityMatch);
			accept = accept || (child.is(ODIMPathElem::DATASET) && descendantMatch);

			if (accept){

				// DATASET counter
				// Otherways accepted now, only check the counter. Counter must not be incremented before this point.
				if (child.is(ODIMPathElem::DATASET)){
					datasets.insert(child);
					mout.debug(3) << "now dataset count=" << datasets.size() << " latest=" << child << mout.endl;
					if (datasets.size() > count){
						mout.debug(3) << " count=" << count << " exceeded, returning" << mout.endl;
						return true; // = dump this child and its descendants
					}
				}

				mout.debug(3) << "accepted " << p << mout.endl;
				odim.clear();
				odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type
				addPathT(pathContainer, odim, p);

			}


		}

		// Append descendant paths now (all have been accepted already)
		for (typename T::const_iterator it = descendantPaths.begin(); it != descendantPaths.end(); ++it){
			pathContainer.insert(pathContainer.end(), *it);
		}

	}

	return groupMatch;
}


/*
template <class T>
void DataSelector::getPaths(const HI5TREE &src, T & container, const std::string & path, const std::string &quantity,
		unsigned int index, unsigned int count, double elangleMin, double elangleMax) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	//const drain::RegExp quantityRE(std::string("^")+quantity+std::string("$"));
	const drain::RegExp quantityRE(quantity);

	std::list<ODIMPath> l0;

	/// Step 1: get paths that match pathRegExp TreeT
	mout.debug(10) << "getKeys " << path << mout.endl;
	src.getKeys(l0, path);
	//src.getKeys(l0, pathRegExp);

	std::set<std::string> roots;
	unsigned int counter = 0; // (this was needed as count would go -1 otherways below)

	/// Step 2: add paths that match path and quantity regexps and are with elevation limits.
	PolarODIM odim;
	for (std::list<ODIMPath>::iterator it = l0.begin(); it != l0.end(); ++it) {

		//mout.debug(2) << *it << mout.endl;
		const hi5::NodeHi5 & node = src(*it).data;
		if (node.noSave){
			mout.debug() << "noSave data, ok: " << *it << mout.endl;
			//continue;
		}

		const drain::image::Image & d = node.dataSet;
		odim.clear();
		odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type
		// odim.set() would be bad! Looses precision in RefMap/Castable << (std::string) << Variable

		if (!quantityRE.test(odim.quantity)){
			mout.debug(8) << *it << "\n\t quantity '" << quantityRE.toStr() << "' !~ '" << odim.quantity << "'" << mout.endl;
			// l.erase(it2);
			continue;
		}

		if ((odim.elangle < elangleMin) || (odim.elangle > elangleMax))
		{
			mout.debug(8) << "elangle inapplicable " << odim.elangle << mout.endl;
			// l.erase(it2);
			continue;
		}

		// Outside index check, because mostly applied by count check as well.
		const std::string root = it->substr(0, it->find('/', 1));  // typically dataset1/

		if (index > 0){ // "Log mode on"

			// mout.warn() << "index studying " << *it << '\t' << root << mout.endl;
			if (roots.find(root) == roots.end()){ // = new
				--index;
				if (index == 0){ // done!
					// mout.warn() << "index ACCEPTED " << *it << '\t' << root << mout.endl;
					roots.clear(); // re-use for count (below)
				}
				else {
					roots.insert(root);
					continue;
				}
			}
			else
				continue;
		}


		if (roots.find(root) == roots.end()){ // = not already in the set
			++counter;
			if (counter > count)
				return;
			roots.insert(root);
		}


		// mout.warn() << "counter(" << counter << ") ACCEPTED " << *it << '\t' << root << mout.endl;
		addPathT(container, odim, *it);

	}

}
*/



/*
template <class T>
bool DataSelector::getQualityPaths(const HI5TREE & srcRoot, const ODIMPath & datapath, T & qualityPaths) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	std::list<ODIMPath> paths;
	srcRoot.getPaths(paths);

	bool result;

	for (std::list<ODIMPath>::const_iterator it = paths.begin(); it != paths.end(); it++){
		if (it->back().is(ODIMPathElem::QUALITY)){
			ODIMPath p = *it;
			p << ODIMPathElem(ODIMPathElem::ARRAY);
			if (addPathT(qualityPaths, p))
				result = true;
		}
	}

	return result;
	//return (qualityPaths.size() > 0);
}
*/

/*
bool DataSelector::getQualityPaths(const HI5TREE & srcRoot, const ODIMPath & datapath, T & qualityPaths) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	mout.debug(1) << datapath << mout.endl;

	const size_t lStart = datapath[0]=='/' ? 1 : 0;

	size_t l = datapath.length();

	while (l > lStart) {

		const std::string parentPath = datapath.substr(lStart, l);

		mout.debug(12) << "checking path: " << parentPath << mout.endl;
		const HI5TREE & src = srcRoot(parentPath);

		/// Iterate children (for the srcRoot[datapath], actually its siblings)
		for (HI5TREE::const_iterator it = src.begin(); it != src.end(); it++){
			mout.debug(14) << "checking subpath: " << it->first << mout.endl;
			if (it->second["what"].data.attributes["quantity"].toStr() == "QIND" ){  // QIND
				mout.debug(5) << "found:" << it->first << mout.endl;
				//qualityPaths.push_back(parentPath + '/' + it->first + "/data");
				if (addPathT(qualityPaths, parentPath + '/' + it->first + "/data"))
					return true;
				// returns if std::string, continues if list
			}
		}

		l = datapath.rfind('/', l-1);
		if (l == std::string::npos)
			l = lStart;
	}

	return (qualityPaths.size() > 0);
}
*/

inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters();
	return ostr;
}

} // rack::

#endif /* DATASELECTOR_H_ */

// Rack
 // REP // REP // REP // REP // REP // REP // REP // REP // REP
 // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP // REP
