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

#ifndef HI5_BASE
#define HI5_BASE



#include <hdf5.h>

#include <string>
#include <list>


#include <drain/util/Log.h>

#include <drain/util/Variable.h>
#include <drain/image/Image.h>
#include <drain/util/Tree.h>

#include <data/ODIMPath.h>




// using namespace std;

namespace hi5 {


extern drain::Log hi5monitor;
extern drain::Logger hi5mout;


/// Rack's hi5 structure that uses Rack classes (Tree, Data, Image).
/**
 *   Node to be used in a h5 structure, ie. TreeNode<NodeHi5>.
 *   A node serves both as a group or a data set; if a data set is empty its node is treated as a group.
 *
 *   ODIM conventions are purposely not included in the Hi5 utilities.
 *   For example, path naming conventions (dataset1/data3/...) and some special attributes like
 *   Conventions="ODIM_H5/V2_0" are left to the application code.
 */
struct NodeHi5 {

	typedef drain::image::Image image_type;

	drain::image::Image dataSet;

	drain::VariableMap  attributes;

	NodeHi5() : noSave(false) {};

	inline
	NodeHi5(const NodeHi5 & n) : noSave(n.noSave) {
		attributes.importMap(n.attributes);
	};

	inline
	NodeHi5 & operator=(const hi5::NodeHi5 & n){
		attributes.importMap(n.attributes); return *this;
	};

	void writeText(std::ostream & ostr = std::cout, const std::string & prefix = "") const;

	/// Experimental
	bool noSave;  // OK!

};

}



//typedef  TreeNode<NodeHi5> TreeHi5;
//#define HI5TREE drain::Tree<std::string, hi5::NodeHi5, lessAlphaNum>  // std::less<std::string> >
//#define HI5TREE drain::Tree<rack::ODIMPathElem, hi5::NodeHi5, rack::ODIMPathLess>  // std::less<std::string> >
typedef drain::Tree<rack::ODIMPathElem, hi5::NodeHi5, rack::ODIMPathLess> HI5TREE;


namespace hi5 {

//typedef rack::ODIMPath path_t;

///
///

/// Base class for Reader and Writer, essentially just wrapping some utilities.
class Hi5Base {

public:

	//static
	// drain::Log hi5monitor;

	// static
	// drain::Logger hi5mout; //(drain::monitor,"hi5");

	/// Given type toOStr of a native C++ type, returns a native HDF5 data type.
	static
	const hid_t & getH5DataType(const std::type_info &t);

	/// Given a native C++ type, returns a native HDF5 data type.
	template <class T>
	static
	const hid_t & getH5DataType(){ return getH5DataType(typeid(T)); }


	/// Creates a 256-element RGB palette to be referenced with linkPalette().
	static
	HI5TREE & getPalette(HI5TREE & dst);
	//drain::image::Image & getPalette(HI5TREE & dst);

	/// Links the palette that has been (or will be) created with createPalette().
	static
	void linkPalette(const HI5TREE & palette, HI5TREE & dst);


	/// Dumps the H5 structure, attributes and data properties.
	static
	void writeText(const HI5TREE &src, const std::list<HI5TREE::path_t> & paths, std::ostream & ostr = std::cout);

	/// Dumps the H5 structure, attributes and data properties. (Calls writeText(src, src.getKeys(), ostr)).
	static
	void writeText(const HI5TREE &src, std::ostream & ostr = std::cout){
		std::list<HI5TREE::path_t> paths;
		src.getPaths(paths);
		writeText(src, paths, ostr);
	};


	/// Constructs a tree from formatted text. See writeText().
	/**
	 *  The grammar should follow to that produced by writeText().
	 */
	static
	void readText(HI5TREE &src, std::istream & istr = std::cin);

	/// Assign a value with optional type specification.
	/**  Creates a node in src, creates the desired attribute with given value of specified storage type.
	 *   If the attribute is \c "image", creates an image with specified value (\c width,\c height ) and type.
	 *
	 *
	 */

	/// Split full path string to path object and attribute key.
	// consider ValueReader, TextReader instead (skipping attrType)
	/**
	 *
	 *   \param s - string containing path and/or attribute
	 *
	 *   Given a path like /dataset1/data3
	 *   \code
	 *
	 *   \endcode
	 *
	 */

	static
	void parsePath(const std::string & line, HI5TREE::path_t & path, std::string & attrKey, drain::Variable & v);

	/*
	static
	void parsePath(const std::string & line, HI5TREE::path_t & path, std::string & attrKey, std::string & attrValue,
			std::string & attrType);
	*/

	/*
	static inline
	void parsePath(const std::string & line, HI5TREE::path_t & path, std::string & attrKey){
		std::string attrValue;
		std::string attrType;
		parsePath(line, path, attrKey, attrValue, attrType);
	}
	*/

	/// Assign a value with optional type specification.
	/**  Creates a node in src, creates the desired attribute with given value of specified storage type.
	 *   If the attribute is \c "image", creates an image with specified value (\c width,\c height ) and type.
	 *
	 */
	static
	void readTextLine(HI5TREE &src, const std::string & line);

	// static	void readTextLine(HI5TREE & dst, const HI5TREE::path_t & path, const std::string & key, const std::string & value);

	/// Delete branches that have been marked with noSave=true .
	static
	void deleteNoSave(HI5TREE &src);


};


/// Dumps a node.
std::ostream & operator<<(std::ostream &ostr, const hi5::NodeHi5 &n);

/// Dumps a complete tree.
std::ostream & operator<<(std::ostream &ostr, const HI5TREE & tree);


/*
struct lessAlphaNum {

	enum comparison {LESS=-1, EQUAL=0, GREATER=+1};

	inline
	static
	bool isNumeric(char c) {
		return ((c >= '0') && (c <= '9'));
	};

	inline
	static
	comparison compareAlphaBetical(std::string::const_iterator & c1, std::string::const_iterator end1,
			std::string::const_iterator & c2, std::string::const_iterator end2){


		while (c1 != end1){

			if (c2 == end2){
				return GREATER; // str1 longer
			}

			// Stop if both are numeric
			if (isNumeric(*c1) && isNumeric(*c2)){
				return EQUAL; // that is, equal as to the alpha part.
			}

			if (*c1 < *c2)
				return LESS;
			else if (*c1 > *c2)
				return GREATER;

			// *c1 == *c2 : strs are equal, non-numeric this far.

			++c1;
			++c2;

		}

		if (c2 != end2) // str2 is longer
			return LESS;
		else
			return EQUAL;

	}

	inline
	static
	comparison compareNumeric(std::string::const_iterator & c1, std::string::const_iterator end1,
			std::string::const_iterator & c2, std::string::const_iterator end2){

		const std::string::const_iterator n1 = c1;
		const std::string::const_iterator n2 = c2;
		size_t length1 = 0;
		size_t length2 = 0;

		// scan  to end of numeric segment
		while ((c1 != end1) && isNumeric(*c1)){
			++c1;
			// todo: if undetectValue padding?
			++length1;
		}

		// scan  to end of numeric segment
		while ((c2 != end2) && isNumeric(*c2)){
			++c2;
			// todo: if undetectValue padding?
			++length2;
		}



		if (length1 < length2)
			return LESS;
		else if (length1 > length2)
			return GREATER;
		else {
			// std::cout << "Equal lengths. Compare element by element\n";
			// end points of numsegs
			const std::string::const_iterator ne1 = c1;
			const std::string::const_iterator ne2 = c2;
			// rescan from start
			c1 = n1;
			c2 = n2;
			while  ((c1 != ne1) && (c2 != ne2)){ //

				if (*c1 < *c2)
					return LESS;
				else if (*c1 > *c2)
					return GREATER;
				++c1;
				++c2;
			}

			return EQUAL;

		}

	}

	// Main function
	bool operator()(const std::string & s1, const std::string & s2) const {

		const std::string::const_iterator end1 = s1.end();
		const std::string::const_iterator end2 = s2.end();
		std::string::const_iterator c1 = s1.begin();
		std::string::const_iterator c2 = s2.begin();

		comparison result;

		// debugging
		std::string d1, d2;

		//while ((c1 != end1) && (c2 != end2)){
		//int n = 10;
		//while (n > 0){
		//--n;
		while (true){

			//std::cout << "Alphabetic check\n";
			//std::cout << d1.assign(c1, end1) << " vs. " << d2.assign(c2, end2) << '\n';

			result = compareAlphaBetical(c1, end1, c2, end2);

			if (result == LESS)
				return true;
			if (result == GREATER)
				return false;

			//if (c1 == end1) // s1 ends, so s2 same this far (ie. same or longer)
			if (c2 == end2) // s2 gives up, so s1 same this far (ie. same or longer)
				return false;

			// std::cout << "Numeric part check\n";
			//std::cout << d1.assign(c1, end1) << " vs. " << d2.assign(c2, end2) << '\n';


			result = compareNumeric(c1, end1, c2, end2);

			if (result == LESS)
				return true;
			if (result == GREATER)
				return false;

			//if (c1 == end1) // s1 ends, so s2 same this far (ie. same or longer)
			if (c2 == end2) // s2 gives up, so s1 same this far (ie. same or longer)
				return false;

			// std::cout << "Equal, continuing \n";
		}




	} // end operator()

};  // end class
*/


} // ::hi5


#endif /* ImageH5_H_ */

// Rack
