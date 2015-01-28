#ifndef SOPNET_SEGMENTS_PAIR_SEGMENT_EXTRACTOR_H__
#define SOPNET_SEGMENTS_PAIR_SEGMENT_EXTRACTOR_H__

#include <sopnet/segments/Segments.h>
#include <pipeline/SimpleProcessNode.h>
#include <sopnet/features/Distance.h>
#include <sopnet/slices/ConflictSets.h>
#include <sopnet/inference/LinearConstraints.h>


class PairSegmentExtractor : public pipeline::SimpleProcessNode<> {

public:
	PairSegmentExtractor();
private:

	std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > > createSynapseGroups();
	bool isConflictPresent(boost::shared_ptr<Segment> s1, boost::shared_ptr<Segment> s2);
	bool checkConflict(std::vector<boost::shared_ptr<Slice> > slices_1, std::vector<boost::shared_ptr<Slice> > slices_2, ConflictSets& cs);
	std::vector<boost::shared_ptr<PairSegment> > makePairs(std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > >& synapse_groups);

	void updateOutputs();

	pipeline::Input<Segments>     _neuron_segments;
	pipeline::Input<Segments>     _synapse_segments;
	pipeline::Input<ConflictSets> _prev_slice_conflict_sets;
	pipeline::Input<ConflictSets> _next_slice_conflict_sets;

	pipeline::Output<Segments>  _segments;
	pipeline::Output<LinearConstraints> _constraints;

	Distance _distance_prev; //distance map for the first section
	Distance _distance_next; //distance map for the second section

	double _minSynToNeuronDistance;

};


#endif // SOPNET_SEGMENTS_PAIR_SEGMENT_EXTRACTOR_H__
