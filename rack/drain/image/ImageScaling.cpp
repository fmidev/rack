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
#include "util/Log.h"
#include "ImageScaling.h"

namespace drain {

namespace image {

//extern drain::Log iLog;

void ImageScaling::setOptimalScale(const std::type_info & t){ // , const std::string &unit ?

	Logger mout(getImgLog(), "ImageScaling", __FUNCTION__);

	if (!isPhysical()){
		mout.warn() << "physical range unset, setting absolute scale" << mout.endl;
		setAbsoluteScale();
		return;
	}

	if (Type::call<drain::typeIsSmallInt>(t)){

		const double codeMin = Type::call<typeMin,double>(t);
		const double codeMax = Type::call<typeMax,double>(t);

		// set physical zero in storage-type zero (consider -32...+50 Celsius)
		if (Type::call<drain::isSigned>(t)){ // ie. codeMin < 0.0
			if (maxPhysValue > -minPhysValue){
				setScale(maxPhysValue/codeMax);
			}
			else {
				setScale(minPhysValue/codeMin);
			}
		}
		else {
			const double s = (maxPhysValue - minPhysValue)/(codeMax-codeMin);
			setScale(s, minPhysValue - s*codeMin);
		}
	}
	else { // warn? (esp. for int and long int)
		if (Type::call<drain::typeIsInteger>(t)){
			mout.warn() << "large int type (" << Type::call<nameGetter>(t) << ") , using absolute scaling" << mout.endl;
		}
		setAbsoluteScale();
	}
}

void ImageScaling::adoptScaling(const ImageScaling & srcScaling, const std::type_info & srcType, const std::type_info & dstType){

		Logger mout(getImgLog(), "ImageScaling", __FUNCTION__);

		if (dstType == typeid(void)){
			mout.warn() << "Dst type not given" << mout.endl;
		}

		mout.debug() << "Src scaling: " <<  srcScaling << ", type: " << Type::getTypeChar(srcType) << mout.endl;
		if (srcScaling.isPhysical()  && (dstType != typeid(void))){
			mout.debug(1) << "Ok, copying physical scaling of src" << mout.endl;
			//setPhysicalRange(srcScaling.getMinPhys(), srcScaling.getMaxPhys());
			if (srcType == dstType){
				set(srcScaling);
			}
			else {
				setPhysicalScale(dstType, srcScaling.getMinPhys(), srcScaling.getMaxPhys());
				mout.debug() << "src:" << srcScaling << "-> dst:" << *this << '\t' << Type::getTypeChar(dstType) << mout.endl;
			}
		}
		else {
			mout.debug(1) << "Src has no physical scaling, trying to guess..." << mout.endl;
			if (Type::call<drain::typeIsSmallInt>(srcType) && Type::call<drain::typeIsSmallInt>(dstType)){
				if (srcType == dstType){
					mout.debug(1) << "Src[" << Type::getTypeChar(srcType) << "], ";
					mout << " Dst[" << Type::getTypeChar(dstType) << "] " << mout.endl;
					set(srcScaling);
					mout.debug() << *this << mout.endl;
				}
				else {
					mout.info() << "Src[" << Type::getTypeChar(srcType) << "], ";
					mout << " Dst[" << Type::getTypeChar(dstType) << "] " << mout.endl;
					const double maxS = drain::Type::call<drain::typeMax, double>(srcType);
					const double maxD = drain::Type::call<drain::typeMax, double>(dstType);
					setScale(srcScaling.getScale() * maxS/maxD);
					//setScale(srcScaling.getScale() * Type::call<Type::maxD>(srcType)/Type::call<Type::maxD>(dstType));
					if (srcScaling.getOffset() != 0.0){
						mout.warn() << " offset conversion unimplemented" << mout.endl;
					}
					mout.note() << *this << mout.endl;
				}
				//mout << *this << mout.endl;
			}
			else if (Type::call<drain::typeIsInteger>(srcType) && Type::call<drain::typeIsInteger>(dstType)) {
				mout.note() << "Int-to-int (either large)";
				mout        << "Src[" << Type::getTypeChar(srcType) << "], Dst[" << Type::getTypeChar(dstType) << "] " << mout.endl;
				set(srcScaling);
			}
			else if (Type::call<drain::typeIsInteger>(dstType)) { // src float
				mout.warn() << "Src[" << Type::getTypeChar(srcType) << "], float data without physical scaling, dst[" << Type::getTypeChar(dstType) << "] problems ahead" << mout.endl;
				set(srcScaling);
			}
			else if (Type::call<drain::typeIsInteger>(srcType)) { // src float
				mout.note() << "No physical range set for adopting ";
				mout << "[" << Type::getTypeChar(srcType) << "] => [" << Type::getTypeChar(dstType) << "], ";
				const double min = drain::Type::call<drain::typeMin, double>(srcType); // = drain::Type::call<Type::minD>(srcType);
				const double max = drain::Type::call<drain::typeMax, double>(srcType); // = drain::Type::call<Type::maxD>(srcType);
				// min = drain::Type::call<drain::typeMin, double>(dstType);
				// max = drain::Type::call<drain::typeMax, double>(dstType);

				//set(srcScaling);
				//mout.note() << "Dst float, Src data small int without physical scaling, assuming physical limits  [" << min << ',' << max << "]" << mout.endl;
				mout << "guessing physical limits [" << min << ',' << max << "]" << mout.endl;
				setPhysicalRange(min, max);
			}
			else {
				mout.debug() << "Float types: src[" << Type::getTypeChar(srcType) << "], dst[" << Type::getTypeChar(dstType) << "]" << mout.endl;
				// TODO: assume
				set(srcScaling);
			}
		}

		/*
		double min = 0.0;
		double max = 0.0;

		if (srcScaling.isPhysical()){
			mout.debug(1) << "Src has physical scaling:" <<  srcScaling << ", type=" << Type::getTypeChar(srcType) << mout.endl;
			min = srcScaling.getMinPhys();
			max = srcScaling.getMaxPhys();
		}
		else if (Type::call<drain::typeIsSmallInt>(dstType)){
			// First, assign range of the storage type
			min = drain::Type::call<drain::typeMin, double>(dstType);
			max = drain::Type::call<drain::typeMax, double>(dstType);
			if (srcScaling.getScale() != 0.0){
				min = srcScaling.fwd(min);
				max = srcScaling.fwd(max);
				mout.warn() << "Src[" << Type::getTypeChar(srcType) << "], Dst[" << Type::getTypeChar(dstType) << "], ";
				mout << "guessing physical scale [" <<  min << ',' << max << ']' << mout.endl;
				//mout.warn() << "mapping src scale (" << srcScaling << ") to type-derived physical range [" << min << ',' << max << "]" << mout.endl;
			}
			else {
				//setPhysicalRange(min, max);
				//mout.warn() << "Requested physical (min,max) from int type with no scaling (" << img.scaling << ")" <<  mout.endl;
				mout.warn() << "Assuming absolute scaling for storage type: [" << min << ',' << max << "] (src: " << srcScaling << ") " << mout.endl;
				//mout.note() << "Setting absolute scaling: " << scaling <<  mout.endl;
			}
		}
		else if (Type::call<drain::typeIsSmallInt>(srcType)){

			// First, assign range of the storage type
			min = Type::call<drain::typeMin, double>(srcType);
			min = srcScaling.fwd(min);
			max = Type::call<drain::typeMax, double>(srcType);
			max = srcScaling.fwd(max);
			mout.warn() << "Src[" << Type::getTypeChar(srcType) << "], ";
			mout << "guessing physical scale [" <<  min << ',' << max << ']' << mout.endl;
			//mout.debug(1) << "Src of type=" << Type::getTypeChar(srcType) << ", guessing physical scale [" <<  min << ',' << max << ']' << mout.endl;
		}
		else {
			//scaling.setOptimalScale(img.getType(), min, max);
			mout.warn() << "Could not derive physical scaling from "; // <<  srcScaling << " type=" << Type::getTypeChar(srcType) << mout.endl;
			mout << "Src[" << Type::getTypeChar(srcType) << "]: " << srcScaling << ", Dst[" << Type::getTypeChar(dstType) << "], ";
			return;
		}

		if (dstType != typeid(void))
			setOptimalScale(dstType, min, max);
		else
			setPhysicalRange(min, max);
		*/
	}

}  // namespace image

}  // namespace drain

// Drain
