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


#include "DamperOp.h"

using namespace drain::image;
using namespace hi5;



namespace rack {

/**  Converts a polar product to topologically equivalent product in Cartesian coordinates
 *
 */

/*
void DamperOp::filterDataGroup(const HI5TREE &srcRoot, HI5TREE &dstRoot, const std::string &path) const {

	drain::MonitorSource mout(drain::monitor,"DamperOp::filterDataGroup");

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
			quality.info(std::cerr);
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

	drain::MonitorSource mout(name, __FUNCTION__);

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

	const std::type_info & t = dstData.data.getType();
	const double min = drain::Type::getMin<double>(t);
	const double max = drain::Type::getMax<double>(t);
	/// NOTE: getMin returns 0 for unsigned integral and ~0 for floats, which is ok here.
	if (drain::Type::isIntegralType(t))
		dstData.data.setLimits( min+2.0, max-2.0);
	else
		dstData.data.setLimits(-max+2.0, max-2.0);


	Image::iterator d  = dstData.data.begin();  // fixme: const object allows non-const iterator
	Image::const_iterator q = srcQuality.data.begin();
	const Image::const_iterator dEnd = dstData.data.end();
	double x;
	double w;
	//data.setLimits(-256.0, 255.0);

	mout.debug(2) << "data, after limits: " << dstData.data << mout.endl;
	mout.debug(2) << "limits: " << dstData.data.getMin<double>() << ',' << dstData.data.getMax<double>() << mout.endl;
	//data.info(std::cout);

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
					*d = dstData.data.limit<double>(srcData.odim.scaleInverse(dbzMin + w*(x-dbzMin)));
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


// Rack
