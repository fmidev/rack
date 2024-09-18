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


#include "drain/image/ImageT.h"
#include "drain/image/SuperProber.h"
#include "drain/util/Output.h"

#include "SuperProberOp.h"



namespace drain
{

namespace image
{

void SuperProberOp::traverseChannel(const Channel & src, Channel & dst) const {

	drain::Logger mout(getImgLog(), __FUNCTION__, getName());

	/*
	drain::Range<int> raw;
	raw.min = src.getScaling().inv(this->intensity.min);
	raw.max = (this->intensity.max == std::numeric_limits<double>::max()) ? src.getConf().getTypeMax<int>() : src.getScaling().inv(this->intensity.max);
	if (raw.min <= src.getConf().getTypeMin<int>()){
		mout.warn("src scaling:   "    , src.getScaling() );
		mout.warn("original range:   " , intensity );
		mout.warn("raw (code) range: " , raw );
		mout.warn("min value=" , (double)raw.min ,  " less or smaller than storage type min=" , src.getConf().getTypeMin<int>() );
	}
	mout.special("raw range: " , (double)raw.min , '-' , (double)raw.max );
	mout.debug2("src: " , src );
	mout.debug2("dst: " , dst );
	//SizeProber sizeProber(src, dst);
	SuperProber sizeProber(src, dst);
	sizeProber.conf.anchor.set(raw.min, raw.max);
	mout.debug2("areaProber:" , sizeProber );
	*/
	SuperProber sizeProber(src, dst);
	sizeProber.conf.anchor.set(threshold, 0xffff);

	mout.attention( sizeProber.conf.anchor);
	mout.attention( sizeProber.src);
	mout.attention(*sizeProber.dst);
	mout.attention( sizeProber.control.markerImage);
	mout.attention( sizeProber.control.handler);
	mout.attention( sizeProber.control.visitedMarker);

	sizeProber.control.markerImage.fill(0);


	FillProber floodFill(src, dst);
	floodFill.conf.anchor.set(threshold, 0xffff); // Min = raw.min;
	mout.special("Floodfill: " , floodFill );

	const double scale = drain::Type::call<typeNaturalMax>(dst.getType());
	// const double scale = drain::Type::call<drain::typeIsSmallInt>(dst.getType()) ? dst.getEncoding().getTypeMax<double>() : 1.0;
	mout.debug("Scale: " , scale );

	// const UnaryFunctor & ftor = getFunctor(scale); // scale problematic, but needed for const Ftor
	// mout.debug("Final functor: " , ftor.getName() , '(' , ftor.getParameters() , ')' );

	const CoordinatePolicy & cp = src.getCoordinatePolicy();
	const bool HORZ_MODE = ((cp.xUnderFlowPolicy != EdgePolicy::POLAR) && (cp.xOverFlowPolicy != EdgePolicy::POLAR));

	if (HORZ_MODE){
		mout.attention("scan HORIZONTAL, coordPolicy=", cp);
	}
	else {
		mout.attention("scan VERTICAL, coordPolicy=", cp);
	}

	typedef drain::typeLimiter<int> Limiter;
	typename Limiter::value_t limit = dst.getConf().getLimiter<int>();

	const size_t width = src.getWidth();
	const size_t height = src.getHeight();

	mout.attention("limiter=", limit);
	mout.attention("handler=", sizeProber.control.handler);

	TreeSVG svg(NodeSVG::SVG);
	TreeSVG & root = svg["segments"](NodeSVG::GROUP);
	TreeSVG & img = root["img"](NodeSVG::IMAGE);

	for (const auto & entry: Direction::offset){

		std::cout << entry.first << ": " << entry.second << '\n';
		for (int i=0; i<8; ++i){
			int deg = 45*i;
			std::cout << '\t' << DIR_TURN_DEG(entry.first, deg);
		}
		std::cout << '\n';

	}

	/*
	for (size_t j=0; j<height; j+=16){
		for (size_t i=0; i<width; i+=16){
			if (sizeProber.control.markerImage.get<int>(i,j) != sizeProber.control.markerImage.at(i,j)){
				mout.error(sizeProber.control.markerImage.get<int>(i,j), "!=", sizeProber.control.markerImage.at(i,j));
			}
		}
	}
	*/


	std::stringstream debugLabel;
	int debugIndex = 0;

	size_t sizeMapped;
	for (size_t j=0; j<height; j++){
		for (size_t i=0; i<width; i++){

			Position pos(i,j);

			/*
			if (sizeProber.control.isVisited(pos)){
				continue;
			}
			*/

			if (i==j){
				mout.warn("start: ",  i, ',', j);
			}

			if (debug > 0){
				debugLabel.str("");
				debugLabel << "debug-";
				debugLabel.width(3);
				debugLabel.fill('0');
				debugLabel << ++debugIndex;
			}

			// STAGE 1: detect size.
			// Note: retries same locations again and again. Could return true in success, i.e. first pixel was univisited and accepted.
			//sizeProber.probe(i,j, HORZ_MODE);
			sizeProber.clear();

			if (debug > 1){
				sizeProber.probe2(pos, Direction::RIGHT, root[debugLabel.str()](NodeSVG::POLYGON));
			}
			else {
				sizeProber.probe2(pos, Direction::RIGHT, root);
			}

			if (sizeProber.size > 0){

				// STAGE 2: mark the segment with size
				sizeMapped = limit(sizeProber.size); //limit(ftor(sizeProber.size));
				if (sizeMapped == 0)
					sizeMapped = 1;

				mout.warn("found segment at ", i, ',', j, " f=", src.get<float>(i,j), " size=", sizeProber.size, " => ", sizeMapped);

				if (debug > 0){
					mout.accept<LOG_WARNING>("Storing ", debugLabel.str());
					ImageFile::write(sizeProber.control.markerImage, debugLabel.str()+".png");
				}
				/*
				floodFill.conf.markerValue = sizeMapped;
				floodFill.probe(i,j, HORZ_MODE);
					*/
			}
			else {
				root.erase(debugLabel.str());
			}
		}
	}

	if (!filename.empty()){

		FilePath filePath(filename);

		// TODO:ctx could have svg, also for drainage ?

		if (NodeSVG::fileInfo.checkExtension(filePath.extension)){

			mout.note("Writing: ", filePath);
			drain::Output output(filePath);

			svg->set("width",  dst.getWidth());
			svg->set("height", dst.getHeight());

			root->setStyle("fill", "yellow");
			root->setStyle("stroke", "red");
			root->setStyle("stroke-width", "0.5px");

			filePath.extension = "png";
			mout.note("Writing: ", filePath);
			ImageFile::write(dst, filePath.str());
			img->set("xlink:href", filePath);
			img->set("width",  dst.getWidth());
			img->set("height", dst.getHeight());

			NodeSVG::toStream(output, svg);
			drain::TreeUtils::dump(svg, std::cout);

			filePath.basename = "dst";
			mout.note("Writing: ", filePath);
			ImageFile::write(sizeProber.control.markerImage, filePath.str());

		}
		else {
			mout.error("Extension '", filePath.extension ,"' not accepted for SVG, path=", filePath);
		}

	}



}

void SuperProberOp::traverseChannel2(const Channel & src, Channel & dst) const {

	ImageT<unsigned char> mask;
	mask.setGeometry(src.getGeometry().area);

	SuperProber prober(src, mask.getChannel(0));
	// Better use conf: prober.conf
	// prober.conf.importCastableMap(conf);

	prober.conf.anchor.min = threshold;
	prober.conf.anchor.max = 0xffff;

	prober.probe(0, 0, true);

}


} // namespace image

} // namespace drain

