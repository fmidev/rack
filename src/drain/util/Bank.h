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
#include <drain/Log.h>
#include <iostream>
#include <map>

#include <drain/Sprinter.h>

#include "Registry.h"
#include "Cloner.h"
#include "Flags.h"
#include "Static.h"

namespace drain
{

// NEW

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
// TODO: remove resolve(key), lower it to superbank, with applying functions; redefine has(), get(), etc.
template <class T, class K=std::string>
class Bank : protected std::map<K, ClonerBase<T> *> {

public:

	/// Public key type. (Key type used in the public interface.)
	typedef K key_t;

	/// Base type of cloned objects.
	typedef T value_t;

	///
	typedef ClonerBase<T> cloner_t;

	/// Base class
	typedef std::map<K, cloner_t *> map_t;

	/// For enumerating keys. @see getKeys().
	typedef std::set<K> key_set_t;

	struct bank_id {
	};

	virtual inline
	~Bank(){
	}

	template <class D>
	static
	Cloner<T,D> & getCloner(){
		static Cloner<T,D> & cloner = Static::get<Cloner<T,D>, bank_id>();
		return cloner;
	}


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
		static Cloner<T,D> & cloner = Bank<T,K>::getCloner<D>(); // Static::get<Cloner<T,D>, bank_id>();
		set(key, cloner);
		return cloner.src;
	}

	// Add something that has getName()
	template <class D>
	D & add(){
		static Cloner<T,D> & cloner = Bank<T,K>::getCloner<D>(); // Static::get<Cloner<T,D>, bank_id>();
		set(cloner.src.getName(), cloner);
		return cloner.src;
	}

	/// Adds class D using a copy constructor on an external instance.
	/**
	 *  For each class D, a single static instance is created.
	 *
	 *  It is allowed to adding a class repeatedly with different keys.
	 *
	 *  Changing static to external instance and vice versa causes a warning.
	 *  Changing external instances is allowed.
	 */
	template <class D>
	D & addExternal(const D & entry, const K & key){
		//static Cloner<T,D> & cloner = Static::get<Cloner<T,D>, bank_id>();
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
			return it->second->getCloned();
		}
		else {
			throw std::runtime_error(resolve(key) + '[' + key + "]: no such entry");
		}
	}

	/// Returns the base instance.
	inline
	const T & get(const K & key) const {
		typename map_t::const_iterator it = this->find(resolve(key));
		if (it != this->end()){
			return it->second->getSource();
		}
		else {
			throw std::runtime_error(resolve(key) + '[' + key + "]: no such entry");
		}
	}

	/// Returns the base instance.
	inline
	T & get(const K & key){
		typename map_t::iterator it = this->find(resolve(key));
		if (it != this->end()){
			return it->second->getSource();
		}
		else {
			throw std::runtime_error(resolve(key) + '[' + key + "]: no such entry");
		}
	}

	/// Returns a map entry: pair<Key,D *>()
	template <class D>
	typename map_t::const_iterator get() const {
		static const cloner_t & cloner = Bank<T,K>::getCloner<D>();
		for (typename map_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (it->second == &cloner)
				return it;
		}
		throw std::runtime_error("find(): no such entry");
		return this->end();
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
	template <class D>
	cloner_t & getCloner() const {
		static const cloner_t & cloner = Bank<T,K>::getCloner<D>();
		for (typename map_t::const_iterator it = this->begin(); it != this->end(); ++it){
			if (it->second == &cloner)
				return it;
		}
		throw std::runtime_error("getCloner(): no such entry");
		return this->end();
	}
	*/

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
	// TODO: remove this stupid resolve(key), it works only with K=std::string, and cannot work (easily) with other classes.
	// lower it to superbank, with applying functions; redefine has(), get(), etc.
	virtual
	const std::string & resolve(const K & key) const {
		return key;
	}

	void toStream(std::ostream & ostr = std::cout) const {
		/*
		Sprinter::sequenceToStream(ostr, getMap(), Sprinter::lineLayout);
		for (typename map_t::const_iterator it = this->begin(); it != this->end(); ++it) {
			ostr << it->first << ':' << it->second->getSource() << '\n';
		}
		*/
		for (const auto & entry: *this) {
			ostr << entry.first << ':' << entry.second->getSource() << '\n';
		}


		//std::cout << it->first << ' ' << it->second->getSource() << '\n';
	}

	inline
	key_set_t & getKeys(){
		return keys;
	}


protected:

	key_set_t keys;

	/**
	 *   Raises a warning if cloner is changed from external to internal, or vice versa.
	 */
	drain::ClonerBase<T> & set(const std::string & key, drain::ClonerBase<T> & cloner){

		const typename map_t::iterator it = this->find(key);

		if (it != this->end()){
			// Redefined existing (making brief?)
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
			keys.insert(key);
		}

		return cloner;

	}

};

template <class T>
std::ostream & operator<<(std::ostream & ostr, const Bank<T> & bank) {
	bank.toStream(ostr);
	return ostr;
}

/// A Bank with additional support for brief, single char keys.
/**
 *   Consider a command like "--outputFile" abbreviated "-o".
 *
 */
template <class T>
class BankSuper : public Bank<T, std::string>  {

public:

	typedef Bank<T, std::string> bank_t;
	typedef typename bank_t::key_t key_t;
	typedef typename bank_t::bank_id bank_id;

	typedef T data_t;

	BankSuper(): bank_t() { //,  sectionFlags(sections){
	}

	BankSuper(const BankSuper & bank): bank_t(bank){ //,  sectionFlags(sections){
		std::cerr << __FUNCTION__ << ": copy const" << std::endl;
	}


	// Short (brief) keys  needs repeating code or Bank::add()
	// Note: BankSuper uses same cloner as Bank. Consider leniency.
	template <class D>
	D & add(const std::string & key, char brief=0){
		static Cloner<T,D> & cloner = Static::get<Cloner<T,D>, bank_id>();
		// static const D & clonerSrc = Static<Cloner<T,D>, bank_id>::getSource().src;
		if (brief)
			setBriefKey(brief, key);
		bank_t::set(key, cloner);
		return cloner.src;
	}

	/// Add something that has getName()
	//  Short (brief) key needs repeating code or Bank::add()
	template <class D>
	D & add(char brief=0){
		static Cloner<T,D> & cloner = Static::get<Cloner<T,D>, bank_id>();
		if (brief)
			setBriefKey(brief, cloner.src.getName());
		bank_t::set(cloner.src.getName(), cloner);
		return cloner.src;
	}


	/*
	template <class D>
	D & addExternal(D & entry, const std::string & key ){
		return Bank<T, std::string>::template addExternal<D>(key, entry);
	}
	*/

	template <class D>
	D & addExternal(const D & entry, const std::string & key, char brief = 0){
		if (brief)
			setBriefKey(brief, key);
		return Bank<T, std::string>::addExternal(entry, key);
		//return Bank<T, std::string>::template addExternal<D>(entry, key);
	}

	/// Now, D::getName() is required.
	/*  well, ok
	template <class D>
	D & addExternal(D & entry, char brief = 0){
		std::string key = entry.getName()+"MIKA";
		if (brief)
			setBriefKey(brief, key);
		// TODO: what if the KEY is a single char?
		return Bank<T, std::string>::template addExternal<D>(key, entry);
	}
	*/



	inline
	void setBriefKey(char brief, const std::string & value){
		briefKeys.add(brief, value);
	}

	inline
	bool hasAlias(const std::string & value) const {
		if (value.empty())
			return false;
		else if (value.length()==1)
			return hasAlias(value.at(0));
		else
			return briefKeys.hasValue(value);
	}

	inline
	bool hasAlias(char brief) const {
		return briefKeys.hasKey(brief);
	}

	inline
	char getAlias(const std::string & value) const {
		if (value.length() > 1)
			return briefKeys.getKey(value);
		else if (value.length()==1){
			// Warn ?
			char c = value.at(0);
			if (briefKeys.hasKey(c))
				return c;
		}
		return '\0';
	}



	inline
	const drain::Dictionary<char, key_t> & getAliases() const {
		return briefKeys;
	}


	/// Given brief or long key, returns the long key .
	/**
	 *
	 */
	// TODO: what is the KEY is a single char?
	virtual inline
	const std::string & resolve(const key_t & value) const {
		//std::cout << __FUNCTION__ << ':' << value << '\n';
		if (value.length() == 1)
			return briefKeys.getValue(value.at(0));
		else
			return value;
	}


	/// Set name and brief description of a program, to appear in help dumps.
	inline
	void setTitle(const std::string & title){
		this->title = title;
	};

	/// Get the name of this program (command set)
	inline
	const std::string & getTitle() const {
		return this->title;
	};

	FlagResolver::ivalue_t addSection(const FlagResolver::dict_t::key_t & title, const FlagResolver::dict_t::value_t index=0){
		// Flagger2<int>::value_t addSection(const FlagResolver::dict_t::key_t & title, const FlagResolver::dict_t::value_t index=0){
		return FlagResolver::addEntry(sections, title, index);
	}


	FlagResolver::dict_t sections;


protected:

	drain::Dictionary<char, std::string> briefKeys;

	/// For example, name of the program, to appear in help dumps etc.
	std::string title;

};


/// Creates an entries offered by a bank and destroys them upon exit.
/**
 *  Creating entries using a static Cloner provided by a drain::Bank.
 *  The entries are basically global.
 *  However, they become destroyed upon desctruction of the LocalCloner instance
 *  that created (requested) them. Hence, the entries share the same scope with
 *  the LocalCloner.
 */
template <class B>
class LocalCloner {

public:

	typedef Bank<B> bank_t;

	// Inherit type
	typedef typename bank_t::cloner_t cloner_t;

	// Inherit type
	typedef typename cloner_t::entry_t entry_t;

	// Inherit type
	typedef typename cloner_t::index_t index_t;

	const bank_t & bank;

	typedef std::map<cloner_t *, std::set<index_t> > book_t;

	LocalCloner(const bank_t & bank) : bank(bank), idx(0) { // , cloner(nullptr)
	};

	~LocalCloner(){
		for (typename book_t::value_type & v : this->book){
			drain::Logger mout(__FILE__, __FUNCTION__);
			mout.debug() << "Freeing: ";
			for (index_t i : v.second){  // << v.first->getSource().getName() not universal
				//std::cerr << __FILE__ << ':' << __FUNCTION__ << ": dropping " << i << '\n';
				mout << i << ", ";
				v.first->drop(i);
			}
			mout << mout;
		}
	};

	B & getCloned(const typename bank_t::key_t & key){

		cloner_t & cloner = bank.getCloner(key);
		entry_t entry = cloner.getClonerEntry();
		idx = entry.first;
		book[&cloner].insert(entry.first);
		return *entry.second;
		//return cloner.cloneUnique();
	}

	template <class T>
	B & getCloned(){

		typename bank_t::map_t::const_iterator it =  bank.template get<T>();
		cloner_t *cloner = it->second;
		entry_t entry = cloner->getClonerEntry();
		idx = entry.first;
		book[cloner].insert(entry.first);
		return *entry.second;
		//idx = entry.first;
		//return *(it->second->getCloned());
		//return cloner.cloneUnique();
	}


	// Returns last index

	inline
	index_t getIndex() const{
		return idx;
	}


protected:

	book_t book;

	//cloner_t *cloner;
	//entry_t entry;
	index_t idx;


};









// OLD:
/*
/// A registry that contains items that can be cloned with getCloned() or referenced directly with get().
template <class T>
class BankOLD : public Registry<ClonerBase<T> > {

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
		return this->Registry<ClonerBase<T> >::get(key).getCloned();
	}

	inline
	T & get(const std::string & key){
		return this->Registry<ClonerBase<T> >::get(key).getSource();
		//return this->Registry<ClonerBase<T> >::get(key).get();
	}


};
*/

}

#endif

// Drain
