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



#ifndef DRAIN_REFERENCE_T
#define DRAIN_REFERENCE_T

#include "Log.h"
#include "VariableBase.h"
// #include "Castable.h" // not really needed

namespace drain {

//class Variable;

/// Intermediate class supporting link() in various forms.
/**

See also comparison of all the three variable classes:
\see drain::VariableT

\section Constructors

\htmlinclude  VariableT-ctors-Reference.html

\subsection Assignment

\htmlinclude  VariableT-assign-Reference.html


 \tparam T - Castable or VariableBase
 */
template <class T> // =Castable
class ReferenceT : public T {

public:

	typedef const drain::ReferenceT<T> reference_t;

	typedef std::pair<const char *,const drain::ReferenceT<T> > init_pair_t;

	/// Tells if the internal pointer can point to an external variable.
	/**
	 *  \return - \c true for Reference and Flexible, and \c false for Variable.
	 */
	virtual inline
	bool isLinkable() const {
		return true;
	}


	template <class T2>
	inline
	ReferenceT & link(ReferenceT<T2> &x){
		this->relink(x);
		return *this;
	}

	// "Originals"
	// ReferenceT & link(VariableT<T2> &x){
	//template <class T2>
	// VariableT<VariableInitializer<ReferenceT<VariableBase> > >

	/// Linkage for Reference, above all for copy constructor.
	inline
	ReferenceT & link(VariableT<ReferenceT<Castable> >  &x){
		this->relink(x);
		return *this;
	}

	/// Linkage for a FlexibleVariable
	inline
	ReferenceT & link(VariableT<VariableInitializer<ReferenceT<VariableBase> > > &x){
		if (x.isLinking()){
			//  Valid for 1) FlexibleVariable that is 2) linking external variable.
			// drain::Logger(__FILE__, __LINE__, __FUNCTION__).success<LOG_WARNING>("accessing external data (", x, ") of ", drain::TypeName< VariableT<VariableInitializer<ReferenceT<VariableBase> > > >::str());
			this->relink(x);
		}
		else {
			//drain::Logger(__FILE__, __LINE__, __FUNCTION__).fail(drain::TypeName< VariableT<T2> >::str(), ' ', x, ": linking internal data forbidden");
			drain::Logger(__FILE__, __LINE__, __FUNCTION__).fail("accessing internal data (", x, ") of ", drain::TypeName< VariableT<VariableInitializer<ReferenceT<VariableBase> > > >::str(), " forbidden");
			this->reset();
		}
		return *this;
	}

	/// Linkage for a Variable - produces a warning, because internal variable has dynamic type.
	inline
	ReferenceT & link(VariableT<VariableInitializer<VariableBase> > &x){
		// drain::TypeName<VariableT<ReferenceT > >::str(),
		drain::Logger(__FILE__, __LINE__, __FUNCTION__).fail(drain::TypeName< VariableT<VariableInitializer<VariableBase> > >::str(), ' ', x, ": linking internal data forbidden");
		this->reset();
		return *this;
	}



	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Reference redirected to relink() )
	 */
	template <class F>
	inline
	ReferenceT & link(F &p){
		try {
			this->setPtr(p);
		}
		catch (const std::exception & e){
			//Logger(__FILE__, __LINE__, __FUNCTION__).error("unsupported type: ", drain::TypeName<F>::str()); // , " msg:", e.what()
			Logger(__FILE__, __LINE__, __FUNCTION__).error("unsupported type: ", typeid(F).name(), " msg:", e.what());
			// std::cerr << __FILE__ << ':' << __FUNCTION__ << ": unsupported type: " << typeid(F).name() << std::endl;
			// throw std::runtime_error("unsupported type");
		}
		return *this;
	}

	/// Set pointer to &p.
	/**
	 *  \tparam T - target object (if Castable or Reference redirected to relink() )
	 */
	template <class F>
	inline
	ReferenceT & link(F *p){
		this->setPtr(p);
		return *this;
	}

	/// Set pointer to p, of given type.
	inline
	ReferenceT & link(void *p, const std::type_info &t, size_t count=1){
		this->setPtr(p, t);
		this->elementCount = count;
		// Why not this->setPtr(p, t, count); ?
		return *this;
	}

	// What about link(void *p){
	inline
	ReferenceT & link(void *p){
		throw std::runtime_error(std::string(__FILE__) + __FUNCTION__ + ": void type unsupported");
		return *this;
	}

	//template <class F>
	inline
	ReferenceT & link(Castable &x){
		this->relink(x);
		return *this;
	}

	inline
	void unlink(){
		this->reset();
		//this->unlink();
	}

protected:

	/// Copy constructor handler - important.
	//template <class T2>
	/*
	void init(const ReferenceT<T> & ref) {
		// std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " " << ref << '[' << typeid(T).name() << ']'<< std::endl;
		std::cerr << __FILE__ << ':' << __LINE__ << ':' << __FUNCTION__ << " " << ref << '[' << TypeName< ReferenceT<T> >::str() << ']' <<  '[' << TypeName<T>::str() << ']'<< std::endl;
		this->reset(); // needed?
		//this->link(ref.getPtr(), ref.getType(), ref.getSize());
		this->relink(ref);

		//this->assignCastable(value);
	}
	*/


	template <class D>
	void init(D & dst){
		this->link(dst);
	}



	template <class S>
	void init(const S & src){
		std::cerr << __FILE__ << ' ' << __LINE__ << ':' << __FUNCTION__ << " " << src << std::endl;

		T::init(src); // Undefined for Castable -> compile time error.
	}



	template <class D>
	void init(D *dst){
		this->link(dst);
	}

	inline
	void init(void *p, const std::type_info &t, size_t count=1){
		this->link(p, t, count);
	}


public:

protected:

	// Terminal
	inline
	void init(){
	}

};

template <>
template <class S>
inline
void ReferenceT<VariableBase>::init(const S & src){
	this->assign(src); // Safe for VariableBase
}





}

#endif
