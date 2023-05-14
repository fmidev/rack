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

#include <sstream>
#include <ostream>

#include "drain/image/FilePng.h"

#include "ImageOp.h"

#include "FlowAverageWindowOp.h"

namespace drain
{

namespace image
{



void FlowAverageWindow::removePixel(Point2D<int> & p){

	if (this->coordinateHandler.validate(p)){
		double magnitude2 = 0.0;
		double v;
		ImageTray<const Channel>::const_iterator sit = srcTray.begin();
		std::vector<double>::iterator vit = sum.begin();
		while (sit != srcTray.end()){
			v = sit->second.getScaled(p.x, p.y);
			*vit -= v;
			magnitude2 += v*v; // note plus, and not w* yet
			++sit;
			++vit;
		}
		sumMagnitude -= sqrt(magnitude2);
		//sumW  -= 1.0;
		--count;
	}

}

void FlowAverageWindow::addPixel(Point2D<int> & p){
	if (this->coordinateHandler.validate(p)){
		double magnitude2 = 0.0;
		double v;
		ImageTray<const Channel>::const_iterator sit = srcTray.begin();
		std::vector<double>::iterator vit = sum.begin();
		while (sit != srcTray.end()){
			v = sit->second.getScaled(p.x, p.y);
			*vit += v;
			magnitude2 += v*v;
			++sit;
			++vit;
		}
		sumMagnitude += sqrt(magnitude2);
		//sumW  += 1.0;
		++count;
	}
}

void FlowAverageWindow::write(){

	if (count > 0){

		/// Compute magnitude of the current sum vector.
		double sumNorm = 0.0;
		for (std::vector<double>::const_iterator it = sum.begin(); it != sum.end(); ++it) {
			sumNorm += (*it)*(*it);
		}
		//if (debugDiag(4)){	std::cerr << sumNorm << '\n';		}

		if (sumNorm > 0.0){
			/// Invert and rescale with current averaged magnitude (sumMagnitude/sumW)
			sumNorm = (sumMagnitude/static_cast<double>(count))/sqrt(sumNorm); // sqrt(sumNorm);
			std::vector<double>::const_iterator vit = sum.begin();
			ImageTray<Channel>::iterator dit = dstTray.begin();
			while (dit != dstTray.end()){
				dit->second.put(this->location.x, this->location.y,
						limiter(dit->second.getScaling().inv(*vit * sumNorm)));
				++vit;
				++dit;
			}
			//dstTray.begin()->second.put(this->location.x, this->location.y, 255);
		}
	}

}





void FlowAverageWindowWeighted::removePixel(Point2D<int> & p){

	if (this->coordinateHandler.validate(p)){

		double w = this->srcWeight.get<double>(p);
		if (w > 0.0){
			double magnitude2 = 0.0;
			double v;
			ImageTray<const Channel>::const_iterator sit = srcTray.begin();
			std::vector<double>::iterator vit = sum.begin();
			while (sit != srcTray.end()){
				v = sit->second.getScaled(p.x, p.y);
				// v = sit->second.get<double>(p.x, p.y);
				*vit -= w*v;
				magnitude2 += v*v; // note plus, and not w* yet
				++sit;
				++vit;
			}
			sumMagnitude -= w*sqrt(magnitude2);
			sumW  -= w;
		}
		--count;

	}
}

void FlowAverageWindowWeighted::addPixel(Point2D<int> & p){

	if (this->coordinateHandler.validate(p)){

		double w = this->srcWeight.get<double>(p);
		if (w > 0.0){
			double magnitude2 = 0.0;
			double v;
			ImageTray<const Channel>::const_iterator sit = srcTray.begin();
			std::vector<double>::iterator vit = sum.begin();
			while (sit != srcTray.end()){
				v = sit->second.getScaled(p.x, p.y);
				//v = sit->second.get<double>(p.x, p.y);
				*vit += w*v;
				magnitude2 += v*v;
				++sit;
				++vit;
			}
			sumMagnitude += w*sqrt(magnitude2);
			sumW  += w;
		}
		++count;
	}

}


void FlowAverageWindowWeighted::write(){

	//if (sumW > 0.0){
	if (count > 0){

		/// Compute magnitude of the current sum vector.
		double sumNorm = 0.0;
		for (std::vector<double>::const_iterator it = sum.begin(); it != sum.end(); ++it) {
			sumNorm += (*it)*(*it);
		}

		if (sumNorm > 0.0){
			/// Invert and rescale with current averaged magnitude (sumMagnitude/sumW)
			sumNorm = (sumMagnitude/sumW)/sqrt(sumNorm);
			std::vector<double>::const_iterator it = sum.begin();
			ImageTray<Channel>::iterator dit = dstTray.begin();
			//if (debugDiag(4)) std::cerr << this->location << '\t' <<  (*it * sumNorm) << ' ' << (sumMagnitude/sumW) << '\n';
			while (dit != dstTray.end()){
				// Check dit->second.getScaling() for all the frames
				dit->second.put(this->location.x, this->location.y, limiter(dit->second.getScaling().inv(*it * sumNorm)));
				++it;
				++dit;
			}
		}

		this->dstWeight.put(this->location.x, this->location.y, sumW/static_cast<double>(count));

	}
	else
		this->dstWeight.put(this->location.x, this->location.y, 0.0);
}

/*
		if (qualitySensitive && false){
			double wOld = this->srcWeight.get<double>(this->location);
			double wNew = sumW/static_cast<double>(count);
				if (wOld > wNew){
				this->dst.put(this->location , src.get<double>(this->location));
				//this->dst2.put(this->location, src2.get<double>(this->location));
				this->dstWeight.put(this->location, wOld);
			}
			else {
				double norm = sqrt(((sumU2 + sumV2)/sumW) / (sumU*sumU + sumV*sumV));
				this->dst.putScaled(this->location.x, this->location.y , sumU*norm);
				this->dst2.putScaled(this->location.x, this->location.y, sumV*norm);
				this->dstWeight.put(this->location.x, this->location.y, wNew);
			}

		}
		else {
 */


}
}


// Drain
