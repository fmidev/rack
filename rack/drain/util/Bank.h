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
#ifndef DRAIN_BANK_H_
#define DRAIN_BANK_H_

//
#include <iostream>
#include <map>

#include "Log.h"
#include "Registry.h"
#include "Cloner.h"


namespace drain
{

// Container storing entries of different classes derived from T.
/**
 *  A single entry for each class D derived from T can be stored.
 *  The that \i base entry, any number of entries can be cloned.
 *  The cloned entries are of type D, yet accessible only through
 *  a reference of base class T .
 *
 *  \tparam T - value type (base type for cloning)
 *  \tparam K - key type
 *
 *  An entry may have several keys. Single-char keys (aliases) are provided by SuperBank
 */
template <class T, class K=std::string>
class Bank2 : protected std::map<K, ClonerBase<T> *> {

public:

	/// Public key type. (Key type used in the public interface.)
	typedef K key_t;

	///
	typedef ClonerBase<T> cloner_t;

	typedef std::map<K, cloner_t *> map_t;

	/// Adds class D as an internal instance.
	/**
	 *  For each class D, a single static instance is created.
	 *
	 *  It is allowed to adding a class repeatedly with different keys.
	 *
	 *  Changing static to external instance and vice versa causes a warning.
	 */
	template <class D>
	D & add(const K & key){
		//const std::string & k = ;
		static drain::Cloner<T,D> cloner;
		set(key, cloner); // why resolve here? Alias should not be used in add calls.
		//set(resolve(key), cloner); // why resolve here? Alias should not be used in add calls.
		return cloner.src;
	}

	// Add something that has getName()
	/*
	template <class D>
	D & add(){
		//const std::string & k = ;
		static drain::Cloner<T,D> cloner;
		set(cloner.src.getName(), cloner);
		return cloner.src;
	}
	*/

	/// Adds class D as an external instance.
	/**
	 *  For each class D, a single static instance is created.
	 *
	 *  It is allowed to adding a class repeatedly with different keys.
	 *
	 *  Changing static to external instance and vice versa causes a warning.
	 *  Changing external instances is allowed.
	 */
	template <class D>
	D & addExternal(const K & key, D & entry){
		static drain::Cloner<T,D> cloner(entry);
		//set(resolve(key), cloner);
		set(key, cloner);
		return cloner.src;
	}

	/// Check if a cloner is defined for this key.
	inline
	bool has(const K & key) const {
		return (this->find(resolve(key)) != this->end());
	}

	/// Return a copy of the base instance.
	inline
	T & clone(const K & key) const {
		typename map_t::const_iterator it = this->find(resolve(key));
		if (it != this->end()){
			return it->second->clone();
		}
		else {
			throw std::runtime_error(resolve(key) + ": no such entry");
		}
	}

	/// Returns the base instance.
	inline
	T & get(const K & key){
		typename map_t::iterator it = this->find(resolve(key));
		if (it != this->end()){
			return it->second->get();
		}
		else {
			throw std::runtime_error(resolve(key) + ": no such entry");
		}
	}

	/// Return the internal static entry.
	/*
	 *  This method is useful if one wants to first set parameters of the
	 *  source and the create clones of it.
	 *
	 */
	inline
	cloner_t & getCloner(const K & key) const {
		typename map_t::const_iterator it = this->find(resolve(key));
		if (it != this->end()){
			return *(it->second);
		}
		else {
			throw std::runtime_error(key + ": no such entry");
		}
	}

	/*
	inline
	void clear(const K &key, T *ptr){
		cloner_t & cloner = getCloner(key);
		cloner.clear(*ptr);
	}
	*/

	///
	inline
	const map_t & getMap() const{
		return *this;
	}

	///
	/**
	 *  If K != std::string, this class must be derived and this function redefined.
	 */
	virtual
	const std::string & resolve(const K & key) const {
		return key;
	}


protected:

	/**
	 *   Raises a warning if cloner is changed from external to internal, or vice versa.
	 */
	drain::ClonerBase<T> & set(const std::string & key, drain::ClonerBase<T> & cloner){

		const typename map_t::iterator it = this->find(key);

		if (it != this->end()){
			// Redefined existing (making alias?)
			if (it->second != &cloner){
				std::cerr << __FILE__ << " warning: changing cloner source for: " << key << "\n";
				//throw std::runtime_error(key + ": no such entry");
			}
			else {
				// Re-introduce existing?
				std::cerr << __FILE__ << " warning: re-defining: " << key << "\n";
			}
			it->second = &cloner;
		}
		else {
			// Create new
			this->operator [](key) = &cloner;
		}

		return cloner;

	}

};

// WIth alias support
template <class T>
class BankSuper : public Bank2<T, std::string> {

public:

	typedef Bank2<T, std::string> bank_t;
	typedef typename bank_t::key_t key_t;

	// Overrides

	template <class D>
	D & add(const std::string & key, char alias=0){
		if (alias)
			setAlias(alias, key);
		// TODO: what if the KEY is a single char?
		return Bank2<T, std::string>::template add<D>(key);
	}

	template <class D>
	D & addExternal(const std::string & key, D & entry){
		return Bank2<T, std::string>::template addExternal<D>(key, entry);
	}

	template <class D>
	D & addExternal(const std::string & key, char alias, D & entry){
		if (alias)
			setAlias(alias, key);
		// TODO: what if the KEY is a single char?
		return Bank2<T, std::string>::template addExternal<D>(key, entry);
	}




	inline
	void setAlias(char alias, const std::string & value){
		aliases.add(alias, value);
	}

	inline
	bool hasAlias(const std::string & value) const {
		if (value.empty())
			return false;
		else if (value.length()==1)
			return hasAlias(value.at(0));
		else
			return aliases.hasValue(value);
	}

	inline
	bool hasAlias(char alias) const {
		return aliases.hasKey(alias);
	}

	inline
	char getAlias(const std::string & value) const {
		if (value.length() > 1)
			return aliases.getKey(value);
		else if (value.length()==1){
			// Warn ?
			char c = value.at(0);
			if (aliases.hasKey(c))
				return c;
		}
		return '\0';
	}



	inline
	const drain::Dictionary2<char, key_t> & getAliases() const {
		return aliases;
	}


	/// Given alias or long key, returns the long key .
	/**
	 *
	 */
	// TODO: what is the KEY is a single char?
	virtual inline
	const std::string & resolve(const key_t & value) const {
		//std::cout << __FUNCTION__ << ':' << value << '\n';
		if (value.length() == 1)
			return aliases.getValue(value.at(0));
		else
			return value;
	}

	void toStream(std::ostream & ostr = std::cout) const {
		for (typename bank_t::const_iterator it = this->begin(); it != this->end(); ++it) {
			ostr << it->first << ':' << it->second->count() << '\n';
		}
	}


protected:

	drain::Dictionary2<char, std::string> aliases;


};


/// A registry that contains items that can be cloned with clone() or referenced directly with get().
template <class T>
class Bank : public Registry<ClonerBase<T> > {

public:

	/// Adds static entry of type D to current section of registry.
	template <class D>
	void add2(const std::string & key){
		static drain::Cloner<T,D> cloner;
		this->add(cloner, key);
	}

	inline
	T & clone(const std::string & key) const {
		// return this->template Registry<ClonerBase<T> >::get(key).clone();
		return this->Registry<ClonerBase<T> >::get(key).clone();
	}

	inline
	T & get(const std::string & key){
		return this->Registry<ClonerBase<T> >::get(key).get();
		//return this->Registry<ClonerBase<T> >::get(key).get();
	}


};

}

#endif

// Drain
