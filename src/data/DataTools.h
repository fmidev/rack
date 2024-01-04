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

#include "drain/util/BeanLike.h"

#include "drain/util/ReferenceMap.h"
#include "drain/util/RegExp.h"

#include "drain/util/Variable.h"

#include "drain/image/Coordinates.h"

#include "ODIM.h"
#include "PolarODIM.h" // elangle

//#include "DataSelector.h" // Range


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
	// const drain::Variable & getAttribute(const Hi5Tree &src, const std::string & path, const std::string & group, const std::string & attributeName);

	/// Collects PolarODIM /where, /what and /how attributes recursively along the path and stores them in a std::map<std::string,T> (e.g. VariableMap or ReferenceMap, ODIM).
	/**
	 *   Does not change attributes of src.
	 *   \see updateAttributes()
	template <class M>
	static
	void getAttributesOLD(const Hi5Tree &src, const std::string & path, M & attributes, bool updateOnly = false);
	*/

	// consider: path is not a reference, it will be copied.
	template <class M>
	static
	void getAttributes(const Hi5Tree &src, const Hi5Tree::path_t & path, M & attributes, bool updateOnly = false);


	/// Copies values of \c what , \c where and \c how attributes to internal attributes down to \c data[n] groups.
	/**
	 *  Traverses the data tree, updates the data (image) attributes along the path down to \c data[n] level;
	 *  the lowest groups containing data arrays, that is \c data groups, are not updated.
	 *
	 *  Note that also the \i coordinate \i policy is updated, because the corresponding variable is linked by ImageFrame::init().
	 *
	 *  Typically, this is called on the root.
	 *
	 *  \see updateAttributes(Hi5Tree & src, const drain::FlexVariableMap & attributes)
	 */
	static
	void updateInternalAttributes(Hi5Tree & src);
	/*
	{
		src.data.dataSet.properties.clear();
		src.data.dataSet.setCoordinatePolicy(4,3,2,1);
		updateInternalAttributes(src, drain::FlexVariableMap());
		//updateInternalAttributes(src, drain::VariableMap());
	}
	*/

	/// This \c const version does nothing, but is needed for Data:: #supdateTree3()
	/**
	 *   \see updateAttributes(Hi5Tree & src, const drain::FlexVariableMap & attributes)
	 */
	static inline
	void updateInternalAttributes(const Hi5Tree & src){ // const drain::FlexVariableMap & attributes = drain::FlexVariableMap()){
		// drain::Logger mout("DataTools", __FUNCTION__);
		// mout.warn("somebody called me");
	};


	static
	void updateCoordinatePolicy(Hi5Tree & src, const drain::image::CoordinatePolicy & policy = drain::image::CoordinatePolicy(drain::image::CoordinatePolicy::LIMIT));
	//void updateCoordinatePolicy(Hi5Tree & src, const CoordinatePolicy & policy = CoordinatePolicy(CoordinatePolicy::LIMIT));

//	static
//	bool dataToStream(const Hi5Tree::node_data_t & data, std::ostream &ostr);

	typedef std::map<std::string, ODIMPathElem> quantity_map;

	static
	void markNoSave(Hi5Tree &src, bool noSave=true);

protected:

	/// Removes the children of the tree if Node::noSave is set.
	/**
	 *  \return - true if children were removed
	 */
	static
	bool removeIfNoSave(Hi5Tree & dst);

	/// Does nothing
	/**
	 *  \return - false (always, as nothing will be removed)
	 */
	static
	bool removeIfNoSave(const Hi5Tree & src){
		return false;
	};


	/// Copies values of \c what , \c where and \c how attributes to internal attributes down to \c data[n] groups.
	/**
	 *  Traverses the data tree, updates the data (image) attributes along the path down to \c data[n] level;
	 *  the lowest groups containing data arrays, that is \c data groups, are not updated.
	 *
	 *  Note that also the \i coordinate \i policy is updated, because the corresponding variable is linked by ImageFrame::init().
	 *
	 *  Typically, this is called on the root.
	 *
	 *    \see updateAttributes(Hi5Tree & src)
	 */
	//const drain::image::CoordinatePolicy & policy = drain::image::CoordinatePolicy(),
	static
	void updateInternalAttributes(Hi5Tree & src, const drain::FlexVariableMap & attributes);
	//void updateInternalAttributes(Hi5Tree & src, const drain::VariableMap & attributes);




};


template <class M>
void DataTools::getAttributes(const Hi5Tree &src, const Hi5Tree::path_t & p, M & attributes, bool updateOnly){

	drain::Logger mout("DataTools", __FUNCTION__);

	if (p.empty() || !p.front().isRoot()){
		mout.debug("add root and restart with path= '" , p , "'" );
		Hi5Tree::path_t pRooted(p);
		pRooted.push_front(Hi5Tree::path_t::elem_t::ROOT);
		DataTools::getAttributes(src, pRooted, attributes, updateOnly);
		return;
	}

	mout.debug("path= '" , p , "'" );

	Hi5Tree::path_t path;
	std::stringstream sstr;

	for (Hi5Tree::path_t::const_iterator pit = p.begin(); pit != p.end(); ++pit){

		path << *pit;

		mout.debug("check='" , path , "'" );

		const Hi5Tree & s = src(path);

		for (ODIMPathElemSeq::const_iterator git = EncodingODIM::attributeGroups.begin(); git != EncodingODIM::attributeGroups.end(); ++git){

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

		// ++pit;
		//  if (pit == p.end())
		// return;
		//   path << *pit;

	}

}



/*
template <class M>
void DataTools::getAttributesOLD(const Hi5Tree &src, const std::string & path, M & attributes, bool updateOnly){

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

		const Hi5Tree & s = src(subpath);

		/////// getO
		for (std::set<ODIMPathElem>::const_iterator git = EncodingODIM::attributeGroups.begin(); git != EncodingODIM::attributeGroups.end(); ++git){
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
*/


} // rack::

#endif /* DATASELECTOR_H_ */

// Rack
