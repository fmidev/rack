/**

    Copyright 2011-2012  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Rack.

    Rack is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Rack is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rack.  If not, see <http://www.gnu.org/licenses/>.

 */


#include "DamperOp.h"

using namespace drain::image;
using namespace hi5;



namespace rack {

/**  Converts a polar product to topologically equivalent product in Cartesian coordinates
 *
 */

/*
void DamperOp::filterDataGroup(const HI5TREE &srcRoot, HI5TREE &dstRoot, const std::string &path) const {

	drain::Logger mout(drain::monitor,"DamperOp::filterDataGroup");

	Image & data = dstRoot[path]["data"].data.dataSet;

	mout.debug(1) << path << mout.endl;

	if (!data.isEmpty()){

		//getQualityData(srcRoot,path);

		//const std::string qPath = path + "/quality1";
		//const Image &quality = srcRoot[qPath]["data"].data.dataSet;
		const Image &quality = getQualityData(srcRoot,path);

		if (quality.isEmpty()){
			mout.note(1) <<  "No quality data, skipping." << mout.endl;
			return;
		}

		mout.debug(1) << " using quality"  << mout.endl;
		if (mout.isDebug(5)){
			quality.toOStr(std::cerr);
		}
		//Image<unsigned char> &dst = ((NodeHi5 &)dstRoot[path+"/data"]).data;


		filterImage(quality, data);
		drain::Options & a = dstRoot[path]["how"].data.attributes;
		a["q_algorithms"] << name;
		a["q_algorithms"].setType<std::string>();

	};
}
*/

void DamperOp::processData(const Data<PolarSrc> & srcData, Data<PolarDst> & dstData) const {
//void DamperOp::filterImage(const PolarODIM & odim, Image &data, Image &quality) const {

	drain::Logger mout(name, __FUNCTION__);

	//drain::image::File::write(data,"Eras0.png");
	//drain::image::File::write(quality,"Erasq.png");
	/*

	Image::iterator  it = srcData.data.begin();
	Image::iterator qit = srcData.getQualityData().data.begin();
	const Image::iterator end = srcData.data.end();
	while (it != end){
		if (*qit < t)
			*it = dstData.odim.nodata;
		++it;
		++qit;
	}
	 */

	drain::FuzzyStep<double> fstep(threshold, threshold+(1.0-threshold)/2.0);

	const PlainData<PolarSrc> & srcQuality = srcData.getQualityData();

	//const std::type_info & t = dstData.data.getType();
	const double min = dstData.data.getMin<double>();
	const double max = dstData.data.getMax<double>();
	/// NOTE: getMin returns 0 for unsigned integral and ~0 for floats, which is ok here.
	/** 2018
	if (drain::Type::call<drain::typeIsInteger>(t))
		dstData.data.scaling.setLimits( min+2.0, max-2.0);
	else
		dstData.data.scaling.setLimits(-max+2.0, max-2.0);
	*/

	Image::iterator d  = dstData.data.begin();  // fixme: const object allows non-const iterator
	Image::const_iterator q = srcQuality.data.begin();
	const Image::const_iterator dEnd = dstData.data.end();
	double x;
	double w;
	//data.setLimits(-256.0, 255.0);

	mout.debug(2) << "data, after limits: " << dstData.data << mout.endl;
	mout.debug(2) << "limits: " << dstData.data.getMin<double>() << ',' << dstData.data.getMax<double>() << mout.endl;
	//data.toOStr(std::cout);

	drain::typeLimiter<double>::value_t limit = dstData.data.getLimiter<double>();

	while (d != dEnd){
		x = *d;
		if ( (x != srcData.odim.nodata) && (x != srcData.odim.undetect) ){

			w = fstep(srcQuality.odim.scaleForward(*q));
			if (w < undetectThreshold){
				*d = dstData.odim.undetect;
			}
			else {
				x = srcData.odim.scaleForward(x);

				if (x < dbzMin)
					*d = dstData.odim.undetect;
				else
					*d = limit(srcData.odim.scaleInverse(dbzMin + w*(x-dbzMin)));
							//dstData.data.scaling.limit<double>(srcData.odim.scaleInverse(dbzMin + w*(x-dbzMin)));
				//*d = scaleDBZ.inverse(x);
				//*d = scaleDBZ.inverse(x);
				//*d = static_cast<double>(*q)/255.0 * x;
			}
		}
		++d;
		++q;
	};

	//drain::image::File::write(data,"Eras1.png");


}

}

