#ifndef SOPNET_PAIR_SEGMENT_H__
#define SOPNET_PAIR_SEGMENT_H__

#include "Segment.h"

// forward declarations
class Slice;

//FIXME: in C++11, we'd have a scoped enum here...
//Pre - segment1 is pre-synaptic, segment2 is post-synaptic
//Post - segment1 is post-synaptic, segment1 is pre-synaptic
enum SynapseDirection {
	Pre,
	Post,
	Zero
};


class PairSegment : public Segment {

public:

	PairSegment(
			unsigned int id,
			boost::shared_ptr<Segment> segment1,
			boost::shared_ptr<Segment> segment2,
			boost::shared_ptr<Segment> synapseSegment,
			SynapseDirection dir);


	boost::shared_ptr<Segment> getSegment1() const;
	boost::shared_ptr<Segment> getSegment2() const;

	/* It's not yet clear if those are needed
	boost::shared_ptr<Slice> getSourceSlice1() const;

	boost::shared_ptr<Slice> getSourceSlice2() const;

	boost::shared_ptr<Slice> getTargetSlice1() const;

	boost::shared_ptr<Slice> getTargetSlice2() const;
    */

	std::vector<boost::shared_ptr<Slice> > getSlices() const;
	SynapseDirection getSynapseDirection() const {return _synapseDirection;}

private:

	boost::shared_ptr<Segment> _segment1;
	boost::shared_ptr<Segment> _segment2;
	boost::shared_ptr<Segment> _synapseSegment;

	SynapseDirection _synapseDirection;



};

#endif // SOPNET_PAIR_SEGMENT_H__
