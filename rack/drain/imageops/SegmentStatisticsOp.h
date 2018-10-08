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

	Unscaled area (A):
	\code
	drainage shapes.png --segmentStats A,64 -o segmentStat-area.png
	\endcode

	\code
	\endcode
	The relative vertical position can be computed similarly, with \c 'y'.
	\~exec
	 drainage shapes.png --segmentStats y,64 -o segmentStat-y.png
	\~

  \code
	drainage shapes.png --segmentStats x,64,255,FuzzyStep:100:0 -o segmentStat-x.png
	drainage shapes.png --segmentStats y,64,255 -o segmentStat-y.png
	drainage shapes.png --segmentStats h,64,255 -o segmentStat-horz.png
	drainage shapes.png --segmentStats v,64,255 -o segmentStat-vert.png
	drainage shapes.png --segmentStats e,32,255 -o segmentStat-elong.png
	drainage shapes.png --segmentStats s,32,255 -o segmentStat-variance.png
  \endcode

	Several statistics can be computed simultaneously simply. For example,
	elongation (e), slimness (l) and y-coordinate (y):

	\code
		drainage shapes.png --segmentStats ely,64,255 -o segmentStat-ely.png
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
    	SegmentOp("SegmentStatistics","Segment statistics: area, mx, my, variance, slimness, horizontality, verticality, elongation") {
		parameters.reference("statistics", this->statistics = statistics, "aAxXyYsSlve");
		parameters.reference("min", this->min = min);
		parameters.reference("max", this->max = max);
		parameters.reference("functor", this->functorStr);  //  = "FuzzyStep"
		//parameters.reference("functorParams", this->functorParams);
		// const std::string & mapping="d",
    };


    std::string statistics;

	/// Resizes dst, ensures src area and channel count for each stat. .
	void makeCompatible(const ImageFrame & src, Image & dst) const;

protected:


    virtual
    void traverseChannels(const ImageTray<const Channel> & src, ImageTray<Channel> & dst) const;
    //void traverseChannel(const Channel & src , Channel &dst) const;


   
};



} // namespace image
} // namespace drain


#endif /* SEGMENTAREAOP_H */

// Drain
