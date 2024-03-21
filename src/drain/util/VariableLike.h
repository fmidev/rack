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



#ifndef DRAIN_VARIABLE_LIKE
#define DRAIN_VARIABLE_LIKE

#include "Castable.h"
#include "CastableIterator.h"
//#include "Convert.h"

//#include "Referencer.h"
#include "Sprinter.h"
#include "String.h"
#include "UniTuple.h" // "Friend class"
#include "Type.h"


namespace drain {





class VariableBase : public Castable {

public:

	// Like in images. Should be fixed.
	typedef CastableIterator const_iterator;
	typedef CastableIterator iterator;

	// typedef std::pair<const char *,const drain::VariableBase> init_pair_t;

	/***
	 *   This is the basic design pattern for all the VariableLikes
	 */
	template <class ...TT>
	inline
	VariableBase(const TT & ...args) {
		init(args...); //
	}

	virtual ~VariableBase(){};

	virtual inline // true also if ptr == null
	bool isReference() const {
		return (caster.ptr != (void *) &data[0]);
	}


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
		reset();
		setType(t);
	};

	// Initialisation, using type of argument or explicit type argument.
	// Copies type, data and separator char. Fails with Reference?
	template <class T>
	void init(const T & value, const std::type_info &t = typeid(void)) {
		// std::cerr << __FILE__ << ':' << __LINE__ << " VariableBase::" << __FUNCTION__ << " " << value << std::endl;
		reset();
		setType(t);
		this->assign(value); // Critical, direct assignment *this = value fails
		// ;
	}


	/// Copies type, data and separator char.
	inline
	void init(const VariableBase & v) {
		reset();
		this->outputSeparator = v.outputSeparator;
		this->inputSeparator = v.inputSeparator;
		assignCastable(v);
	};

	/// Copies type, data and separator char.
	inline
	void init(const Castable & c) {
		reset();
		//this->outputSeparator = c.outputSeparator;
		//this->inputSeparator  = c.inputSeparator;
		assignCastable(c);
	};

	/// Copies type, data and separator char.
	inline
	void init(const char * s) {
		reset();
		assignString(s);
	};

	/// Initialisation with type of the first element or explicit type argument.
	template<typename T>
	inline
	void init(std::initializer_list<T> l, const std::type_info &t = typeid(void)) {
		reset();
		setType(t);
		assignContainer(l, true);
	};



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
	inline
	void reset(){
		caster.unsetType();
		//resize(0);
		updateSize(0);
		setSeparator(',');
		//this->separator = ','; // semantics ?
	}

	/// Sets basic type or void.
	/**
	 *   Also std::string is accepted, but handled as request of \c char array.
	 */
	virtual
	void setType(const std::type_info & t);

	/// Stronger than suggestType, which only sets type if unset.
	/**
	 * Changes type by calling setType() directly. Always supported for VariableBase.
	 *
	 *  \param t - new type
	 *  \return - always true
	 *
	 *  \see suggestType()
	 *  \see setType()
	 */


	/// Sets type, if unset.
	/**
	 *   Could be protected, but requestType() will not be.

	virtual inline
	bool suggestType(const std::type_info & t){
		 if (isReference()){
			return Castable::suggestType(t);
		}
		else

		if (!typeIsSet())
			setType(t);
		return true;
	}
	*/

	/// Request to change the array size. For Castable (and Reference) does nothing and returns false.
	/**
	 *   Could be protected, but requestType() will not be.
	 *   Does not apply to std::string;
	 *
		*/



	/// Extends the array by one element.
	/*
	inline
	Variable &operator<<(const char *s){
		// Castable::operator<<(s);
		append(s);
		return *this;
	}
	*/

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



protected:


	/// Extends the array to include \c elementCount elements of current type.
	virtual inline
	bool setSize(size_t elementCount){
		return updateSize(elementCount); // fix/check: always returns true?
	}

protected:


private:

	std::vector<char> data;

	mutable CastableIterator dataBegin;
	mutable CastableIterator dataEnd;


private:

	bool updateSize(size_t elementCount);

	void updateIterators();

};




/// Intermediate class supporting link() in various forms.
/**
 *
 */
template <class T=Castable>
class ReferenceBase : public T {

public:


	template <class D>
	void init(D & dst){
		link(dst);
	}

	template <class S>
	void init(const S & src){
		// std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "ReferenceBase::" << __FUNCTION__ << " " << src << std::endl;
		T::init(src); // Undefined for Castable -> compile time error.
	}



	template <class D>
	void init(D *dst){
		link(dst);
	}

	inline
	void init(void *p, const std::type_info &t, size_t count=1){
		link(p, t, count);
	}

	// Terminal
	inline
	void init(){
	}


	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Reference redirected to relink() )
	 */
	template <class F>
	inline
	ReferenceBase & link(F &p){
		try {
			this->setPtr(p);
		}
		catch (const std::exception & e){
			std::cerr << __FILE__ << ':' << __FUNCTION__ << ": unsupported type: " << typeid(F).name() << std::endl;
			throw std::runtime_error("unsupported type");
		}
		return *this;
	}

	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Reference redirected to relink() )
	 */
	template <class F>
	inline
	ReferenceBase & link(F *p){
		this->setPtr(p);
		return *this;
	}

	/// Set pointer to p, of given type.
	inline
	ReferenceBase & link(void *p, const std::type_info &t, size_t count=1){
		this->setPtr(p, t);
		this->elementCount = count;
		return *this;
	}

	inline
	ReferenceBase & link(void *p){
		throw std::runtime_error(std::string("ReferenceBase::") + __FUNCTION__ + ": void type unsupported");
		return *this;
	}

	template <class F>
	inline
	ReferenceBase & link(Castable &x){
		this->relink(x);
		return *this;
	}

	template <class F>
	inline
	ReferenceBase & link(ReferenceBase &x){
		this->relink(x);
		return *this;
	}



};

template <>
template <class S>
void ReferenceBase<VariableBase>::init(const S & src){
	this->assign(src); // Safe
}




/// "Final" class, to be applied through typedefs
template <class T>
class VariableT : public T {

public:

	typedef T varbase_t;

	/// Single constructor template, forwarded to init(args...) defined in base classes.
	/**
	 *   This is the basic design pattern for all the VariableLikes
	 */
	template <class ...TT>
	inline
	VariableT(const TT & ...args) {
		this->init(args...); // invokes  T::init(...)
	}

	/// Single constructor template, forwarded to init(arg).
	/**
	 *   For a Reference,
	 */
	template <class D>
	inline
	VariableT(D & arg) {
		this->init(arg); // invokes  T::init(...)
	}


	/// Sets or changes the type if possible - that is, object is not a Reference.
	/**
	 */
	virtual
	bool requestType(const std::type_info & t);


	/// Sets type, only if unset, and object is not a Reference.
	/**
	 *
	 *   Does not change the current type, if set.
	 *
	 *   Could be protected, but requestType() will not be, so for consistency this is also public.
	 */
	virtual
	bool suggestType(const std::type_info & t);


	/// Change the array size, if  For Castable (and Reference) does nothing and returns false.
	/**
	 *
	 *
	 *   Does not apply to std::string .
	 *
	 *   Could be protected, but requestType() will not be, so for consistency this is also public.
	 *
	 *   \return - \c true , if final the element count is what was requested.
	 *
	 */
	virtual
	bool requestSize(size_t elementCount);

	/// Assignment of objects of the same class.
	/**
	 *
	 */
	inline
	VariableT & operator=(const VariableT<T> &x){
		assignCastable(x);
		return *this;
	}

	/// Assignment of base classes (VariableLike or Castable)
	inline
	VariableT & operator=(const T &x){
		assignCastable(x);
		return *this;
	}

	/// General assignment operator.
	template <class T2>
	inline
	VariableT & operator=(const T2 &x){
		this->assign(x);
		return *this;
	}

	/// General assignment operator.
	template <class T2>
	inline
	VariableT & operator=(std::initializer_list<T2> l){
		this->assignContainer(l, false);
		return *this;
	}

	/// Assignment of C strings. [Obligatory]
	/**
	 *   Assignment from char * and std::string splits the input by separator.
	 *
	 *   STL strings will be handled by the default operator operator=(const T &x) .
	 */
	inline
	VariableT & operator=(const char *x){
		this->assign(x);
		return *this;
	}


	// Could be removed ? Handled by assign<T>() ?
	template <class T2, size_t N>
	inline
	VariableT &operator=(const UniTuple<T2,N> & unituple){
		this->assignContainer(unituple);
		return *this;
	}

	// Default
	inline
	void info(std::ostream & ostr = std::cout) const {
		Castable::info(ostr);
	}


};



typedef VariableT<VariableBase>        Variable;
typedef VariableT<ReferenceBase<Castable> >  Reference;
typedef VariableT<ReferenceBase<Variable> >  FlexibleVariable;


template <>
template <class S>
void ReferenceBase<VariableBase>::init(const S & src);




template <>
bool Variable::requestType(const std::type_info & t);

template <>
bool Reference::requestType(const std::type_info & t);

template <>
bool FlexibleVariable::requestType(const std::type_info & t);


template <>
bool Variable::suggestType(const std::type_info & t);

template <>
bool Reference::suggestType(const std::type_info & t);


template <>
bool FlexibleVariable::suggestType(const std::type_info & t);


template <>
bool Variable::requestSize(size_t elementCount);

template <>
bool Reference::requestSize(size_t elementCount);

template <>
bool FlexibleVariable::requestSize(size_t elementCount);



// Override for "ambivalent" FlexibleVariable
template <>
void FlexibleVariable::info(std::ostream & ostr) const;


template <>
const std::string TypeName<Variable>::name;

template <>
const std::string TypeName<Reference>::name;

template <>
const std::string TypeName<FlexibleVariable>::name;






}

#endif
