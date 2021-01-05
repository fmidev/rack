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
#ifndef DRAIN_REGISTRY_H_
#define DRAIN_REGISTRY_H_

#include <map>
#include <set>

#include "Log.h"
#include "Dictionary.h"
#include "ReferenceMap.h"
#include "VariableMap.h"
//#include "../util/StringMapper.h"

//#include "Command.h"


namespace drain {

//#define REGISTRY_DEFAULT "DEFAULT"



/// A container for storing global objects like functors, operators or commands. Supports aliases.
/*
 *  Commands are stored in a map of type  std::map<std::string, T &>.
 *
 *
 *  \see drain::Bank<T>
 */
template <class T>
class Registry {

public:

	typedef std::map<std::string, T &> map_t; /// TODO: reference not recommended

	//typedef map<string, set<string> > SectionMap;

	inline
	Registry(){}; // : DEFAULT_HANDLER("_handler"), expandVariables(false), statusFormatter("[a-zA-Z0-9_:]+") { ++index; };  // note ':' in statusFormatter


	virtual inline
	~Registry(){};

	/// Adds entry of (base) class T to current section of registry.
	virtual
	void add(T & r, const std::string & key, char alias = 0);



	/// Queries whether a command has been added.
	bool has(const std::string & key) const;

	/// Returns a single-letter abbreviation of a command.
	char getAlias(const std::string & key) const;

	/// Given a single-letter abbreviation, return the command name.
	const std::string & getKey(char alias) const;

	/// Given a single-letter abbreviation or full command name, returns the command object.
	T & get(const std::string & name) const;

	/// Returns the actual map containig the entries
	map_t & getMap(){ return entryMap; };

	/// Returns the actual map containig the entries
	const map_t & getMap() const { return entryMap; };

	/// Like find(), but handles aliases.
	inline
	const typename map_t::const_iterator find(const std::string & name) const {
		return (name.length() == 1) ? entryMap.find(getKey(name.at(0))) : entryMap.find(name);
	};

	/// Like find(), but handles aliases.
	inline
	typename map_t::iterator find(const std::string & name) {
		//return entryMap.find(resolveKey(name));
		return (name.length() == 1) ? entryMap.find(getKey(name.at(0))) : entryMap.find(name);
	};

	inline
	const drain::Dictionary2<char, std::string> & getAliases() const {
		return aliasesNew;
	}

protected:

	map_t entryMap;

	drain::Dictionary2<char, std::string> aliasesNew;

};


template <class T>
void Registry<T>::add(T & r, const std::string & name, char alias){

	entryMap.insert(std::pair<std::string, T &>(name, r));

	if (alias){
		aliasesNew.add(alias, name);
	}

}



template <class T>
bool Registry<T>::has(const std::string & name) const {
	//const typename map_t::const_iterator it = find(name);
	return (find(name) != entryMap.end());
}

template <class T>
const std::string & Registry<T>::getKey(char alias) const{

	Dictionary2<char,std::string>::const_iterator it = aliasesNew.findByKey(alias);

	if (it != aliasesNew.end())
		return it->second;
	else{
		static const std::string empty;
		return empty;
	}


}


template <class T>
char Registry<T>::getAlias(const std::string &name) const{

	Dictionary2<char,std::string>::const_iterator it = aliasesNew.findByValue(name);

	if (it != aliasesNew.end()){
		return it->first;
	}
	else{
		//std::cout << "not found: " << name << '\n';
		return 0;
	}

}

template <class T>
T & Registry<T>::get(const std::string & name) const {
	const typename map_t::const_iterator it = find(name);
	if (it != entryMap.end())
		return it->second;
	else
		throw std::runtime_error(name + ": no such entry");

}


} /* namespace drain */

#endif /* DRAINREGISTRY_H_ */

// Drain
