#include "PairSegmentExtractor.h"
#include "Segment.h"

PairSegmentExtractor::PairSegmentExtractor():
					_segments(new Segments()),
					_constraints(new LinearConstraints()){
	_minSynToNeuronDistance = 1;

	registerInput(_neuron_segments, "neuron segments");
	registerInput(_synapse_segments, "synapse segments");
	registerInput(_prev_slice_conflict_sets, "prev neuron slice conflict sets");
	registerInput(_next_slice_conflict_sets, "next neuron slice conflict sets");

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

	std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > > synapse_groups = createSynapseGroups();
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
	_distance_prev.clearCache();
	_distance_next.clearCache();


}

std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > >
PairSegmentExtractor::createSynapseGroups(){
	std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > > synapse_groups; //mapping from synapse id to its neighboring neurons
	double avg_distance, max_distance, min_distance;
	std::cout<<"creating synapse groups"<<std::endl;

	foreach (boost::shared_ptr<Segment> synapseSegment, _synapse_segments->getSegments()){
		std::vector<boost::shared_ptr<Slice> > prev_syn_slices = synapseSegment->getSourceSlices();
		std::vector<boost::shared_ptr<Slice> > next_syn_slices = synapseSegment->getTargetSlices();

		foreach (boost::shared_ptr<Segment> neuronSegment, _neuron_segments->getSegments()){
			std::vector<boost::shared_ptr<Slice> > prev_neuron_slices = neuronSegment->getSourceSlices();
			std::vector<boost::shared_ptr<Slice> > next_neuron_slices = neuronSegment->getTargetSlices();
			bool found_in_prev = false;
			foreach (boost::shared_ptr<Slice> synapseSlice, prev_syn_slices){
				foreach (boost::shared_ptr<Slice> neuronSlice, prev_neuron_slices) {
					_distance_prev(*synapseSlice, *neuronSlice, true, false, avg_distance, max_distance, min_distance);
					if (min_distance<_minSynToNeuronDistance){
						synapse_groups[synapseSegment->getId()].push_back(neuronSegment);
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
					if (min_distance<_minSynToNeuronDistance){
						synapse_groups[synapseSegment->getId()].push_back(neuronSegment);
						break;
					}
				}
			}

		}
		std::cout<<"for synapse with id "<<synapseSegment->getId()<<" collected "<<synapse_groups[synapseSegment->getId()].size()<<" neuron segments"<<std::endl;
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
PairSegmentExtractor::isConflictPresent(boost::shared_ptr<Segment> s1, boost::shared_ptr<Segment> s2){

	bool conflict_prev = checkConflict(s1->getSourceSlices(), s2->getSourceSlices(), *_prev_slice_conflict_sets);
	bool conflict_next = checkConflict(s1->getTargetSlices(), s2->getTargetSlices(), *_next_slice_conflict_sets);
	if (conflict_prev || conflict_next){
		return true;
	} else {
		return false;
	}
}

std::vector<boost::shared_ptr<PairSegment> >
PairSegmentExtractor::makePairs(std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > >& synapse_groups){

	std::vector<boost::shared_ptr<PairSegment> > segments;
	typedef std::map<unsigned int, std::vector<boost::shared_ptr<Segment> > > ::iterator map_iter;
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
				bool conflict = isConflictPresent(neuron_segments[i1], neuron_segments[i2]);
				if (!conflict){
					boost::shared_ptr<PairSegment> ps = boost::make_shared<PairSegment>(Segment::getNextSegmentId(), neuron_segments[i1], neuron_segments[i2]);
					//segments.push_back(ps);
					_segments->add(ps);
					//std::cout<<"added segment from two neuron segments, "<<ps->getId()<<" "<<neuron_segments[i1]->getId()<<" "<<neuron_segments[i2]->getId()<<std::endl;
				}

			}
		}
	}
	//std::cout<<"PAIR SEGMENT EXTRACTOR"<<" extracted pair segments for all synapses, total number: "<<_segments->size()<<std::endl;
	return segments;


}

