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
# convert -append flowers-rgba.png  row-{a,g,d,D}.png -background black recRep.png
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

// Drain
