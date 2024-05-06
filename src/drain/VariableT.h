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



#ifndef DRAIN_VARIABLE_T
#define DRAIN_VARIABLE_T

#include <drain/Castable.h>
//#include <drain/Sprinter.h>
#include <drain/UniTuple.h> // "Friend class"

namespace drain {

//class Variable;
//class FlexibleVariable;
//class Reference;


/// VariableT is a final class applied through typedefs Variable, Reference and FlexibleVariable.
/**

\section VariableT_typedefs Type definitions


\code
typedef VariableT<VariableInitializer<VariableBase> >                       Variable;
typedef VariableT<VariableInitializer<ReferenceT<VariableBase> > >  FlexibleVariable;
typedef VariableT<ReferenceT<Castable> >                                   Reference;
\endcode

\section ctors-all Constructors Variable,FlexibleVariable,Reference

\htmlinclude  VariableT-ctors-Variable-FlexibleVariable-Reference.html

\section variables-comparison-assignments Comparison of assignments

\subsection Variable Variable

\htmlinclude  VariableT-assign-Variable.html

\subsection FlexibleVariable FlexibleVariable

\htmlinclude  VariableT-assign-FlexibleVariable.html

\subsection Reference Reference

\htmlinclude  VariableT-assign-Reference.html


\see Reference
\see Variable
\see FlexibleVariable

 \tparam T - see typedefs: drain::Reference, drain::Variable, and drain::FlexibleVariable


 */
template <class T>
class VariableT : public T {

public:

	typedef T varbase_t;

	typedef std::pair<const char *,const drain::VariableT<T> > init_pair_t;

	/// Single constructor template, forwarded to init(args...) defined in base classes.
	/**
	 *   This is the basic design pattern for all the VariableLikes
	 */
	template <class ...TT>
	inline
	VariableT(const TT & ...args) {
		// std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << "( variadics )" << '[' << typeid(VariableT<T>).name() << ']'<< std::endl;
		// std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " " << value <<  std::endl;
		this->init(args...); // invokes  T::init(...)
	}

	/// C++ bug? : Copy constructor will not catch
	// VariableT(const VariableT<T> & v);



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
	 *   Does not change the current type, if set.
	 *
	 *   Could be protected, but requestType() will not be, so for consistency this is also public.
	 */
	virtual
	bool suggestType(const std::type_info & t);


	/// Change the array size, if  For Castable (and Reference) does nothing and returns false.
	/**
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
	VariableT & operator=(const VariableT<T> &x){  // VariableT<T>
		this->assignCastable(x);
		// this->append(7); // DEBUG
		return *this;
	}

	/// Assignment of objects of similar classes.
	/**
	 *  Note: this does not remove need of exactly matching type assignment (above).
	 */
	template <class T2>
	inline
	VariableT & operator=(const VariableT<T2> &x){  // VariableT<T>
		this->assignCastable(x);
		// this->append(5); // DEBUG
		return *this;
	}

	/// Assignment of base classes (VariableLike or Castable)
	// This will never be called?
	inline
	VariableT & operator=(const T &x){
		this->assignCastable(x);
		std::cerr << __FILE__ << ':' << __FUNCTION__ << ": ? for inner class: "<< TypeName<T>::str() << std::endl;
		//this->append("Inner");
		return *this;
	}

	/// Assignment of objects of the same class.
	/**
	 *
	 */
	inline
	VariableT & operator=(const Castable &x){
		this->assignCastable(x);
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

	/// debugging
	/*
	template <class T2>
	inline
	T2 get() const {
		//this->caster.get<double>();
		return T2();
	}
	*/

	// Consider VariableT<T2>
	/*
	inline
	bool operator==(const VariableT &x) const {
		throw std::runtime_error("Castable: operator== not implemented.");
		return false;
	}

	// Consider VariableT<T2>
	inline
	bool operator!=(const VariableT &x) const {
		return !operator==(x);
	}
	*/

	template <class T2>
	inline
	bool operator==(const VariableT<T2> & v){

		if (this->isString() || v.isString()){
			return (this->toStr() == v.toStr());
		}

		if (this->getType() == v.getType()){
			// typename const_iterator it;

		}
		std::cerr << __FILE__ << ':' << __FUNCTION__ << " unimplemented... " << std::endl;
		return false;
	}

	template <class T2>
	inline
	bool operator==(const T2 &x){
		return Castable::operator==(x);
	}

	// Default
	inline
	void info(std::ostream & ostr = std::cout) const {
		Castable::info(ostr);
	}

};

/*
template <>
template <class T>
std::string & std::string::operator=(const VariableT<T> &t){
	this->assign(t.str());
	return *this;
}


template <class T>
inline
std::ostream & Sprinter::toStream(std::ostream & ostr, const VariableT<T> & v, const SprinterLayout & layout){
	return Sprinter::toStream(ostr, (const drain::Castable &) v, layout);
};

\~remark

   <TABLE>
   <TR><TD></TD><TD></TD><TH>Reference</TH><TH>Variable</TH><TH>FlexibleVariable</TH></TR>
   <TR>
   	   <TD>Definition</TD><TD></TD>
   	   <TD>VariableT<ReferenceT<Castable> ></TD>
   	   <TD>VariableT<VariableBase></TD>
   	   <TD>VariableT<ReferenceT<Variable> ></TD>
   </TR>
   <TR>
   	   <TD>Default constructor</TD>
   	   <TD><tt>V v;</tt></TD>
   	   <TD>Compiler error</TD>
   	   <TD>Ok (type undefined)</TD>
   	   <TD>Ok (type undefined)</TD></TR>
   <TR>
   	<TD>Constructor with type</TD>
   	<TD><tt>V v(typeid(double));<br/>V v(typeid(int));<br/>V v(typeid(std::string));</tt></TD>
   	<TD>Compiler error</TD>
   	<TD>Ok, sets type</TD>
   	<TD>Ok, sets type</TD>
   </TR>
   <TR>
   	<TD>Constructor with initialisation</TD>
   	<TD><tt>V v(x);</tt></TD>
   	<TD>links</TD>
   	<TD>assigns</TD>
   	<TD>assigns</TD>
   </TR>
   <TR>
   	<TD>Constructor with assignment</TD>
   	<TD><tt>V v(x) = 7.0;</tt></TD>
   	<TD>links and assigns</TD>
   	<TD>ok, yet odd (initialises and reassigns)</TD>
   	<TD>ok, yet odd (initialises and reassigns)</TD>
   </TR>
   </TABLE>

\~

*/


} // drain

#endif
