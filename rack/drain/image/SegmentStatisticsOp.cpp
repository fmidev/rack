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

#include "SegmentStatisticsOp.h"
#include "SegmentProber.h"
#include "SegmentStatisticsProber.h"

namespace drain
{
namespace image
{

void SegmentStatisticsOp::makeCompatible(const Image & src, Image & dst) const  {

	drain::MonitorSource mout(drain::image::iMonitor, name, __FUNCTION__);

	// mout.warn() << dst << mout.endl;

	if (!dst.isIntegerType()){
		if (dst.typeIsSet())
			throw std::runtime_error("SegmentAreaOp: float valued destination image not supported.");
		dst.setType<unsigned short>();
	}

	/// Todo: check view-images?
	dst.setGeometry(src.getWidth(), src.getHeight(), statistics.size(), dst.getAlphaChannelCount());

	// mout.warn() << dst << mout.endl;

	if (clearDst)
		dst.clear();

	/*
	if (dst.setGeometry(src.getWidth(), src.getHeight(),
			std::max(src.getImageChannelCount(),dst.getImageChannelCount()), dst.getAlphaChannelCount()))
		if (clearDst)
			dst.clear();
	*/
	//dst.info(std::cerr);
}



///
void SegmentStatisticsOp::traverse(const Image & src, Image & dst) const {

	drain::MonitorSource mout(drain::image::iMonitor, name, __FUNCTION__);

	const unsigned int width  = src.getWidth();
	const unsigned int height = src.getHeight();
	const unsigned int count  = statistics.size();

	const float widthF = static_cast<float>(width);
	const float heightF = static_cast<float>(height);
	const float areaF  = widthF*heightF;

	//dst.setGeometry(width, height, count);
	/*
	if (dst.isEmpty()){
		if (!dst.isView()){
			dst.setGeometry(width, height, count);
		}
		else {
			mout.warn() << "could not change geometry of dst, " << dst.getGeometry() << mout.endl;
		}
	}
	 */

	/// TODO: should use the actual types of src and dst.
	SegmentProber<int,int> floodFill(src);
	SegmentStatisticsProber<int,int> prober(src, dst, statistics);

	FunctorBank & functorBank = getFunctorBank();
	if (!functorBank.has(functorName)){
		mout.error() << "functor not found: " << functorName << mout.endl;
		return;
	}

	mout.debug(2) << "functorName: " << functorName << mout.endl;

	UnaryFunctor & functor = functorBank.get(functorName).clone();
	mout.debug() << functor.getName() << ':' << functor.getDescription() << mout.endl;
	functor.getParameters().separator = ':';

	functor.setScale(dst.getMax<double>(), 0.0);
	functor.setParameters(functorParams);

	mout.debug(1) << functor.getName() << ':' << functor << mout.endl;

	const float prescale = dst.getMax<double>();

	//mout.note(3) << *this << mout.endl;
	mout.debug(2) << "statistics: " << statistics << " count=" << count << '\n';
	mout << "src: " << src  << '\n';
	mout << "dst: " << dst  << '\n';
	mout << "range:" << (float)min  << '-' << (float)max;
	mout << "dst scale:" << (float)min  << '-' << (float)max;
	mout << mout.endl;

	const SegmentStatistics & s = prober.statistics;
	/// TODO: handle std::exceptions better
	for (size_t i=0; i<width; i++){
		//std::cerr << "keijo" << std::endl;
		for (size_t j=0; j<height; j++){
			//if ((drain::Debug > 5) && ((i&15)==0) && ((j&15)==0)) std::cerr << i << ',' << j << "\n";
			if (dst.get<int>(i,j) == 0){ // not visited

				// STAGE 1: detect statistics.
				prober.probe(i,j,1, static_cast<int>(min), static_cast<int>(max));  // painting with '1' does not disturb dst

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
						quantity = dst.limit<size_t>(functor(quantity));

						//mout.debug(12)
						//std::cerr << " quantity=" <<  quantity;
						// Marker must be > 0.
						if (quantity == 0.0)
							quantity = 1.0;
						//std::cerr << " >> "  << quantity << std::endl;

						floodFill.setDst(dst.getChannel(k));
						floodFill.probe(i,j, static_cast<int>(quantity), static_cast<int>(min), static_cast<int>(max));
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
