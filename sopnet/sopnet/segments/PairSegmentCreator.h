#ifndef CELLTRACKER_PAIR_CREATOR_H__
#define CELLTRACKER_PAIR_CREATOR_H__

#include "PairSegment.h"
#include <util/point.hpp>
#include "Segments.h"
#include <imageprocessing/ImageStack.h>
#include <inference/LinearConstraint.h>

/*
 * Create pair segments out of continuation segments of neurons
 * It's not called Extractor, because it's not part of the SegmentExtractionPipeline (yet)
 */
class PairSegmentCreator {
public:
	//PairSegmentCreator(std::vector<boost::shared_ptr<Segments> >& segments, std::string prob_map_directory,  double distance);
	//PairSegmentCreator(std::vector<boost::shared_ptr<Segments> >& segments, ImageStack& pmap,  double distance);
	PairSegmentCreator(Segments& segments, ImageStack& pmap_th, double distance);


	boost::shared_ptr<std::vector<PairSegment> > getSegments() const;
	boost::shared_ptr<std::vector<LinearConstraint> > getConstraints() const;

	void groupBySynapse();

private:
	void readPMap();



	//std::vector<boost::shared_ptr<Segments > >_neuron_segments;
	Segments _neuron_segments;
	std::string _pmap_dir;
	ImageStack _synapse_map;
	double _distance;
	std::map<uint32_t, std::vector<boost::shared_ptr<ContinuationSegment> > > _synapseGroups; //segments around a synapse object
	std::vector<boost::shared_ptr<PairSegment> > _pairs;
	std::vector<boost::shared_ptr<LinearConstraint> > _constraints;


};

#endif // CELLTRACKER_PAIR_CREATOR_H__
