#ifndef SOPNET_PAIR_SEGMENT_H__
#define SOPNET_PAIR_SEGMENT_H__

#include "Segment.h"

// forward declarations
class Slice;

class PairSegment : public Segment {

public:

	PairSegment(
			unsigned int id,
			boost::shared_ptr<Segment> segment1,
			boost::shared_ptr<Segment> segment2);



	boost::shared_ptr<Segment> getSegment1() const;
	boost::shared_ptr<Segment> getSegment2() const;

	/* It's not yet clear if those are needed
	boost::shared_ptr<Slice> getSourceSlice1() const;

	boost::shared_ptr<Slice> getSourceSlice2() const;

	boost::shared_ptr<Slice> getTargetSlice1() const;

	boost::shared_ptr<Slice> getTargetSlice2() const;
    */

	std::vector<boost::shared_ptr<Slice> > getSlices() const;

private:

	boost::shared_ptr<Segment> _segment1;
	boost::shared_ptr<Segment> _segment2;

};

#endif // SOPNET_PAIR_SEGMENT_H__
