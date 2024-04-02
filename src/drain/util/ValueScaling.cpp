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
#include <drain/Log.h>
#include "ValueScaling.h"

namespace drain {

void ValueScaling::setOptimalScale(const std::type_info & t){ // , const std::string &unit ?

	Logger mout(__FILE__, __FUNCTION__);

	if (!isPhysical()){
		mout.warn("physical range unset, setting absolute scale" );
		setAbsoluteScale();
		return;
	}

	if (Type::call<drain::typeIsSmallInt>(t)){

		const double codeMin = Type::call<typeMin,double>(t);
		const double codeMax = Type::call<typeMax,double>(t);

		// set physical zero in storage-type zero (consider -32...+50 Celsius)
		if (Type::call<drain::isSigned>(t)){ // ie. codeMin < 0.0
			if (physRange.max > -physRange.min){
				set(physRange.max/codeMax, 0.0);
			}
			else {
				set(physRange.min/codeMin, 0.0);
			}
		}
		else {
			const double s = (physRange.max - physRange.min)/(codeMax-codeMin);
			set(s, physRange.min - s*codeMin);
		}
	}
	else { // warn? (esp. for int and long int)
		if (Type::call<drain::typeIsInteger>(t)){
			mout.warn("large int type (" , Type::call<nameGetter>(t) , ") , using absolute scaling" );
		}
		setAbsoluteScale();
	}
}

void ValueScaling::adoptScaling(const ValueScaling & srcScaling, const std::type_info & srcType, const std::type_info & dstType){

	Logger mout(__FILE__, __FUNCTION__);

	if (dstType == typeid(void)){
		mout.warn("Dst type not given" );
	}

	mout.debug("Src scaling: " ,  srcScaling , ", type: " , Type::getTypeChar(srcType) );
	if (srcScaling.isPhysical()  && (dstType != typeid(void))){
		mout.debug2("Ok, copying physical scaling of src" );
		//setPhysicalRange(srcScaling.getMinPhys(), srcScaling.getMaxPhys());
		if (srcType == dstType){
			assign(srcScaling);
		}
		else {
			setPhysicalScale(dstType, srcScaling.getMinPhys(), srcScaling.getMaxPhys());
			mout.debug("src:" , srcScaling , "-> dst:" , *this , '\t' , Type::getTypeChar(dstType) );
		}
	}
	else {
		mout.debug2("Src has no physical scaling, trying to guess..." );
		if (Type::call<drain::typeIsSmallInt>(srcType) && Type::call<drain::typeIsSmallInt>(dstType)){
			if (srcType == dstType){
				mout.debug2() << "Src[" << Type::getTypeChar(srcType) << "], ";
				mout << " Dst[" << Type::getTypeChar(dstType) << "] " << mout.endl;
				assign(srcScaling);
				mout.debug(*this );
			}
			else {
				mout.info() << "Src[" << Type::getTypeChar(srcType) << "], ";
				mout << " Dst[" << Type::getTypeChar(dstType) << "] " << mout.endl;
				const double maxS = drain::Type::call<drain::typeMax, double>(srcType);
				const double maxD = drain::Type::call<drain::typeMax, double>(dstType);
				set(srcScaling.getScale() * maxS/maxD, 0.0);
				//setScale(srcScaling.getScale() * Type::call<Type::maxD>(srcType)/Type::call<Type::maxD>(dstType));
				if (srcScaling.getOffset() != 0.0){
					mout.warn(" offset (", srcScaling.getOffset() , ") conversion unimplemented ");
				}
				mout.note(*this );
			}
			//mout << *this << mout.endl;
		}
		else if (Type::call<drain::typeIsInteger>(srcType) && Type::call<drain::typeIsInteger>(dstType)) {
			mout.note() << "Int-to-int (either large)";
			mout        << "Src[" << Type::getTypeChar(srcType) << "], Dst[" << Type::getTypeChar(dstType) << "] " << mout.endl;
			assign(srcScaling);
		}
		else if (Type::call<drain::typeIsInteger>(dstType)) { // src float
			mout.warn("Src[" , Type::getTypeChar(srcType) , "], float data without physical scaling, dst[" , Type::getTypeChar(dstType) , "] problems ahead" );
			assign(srcScaling);
		}
		else if (Type::call<drain::typeIsInteger>(srcType)) { // src float
			mout.note() << "No physical range set for adopting ";
			mout << "[" << Type::getTypeChar(srcType) << "] => [" << Type::getTypeChar(dstType) << "], ";
			const double min = drain::Type::call<drain::typeMin, double>(srcType); // = drain::Type::call<Type::minD>(srcType);
			const double max = drain::Type::call<drain::typeMax, double>(srcType); // = drain::Type::call<Type::maxD>(srcType);
			// min = drain::Type::call<drain::typeMin, double>(dstType);
			// max = drain::Type::call<drain::typeMax, double>(dstType);

			//set(srcScaling);
			//mout.note("Dst float, Src data small int without physical scaling, assuming physical limits  [" , min , ',' , max , "]" );
			mout << "guessing physical limits [" << min << ',' << max << "]" << mout.endl;
			setPhysicalRange(min, max);
		}
		else {
			mout.debug("Float types: src[" , Type::getTypeChar(srcType) , "], dst[" , Type::getTypeChar(dstType) , "]" );
			// TODO: assume
			assign(srcScaling);
		}
	}

}


}  // namespace drain
