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
#ifndef SEGMENT_STATISTICS_OP_H
#define SEGMENT_STATISTICS_OP_H

#include "SegmentAreaOp.h" // TODO prune to SegmentOp.h


namespace drain
{
namespace image
{

/// Computes statistics of segments: area, centroid, horizontality, verticality, variance and elongation. Segment is an area of connected pixels.
/*!

	Most descriptors have unscaled and scaled (normalized) versions, distinguished with uppercase and lowercase letters.

	For example, the unscaled area is obtained with \c 'A':
	\code
	drainage shapes.png --iSegmentStats A,64 -o segmentStat-area.png
	\endcode

	Respectively, the scaled area is obtained with \c 'a'.
	\code
	drainage shapes.png --iSegmentStats a,64,functor=FuzzyStep:0.5:0.75  -o segmentStat-areaRel.png
	\endcode

	The relative vertical position can be computed similarly, with \c 'y'.
	\~exec
	 drainage shapes.png --iSegmentStats y,64 -o segmentStat-y.png
	\~

  \code
	drainage shapes.png --iSegmentStats x,64,functor=FuzzyStep:100:0 -o segmentStat-x.png
	drainage shapes.png --iSegmentStats h,64 -o segmentStat-horz.png
	drainage shapes.png --iSegmentStats v,64 -o segmentStat-vert.png
	drainage shapes.png --iSegmentStats e,32 -o segmentStat-elong.png
	drainage shapes.png --iSegmentStats s,32,functor=FuzzyStepsoid:0:10 -o segmentStat-variance.png
  \endcode

	Several statistics can be computed simultaneously simply. For example,
	elongation (e), slimness (l) and y-coordinate (y):

	\code
		drainage shapes.png --iSegmentStats ely,64:255 -o segmentStat-ely.png
    \endcode

 */
class SegmentStatisticsOp: public SegmentOp
{
public:

	/// Default constructor.
	/**
	 */
	inline
	SegmentStatisticsOp(const std::string & statistics="d", double min=1, double max=255, double scale=1.0, double offset=0) :
    	SegmentOp(__FUNCTION__,"Segment statistics: area, mx, my, variance, slimness, horizontality, verticality, elongation") {
		parameters.link("statistics", this->statistics = statistics, "aAxXyYsSlhve");
		parameters.link("intensity", this->intensity.tuple(), "min:max");
		//parameters.link("min", this->min = min);
		//parameters.link("max", this->max = max);
		parameters.link("functor", this->functorStr);  //  = "FuzzyStep"
		this->intensity.set(min,max);
    };

	SegmentStatisticsOp(const SegmentStatisticsOp & op) : SegmentOp(__FUNCTION__, "Segment statistics: area, mx, my, variance, slimness, horizontality, verticality, elongation."){
		parameters.copyStruct(op.getParameters(), op, *this);
	}

    std::string statistics;

	/// Resizes dst, ensures src area and channel count for each stat. .
	//void makeCompatible(const ImageFrame & src, Image & dst) const;

    void getDstConf(const ImageConf & src, ImageConf & dst) const; // TODO: src, src2, dst


//protected:

    virtual
    void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;


   
};



} // namespace image
} // namespace drain


#endif /* SEGMENTAREAOP_H */

// Drain
