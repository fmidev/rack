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

#include "image/SegmentProber.h"

#include "image/SegmentStatisticsProber.h"

#include "SegmentStatisticsOp.h"

namespace drain
{
namespace image
{

void SegmentStatisticsOp::makeCompatible(const ImageFrame & src, Image & dst) const  {

	drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	mout.debug() << dst << mout.endl;

	const std::type_info & t = typeid(unsigned short);

	if (!dst.typeIsSet()){
		dst.setType(t);
	}
	else if (Type::call<typeIsFloat>(dst.getType())){
		dst.setType(t);
		mout.warn() << "float valued destination data not supported, setting: " << Type::getTypeChar(t) << mout.endl;
		//throw std::runtime_error("SegmentAreaOp: float valued destination image not supported.");
	}

	/// Todo: check view-images?
	dst.setGeometry(src.getWidth(), src.getHeight(), statistics.size(), dst.getAlphaChannelCount());

	if (clearDst)
		dst.clear();

}



///
void SegmentStatisticsOp::traverseChannels(const ImageTray<const Channel> & srcTray, ImageTray<Channel> & dstTray) const {
	//void SegmentStatisticsOp::traverseFrame(const ImageFrame & src, ImageFrame & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, __FILE__);

	const Channel & src = srcTray.get();
	Channel & dst = dstTray.get();

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();
	const unsigned int count  = statistics.size();

	const float widthF = static_cast<float>(width);
	const float heightF = static_cast<float>(height);
	const float areaF  = widthF*heightF;

	//const int minI = static_cast<int>(min);
	//const int maxI = static_cast<int>(max);
	const double minRaw = src.getScaling().inv(min);
	const double maxRaw = (max == std::numeric_limits<double>::max()) ? src.getEncoding().getTypeMax<double>() : src.getScaling().inv(max);

	if (minRaw <= src.getEncoding().getTypeMin<double>()){
		mout.warn()  << "min value=" << (double)minRaw <<  " less or smaller than storage type min=" << src.getEncoding().getTypeMin<double>() << mout.endl;
	}

	mout.debug()  << "raw range: " << (double)minRaw << '-' << (double)maxRaw << mout.endl;


	/// TODO: should use the actual types of src and dst.
	FillProber floodFill(src);
	floodFill.conf.anchorMin = minRaw;
	floodFill.conf.anchorMax = maxRaw;
	floodFill.init();

	SegmentStatisticsProber<int,int> prober(src, dst, statistics);
	prober.conf.anchorMin = minRaw;
	prober.conf.anchorMax = maxRaw;
	prober.conf.markerValue = 1;
	prober.init();

	const UnaryFunctor & functor = getFunctor(dst.getEncoding().getTypeMax<double>());  // what if dst float?
	mout.debug(1) << functor.getName() << ':' << functor << mout.endl;

	const float prescale = dst.getEncoding().getTypeMax<float>();

	//mout.note(3) << *this << mout.endl;
	mout.debug(1) << "statistics: " << statistics << " count=" << count << '\n';
	mout << "src: " << src  << '\n';
	mout << "dst: " << dst  << '\n';
	mout << "range:" << (float)min  << '-' << (float)max;
	mout << "dst scale:" << (float)min  << '-' << (float)max;
	mout << mout.endl;

	mout.debug() << "prober:" << prober    << mout.endl;
	mout.debug() << "fill:  " << floodFill << mout.endl;

	typedef drain::typeLimiter<size_t> Limiter;
	Limiter::value_t limiter = Type::call<Limiter>(dst.getType());


	const SegmentStatistics & s = prober.statistics;
	/// TODO: handle std::exceptions better
	for (size_t i=0; i<width; i++){
		//std::cerr << "keijo" << std::endl;
		for (size_t j=0; j<height; j++){
			//if ((drain::Debug > 5) && ((i&15)==0) && ((j&15)==0)) std::cerr << i << ',' << j << "\n";

			// STAGE 1: detect statistics.
			prober.probe(i,j);  // painting with '1' does not disturb dst

			if (s.getSize() > 0){

				//std::cerr << "segment @" << i << ',' << j << " size="  << s.getSize() << " f="  << src.get<float>(i,j) << '\n'; // " prober=" << prober << std::endl;

				//mout.debug(8) << "segment @" << i << ',' << j << " f=" << src.get<float>(i,j) << " prober=" << prober << mout.endl;
				//std::cerr << "segment @" << i << ',' << j << " f=" << src.get<float>(i,j) << " prober: " << prober << std::endl;

				for (size_t k=0; k<count; k++){

					/// Phase 1: pick statistic
					double quantity = 0.0;
					switch (statistics.at(k)){
					case 'A':
						quantity = s.getSize();
						break;
					case 'a':
						quantity = prescale * s.getSize() / areaF;
						break;
					case 'X':
						quantity = s.getMeanX();
						break;
					case 'x':
						quantity = prescale * s.getMeanX() / widthF;
						break;
					case 'Y':
						quantity = s.getMeanY();
						break;
					case 'y':
						quantity = prescale * s.getMeanY() / heightF;
						break;
					case 'S':
						quantity = s.getVariance();
						break;
					case 's':
						quantity = prescale * s.getVariance() / areaF;
						break;
					case 'l':
						quantity = prescale * s.getSlimness();
						break;
						// TODO: annularity, slimness divided by
						//case 'c':
						//	quantity = prescale * s.getAnnularity();
						//	break;
					case 'h':
						quantity = prescale * s.getHorizontality();
						break;
					case 'v':
						quantity = prescale * s.getVerticality();
						break;
					case 'e':
						quantity = prescale * s.getElongation();
						break;
					default:
						mout.error() << "prober: unknown statistic: " << statistics.at(k) << mout.endl;
						return;
					}

					/// Phase 2: mapping (ie. scale the statistic)
					quantity = limiter(functor(quantity));
					// Marker must be > 0.
					if (quantity == 0.0)
						quantity = 1.0;

					//std::cerr << "segment @" << i << ',' << j << ": value="  << quantity << ", stats"  << s << " d="  << dst.get<float>(i,j) << '\n'; // " prober=" << prober << std::endl;
					// floodFill.setDstFrames(dst.getChannel(k));
					floodFill.setDst(dstTray.get(k));
					floodFill.count = 0;
					floodFill.conf.markerValue = static_cast<int>(quantity);
					//std::cerr << " fill VIS=" << floodFill.isVisited(i,j) << '\n';
					//std::cerr << " SEG=" << prober.isValidSegment(i,j) << '|' << floodFill.isValidSegment(i,j) << '\t';
					//std::cerr << " src=" << src.get<double>(i,j) << '[' << prober.conf.anchorMin << ',' << prober.conf.anchorMax <<  ']'<< '\t';
					floodFill.probe(i,j);
					//std::cerr << " size=" << floodFill.count << '\n';
					//floodFill.fill(i,j,size&254,min,max);
					//std::cerr << "filled\n";
				}


			}
			//}
		}
	}


}


} // namespace image

} // namespace drain


// Drain
