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
#ifndef SMARTMAP_H
#define SMARTMAP_H

#include <iostream>
#include <string>
#include <set>
#include <list>
#include <vector>
#include <map>

#include "Castable.h"
#include "String.h"

//#include <regex.h> // wants malloc?

// g++ deer_regexp.cpp -o deer_regexp
// typedef size_t int;
// // // using namespace std;

namespace drain {

/// A base class for smart maps providing methods for importing and exporting values, among others
/**
 *  Unlike with std::map<>, operator[] is defined as const, too, returning reference to a static empty instance.
 *
 *  SmartMap maintains an ordered list of keys, accessible with getKeys(). The items in the key list are
 *  appended in the order they have been created. This is order is generally different from the (alphabetical) order
 *  appearing in std::map iteration.
 *
 *  Comma-separated values can be assigned simultaneously with setValues() and updateValues()
 */
template<class T>
class SmartMap : public std::map<std::string, T> {

public:

	typedef std::list<std::string> keylist_t;

	/// Default character used for splitting input and output. See setValues
	char separator;

	/// Default separator character for array elements (std::vector's)
	char arraySeparator;

	/**
	 *   \param separator  - default character used for splitting input and output. See setValues
	 *   \param arraySeparator - default character for inner elements that are array (std::vector's)
	 */
	SmartMap(char separator='\0', char arraySeparator=':') : separator(separator), arraySeparator(arraySeparator) {};

	/**
	 *   \param ordered   - several comma-separated values can be assigned with setValues() and updateValues()
	 *   \param separator - default character used for splitting input and output. See setValues
	 */
	// SmartMap(bool ordered=true, char separator='\0') : ORDERED(ordered), separator(separator){};


	virtual
	inline
	~SmartMap(){};

	/// Needed?
	typedef typename std::map<std::string, T>::iterator iterator;
	typedef typename std::map<std::string, T>::const_iterator const_iterator;

	virtual inline
	void clear(){
		std::map<std::string, T>::clear();
		keyList.clear();
	}

	inline
	bool hasKey(const std::string &key) const {
		return (this->find(key) != this->end());
	}

	/// Todo: implement default values... get()

	/// Retrieves a value, if set, else returns the given default value.
	/**
	 *  \param key - name of the variable
	 *  \param defaultValue - the value to be returned if the variable is unset
	 *  \return - the value, if set; else the default value.
	 */
	inline
	std::string get(const std::string &key, const std::string & defaultValue) const {
		const_iterator it = this->find(key);
		if (it == this->end())
			return defaultValue;
		else {
			// return static_cast<std::string>(it->second);  DOES NOT WORK, http://stackoverflow.com/questions/7741531/conversion-operator-template-specialization
			// return (std::string)(it->second);   //it->second.toStr();
			// std::stringstream sstr; ?? FULL
			// sstr << *this;
			//return sstr.str();
			//return it->second.toStr();
			return it->second;
		}
	}


	inline
	std::string get(const std::string & key, const char *defaultValue) const {
		return get(key, std::string(defaultValue));
	}


	/// Retrieves a value, if set, else returns the given default value.
	/**
	 *  \param key - name of the variable
	 *  \param defaultValue - the value to be returned if the variable is unset
	 *  \return - the value, if set; else the default value.
	 */
	template <class T2>
	T2 get(const std::string &key, T2 defaultValue) const {
		const_iterator it = this->find(key);
		if (it == this->end())
			return defaultValue;
		else
			return static_cast<T2>(it->second);
	}

	/// Returns an element. Creates one, conditionally.
	/**
	 *   - OPEN:  creates a new elment
	 *   - CLOSED:  returns a dummy element.
	 *   - STRICTLY_CLOSED:  throws exception
	 */
	inline
	virtual
	T & operator[](const std::string &key){

		iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			// Create:
			keyList.push_back(key);
			T & element = std::map<std::string, T>::operator[](key);
			//element.setSeparator(arraySeparator);
			return element;
		}
	}

	/// Unlike with std::map, operator[] const  is defined, returning reference to a static empty instance.
	inline
	virtual
	const T & operator[](const std::string &key) const {

		const_iterator it = this->find(key);
		if (it != this->end()) {
			return it->second;
		}
		else {
			static const T empty;
			return empty;
		}
	}


	/// Derived versions may produce an ordered set of keys.
	virtual inline
	const std::list<std::string> & getKeyList() const {
		return keyList;
	}


	inline
	const std::map<std::string,T> & getMap() const {  /// ? but <T> ?
		return *this;
	};

	/// Copies the contents to another map.
	template <class T2>
	void exportMap(std::map<std::string,T2> & m) const {
		for (const_iterator it = this->begin(); it != this->end(); ++it)
			m[it->first] = it->second;
	}

	/// Assigns a value to given key; if the entry does not exist, tries to create it with directly with operator[].
	/**
	 *   \param updateOnly - if true, only existing elements are updated, otherwise skipped; if false, tries to add elements.
	 */
	template <class T2>
	inline
	void importEntry(const std::string & key, const T2 & value, bool updateOnly = true){

		iterator rit = this->find(key);

		if (rit != this->end()){
			rit->second = value;  // ? Castable = Variable  (T &)
		}
		else {
			// RefMap = flexible =  updateOnly
			// Varmap = flexible = !updateOnly
			if (updateOnly){
				// skip!
			}
			else {
				(*this)[key] = value;  // throws exception if STRICTLY CLOSED
			}
		}
	}

	/// Assign values from a map. Updates existing entries only.
	/**
	 *  \par m - source of keys and values
	 */
	template <class T2>
	void updateFromMap(const std::map<std::string,T2> & m){
		for (typename std::map<std::string,T2>::const_iterator it = m.begin(); it != m.end(); ++it)
			importEntry(it->first, it->second, true);
	}

	template <class T2>
	void updateFromCastableMap(const drain::SmartMap<T2> & m){
		for (typename SmartMap<T2>::const_iterator it = m.begin(); it != m.end(); ++it)
			importEntry(it->first, (const Castable &)it->second, true);
	}


	/// Assign values from a map, overriding existing entries.
	/**
	 *  \par m - source of keys and values
	 *
	 *  If a key is not known, and the map is of fixed type like ReferenceMap,  throws exception.
	 */
	template <class T2>
	void importMap(const std::map<std::string,T2> & m){
		for (typename std::map<std::string,T2>::const_iterator it = m.begin(); it != m.end(); ++it)
			importEntry(it->first, it->second, false);
	}

	/// Assign values from a map, possibly extending the map.
	/**
	 *  \par m - source of keys and values
	 *
	 *  If a key is not known, and the map is of fixed type like ReferenceMap,  throws exception.
	 */
	template <class T2>
	void importCastableMap(const drain::SmartMap<T2> & m){
		for (typename SmartMap<T2>::const_iterator it = m.begin(); it != m.end(); ++it)
			importEntry(it->first, (const Castable &)it->second, false);
	}


	/// Sets values. If strictness==STRICTLY_CLOSED, throws exception if tries to assign a non-existing entry.
	// TODO: consider: std::string assignmentSymbols="=:", std::string separatorSymbols=", ", std::string trimSymbols=" \t\n\r",
	inline
	void setValues(const std::string & entries, char assignmentSymbol='=', char separatorSymbol=0){  // char separatorSymbol=','
		assignEntries(entries, false, assignmentSymbol, separatorSymbol ? separatorSymbol : separator);
	}

	inline
	void setValues(const char * entries, char assignmentSymbol='=', char separatorSymbol=0){
		assignEntries(entries, false, assignmentSymbol, separatorSymbol ? separatorSymbol : separator);
	}

	template <class C>
	void setValues(const C & container);

	/// Sets applicable values ie. modifies existing entries only. In ordered maps, skips extra entries silently.
	inline
	void updateValues(const std::string & entries, char assignmentSymbol='=', char separatorSymbol=0){// char separatorSymbol=','
		assignEntries(entries, true, assignmentSymbol, separatorSymbol ? separatorSymbol : separator);
	}


	inline
	void getKeys(std::ostream &ostr) const {

		const std::list<std::string> & l = getKeyList();
		for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it ){
			if (it != l.begin())
				ostr << ',';
			ostr <<	*it;
		}

	}

	/// Convenience function for std::string output.
	inline
	std::string getKeys() const {
		std::stringstream s;
		getKeys((std::ostream &)s);
		return s.str();
	};

	/// Dumps the values
	inline
	void getValues(std::ostream &ostr) const {

		const std::list<std::string> & l = getKeyList();
		for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it ){
			if (it != l.begin())
				ostr << ',';
			if (this->find(*it) != this->end())
				ostr <<	(*this)[*it]; //  << this->find(*it).getType();
			else {
				ostr << "*SMARTMAP::FAIL* " << __FUNCTION__;
			}
		}

	}

	/// Convenience function for std::string output.
	inline
	std::string getValues() const {
		std::stringstream s;
		getValues(s);
		return s.str();
	};


	///
	/*
	 *    \param equal - typically =, :, or -
	 *    \param start - typically hyphen or leading parenthesis (, {, [
	 *    \param end   - typically hyphen or trailing parenthesis ), }, [
	 *    \param separator - typically comma or semicolon
	 */
	template <class S>
	void toOStream(S & ostr, char equal='=', char startChar=0, char endChar=0, char separatorChar=0) const {

		const std::list<std::string> & keys = this->getKeyList();

		separatorChar = separatorChar ? separatorChar : this->separator;
		separatorChar = separatorChar ? separatorChar : ',';  // needed?

		for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){

			if (separatorChar){
				if (it != keys.begin())
					ostr << separatorChar;
			}

			ostr << *it << equal;
			if (startChar)
				ostr << startChar;
			ostr << (*this)[*it];
			if (endChar)
				ostr << endChar;
			/*
			const_iterator pit = this->find(*it);
			if (pit != this->end()){
				ostr << "pit->second";
			}
			else {
				std::cerr << *it << " not found\n";
				ostr << "SmartMap test";
			}
			*/
		}

	}


	//std::string toStr(char equal='=', char start='{', char end='}', char separator=0) const {
	std::string toStr(char equal='=', char start=0, char end=0, char separator=0) const {
		std::stringstream sstr;
		toOStream(sstr, equal, start, end, separator);
		return sstr.str();
	}

	void toJSON(std::ostream & ostr, size_t indent = 0) const {

		const std::string space(indent, ' ');

		char sep = 0;
		ostr << "{\n";

		//for (std::list<std::string>::const_iterator it = getKeyList().begin(); it != getKeyList().end(); ++it){
		for (const_iterator it = this->begin(); it != this->end(); ++it){
			//const string & key = *it;
			const std::string & key = it->first;
			if (sep){
				ostr << sep;
				ostr << '\n';
			}
			else {
				sep = ',';
			}
			//ostr << '\n';
			ostr << space << "\"" << key << "\" : ";
			const T & item = it->second; //(*this)[key];

			//if (item.getType() == typeid(std::string)){
			if (item.T::isString()){
				//
				ostr << '"';
				// ostr << '"' << item.getCharArray() << '"';
				const char *c = item.getCharArray();
				while (*c != '\0'){
					if (*c == '"')
						ostr << '\\';  // TODO; implement also \n \t ...
					ostr << *c;
					++c;
				}
				ostr << '"';
				//ostr << '"' << item << '"';
			}
			else {
				switch (item.T::getElementCount()) {
					case 0:
						ostr << '[' << ']'; // or NaN?
						break;
					case 1:
						ostr << item;
						break;
					default:
						ostr << '[' << item << ']';
				}
			}
		}
		// ostr << "{\n  \"value\":" << *this << ",\n";
		//ostr << "  \"type\":" << drain::Type::getTypeChar(getType()) << ",\n";
		ostr << "\n" << space << "}\n";  // \n needed?
	}

protected:

	/// Assigns values from std::string of type "value,value2,...valueN".
	// ?? If specific, allows also "key=value,key1=value2,...".
	/**
	 *   Allows setting values in python style function calls:
	 *   -# in the definition order of entries: "value,value2,...valueN" become assined as key=value, key2=value2, ... and so on.
	 *   -# specified mode, allowing specific assignments "key=value,key2=value2,key3=value3"
	 *
	 *   Mixed types like \c "value,key4=value2,value3" accepted but not encouraged. In the mixed mode, the unspecified keys become
	 *   assigned in \i their order of appearance. Hence, in the above example \c value3 will be assigned to \c key2 , not \c key3.
	 *
	 *   \param updateOnly - if true, skip non-existing entries silently
	 */
	void assignEntries(const std::string & entries, bool updateOnly = false, char assignmentSymbol='=', char separatorSymbol=0){
		// void setValues(const std::string & entries, char assignmentSymbol, bool updateOnly = false){

		Logger mout(__FUNCTION__, __FILE__);
		//mout.debug(10) << entries << mout.endl;

		if (entries.empty()){
			return;
		}

		separatorSymbol = separatorSymbol ? separatorSymbol : separator;

		// Input parameter assignments, separated by the separator: "a=1", "b=2", "c=3", ...
		std::list<std::string> p;
		drain::StringTools::split(entries, p, std::string(1, separatorSymbol));  // separators);

		assignEntries(p, updateOnly, assignmentSymbol);
	}


	void assignEntries(const std::list<std::string> & p, bool updateOnly = false, char assignmentSymbol='='){

		Logger mout(__FUNCTION__, __FILE__);
		const std::string assignmentSymbols(1, assignmentSymbol);

		const std::list<std::string> & keys = getKeyList();
		std::list<std::string>::const_iterator kit = keys.begin();

		bool acceptOrderedParams = true;

		for (std::list<std::string>::const_iterator pit = p.begin(); pit != p.end(); ++pit){

			// Check specific assignment, ie. check if the key=value is given explicitly.
			if (assignmentSymbol){ // typically '='
				std::string key;
				std::string value;
				if (StringTools::split2(*pit, key, value, assignmentSymbols)){
					// mout.warn() << " specified " <<  key << "=\t" << value << mout.endl;
					importEntry(key, value, updateOnly);
					acceptOrderedParams = false;
					continue;
				}
				/*
				size_t i = pit->find(assignmentSymbol);
				if (i != std::string::npos){
					importEntry(pit->substr(0,i), pit->substr(i+1), updateOnly);
					acceptOrderedParams = false;
					continue;
				}
				*/
			}

			// Key and assignment symbol not given.

			if (kit != keys.end()){
				// Assignment-by-order
				if (!acceptOrderedParams){
					mout.warn() << "unspecified (ordered) param ["<< *kit << "=] '" << *pit << "' given after specified params" << mout.endl;
				}
				//mout.warn() << " ordered  " <<   << mout.endl;
				(*this)[*kit] = *pit;  // does not need to call import() because *kit exists.
				++kit; // NUEVO
			}
			else {
				if (!updateOnly)
					mout.error() << "too many (over "<< this->size() << ") params, run out of keys with entry=" << *pit << mout.endl;
				//return; // NUEVO
			}

			// ++kit;  NUEVO

		}
	}


	/// Defines the 1) valid keys and 2) their order.
	// std::list<std::string> orderedKeyList;

	mutable	std::list<std::string> keyList;

};

template <class T>
template <class C>
void SmartMap<T>::setValues(const C & container){

	Logger log(__FUNCTION__, __FILE__);

	const std::list<std::string> & keys = getKeyList();
	std::list<std::string>::const_iterator kit = keys.begin();

	for (typename C::const_iterator it = container.begin(); it != container.end(); ++it){

		if (kit != keys.end()){
			// Assignment-by-order
			(*this)[*kit] = *it;  // does not need to call import() because *kit exists.
			++kit; // NUEVO
		}
		else {
			log.error() << "too many ("<< container->size() << ") params for map of size ("<< this->size() << "), run out of keys with entry=" << *it << log.endl;
		}

	}

}


template<class T>
std::ostream &operator<<(std::ostream &ostr, const SmartMap<T> & m){
	m.toOStream(ostr, '=', 0, 0, m.separator);
	return ostr;
}

} // drain


#endif

// Drain
