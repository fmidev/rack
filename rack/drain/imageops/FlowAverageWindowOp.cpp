/**

    Copyright 2001 - 2010  Markus Peura, Finnish Meteorological Institute (First.Last@fmi.fi)


    This file is part of Drain library for C++.

    Drain is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Drain is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Drain.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <sstream>
#include <ostream>

#include "image/FilePng.h"

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

