/**

    Copyright 2015  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef DRAIN_REGISTRY_H_
#define DRAIN_REGISTRY_H_

#include <map>
#include <set>

#include "../util/Log.h"
#include "../util/ReferenceMap.h"
#include "../util/VariableMap.h"
//#include "../util/StringMapper.h"

//#include "Command.h"


namespace drain {

//#define REGISTRY_DEFAULT "DEFAULT"



/// A container for storing global objects like functors, operators or commands. Supports aliases.
/*
 *  Commands are stored in a map of type  std::map<std::string, T &>.
 *
 *  Command sections can be used for grouping.
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

	/// Adds entry of type T to current section of registry.
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

protected:

	map_t entryMap;
	std::map<char,std::string> aliases;
	std::map<std::string,char> aliasesInv;

};


template <class T>
void Registry<T>::add(T & r, const std::string & name, char alias){

	entryMap.insert(std::pair<std::string, T &>(name, r));

	if (alias){
		aliases[alias]   = name;
		aliasesInv[name] = alias;
	}

}



template <class T>
bool Registry<T>::has(const std::string & name) const {
	//const typename map_t::const_iterator it = find(name);
	return (find(name) != entryMap.end());
}

template <class T>
const std::string & Registry<T>::getKey(char alias) const{
	static const std::string empty;
	const std::map<char,std::string>::const_iterator it = aliases.find(alias);
	if (it != aliases.end())
		return it->second;
	else
		return empty;
}


template <class T>
char Registry<T>::getAlias(const std::string &name) const{
	const std::map<std::string,char>::const_iterator it = aliasesInv.find(name);
	if (it != aliasesInv.end())
		return it->second;
	else
		return 0;
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
