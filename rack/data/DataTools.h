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

#ifndef DATATOOLS_H_
#define DATATOOLS_H_

#include <set>
#include <list>
#include <map>

#include <drain/util/BeanLike.h>

#include <drain/util/ReferenceMap.h>
#include <drain/util/RegExp.h>

#include <drain/util/Variable.h>

#include <drain/image/Coordinates.h>

#include "ODIM.h"
#include "PolarODIM.h" // elangle

#include "DataSelector.h" // Range


namespace rack {

/// Tool for selecting datasets based on paths, quantities and min/max elevations.
/**
 *
 *    Applies drain::RegExp in matching.
 */
class DataTools { //: public drain::BeanLike {

public:

	/// Traverses upward in hierachy to find the value of the given PolarODIM where, what, or how attribute.
	/**
	 *  \param src   - structure to be searched
	 *  \param path  - starting path, to be continued upwards
	 *  \param group - "what", "where", or "how"
	 *  \param attributeName - name of the attribute
	 */
	// static
	// const drain::Variable & getAttribute(const HI5TREE &src, const std::string & path, const std::string & group, const std::string & attributeName);

	/// Collects PolarODIM /where, /what and /how attributes recursively along the path and stores them in a std::map<std::string,T> (e.g. VariableMap or ReferenceMap, ODIM).
	/**
	 *   Does not change attributes of src.
	 *   \see updateAttributes()
	 */
	template <class M>
	static
	void getAttributes(const HI5TREE &src, const std::string & path, M & attributes, bool updateOnly = false);

	/// Traverses the whole h5 structure, updates the data (image) attributes along the path.
	/**!
	 *    Typically, this is called on the root.
	 *
	 */
	//const drain::image::CoordinatePolicy & policy = drain::image::CoordinatePolicy(),
	static
	void updateAttributes(HI5TREE & src, const drain::VariableMap & attributes); // = drain::VariableMap()

	static inline
	void updateAttributes(HI5TREE & src){
		src.data.dataSet.properties.clear();
		updateAttributes(src, drain::VariableMap());
	}

	// Does nothing
	static inline
	void updateAttributes(const HI5TREE & src, const drain::VariableMap & attributes = drain::VariableMap()){
		drain::Logger mout("DataTools", __FUNCTION__);
		//mout.warn() << "somebody called me" << mout.endl;
	};

	/// Removes the children of the tree if Node::noSave is set.
	/**
	 *  \return - true if children were removed
	 */
	static
	bool removeIfNoSave(HI5TREE & dst);

	/// Does nothing
	/**
	 *  \return - false (always, as nothing will be removed)
	 */
	static
	bool removeIfNoSave(const HI5TREE & src){
		return false;
	};


	static
	void updateCoordinatePolicy(HI5TREE & src, const drain::image::CoordinatePolicy & policy = drain::image::CoordinatePolicy(drain::image::CoordinatePolicy::LIMIT));
	//void updateCoordinatePolicy(HI5TREE & src, const CoordinatePolicy & policy = CoordinatePolicy(CoordinatePolicy::LIMIT));
	/*
	static
	inline
	void updateAttributes(HI5TREE & src){
		_updateAttributes(src, drain::VariableMap());
	}

	static
	void _updateAttributes(HI5TREE & src, const drain::VariableMap & attributes);
     */


	/// Returns the path std::string one step upwards, ie. up to the preceding '/'.
	static inline
	// FIX: redesign with ODIMPath
	std::string getParent(const std::string &path) {
		return path.substr(0,path.rfind('/'));
	};

	static inline
	// FIX: redesign with ODIMPath
	void getParentAndChild(const std::string & path, std::string & parent, std::string & child) {
		//const size_t n = path.length();
		const size_t i = path.rfind('/');
		parent.assign(path,0,i);
		child.assign(path,i+1, path.length());
		//return path.substr(0,path.rfind('/'));
	};


	/// Returns the path std::string one step upwards, ie. up to the preceding '/'.
	// FIX: redesign with ODIMPath
	static inline
	std::string getChild(const std::string &path) {
		const size_t i = path.rfind('/');
		if (i != std::string::npos)
			return path.substr(i+1);
		else
			return std::string("");
	};



	/// Searches the children of child.getType(), or g if given, and stores the one with largest index.
	static
	bool getLastChild(HI5TREE & tree, ODIMPathElem & child); //, BaseODIM::group_t g =  BaseODIM::NONE);

	/// Within children of type child.group, return a non-existing child with index greater than child.index.
	/**
	 *   Unused indices may be returned.
	 */
	static
	bool getNewChild(HI5TREE & tree, ODIMPathElem & child, ODIMPathElem::index_t iMax=100);

	/// Derive a child with index one greater of the largest index encountered.
	/**
	 *  \param tree - parent structure
	 *  \param child - child to be set index = last+1, hence 1 if none found.
	 *  \return - true if child's index was incremented (ie.)
	 */
	static inline
	bool getNextChild(HI5TREE & tree, ODIMPathElem & child){

		if (getLastChild(tree, child)){
			++child.index;
			return true;
		}
		else {
			child.index = 1;
			return false;
		}
	}

	static inline
	bool getNextDescendant(HI5TREE & tree, BaseODIM::group_t g, ODIMPath & path){
		//path.clear()?
		if (BaseODIM::isIndexed(g)){
			ODIMPathElem parent(BaseODIM::DATASET);
			getLastChild(tree, parent);
			if (g == BaseODIM::DATASET){
				++parent.index;
				path.push_back(parent);
			}
			else {
				path.push_back(parent);

				ODIMPathElem child(g); // BaseODIM::DATA or BaseODIM::QUALITY
				getNextChild(tree[parent], child);
				path.push_back(child);
			}
			return true;
		}

		return false;

	}

protected:

};



template <class M>
void DataTools::getAttributes(const HI5TREE &src, const std::string & path, M & attributes, bool updateOnly){

	drain::Logger mout("DataTools", __FUNCTION__);

	//drain::VariableMap::const_iterator it;

	int iStart = 0;
	if (!path.empty())
		if (path[0] == '/')
			iStart = 1;

	mout.debug(5) << "'" << path << "'" << mout.endl;

	std::stringstream sstr; // for speed
	std::string s;
	size_t i = 0;
	while(true){

		const std::string subpath = path.substr(iStart, i);  // with i=npos
		mout.debug(5) << "'" << subpath << "'\t" << i << mout.endl;
		//attributes[std::string("@")+subpath] = 0;

		const HI5TREE & s = src(subpath);

		/////// getO
		for (std::set<std::string>::const_iterator git = EncodingODIM::attributeGroups.begin(); git != EncodingODIM::attributeGroups.end(); ++git){
			const hi5::NodeHi5 & group = s[*git].data;
			for(drain::VariableMap::const_iterator ait = group.attributes.begin(); ait != group.attributes.end(); ait++){
				sstr.str("");
				sstr << *git << ':' << ait->first;
				//mout.debug(8) << "getAttributes: " << sstr.toStr() << '=' << it->second << mout.endl;
				if (!updateOnly)
					attributes[sstr.str()] = ait->second;
				else {
					typename M::iterator it = attributes.find(sstr.str());
					if (it != attributes.end())
						it->second = ait->second;
				}
			}
		}


		if (i == std::string::npos){
			//if (drain::Debug > 4){ std::cerr << "attributes " << attributes << '\n';	}
			return;
		}

		i = path.find('/', i+1);
	}
	//while (i < path.size()); //(i != std::string::npos);
}



class DataSelector2 : public drain::BeanLike {

public:

	DataSelector2();

	// For compatibility
	std::string path;


	// Path criteria
	Range<int> dataset;
	Range<int> data;

	bool isValidPath(const ODIMPath & path) const;

	// Data criteria
	Range<double> elangle;

	std::string quantityStr;
	//drain::RegExp quantity; // consider other, like std::set?

	bool isValidData(const drain::ReferenceMap & properties) const ;

	/// Restore default values.
	void reset();

	// Restore all the ranges to [min,min]; useful when explicitly set ranges are desired.
	void collapse();

protected:

	mutable
	std::set<std::string> quantitySet;

};


} // rack::

#endif /* DATASELECTOR_H_ */

// Rack
