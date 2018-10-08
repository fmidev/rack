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

	drain::Logger mout(getImgLog(), getName(), __FUNCTION__);

	mout.debug() << dst << mout.endl;

	const std::type_info & t = typeid(unsigned short);

	if (!dst.typeIsSet()){
		dst.setType(t);
	}
	else if (!dst.isIntegerType()){
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

	drain::Logger mout(getImgLog(), name, __FUNCTION__);

	const Channel & src = srcTray.get();
	Channel & dst = dstTray.get();

	const size_t width  = src.getWidth();
	const size_t height = src.getHeight();
	const unsigned int count  = statistics.size();

	const float widthF = static_cast<float>(width);
	const float heightF = static_cast<float>(height);
	const float areaF  = widthF*heightF;

	const int minI = static_cast<int>(min);
	const int maxI = static_cast<int>(max);


		/// TODO: should use the actual types of src and dst.
	SegmentProber<int,int> floodFill(src);
	SegmentStatisticsProber<int,int> prober(src, dst, statistics);


	const UnaryFunctor & functor = getFunctor(dst.getMax<double>());  // what if dst float?
	mout.debug(1) << functor.getName() << ':' << functor << mout.endl;

	const float prescale = dst.getMax<float>();

	//mout.note(3) << *this << mout.endl;
	mout.debug(2) << "statistics: " << statistics << " count=" << count << '\n';
	mout << "src: " << src  << '\n';
	mout << "dst: " << dst  << '\n';
	mout << "range:" << (float)min  << '-' << (float)max;
	mout << "dst scale:" << (float)min  << '-' << (float)max;
	mout << mout.endl;

	typedef drain::typeLimiter<size_t> Limiter;
	Limiter::value_t limiter = Type::call<Limiter>(dst.getType());


	const SegmentStatistics & s = prober.statistics;
	/// TODO: handle std::exceptions better
	for (size_t i=0; i<width; i++){
		//std::cerr << "keijo" << std::endl;
		for (size_t j=0; j<height; j++){
			//if ((drain::Debug > 5) && ((i&15)==0) && ((j&15)==0)) std::cerr << i << ',' << j << "\n";
			if (dst.get<int>(i,j) == 0){ // not visited

				// STAGE 1: detect statistics.
				prober.probe(i,j,1, minI, maxI);  // painting with '1' does not disturb dst

				//std::cerr << "segment @" << i << ',' << j << " size="  << s.getSize() << " f="  << src.get<float>(i,j) << '\n'; // " prober=" << prober << std::endl;

				if (s.getSize() > 0){

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
						//mout.debug(12)
						//std::cerr << " quantity=" <<  quantity;
						// Marker must be > 0.
						if (quantity == 0.0)
							quantity = 1.0;

						// floodFill.setDstFrames(dst.getChannel(k));
						floodFill.setDst(dstTray.get(k));
						floodFill.probe(i,j, static_cast<int>(quantity), minI, maxI);
						//floodFill.fill(i,j,size&254,min,max);
						//std::cerr << "filled\n";
					}


				}
			}
		}
	}


}


} // namespace image

} // namespace drain


// Drain
