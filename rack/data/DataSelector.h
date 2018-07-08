/*

    Copyright 2012-  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack for C++.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef DATASELECTOR_H_
#define DATASELECTOR_H_

#include <set>
#include <list>
#include <map>

#include <drain/util/BeanLike.h>

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

	// Under consideration
	// mutable drain::RegExp pathRegExp;

	/// Regular expression of accepted PolarODIM what::quantity, for example "DBZ.?" .
	std::string quantity;

	/// The (minimum) index of the key in the list of matching keys.
	unsigned int index;

	/// The maximum length of the list of matching keys.
	unsigned int count;

	/// The minimum elevation angle (when applicable).
	//double elangleMin;

	/// The maximum elevation angle (when applicable).
	//double elangleMax;

	std::vector<double> elangle;

	/// Sets the default values.
	void reset();

	/// Sets this selector similar to given selector.
	// void copy(const DataSelector & selector);

	/// Sets parameters in predefined order or sets specified parameters. (Python style calling alternatives.)
	//  *   - \c index - integer value, only changing the \c index member. ???
	/**
	 *  \param parameters - the parameters to be changed, using some of the syntaxes:
	 *   - \c path,quantity,index,count>,elangleMin,elangleMax (complete or partial list of parameter values, in this order)
	 *   - \c parameter1=value1,parameter2=value2,parameter3=value3   (specific assignments as a list)
	 *   The regular expressions should not contain comma (,).
	 *   \param specific - if true, the specific assignments are supported
	 */
	//void setParameters(const std::string &parameters, bool specific = true){ parameters.copy(p, specific); };
	// DataSelector & setParameters(const std::string &parameters, bool allowSpecific = true);

	/// Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	/**
	 *   Applicable for list\<string\> and std::map<double,std::string>.
	 */
	template <class T>
	static
	void getPaths(const HI5TREE & src, T & container, const std::string & path="", const std::string & quantity="",
			unsigned int index=0, unsigned int count = 1000,
			double elangleMin = -90.0, double elangleMax = 180.0);


	/// Traverses down the tree and returns matching paths as a list.
	/**
	 *   \param container - std::list<std::string> and std::map<double,std::string>
	 *
	 *   In the order of appearance in the hdf5 structure.
	 */
	//template <class T>
	static
	inline
	void getPaths(const HI5TREE &src, const DataSelector & selector, std::list<std::string> & l){
		getPathsT(src, selector, l);
	}

	/// Traverses down the tree and returns matching paths as a list.
	/**
	 *   \param container - std::set<std::string>
	 *
	 *   In the order of appearance in the hdf5 structure.
	 */
	static
	inline
	void getPaths(const HI5TREE &src, const DataSelector & selector, std::set<std::string> & s){
		getPathsT(src, selector, s);
	}

	/// Traverses down the tree and returns matching paths as a map with elevations as keys.
	/**
	 *   \param m
	 */
	//template <class T>
	static
	inline
	void getPathsByElevation(const HI5TREE &src, const DataSelector & selector, std::map<double,std::string> & m){
		getPathsT(src, selector, m);
	}


	static
	inline
	void getPathsByQuantity(const HI5TREE &src, const DataSelector & selector, std::map<std::string,std::string> & m){
		getPathsT(src, selector, m);
	}


	/// Returns data paths, mapped by elevation.
	static
	void getPaths(const HI5TREE &src, std::map<double,std::string> & path);

	/// Convenience function. Often only o ne path is return in the list, or only the first path is needed.
	static
	inline
	bool getPath(const HI5TREE &src, const DataSelector & selector, std::string & path){
		std::list<std::string> l;
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



	// TODO ? bool getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path, int & index){

	/// Finds the path that is numerically greatest with respect to the last numeric
	/*
	 *
	 *  \return - true, if a path was found with the given selector.
	 */
	static
	bool getLastOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & basePath, int & index);

	/// A shortcut.
	/*
	 */
	static
	bool getLastOrdinalPath(const HI5TREE &src, const std::string & pathRegexp, std::string & path);


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
	static
	inline
	bool getNextOrdinalPath(const HI5TREE &src, const std::string & pathRegExp, std::string & path){
		DataSelector s;
		s.path = pathRegExp;
		return getNextOrdinalPath(src, s, path);
	}

	/// A shortcut
	/*
	 *
	 */
	static
	bool getNextOrdinalPath(const HI5TREE &src, const DataSelector & selector, std::string & path);



	static
	inline
	const hi5::NodeHi5 & getNode(const HI5TREE & src, const DataSelector & selector){
		std::string path;
		if (getPath(src, selector, path))
			return src(path).data;
		else {
			static const hi5::NodeHi5 _empty;
			return _empty;
		}
	}

	static
	inline
	hi5::NodeHi5 & getNode(HI5TREE &src, const DataSelector & selector){
		std::string path;
		if (getPath(src, selector, path))
			return src(path).data;
		else {
			static hi5::NodeHi5 _empty;
			return _empty;
		}
	}


	static
	inline
	const drain::image::Image & getData(const HI5TREE &src, const DataSelector & selector){
		return getNode(src, selector).dataSet;
		/*
		std::string path;
		if (getPath(src,  selector, path))
			return src[path].data.dataSet;
		else {
			static const drain::image::Image _empty;
			return _empty;
		}
		*/
	}

	static
	inline
	drain::image::Image & getData(HI5TREE &src, const DataSelector & selector){
		return getNode(src, selector).dataSet;
		/*
		std::string path;
		if (getPath(src,  selector, path))
			return src[path].data.dataSet;
		else {
			static drain::image::Image _empty;
			return _empty;
		}
		*/
	}

	/// Given a path to data, tries to find the respective "quality1" field. Currently, does not check quantity.
	/**
	 *  \param dataPath - start address, to be ascended
	 *  \param qualityPaths  - container for the result(s);  std::string or std::list<std::string>
	 *  \return - true, if at least one path has been found
	 */
	template <class T>
	static
	bool getQualityPaths(const HI5TREE &srcRoot, const std::string & datapath, T & qualityPaths);

	/// Returns the path to associated quality data.
	/**
	    \return empty string, if no quality data found.
	 */
	static
	inline
	bool getQualityPath(HI5TREE &srcGroup, std::string & path){
		std::string qualityPath;
		if (getQualityPaths(srcGroup, path, qualityPath)){
			path = qualityPath;
			return true;
		}
		else {
			path.clear();
			return false;
		}
	}


	static
	inline
	const drain::image::Image & getQualityData(const HI5TREE &srcGroup, const std::string & dataPath){
		static const drain::image::Image _empty;
		//std::string qualityPath;
		std::list<std::string> l;
		getQualityPaths(srcGroup, dataPath, l);
		//if (getQualityPath(srcGroup, dataPath, qualityPath))
		if (l.size() > 0)
			return srcGroup[*l.begin()].data.dataSet;
		else
			return _empty;
	}

	static
	inline
	drain::image::Image & getQualityData(HI5TREE &srcGroup, const std::string & dataPath){
		static drain::image::Image _empty;
		std::string qualityPath;
		if (getQualityPaths(srcGroup, dataPath, qualityPath))
			return srcGroup[qualityPath].data.dataSet;
		else
			return _empty;
	}



	/*
	inline
	std::string getKeys() const {
		return _parameters.getKeys();
	}

	inline
	std::string getValues() const {
		return _parameters.getValues();
	}

	inline
	const drain::ReferenceMap & getParameters() const {
		return _parameters;
	}
	*/

protected:

	//drain::ReferenceMap _parameters;

	/// Sets the default values.
	void init();

	/// Traverses down the tree and returns matching paths as a list or map (ordered by elevation).
	/**
	 *   Applicable for std::list<std::string> and std::map<double,std::string>.
	 *   \param container - std::list<std::string> and std::map<double,std::string>
	 */
	template <class T>
	static
	inline
	void getPathsT(const HI5TREE &src, const DataSelector & selector, T & container){
		getPaths(src, container, selector.path, selector.quantity,
				selector.index, selector.count,
				selector.elangle[0], selector.elangle[1]); // min, max
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

	/// Collects paths to a list.
	static
	inline
	void addPathT(std::list<std::string> & l, const PolarODIM & odim, const std::string &path){ l.push_back(path); }; // discards elevation

	/// Collects paths to a set. (unused?)
	static
	inline
	void addPathT(std::set<std::string> & s, const PolarODIM & odim, const std::string &path){ s.insert(path); }; // discards elevation


	/// Collects paths by their elevation angle (elangle).
	static
	inline
	void addPathT(std::map<double,std::string> & m, const PolarODIM & odim, const std::string & path){ m[odim.elangle] = path; };

	/// Collects paths by their quantity, eg. DBZH, VRAD, and RHOHV.
	static
	inline
	void addPathT(std::map<std::string,std::string> & m, const PolarODIM & odim, const std::string & path){ m[odim.quantity] = path; };


	static
	inline
	bool addPathT(std::list<std::string> & l, const std::string &path){ l.push_back(path); return false; };  // ??

	/// Add a std::string. Search ends with the first entry encountered.
	static
	inline
	bool addPathT(std::string & str, const std::string &path){ str.assign(path); return true; };  // ??

};

template <class T>
void DataSelector::getPaths(const HI5TREE &src, T & container, const std::string & path, const std::string &quantity,
		unsigned int index, unsigned int count, double elangleMin, double elangleMax) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	//const drain::RegExp quantityRE(std::string("^")+quantity+std::string("$"));
	const drain::RegExp quantityRE(quantity);

	std::list<std::string> l0;

	/// Step 1: get paths that match pathRegExp TreeT
	mout.debug(10) << "getKeys " << path << mout.endl;
	src.getKeys(l0, path);
	//src.getKeys(l0, pathRegExp);

	std::set<std::string> roots;
	unsigned int counter = 0; // (this was needed as count would go -1 otherways below)

	/// Step 2: add paths that match path and quantity regexps and are with elevation limits.
	PolarODIM odim;
	for (std::list<std::string>::iterator it = l0.begin(); it != l0.end(); ++it) {

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





template <class T>
bool DataSelector::getQualityPaths(const HI5TREE & srcRoot, const std::string & datapath, T & qualityPaths) {

	drain::Logger mout("DataSelector", __FUNCTION__);

	mout.debug(1) << datapath << mout.endl;

	//if (qpath.empty())
	//	return false;

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

inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters();
	return ostr;
}

} // rack::

#endif /* DATASELECTOR_H_ */
