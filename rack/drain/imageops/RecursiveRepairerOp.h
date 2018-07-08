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
#ifndef RECURSIVEREPAIREROP_H_
#define RECURSIVEREPAIREROP_H_

#include "QualityOp.h"

namespace drain
{

namespace image
{

/// Image restoration operator.
//
/*
for i in  15,15,5,a  15,15,5,g  50,50,1,d  25,25,1,D; do
	LABEL=${i##*,}
  	echo drainage flowers-rgba.png --recursiveRepairer "$i" -o recRep-$LABEL.png --view a -o recRep-$LABEL-alpha.png
    echo convert +append -frame 1 recRep-$LABEL.png recRep-$LABEL-alpha.png row-$LABEL.png
done
convert -append flowers-rgba.png  row-{a,g,d,D}.png -background black recRep.png
 */
/**

utility applying recursive interpolation from neighboring pixels.

Examples:
\code
 drainage flowers-rgba.png --recursiveRepairer 15,15,5,a -o recRep-a.png --view a -o recRep-a-alpha.png
 drainage flowers-rgba.png --recursiveRepairer 15,15,5,g -o recRep-g.png --view a -o recRep-g-alpha.png
 drainage flowers-rgba.png --recursiveRepairer 50,50,1,d -o recRep-d.png --view a -o recRep-d-alpha.png
 drainage flowers-rgba.png --recursiveRepairer 25,25,1,D -o recRep-D.png --view a -o recRep-D-alpha.png
\endcode

 */
//  drainage shapes1.png --resize 500,294 -o flower-mask.png
//  drainage flower-mask.png flowers.png --mul 1.0  -a flower-mask.png -o flowers-rgba.png
//
class RecursiveRepairerOp : public QualityOp
{
public:
	
	RecursiveRepairerOp(int width=5, int height=5, int loops=3, float decay=1.0f): QualityOp(__FUNCTION__,
			"Applies weighted averaging window repeatedly, preserving the pixels of higher weight.") {
		parameters.reference("width", this->width = width, "pix");
		parameters.reference("height", this->height = height, "pix");
		parameters.reference("loops", this->loops = loops);
		parameters.reference("smoother", this->smoother = "average", "a=average|g=gaussianAverage|d=distanceTransformFill|D=distanceTransformFillExp");
		//parameters.reference("decay", this->decay = decay);
	};

	int width;
	int height;
	int loops;
	// float decay;
	std::string smoother;


	void traverseChannels(const ImageTray<const Channel> &src, ImageTray<Channel> &dst) const;



};


}

}

#endif /*RECURSIVEREPAIRER_H_*/
