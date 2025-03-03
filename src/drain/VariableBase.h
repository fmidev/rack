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



#ifndef DRAIN_VARIABLE_BASE
#define DRAIN_VARIABLE_BASE

#include "Castable.h"
#include "CastableIterator.h"

#include "VariableT.h"


namespace drain {


///  Base class for variables: Variable, Reference and FlexibleVariable.
/**
 *   - Variable
 *   - Reference
 *   - FlexibleVariable
 *
 *   \see Variable
 *   \see Reference
 *   \see FlexibleVariable
 */
class VariableBase : public Castable {

public:

	// Like in images. Should be fixed.
	typedef CastableIterator const_iterator;
	typedef CastableIterator iterator;

	//typedef std::pair<const char *,const drain::VariableBase> init_pair_t;


	virtual inline
	~VariableBase(){};

	/// Tells if the pointer points to an external variable.
	/**
	 *
	 */
	virtual inline // true also if ptr == null
	bool isLinking() const {
		if (caster.ptr == nullptr){
			return false;
		}
		else if (data.empty()){ // this is never true
			return true;
		}
		else {
			return (caster.ptr != (void *) &data[0]);
			/*
			if (caster.ptr != (void *) &data[0]){ // Debugging
				std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " non-empty data [" << getTypeName() << "], but pointer discarding it " << std::endl;
				std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " value: '" << *this << "'" << std::endl;
				//info(std::cerr);
			}
			return false;
			*/
		}

	}

	/// Tells if internal memory exists.
	/**
	 *   Note.
	 *   In a drain::Variable,  \c ptr always points to data array, or to null, if the array is empty.
	 *   In a drain::FlexibleVariable, \c ptr can point to owned data array or to an external (base type) variable.
	 *
	 *   \return - \c true for drain::Variable and drain::FlexibleVariable, and \c false for drain::Reference.
	 */
	virtual inline
	bool isVariable() const {
		return true;
	}


	/// Tells if the internal pointer can point to an external variable.
	/**
	 *  \return - \c true for Reference and FlexibleVariable, and \c false for Variable.
	 */
	virtual inline
	bool isLinkable() const {
		return false;
	}


	/*  Trying to raise to Castable
	VariableBase & append(){
		return *this;
	}

	template <class T, class ...TT>
	VariableBase & append(const T &x, const TT& ...rest){
		Castable::operator<<(x);
		append(rest...);
		return *this;
	}
	*/

	template <class T, class ...TT>
	// VariableBase &
	void set(const T &arg, const TT& ...args){
		clear();
		// (*this) = x; // 2025
		assign(arg); // 2025
		//return append(rest...);
		append(args...);
	}

	/// Does not change separator chars.
	/*
	inline
	void reset(){
		caster.unsetType();
		//resize(0);
		updateSize(0);
		setSeparator(',');
		//this->separator = ','; // semantics ?
	}
	*/

	/// Sets basic type or void.
	/**
	 *   Also std::string is accepted, but handled as request of \c char array.
	 */
	virtual
	void setType(const std::type_info & t) override ;

	/// Overriding (shadowing) Castable::setType()
	template <class T>
	inline
	void setType(){
		setType(typeid(T));
	}

	/// Extends the array by one element.
	/*
	 *  Note: if a std::string is given, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	VariableBase & operator<<(const T &x){
		//Castable::operator<<(x);
		append(x);
		return *this;
	}



	/// Like with std::iterator.
	/**
	 *   Notice: in the case of std::strings, will iterate through characters.
	 */
	inline
	const CastableIterator & begin() const { return dataBegin; };

	/// Like with std::iterator.
	/*
	 *  Notice: in the case of std::strings, will iterate through characters.
	 */
	inline
	const CastableIterator & end() const { return dataEnd; };


	/// Extends the array to include \c elementCount elements of current type.
	/**
	 *  Needed for example in reading data directly using a pointer.
	 */
	virtual inline
	bool setSize(size_t elementCount){
		updateSize(elementCount);
		return true;
	}

	///
	virtual inline
	void ensureSize(size_t elementCount){
		if (this->elementCount < elementCount){
			updateSize(elementCount);
		}
		// return true;
	}

protected:


	/***
	 *   This is the basic design pattern for all the VariableLikes
	template <class ...TT>
	inline
	VariableBase(const TT & ...args) {
		init(args...); //
	}
	 */

	virtual
	void updateSize(size_t elementCount);

private:

	std::vector<char> data;

	mutable CastableIterator dataBegin;
	mutable CastableIterator dataEnd;


private:

	void updateIterators();

};

/*
template <class T>
class VariableT<T>;
*/

/**
 *  \tparam T - VariableBase for Variable
 */
template <class V>
class VariableInitializer : public V {

protected:

	/// Copy constructor handler
	/*
	template <class T>
	void init(const VariableInitializer<V> & value) {
		std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " " << value << '[' << typeid(T).name() << ']'<< std::endl;
		this->reset();
		this->assignCastable(value);
	}
	*/


	/// Copy constructor handler
	/**
	 *     | V | F | R |
	 *   V | % | % | % |
	 *   F | V | F | R |
	 *   R | V | F | R |
	 */
	template <class T>
	void init(const VariableT<T> & value) {
		// drain::Logger(__FILE__, __LINE__, __FUNCTION__).warn(drain::TypeName<VariableT<VariableInitializer<V> > >::str(),
		// 		'(', drain::TypeName<VariableT<T> >::str(), ' ', value, ')');
		this->reset();
		this->assignCastable(value);
	}

	template <class T>
	void init(VariableT<T> & value) {
		// drain::Logger(__FILE__, __LINE__, __FUNCTION__).warn(drain::TypeName<VariableT<VariableInitializer<V> > >::str(),
		// 		'(', drain::TypeName<VariableT<T> >::str(), ' ', value, ')');
		this->reset();
		if (value.isLinking() && this->isLinkable()){
			this->relink(value);
			// this->setPtr(value.getPtr(), value.getType(), value.getElementCount());
		}
		this->assignCastable(value);
	}


	/*
	template <class D>
	void init(const D & dst){
		reset();
		assign(dst);
	}
	*/
	/// Default constructor generates an empty array.
	inline
	void init(const std::type_info &t = typeid(void)) {
		this->reset();
		this->setType(t);
	};

	// Initialisation, using type of argument or explicit type argument.
	// Copies type, data and separator char. Fails with Reference?
	template <class T>
	void init(const T & value) {
		// std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " " << value << '[' << typeid(T).name() << ']'<< std::endl;
		// std::cerr << __LINE__ << " reset: " << std::endl;
		this->reset();
		// std::cerr << __LINE__ << " assign: " << std::endl;
		this->assign(value); // Critical, direct assignment *this = value fails
		// std::cerr << __LINE__ << " ...done " << std::endl;
	}


	template <class T>
	void init(const T & value, const std::type_info &t) { //  = typeid(void)
		std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " " << value << '[' << t.name() << ']' << std::endl;

		std::cerr << __LINE__ << " reset: " << std::endl;
		this->reset();
		std::cerr << __LINE__ << " setType: " << std::endl;
		this->setType(t);
		std::cerr << __LINE__ << " assign: " << std::endl;
		this->assign(value); // Critical, direct assignment *this = value fails
		std::cerr << __LINE__ << " ...done " << std::endl;
		// ;
	}


	/// Copies type, data and separator char.
	inline
	void init(const VariableBase & v) {
		std::cerr << __FILE__ << ':' << __LINE__ << __FUNCTION__ << " " << v << std::endl;
		this->reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		this->assignCastable(v);
	};

	/// Copies type, data and separator char.
	inline
	void init(const Castable & c) {
		std::cerr << __FILE__ << ':' << __LINE__ << __FUNCTION__ << " " << c << std::endl;
		this->reset();
		//this->outputSeparator = c.outputSeparator;
		//this->inputSeparator  = c.inputSeparator;
		this->assignCastable(c);
	};

	/// Copies type, data and separator char.
	inline
	void init(const char * s) {
		// std::cerr << __FILE__ << ':' << __LINE__ << __FUNCTION__ << " " << s << std::endl;
		this->reset();
		this->assignString(s);
	};

	/// Initialisation with type of the first element or explicit type argument.
	template<typename T>
	inline
	void init(std::initializer_list<T> l, const std::type_info &t = typeid(void)) {
		// std::cerr << __FILE__ << ':' << __LINE__ << __FUNCTION__ << " " << std::endl;
		this->reset();
		this->setType(t);
		this->assignContainer(l, true);
	};

	template <class ...TT>
	void init(const TT& ...args){
		// std::cerr << __FILE__ << ':' << __LINE__ << __FUNCTION__ << " (variadic args)" << std::endl;
		this->reset();
		this->append(args...);
	}

};

}

#endif
