#include "PairSegmentExtractor.h"
#include "Segment.h"

int MAGIC_NUMBER = 10;


PairSegmentExtractor::PairSegmentExtractor():
					_segments(new Segments()),
					_constraints(new LinearConstraints()),
					_distance_prev(-1),
					_distance_next(-1),
					_overlap(false, false),
					_intersectionInterval(9999){
	_minSynToNeuronDistance = 1;

	registerInput(_neuron_segments, "neuron segments");
	registerInput(_synapse_segments, "synapse segments");
	registerInput(_prev_slice_conflict_sets, "prev neuron slice conflict sets", pipeline::Optional);
	registerInput(_next_slice_conflict_sets, "next neuron slice conflict sets", pipeline::Optional);

	registerOutput(_segments, "segments");
	registerOutput(_constraints, "linear constraints");

	//TODO: activate those
	//_prevConflictSets.registerCallback(&SegmentExtractor::onConflictSetsModified, this);
	//_nextConflictSets.registerCallback(&SegmentExtractor::onConflictSetsModified, this);
}

void
PairSegmentExtractor::updateOutputs(){
	//1) go over all synapse segments
	//2) find all neuron segments within distance of a parameter
	//3) for all these segments, extract pairs
	//4) for all pairs, check if members are not in conflict
	//5) for all pairs, check, that it is not yet present

	_distance_prev.clearCache();
	_distance_next.clearCache();


	std::cout<<"updating outputs of pair segment extractor at intersection interval "<<_intersectionInterval<<std::endl;
	std::map<boost::shared_ptr<Segment>, std::vector<boost::shared_ptr<Segment> > > synapse_groups = createSynapseGroups();
	makePairs(synapse_groups);
	//std::vector<boost::shared_ptr<PairSegment> > pair_segments;
	//pair_segments = makePairs(synapse_groups);
	//std::cout<<"pair segments returned, empty? "<<pair_segments.size()<<std::endl;
	//if (pair_segments.size()>0){
	//	for (unsigned int i=0; i<pair_segments.size(); ++i){
	//		std::cout<<"adding segment: "<<i<<" "<<"segment id"<<pair_segments[i]->getId()<<std::endl;
	//		_segments->add(pair_segments[i]);
	//	}

		//foreach (boost::shared_ptr<PairSegment> ps, pair_segments){

			//_segments->add(ps);
		//}
	//}



}

std::map<boost::shared_ptr<Segment>, std::vector<boost::shared_ptr<Segment> > >
PairSegmentExtractor::createSynapseGroups(){
	//std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > > synapse_groups; //mapping from synapse id to its neighboring neurons
	std::map<boost::shared_ptr<Segment>, std::vector<boost::shared_ptr<Segment> > > synapse_groups; //mapping from synapse id to its neighboring neurons
	double avg_distance, max_distance, min_distance;
	std::cout<<"creating synapse groups for "<<_synapse_segments->getSegments().size()<<" synapse segments"<<std::endl;

	foreach (boost::shared_ptr<Segment> synapseSegment, _synapse_segments->getSegments()){
		std::vector<boost::shared_ptr<Slice> > prev_syn_slices = synapseSegment->getSourceSlices();
		std::vector<boost::shared_ptr<Slice> > next_syn_slices = synapseSegment->getTargetSlices();

		foreach (boost::shared_ptr<Segment> neuronSegment, _neuron_segments->getEnds()){
			std::vector<boost::shared_ptr<Slice> > prev_neuron_slices = neuronSegment->getSourceSlices();
			std::vector<boost::shared_ptr<Slice> > next_neuron_slices = neuronSegment->getTargetSlices();

			if (prev_neuron_slices.size()==2 || next_neuron_slices.size()==2){
				//it's a branch, skip it
				continue;
			}

			if (neuronSegment->getDirection() != synapseSegment->getDirection())
				std::swap(prev_neuron_slices, next_neuron_slices);

			bool found_in_prev = false;
			foreach (boost::shared_ptr<Slice> synapseSlice, prev_syn_slices){
				foreach (boost::shared_ptr<Slice> neuronSlice, prev_neuron_slices) {
					_distance_prev(*synapseSlice, *neuronSlice, true, false, avg_distance, max_distance, min_distance);
					//if (min_distance<_minSynToNeuronDistance){
					if (min_distance==0){
						if (synapseSegment->getId()==9728){
							double over = _overlap(*synapseSlice, *neuronSlice);
							std::cout<<"for synapse 9728 and neuron "<<neuronSegment->getId()<<" slice overlap is "<<over<<", min_distance is "<<min_distance<<std::endl;

						}
						synapse_groups[synapseSegment].push_back(neuronSegment);
						found_in_prev = true;
						break;
					}
				}
			}

			if (found_in_prev){
				continue;
			}
			//if no source slices are close enough, let's check the target slices
			foreach (boost::shared_ptr<Slice> synapseSlice, next_syn_slices){
				foreach (boost::shared_ptr<Slice> neuronSlice, next_neuron_slices) {
					_distance_next(*synapseSlice, *neuronSlice, true, false, avg_distance, max_distance, min_distance);
					//if (min_distance<_minSynToNeuronDistance){
					if (min_distance==0) {
						synapse_groups[synapseSegment].push_back(neuronSegment);
						break;
					}
				}
			}
		}
		std::cout<<"for synapse with id "<<synapseSegment->getId()<<" collected "<<synapse_groups[synapseSegment].size()<<" neuron segments"<<std::endl;
		//std::set<boost::shared_ptr<Segment> > sanity_check(synapse_groups[synapseSegment->getId()].begin(), synapse_groups[synapseSegment->getId()].end() );
		//std::cout<<"set size for the same vector: "<<sanity_check.size()<<std::endl;
		if (synapseSegment->getId()==9728){
			std::cout<<"synapse 9728, center: "<<synapseSegment->getCenter().x<<" "<<synapseSegment->getCenter().y<<", neighboring neurons: "<<std::endl;
			foreach(boost::shared_ptr<Segment> neuronSegment, synapse_groups[synapseSegment]){
				std::cout<<neuronSegment->getId()<<" "<<neuronSegment->getCenter().x<<" "<<neuronSegment->getCenter().y<<std::endl;
			}
		}
	}
	return synapse_groups;
}

bool
PairSegmentExtractor::checkConflict(std::vector<boost::shared_ptr<Slice> > slices_1, std::vector<boost::shared_ptr<Slice> > slices_2, ConflictSets& cs){
	for (ConflictSets::iterator cs_it=cs.begin(); cs_it!=cs.end(); ++cs_it){
		bool n1_in = false;
		bool n2_in = false;
		foreach (boost::shared_ptr<Slice> slice_n1, slices_1){
			if (cs_it->getSlices().find(slice_n1->getId())!=cs_it->getSlices().end()){
				n1_in = true;
				break;
			}
		}
		foreach (boost::shared_ptr<Slice> slice_n2, slices_2){
			if (cs_it->getSlices().find(slice_n2->getId())!=cs_it->getSlices().end()){
				n2_in = true;
				break;

			}
		}
		if (n1_in && n2_in){
			return true;
		}
	}
	return false;
}


bool
PairSegmentExtractor::canBePaired(boost::shared_ptr<Segment> s1, boost::shared_ptr<Segment> s2){

	bool conflict_prev = false;
	bool conflict_next = false;
	double avg_distance, max_distance, min_distance=1000000;
	std::vector<boost::shared_ptr<Slice> > s1_sources = s1->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > s2_sources = s2->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > s1_targets = s1->getTargetSlices();
	std::vector<boost::shared_ptr<Slice> > s2_targets = s2->getTargetSlices();

	if (s1->getDirection() != s2->getDirection())
		std::swap(s2_sources, s2_targets);


	if (_prev_slice_conflict_sets.isSet())
		conflict_prev = checkConflict(s1_sources, s2_sources, *_prev_slice_conflict_sets);
	if (_next_slice_conflict_sets.isSet())
		conflict_next = checkConflict(s1_targets, s2_targets, *_next_slice_conflict_sets);
	if (conflict_prev || conflict_next)
		return false;

	//now check the distance b/w segments. Either source or target slices should be very close
	bool close_prev = false;
	bool close_next = false;
	foreach (boost::shared_ptr<Slice> s1_source, s1_sources){
		foreach(boost::shared_ptr<Slice> s2_source, s2_sources){
			_distance_prev(*s1_source, *s2_source, true, false, avg_distance, max_distance, min_distance);
			if (min_distance<MAGIC_NUMBER){
				close_prev = true;
				break;
			}
		}
	}
	foreach (boost::shared_ptr<Slice> s1_target, s1_targets){
		foreach(boost::shared_ptr<Slice> s2_target, s2_targets){
			_distance_next(*s1_target, *s2_target, true, false, avg_distance, max_distance, min_distance);
			if (min_distance<MAGIC_NUMBER){
				close_next = true;
				break;
			}
		}
	}

	if (close_prev || close_next) {
		return true;
	} else {
		return false;
	}

}

LinearConstraint
PairSegmentExtractor::makeConstraint(unsigned int segment_id_1, unsigned int segment_id_2){
	LinearConstraint constraint;
	constraint.setCoefficient(segment_id_1, 1.0);
	constraint.setCoefficient(segment_id_2, 1.0);
	constraint.setRelation(Equal);
	constraint.setValue(2.0);
	return constraint;
}

std::vector<boost::shared_ptr<PairSegment> >
PairSegmentExtractor::makePairs(std::map<boost::shared_ptr<Segment>, std::vector<boost::shared_ptr<Segment> > >& synapse_groups){

	std::vector<boost::shared_ptr<PairSegment> > segments;
	typedef std::map<boost::shared_ptr<Segment>, std::vector<boost::shared_ptr<Segment> > > ::iterator map_iter;
	for (map_iter it=synapse_groups.begin(); it!=synapse_groups.end(); ++it){
		std::vector<boost::shared_ptr<Segment> > neuron_segments = it->second;
		//std::cout<<"making pairs for synapse id "<<it->first<<std::endl;
		if (neuron_segments.size()==0){
			//std::cout<<"no neuron segments for synapse with id "<<it->first<<std::endl;
			continue;
		}
		unsigned int nsegments = neuron_segments.size();
		for (unsigned int i1=0; i1!=nsegments-1; ++i1 ){
			for (unsigned int i2=i1+1; i2!=nsegments; ++i2){
				//check, if the slices of the two segments are in some conflict set
				bool paired = canBePaired(neuron_segments[i1], neuron_segments[i2]);
				if (paired){
					boost::shared_ptr<PairSegment> ps_pre = boost::make_shared<PairSegment>(Segment::getNextSegmentId(), neuron_segments[i1], neuron_segments[i2],
							                                                            it->first, SynapseDirection::Pre );
					boost::shared_ptr<PairSegment> ps_post = boost::make_shared<PairSegment>(Segment::getNextSegmentId(), neuron_segments[i1], neuron_segments[i2],
                                                                                        it->first, SynapseDirection::Post );
					boost::shared_ptr<PairSegment> ps_zero = boost::make_shared<PairSegment>(Segment::getNextSegmentId(), neuron_segments[i1], neuron_segments[i2],
                                                                                        it->first, SynapseDirection::Zero );
					//segments.push_back(ps);
					_segments->add(ps_pre);
					_segments->add(ps_post);
					_segments->add(ps_zero);
					//either pre- or post- or nothing
					LinearConstraint constraintPrePost;
					constraintPrePost.setCoefficient(ps_pre->getId(), 1.0);
					constraintPrePost.setCoefficient(ps_post->getId(), 1.0);
					constraintPrePost.setCoefficient(ps_zero->getId(), 1.0);
					constraintPrePost.setRelation(LessEqual);
					constraintPrePost.setValue(1.0);
					_constraints->add(constraintPrePost);

					//if a pair is on, both segments and the synapse should be on
					LinearConstraint constraintFromAbove;
					constraintFromAbove.setCoefficient(ps_pre->getId(), 3.0);
					constraintFromAbove.setCoefficient(ps_post->getId(), 3.0);
					constraintFromAbove.setCoefficient(ps_zero->getId(), 3.0);
					constraintFromAbove.setCoefficient(neuron_segments[i1]->getId(), -1.0);
					constraintFromAbove.setCoefficient(neuron_segments[i2]->getId(), -1.0);
					constraintFromAbove.setCoefficient(it->first->getId(), -1.0);
					constraintFromAbove.setRelation(LessEqual);
					constraintFromAbove.setValue(0.0);
					_constraints->add(constraintFromAbove);

					//if both segments and the synapse is on, one of the pair should be on
					LinearConstraint constraintFromBelow;
					constraintFromBelow.setCoefficient(ps_pre->getId(), 3.0);
					constraintFromBelow.setCoefficient(ps_post->getId(), 3.0);
					constraintFromBelow.setCoefficient(ps_zero->getId(), 3.0);
					constraintFromBelow.setCoefficient(neuron_segments[i1]->getId(), -1.0);
					constraintFromBelow.setCoefficient(neuron_segments[i2]->getId(), -1.0);
					constraintFromBelow.setCoefficient(it->first->getId(), -1.0);
					constraintFromBelow.setRelation(GreaterEqual);
					constraintFromBelow.setValue(-2.0);
					_constraints->add(constraintFromBelow);

					//std::cout<<"added segment from two neuron segments, "<<ps->getId()<<" "<<neuron_segments[i1]->getId()<<" "<<neuron_segments[i2]->getId()<<std::endl;
				}

			}
		}
	}
	//std::cout<<"PAIR SEGMENT EXTRACTOR"<<" extracted pair segments for all synapses, total number: "<<_segments->size()<<std::endl;
	return segments;


}

