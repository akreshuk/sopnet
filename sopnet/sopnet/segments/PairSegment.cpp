#include <imageprocessing/ConnectedComponent.h>
#include "PairSegment.h"

PairSegment::PairSegment(
		unsigned int id,
		boost::shared_ptr<Segment> segment1,
		boost::shared_ptr<Segment> segment2) :
		Segment(id,
				segment1->getDirection(),
				(segment1->getCenter()+segment2->getCenter())/2,
				segment1->getInterSectionInterval()){}

boost::shared_ptr<Segment>
PairSegment::getSegment1() const {
	return _segment1;
}

boost::shared_ptr<Segment>
PairSegment::getSegment2() const {
	return _segment2;
}

std::vector<boost::shared_ptr<Slice> >
PairSegment::getSlices() const {

	std::vector<boost::shared_ptr<Slice> > slices;

	slices.insert(slices.end(), _segment1->getSlices().begin(), _segment1->getSlices().end());
	slices.insert(slices.end(), _segment2->getSlices().begin(), _segment2->getSlices().end());

	return slices;
}
