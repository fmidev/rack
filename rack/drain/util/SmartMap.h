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
#include <sys/syslog.h>
#include <syslog.h>

#include "Log.h"
#include "Castable.h"
#include "Sprinter.h"
#include "String.h"

#include "SmartMapTools.h"



// TODO: use SmartMapTools for assignments

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

	typedef SmartMap<T> smap_t;

	typedef std::map<std::string, T> map_t;
	typedef typename map_t::key_type     key_t;
	typedef typename map_t::mapped_type value_t; // == T
	typedef typename map_t::value_type 	entry_t; // pair<key_t,value_t>

	typedef std::list<std::string> keylist_t;

	/// Needed?
	typedef typename map_t::iterator iterator;
	typedef typename map_t::const_iterator const_iterator;


	/// Default character used for splitting input and output. See setValues
	char separator;

	/// Default separator character for array elements (std::vector's)
	char arraySeparator;

	/**
	 *   \param separator  - default character used for splitting input and output. See setValues
	 *   \param arraySeparator - default character for inner elements that are array (std::vector's)
	 */
	inline
	SmartMap(char separator='\0', char arraySeparator=':') : separator(separator), arraySeparator(arraySeparator) {};

	// Copy constructor. Does not copy items.
	/**
	 */
	inline
	SmartMap(const SmartMap & smap) : separator(smap.separator), arraySeparator(smap.arraySeparator){};


	virtual
	inline
	~SmartMap(){};


	virtual inline
	void clear(){
		map_t::clear();
		keyList.clear();
	}

	inline
	bool hasKey(const std::string &key) const {
		return (this->find(key) != this->end());
	}

	/// Retrieves a value, or default value if value is unset.
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
			T & element = map_t::operator[](key);
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
	const keylist_t & getKeyList() const {
		return keyList;
	}


	inline
	const map_t & getMap() const {  /// ? but <T> ?
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
	/*
	template <class V,bool STRICT=false> // TODO: true...
	//void importEntry(const std::string & key, const T2 & value, bool updateOnly = true){
	void importEntry(const std::string & key, const V & value){ //, bool updateOnly = true) { //, unsigned int criticality = LOG_DEBUG){

		// SmartMapTools::setValue<map_t,V,STRICT>(*this, key, value);
		SmartMapTools::setValue<smap_t,V,STRICT>(*this, key, value);
    */
		/* //new
		iterator rit = this->find(key);

		if (rit != this->end()){
			rit->second = value;  // ? Castable = Variable  (T &)
		}
		else {
			if (updateOnly){
				// skip!
			}
			else {
				// This test cannot be here, because VariableMap supports and ReferenceMap rejects assigning new values directly.
				// Logger mout(__FUNCTION__, __FILE__);
				// mout.log(criticality) << " No key for "<< key << separator << value << " (new) " << criticality << mout.endl;
				(*this)[key] = value;  // throws exception in derived classes
			}
		}
		*/
	//  }

	/**
	 *   \param entries - string containing key=value pairs separated by separator
	 */
	template <bool STRICT=true>
	void importEntries(const std::string & entries, char assignmentSymbol='=', char separatorSymbol=0); //, bool updateOnly = false);
	//, unsigned int criticality = LOG_ERR);


	/**
	 *   \param entries - a sequence containing key=value pairs separated by separator
	 *   TODO: sequence
	 */
	template <bool STRICT=true>
	void importEntries(const std::list<std::string> & entries, char assignmentSymbol='='){ //; //, bool updateOnly = false);
		//SmartMapTools::setValues<T,STRICT>(*this, getKeyList(), entries, assignmentSymbol);
		SmartMapTools::setValues<smap_t,STRICT>(*this, getKeyList(), entries, assignmentSymbol);
	}

	/// Assign values from a map, overriding existing entries.
	/**
	 *  \par m - source of keys and values
	 *
	 *  If a key is not known, and the map is of fixed type like ReferenceMap,  throws exception.
	 */
	template <class S,bool STRICT=true>
	void importMap(const std::map<std::string,S> & m){ //, bool updateOnly = false){ //, unsigned int criticality = LOG_ERR){
		// SmartMapTools::setValues<map_t,S,STRICT>(*this, m);
		// SmartMapTools::setValues(*this, m);
		SmartMapTools::setValues<smap_t,S,STRICT>(*this, m);
		 //new
		//for (typename std::map<std::string,S>::const_iterator it = m.begin(); it != m.end(); ++it)
		//	importEntry(it->first, it->second); //, updateOnly); //, criticality); // false);

	}

	/// Assign values from a map, possibly extending the map.
	/**
	 *  \param m - source of keys and values
	 *  \param updateOnly - bypass unexisting keys silently, otherwise throw exception.
	 *  \param criticality - define sensibility if new keys cannot be added (LOG_NOTE, LOG_ERROR)
	 *
	 *  If a key is not known, and the map is of fixed type like ReferenceMap,  throws exception.
	 */
	template <class T2,bool STRICT=true>
	void importCastableMap(const drain::SmartMap<T2> & m){ //, bool updateOnly = false, unsigned int criticality = LOG_ERR){
		// NUOEVO:
		SmartMapTools::setCastableValues<smap_t,T2,STRICT>(*this, m);
		// SmartMapTools::setValues<smap_t,T2,STRICT>(*this, m);
		//SmartMapTools::setValues<map_t,T2,STRICT>(*this, m);
		/*
		for (const typename SmartMap<T2>::keylist_t::value_type &key : m.getKeyList()){
			importEntry(key, (const Castable &)m[key]); //, updateOnly);
		}
		*/
		// for (typename SmartMap<T2>::const_iterator it = m.begin(); it != m.end(); ++it)
		//	importEntry(it->first, (const Castable &)it->second, updateOnly); //, criticality); // false);
	}



	/// Assign values from a map. Updates existing entries only.
	/**
	 *  \par m - source of keys and values
	 */
	/// Convenience
	template <class T2>
	inline
	void updateFromMap(const std::map<std::string,T2> & m){
		importMap<T2,false>(m); //, LOG_DEBUG);
	}

	/// Convenience
	template <class T2>
	inline
	void updateFromCastableMap(const drain::SmartMap<T2> & m){
		importCastableMap<T2,false>(m); //, true);
		// for (typename SmartMap<T2>::const_iterator it = m.begin(); it != m.end(); ++it)
		//	importEntry(it->first, (const Castable &)it->second, LOG_DEBUG); //true);
	}


	/// Sets values. If strictness==STRICTLY_CLOSED, throws exception if tries to assign a non-existing entry.
	// TODO: consider: std::string assignmentSymbols="=:", std::string separatorSymbols=", ", std::string trimSymbols=" \t\n\r",
	inline
	void setValues(const std::string & entries, char assignmentSymbol='=', char separatorSymbol=0){  // char separatorSymbol=','
		importEntries<true>(entries, assignmentSymbol, separatorSymbol); //, false);  //, LOG_ERR);
	}

	inline
	void setValues(const char * entries, char assignmentSymbol='=', char separatorSymbol=0){
		importEntries<true>(entries, assignmentSymbol, separatorSymbol); //, false); //, LOG_ERR);
	}

	template <class S>
	void setValuesSEQ(const S & sequence);

	/// Sets applicable values ie. modifies existing entries only. In ordered maps, skips extra entries silently.
	inline
	void updateValues(const std::string & entries, char assignmentSymbol='=', char separatorSymbol=0){// char separatorSymbol=','
		importEntries<false>(entries, assignmentSymbol, separatorSymbol); //, true); //, LOG_DEBUG);
	}


	inline
	void getKeys(std::ostream &ostr) const {

		const keylist_t & l = getKeyList();
		for (keylist_t::const_iterator it = l.begin(); it != l.end(); ++it ){
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

		const keylist_t & l = getKeyList();
		for (keylist_t::const_iterator it = l.begin(); it != l.end(); ++it ){
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


	/// Note: parameters discarded
	/*
	 *    \param equal - typically =, :, or -
	 *    \param start - typically hyphen or leading parenthesis (, {, [
	 *    \param end   - typically hyphen or trailing parenthesis ), }, [
	 *    \param separator - typically comma or semicolon
	 */
	// template <class S>
	// S & toStream(S & ostr, char equal='=', char startChar=0, char endChar=0, char separatorChar=0) const;
	std::ostream & toStream(std::ostream & ostr, char equal='=', char startChar='{', char endChar='}', char separatorChar=',') const {
		//drain::TypeLayout pairLayout(equal);
		drain::TypeLayout mainLayout(startChar, separatorChar, endChar);
		SprinterLayout layout(SprinterBase::jsonLayout);
		//layout.pairChars.separator = equal;
		layout.pairChars.separator = equal;
		//layout.arrayChars.setLayout();
		return SprinterBase::sequenceToStream(ostr, getMap(), mainLayout, layout);
		//return SprinterBase::sequenceToStream(ostr, vmap.getMap(), layout.mapChars, layout);
		//return SprinterBase::mapToStream(ostr, *this, SprinterBase::jsonLayout, this->getKeyList());
	}

	//std::string toStr(char equal='=', char start='{', char end='}', char separator=0) const {
	std::string toStr(char equal='=', char start=0, char end=0, char separator=0) const {
		std::stringstream sstr;
		toStream(sstr, equal, start, end, separator);
		return sstr.str();
	}

	/// Write map as a JSON code
	void toJSON(std::ostream & ostr = std::cout, size_t indent = 0) const;

	/// Write map as a JSON code (array or scalar)
	void valuesToJSON(std::ostream & ostr = std::cout) const;

	/// Debugging
	void dump(std::ostream & ostr = std::cout) const;



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
	 *
	 *   \param criticality - if true, skip non-existing entries silently
	 */
	// \param updateOnly - if true, skip non-existing entries silently
	//void assignEntries2(const std::string & entries, bool updateOnly = false, char assignmentSymbol='=', char separatorSymbol=0){

protected:


	/// Defines the 1) valid keys and 2) their order.
	// std::list<std::string> orderedKeyList;

	mutable	std::list<std::string> keyList;

};

template <class T>
template <bool STRICT>
void SmartMap<T>::importEntries(const std::string & entries, char assignmentSymbol, char separatorSymbol){ //, bool updateOnly){

	Logger mout(__FUNCTION__, __FILE__);
	//mout.debug(10) << entries << mout.endl;

	if (entries.empty()){ //old
		return;
	}

	separatorSymbol = separatorSymbol ? separatorSymbol : separator;

	// Input parameter assignments, separated by the separator: "a=1", "b=2", "c=3", ...
	std::list<std::string> p;
	// separatorSymbol = separatorSymbol ? separatorSymbol : separator;
	if (separatorSymbol)
		drain::StringTools::split(entries, p, std::string(1, separatorSymbol));  // separators);
	else {
		// no separator => single-param cmd, BUT explicit key=value possible
		// mout.warn() << "push back entries:" << entries << mout.endl;
		p.push_back(entries);
	}
	importEntries<STRICT>(p, assignmentSymbol);

}

/*

template <class T>
template <bool STRICT>
void SmartMap<T>::importEntries(const std::list<std::string> & p, char assignmentSymbol){ // , bool updateOnly){

	Logger mout(__FUNCTION__, __FILE__);

	// NUEVO3 SmartMapTools::setValues<T,STRICT>(*this, p, assignmentSymbol);

	//SmartMapTools::setValues<T,STRICT>(*this, getKeyList(), p, assignmentSymbol);

	const std::string assignmentSymbols(1, assignmentSymbol);

	const std::list<std::string> & keys = getKeyList();
	std::list<std::string>::const_iterator kit = keys.begin();


	bool acceptOrderedParams = true;

	// mout.warn() << " assignmentSymbol: " <<  assignmentSymbol << mout.endl;
	// mout.warn() << " size: " <<  this->size() << mout.endl;

	for (std::list<std::string>::const_iterator pit = p.begin(); pit != p.end(); ++pit){

		//mout.warn() << " entry: " << *pit << mout.endl;

		// Check specific assignment, ie. check if the key=value is given explicitly.
		if (assignmentSymbol){ // typically '='
			std::string key;
			std::string value;
			if (StringTools::split2(*pit, key, value, assignmentSymbols)){
				// mout.warn() << " specified " <<  key << "=\t" << value << mout.endl;
				if (this->size()==1){
					iterator it = this->begin();
					if (key == it->first)
						it->second = value;
					else
						it->second = *pit;
					return;
				}

				importEntry<std::string,STRICT>(key, value); //, criticality);
				acceptOrderedParams = false;
				continue;
			}
			else {
				// mout.warn() << " could not split: " << *pit << mout.endl;
			}
		}

		// Key and assignment symbol not given.

		if (kit != keys.end()){
			// Assignment-by-order
			if (!acceptOrderedParams){
				mout.warn() << "positional arg '" << *pit << "' for ["<< *kit << "] given after explicit args" << mout.endl;
			}
			//mout.warn() << " ordered  " <<   << mout.endl;
			(*this)[*kit] = *pit;  // does not need to call import() because *kit exists.
			++kit; // NUEVO
		}
		else {
			//mout.log(criticality)
			// << "too many (over "<< this->size() << ") params, run out of keys with entry=" << *pit << mout.endl;
			if (STRICT){
				mout.error() << "too many (over "<< this->size() << ") params, run out of keys with entry=" << *pit << mout.endl;
			}

		}

	}

}
*/


template <class T>
template <class S>
void SmartMap<T>::setValuesSEQ(const S & sequence){

	Logger log(__FUNCTION__, __FILE__);

	const std::list<std::string> & keys = getKeyList();
	std::list<std::string>::const_iterator kit = keys.begin();

	for (typename S::const_iterator it = sequence.begin(); it != sequence.end(); ++it){

		if (kit != keys.end()){
			// Assignment-by-order
			(*this)[*kit] = *it;  // does not need to call import() because *kit exists.
			++kit; // NUEVO
		}
		else {
			log.error() << "too many ("<< sequence.size() << ") params for map of size ("<< this->size() << "), run out of keys with entry=" << *it << log.endl;
		}

	}

}

/*
template <class T>
std::ostream & SmartMap<T>::toStream(std::ostream & ostr, char equal, char startChar, char endChar, char separatorChar) const {

	// Otherways ok, but key order not used: SprinterBase::sequenceToStream(ostr, *this, SprinterBase::jsonLayout);
	//return SprinterBase::mapToStream(ostr, *this, SprinterBase::jsonLayout, this->getKeyList());


	const std::list<std::string> & keys = this->getKeyList();

	separatorChar = separatorChar ? separatorChar : this->separator;
	//separatorChar = separatorChar ? separatorChar : ',';  // needed?
	char sep = 0;

	for (std::list<std::string>::const_iterator it = keys.begin(); it != keys.end(); ++it){

		if (sep){
			//if (it != keys.begin())
			ostr << sep;
		}
		else {
			sep = separator;
		}

		ostr << *it << equal;
		if (startChar)
			ostr << startChar;
		ostr << (*this)[*it];
		if (endChar)
			ostr << endChar;

	}
}
 */


template <class T>
void SmartMap<T>::toJSON(std::ostream & ostr, size_t indent) const {

	const std::string space(indent, ' ');

	char sep = 0;
	ostr << "{\n";

	// NOTE: alphabetic order. Should JSON dump have orig. order?
	for (const_iterator it = this->begin(); it != this->end(); ++it){

		const std::string & key = it->first;
		const T & item = it->second; //(*this)[key];

		if (sep){
			ostr << sep;
			ostr << '\n';
		}
		else {
			sep = ',';
		}
		//ostr << '\n';
		ostr << space << "\"" << key << "\" : ";

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
					SprinterBase::toStream(ostr, item, SprinterBase::plainLayout);
					// JSONwriter::toStream(item, ostr); // Better! Forces commas.
					// ostr << '[' << item << ']';
			}
		}
	}
	// ostr << "{\n  \"value\":" << *this << ",\n";
	// ostr << "  \"type\":" << drain::Type::getTypeChar(getType()) << ",\n";
	ostr << "\n" << space << "}\n";  // \n needed?
}

template <class T>
void SmartMap<T>::valuesToJSON(std::ostream & ostr) const {


	char sep = 0;

	if (this->size() > 1)
			ostr << "[";

	// NOTE: alphabetic order. Should JSON dump have orig. order?
	for (const_iterator it = this->begin(); it != this->end(); ++it){

		//const std::string & key = it->first;
		const T & item = it->second; //(*this)[key];

		if (sep){
			ostr << sep;
			ostr << ' ';
		}
		else {
			sep = ',';
		}

		if (item.T::isString()){
			ostr << '"';
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

	if (this->size() > 1)
		ostr << "]";

}

template <class T>
void SmartMap<T>::dump(std::ostream & ostr) const {

	//const std::string space(indent, ' ');
	for (const_iterator it = this->begin(); it != this->end(); ++it){
		ostr << it->first << ':' <<  ' ';
		it->second.info(ostr);
		ostr << '\n';
	}

}

template<class T>
std::ostream &operator<<(std::ostream &ostr, const SmartMap<T> & m){
	m.toStream(ostr, '=', 0, 0, m.separator);
	return ostr;
}

/*
template <class T>
//template <>
inline
std::ostream & SprinterBase::toStream(std::ostream & ostr, const SmartMap<T> & smap, const SprinterLayout & layout){
	return SprinterBase::mapToStream(ostr, smap.getMap(), layout, smap.getKeys());
}
*/

} // drain


#endif // Drain
