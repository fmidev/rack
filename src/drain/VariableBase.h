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
	bool isReference() const {
		return (caster.ptr != (void *) &data[0]);
	}

	/// Tells if the internal pointer can point to an external variable.
	/**
	 *
	 */
	/// Returns always true.
	/**
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
	 *  \return - \c true for Reference and Flexible, and \c false for Variable.
	 */
	virtual inline
	bool isLinkable() const {
		return false;
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

	/*
	template <class D>
	void init(const D & dst){
		reset();
		assign(dst);
	}
	*/

	/// Default constructor generates an empty array.
	inline
	void initOLD(const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
	};

	// Initialisation, using type of argument or explicit type argument.
	// Copies type, data and separator char. Fails with Reference?
	template <class T>
	void initOLD(const T & value, const std::type_info &t = typeid(void)) {
		// std::cerr << __FILE__ << ':' << __LINE__ << " VariableBase::" << __FUNCTION__ << " " << value << std::endl;
		reset();
		setType(t);
		this->assign(value); // Critical, direct assignment *this = value fails
		// ;
	}


	/// Copies type, data and separator char.
	inline
	void initOLD(const VariableBase & v) {
		reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		assignCastable(v);
	};

	/// Copies type, data and separator char.
	inline
	void initOLD(const Castable & c) {
		reset();
		//this->outputSeparator = c.outputSeparator;
		//this->inputSeparator  = c.inputSeparator;
		assignCastable(c);
	};

	/// Copies type, data and separator char.
	inline
	void initOLD(const char * s) {
		reset();
		assignString(s);
	};

	/// Initialisation with type of the first element or explicit type argument.
	template<typename T>
	inline
	void initOLD(std::initializer_list<T> l, const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
		assignContainer(l, true);
	};

	template <class ...TT>
	void initOLD(const TT& ...args){
		reset();
		append(args...);
	}



public:

	//template <class T>
	VariableBase & append(){
		return *this;
	}

	template <class T, class ...TT>
	VariableBase & append(const T &x, const TT& ...rest){
		Castable::operator<<(x);
		append(rest...);
		return *this;
	}

	template <class T, class ...TT>
	VariableBase & set(const T &x, const TT& ...rest){
		clear();
		(*this) = x;
		return append(rest...);
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
	void setType(const std::type_info & t);



	/// Extends the array by one element.
	/*
	 *  Note: if a std::string is given, does not split it like with Variable::operator=(const char *).
	 */
	template <class T>
	inline
	VariableBase &operator<<(const T &x){
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



//protected:


	/// Extends the array to include \c elementCount elements of current type.
	/**
	 *  Needed for example in reading data directly using a pointer.
	 */
	virtual inline
	bool setSize(size_t elementCount){
		// TODO: if is reference?
		//return updateSize(elementCount); // fix/check: always returns true?
		updateSize(elementCount);
		return true;
	}

protected:

	virtual
	// bool
	void updateSize(size_t elementCount);

private:

	std::vector<char> data;

	mutable CastableIterator dataBegin;
	mutable CastableIterator dataEnd;


private:



	void updateIterators();

};

/**
 *  \tparam T - VariableBase for Variable
 */
template <class V>
class VariableInitializer : public V {
protected:

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
	void init(const T & value, const std::type_info &t = typeid(void)) {
		// std::cerr << __FILE__ << ':' << __LINE__ << " VariableBase::" << __FUNCTION__ << " " << value << std::endl;
		this->reset();
		this->setType(t);
		this->assign(value); // Critical, direct assignment *this = value fails
		// ;
	}


	/// Copies type, data and separator char.
	inline
	void init(const VariableBase & v) {
		this->reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		this->assignCastable(v);
	};

	/// Copies type, data and separator char.
	inline
	void init(const Castable & c) {
		this->reset();
		//this->outputSeparator = c.outputSeparator;
		//this->inputSeparator  = c.inputSeparator;
		this->assignCastable(c);
	};

	/// Copies type, data and separator char.
	inline
	void init(const char * s) {
		this->reset();
		this->assignString(s);
	};

	/// Initialisation with type of the first element or explicit type argument.
	template<typename T>
	inline
	void init(std::initializer_list<T> l, const std::type_info &t = typeid(void)) {
		this->reset();
		this->setType(t);
		this->assignContainer(l, true);
	};

	template <class ...TT>
	void init(const TT& ...args){
		this->reset();
		this->append(args...);
	}

};

}

#endif
