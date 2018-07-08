/*

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
	drainage shapes.png --segmentStats x,64,255,FuzzyStep,100:0 -o segmentStat-x.png
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
