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



/// Intermediate class supporting link() in various forms.
/**
 *  \tparam T - Castable or VariableBase
 */
template <class T> // =Castable
class ReferenceT : public T {

public:

	typedef std::pair<const char *,const drain::ReferenceT<T> > init_pair_t;


	template <class D>
	void init(D & dst){
		this->link(dst);
	}

	template <class S>
	void init(const S & src){
		// std::cerr << __FILE__ << ' ' << __LINE__ << ':' << "ReferenceBase::" << __FUNCTION__ << " " << src << std::endl;
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
		return *this;
	}

	inline
	ReferenceT & link(void *p){
		throw std::runtime_error(std::string("ReferenceBase::") + __FUNCTION__ + ": void type unsupported");
		return *this;
	}

	//template <class F>
	inline
	ReferenceT & link(Castable &x){
		this->relink(x);
		return *this;
	}

	template <class T2>
	inline
	ReferenceT & link(ReferenceT<T2> &x){
		this->relink(x);
		return *this;
	}

	/// Tells if the internal pointer can point to an external variable.
	/**
	 *  \return - \c true for Reference and Flexible, and \c false for Variable.
	 */
	virtual inline
	bool isLinkable() const {
		return true;
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
