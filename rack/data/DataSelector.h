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


	/// Regular expression of accepted PolarODIM what::quantity, for example "DBZ.?" .
	std::string quantity;

	// Path criteria
	drain::Range<unsigned short> dataset;
	drain::Range<unsigned short> data;

	/// The (minimum) index of the key in the list of matching keys.
	unsigned int index;

	/// The maximum length of the list of matching keys.
	unsigned int count;

	/// The minimum and maximum elevation angle (applicable with volume scan data only).
	drain::Range<double> elangle;

	drain::Flags groups;

protected:

	std::string  groupStr; // converted to 'groups' with update.

	virtual
	void update();




public:

	/// Regular expression of accepted paths, for example ".*/data$". Deprecated
	/**
	 *  \deprecated Use \c dataset and \c data parameters instead
	 */
	std::string path;


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
	template <class T>
	inline
	void getPaths(const Hi5Tree & src, T & pathContainer, ODIMPathElem::group_t groupFilter) const {
		getPaths(src, pathContainer, drain::RegExp(quantity), groupFilter, ODIMPath());
	}

	template <class T>
	inline
	void getPaths(const Hi5Tree & src, T & pathContainer) const {

		if (groups.value > 0){
			getPaths(src, pathContainer, groups.value);
		}
		else {
			drain::Logger mout(getName(), __FUNCTION__);
			mout.note() << "groups flag unset, using DATA + DATASET" << mout.endl;
			getPaths(src, pathContainer, (ODIMPathElem::DATA | ODIMPathElem::DATASET));
			//groups = groupStr;
		}

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
	bool getPaths(const Hi5Tree & src, T & pathContainer, const drain::RegExp & quantityRE, ODIMPathElem::group_t groupFilter, const ODIMPath & path ) const;


	/// Returns the first path encountered with selector attributes and given groupFilter .
	bool getPathNEW(const Hi5Tree & src, ODIMPath & path, ODIMPathElem::group_t groupFilter) const;

	/// Returns the first path encountered with selector attributes and given groupFilter .
	inline
	bool getPathNEW(const Hi5Tree & src, ODIMPath & path) const {

		if (groups.value > 0){ // if (groupStr.empty())
			return getPathNEW(src, path, groups.value);
		}
		else {
			drain::Logger mout(getName(), __FUNCTION__);
			mout.note() << "groups flag unset, using DATA + DATASET" << mout.endl;
			return getPathNEW(src, path, (ODIMPathElem::DATA | ODIMPathElem::DATASET));
		}

	}

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
	static inline
	void getPathsByElevation(const Hi5Tree &src, const DataSelector & selector, std::map<double, ODIMPath> & m){
		//getPathsT(src, selector, m);
		selector.getPaths(src, m, ODIMPathElem::DATASET); // TODO check
	}


	static inline
	void getPathsByQuantity(const Hi5Tree &src, const DataSelector & selector, std::map<std::string, ODIMPath> & m){
		//getPathsT(src, selector, m);
		selector.getPaths(src, m, ODIMPathElem::DATASET); // TODO check
	}



	/// Temporary fix: try to derive dataset and data indices from path regexp.
	/**
	 *   Variable 'path' will be probably obsolete in future.
	 */
	void convertRegExpToRanges(const std::string & param);

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
	template <class T>
	static
	inline
	void getPathsT(const Hi5Tree &src, const DataSelector & selector, T & container){
		getPaths(src, container, selector.path, selector.quantity,
				selector.index, selector.count,
				selector.elangle.min, selector.elangle.max); // min, max
	}


	/// NEW Collects paths to a list.
	template <class P>
	static
	void addPathT(std::list<P> & l, const PolarODIM & odim, const P & path){ l.push_back(path); }


	/// Collects paths to a set. (unused?)
	/**
	 *  \tparam P - list type: ODIMPath (preferred) or std::string
	 */
	template <class P>
	static inline
	void addPathT(std::set<P> & l, const PolarODIM & odim, const P & path){
		l.insert(path);
	}


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
bool DataSelector::getPaths(const Hi5Tree &src, T & pathContainer, const drain::RegExp & quantityRE, ODIMPathElem::group_t groupFilter, const ODIMPath & path) const {

	drain::Logger mout(getName(), __FUNCTION__);

	mout.debug(3) << "considering '" << path << "/' -> ..." << mout.endl;

	// ODIM struct is needed to communicate keys for maps (elangle or quantity) to addPathT
	PolarODIM odim;

	if (path.empty())
		mout.debug(3) << "empty path (ok)"  << mout.endl;
	else if (!src.hasPath(path)){
		mout.warn() << "data structure has no path:"  << path << mout.endl;
		return false;
	}

	// Current search point
	const Hi5Tree & s = src(path); // =src, if path empty


	// Selector contains a rule for desired quantity. (This also indicates that quantityRE is set).
	const bool QUANTITY_CONSTRAINT = !quantity.empty();


	// Changed to true, if a descendant contains the desired quantity.
	bool quantityGroupOK = QUANTITY_CONSTRAINT ? false : true;

	//const bool IS_ROOT = (path.size() == 1);
	std::set<ODIMPathElem> datasets;

	for (Hi5Tree::const_iterator it = s.begin(); it != s.end(); ++it) {

		//ODIMPathElem child(it->first);
		const ODIMPathElem & currentElem = it->first;
		//mout.debug(3) << "*it='" << it->first << "' (" << it->first.group << "),\t currentElem=" << "'" << currentElem <<"' (" << currentElem.group <<  ") include=" << currentElem.belongsTo(groupFilter) << mout.endl;
		mout.debug(3) << "currentElem=" << "'" << currentElem <<"' (" << currentElem.group <<  ") incl=" << currentElem.belongsTo(groupFilter) << mout.endl;


		if (currentElem.is(ODIMPathElem::DATASET)){ // what about quality?
			if (!dataset.contains(currentElem.index)){
				mout.debug(3) << "dataset " << currentElem.index << " not in [" <<  dataset << "], skipping" << mout.endl;
				continue;
			}
		}
		else if (currentElem.is(ODIMPathElem::DATA)){ // what about quality?
			if (!data.contains(currentElem.index)){
				mout.debug(3) << "data " << currentElem.index << " not in [" <<  data << "], skipping" << mout.endl;
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
		//mout.note() << "reconsidering " << currentElem << ':' << d.properties["what:quantity"] << mout.endl;

		if (currentElem.is(ODIMPathElem::DATASET)){ // what about quality?
			if (d.properties.hasKey("where:elangle")){
				if (!elangle.contains(d.properties["where:elangle"])){
					mout.debug() << "outside elangle range"<< mout.endl;
					continue;
				}
			}
		}

		bool quantityOK = true;
		if (QUANTITY_CONSTRAINT){

			// IS this check needed (here)? Consider above: QUANTITY_CONSTRAINT && (currentElem.is(ODIMPathElem::DATA) || currentElem.is(ODIMPathElem::QUALITY) )
			if (currentElem.is(ODIMPathElem::ARRAY)){
				if (groupFilter & ODIMPathElem::ARRAY){
					mout.warn() << "path " << path << '/' << currentElem << " will not be detected; ARRAY does not support quantity (request=" << quantity << ")" << mout.endl;
				}
			}

			if (currentElem.is(ODIMPathElem::DATA) || currentElem.is(ODIMPathElem::QUALITY) ){

				if (d.properties.hasKey("what:quantity")){

					quantityOK = quantityRE.test(d.properties["what:quantity"]);
					if (quantityOK){
						quantityGroupOK = true;
						mout.debug(1) << it->first << ":\t found quantity '" << d.properties["what:quantity"] << "'" << mout.endl;
					}
					else {
						mout.debug(2) << it->first << ":\t quantity '" << quantity << "' !~ '" << d.properties["what:quantity"] << "'" << mout.endl;
						// allow sub-QIND, so do not: continue;
					}
				}
				else {
					if (currentElem.belongsTo(groupFilter))
						mout.warn() << "testing '" << quantity << "': no what:quantity in " << path << '/' << it->first  << mout.endl;
					// else check  currentElem.is(ODIMPathElem::QUALITY) and QIND?
				}
			}
		}



		ODIMPath p(path);
		p << currentElem;

		/// Recursion: traverse descendants. Note: only quantities may be checked (and zero paths returned)
		//mout.warn() << "descending to " << p << mout.endl;
		T descendantPaths;
		const bool quantitySubtreeOK = getPaths(src, descendantPaths, quantityRE, groupFilter, p) ; // note: original "root" src

		if (quantitySubtreeOK)
			quantityGroupOK = true;


		/// Accept (elevation and quantities)
		bool accept = true;

		if (currentElem.belongsTo(ODIMPathElem::DATA | ODIMPathElem::QUALITY)){
			if (!quantityOK)
				accept = false;
		}
		else if (currentElem.is(ODIMPathElem::DATASET)){
			if (!quantitySubtreeOK)
				accept = false;
		}

		if (accept){

			// COUNTER. Otherways accepted now, only check the counter. Counter must not be incremented before this point.
			if (currentElem.is(ODIMPathElem::DATASET)){
				datasets.insert(currentElem);
				mout.debug(2) << "now dataset count=" << datasets.size() << " latest=" << currentElem << mout.endl;
				if (datasets.size() > count){
					mout.debug(1) << " count=" << count << " exceeded, returning" << mout.endl;
					return true; // = dump this currentElem and its descendants
				}
			}

			if (currentElem.belongsTo(groupFilter)){
				mout.debug(2) << "accepted " << p << mout.endl;
				odim.clear();
				odim.copyFrom(d);  // OK, uses true type ie. full precision, also handles img type
				addPathT(pathContainer, odim, p);
				mout.debug(3) << "pathContainer size=" <<  pathContainer.size() << mout.endl;
			}
		}


		// Note: all the descendantPaths (if exist) are confirmed already in the inner call.
		mout.debug(3) << "adding descendants: " << descendantPaths.size() << mout.endl;

		// Append descendant paths now (all have been accepted already)
		for (typename T::const_iterator it = descendantPaths.begin(); it != descendantPaths.end(); ++it){
			pathContainer.insert(pathContainer.end(), *it);
			//pathContainer.insert(*it);
		}

		//mout.warn() << "pathContainer size=" <<  pathContainer.size() << mout.endl;

	}

	return quantityGroupOK;
}


inline
std::ostream & operator<<(std::ostream & ostr, const DataSelector &selector){
	ostr << selector.getParameters();
	return ostr;
}

} // rack::

#endif /* DATASELECTOR_H_ */
