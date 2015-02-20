#include "PairFeatureExtractor.h"
#include <sopnet/segments/PairSegment.h>

util::ProgramOption optionPairSegmentsRandomForest(
		util::_module           = "sopnet.features",
		util::_long_name        = "pairSegmentsRandomForest",
		util::_description_text = "A trained random forest file for computing unaries of pair segments.");

PairFeatureExtractor::PairFeatureExtractor():
		_features(new Features()){

	registerInput(_segments, "segments");
	//registerInput(_synapseImages, "synapse images");
	//registerInput(_vesicleImages, "vesicle images");

	registerOutput(_features, "features");
}

void
PairFeatureExtractor::updateOutputs() {

	_features->clear();

	// end, continuation, branch
	_features->createSegmentIdsMap(*_segments);
	_features->resize(1);
	_features->addName("prob on");

	//_features->addName("p prob pre-post");
	//_features->addName("p prob post-pre");
	//_features->addName("p prob no-connection");

	foreach (boost::shared_ptr<EndSegment> segment, _segments->getEnds()) {

		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		//_features->get(segment->getId())[1] = Features::NoFeatureValue;
		//_features->get(segment->getId())[2] = Features::NoFeatureValue;
	}

	foreach (boost::shared_ptr<ContinuationSegment> segment, _segments->getContinuations()) {

		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		//_features->get(segment->getId())[1] = Features::NoFeatureValue;
		//_features->get(segment->getId())[2] = Features::NoFeatureValue;
	}

	foreach (boost::shared_ptr<BranchSegment> segment, _segments->getBranches()) {

		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		//_features->get(segment->getId())[1] = Features::NoFeatureValue;
		//_features->get(segment->getId())[2] = Features::NoFeatureValue;
	}

	foreach (boost::shared_ptr<PairSegment> segment, _segments->getPairs()) {

		// TODO: ...
		_features->get(segment->getId())[0] = computeSizeFeature(segment);

		//_features->get(segment->getId())[1] = Features::NoFeatureValue;
		//_features->get(segment->getId())[2] = Features::NoFeatureValue;

	}
}

unsigned int
PairFeatureExtractor::computeSize(std::vector<boost::shared_ptr<Slice> > slices){
	if (slices.size()!=1) {
		//>1: this is a branching segment, this shouldn't happen
		//=0: end segment, has no size
		return 0;
	}
	unsigned int size = slices[0]->getComponent()->getSize();
	return size;
}

double
PairFeatureExtractor::computeSizeFeature(boost::shared_ptr<PairSegment> pair){

	std::vector<boost::shared_ptr<Slice> > sources1 = pair->getSegment1()->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > sources2 = pair->getSegment2()->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > targets1 = pair->getSegment1()->getTargetSlices();
	std::vector<boost::shared_ptr<Slice> > targets2 = pair->getSegment2()->getTargetSlices();

	//take the average size b/w source and target
	unsigned int size1 = (computeSize(sources1)+computeSize(targets1))/2;
	unsigned int size2 = (computeSize(sources2)+computeSize(targets2))/2;

	SynapseDirection sd = pair->getSynapseDirection();

	if (abs(size1-size2)<20 && sd==SynapseDirection::Zero){
			return 1.0;
	}
	if (size1 > size2 && sd==SynapseDirection::Pre){
		return 1.0;
	}
	if (size2 > size1 && sd==SynapseDirection::Post){
		return 1.0;
	}
	return 0.0;

}

//void
//computeProbStatsSlices(boost::shared_ptr<Slice> slice1, boost::shared_ptr<Slice> slice2){

//}

/*
void 
computeProbFeatures(const PairSegment& pair, std::vector<double>& features){
    //we actually know we have one or less source or target slice
	std::vector<boost::shared_ptr<Slice> > sources1 = pair.getSegment1()->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > sources2 = pair.getSegment2()->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > targets1 = pair.getSegment1()->getTargetSlices();
	std::vector<boost::shared_ptr<Slice> > targets2 = pair.getSegment2()->getTargetSlices();
    std::vector<double> result_stats;
    if (sources1.size()==0 || sources2.size()==0){
        result_stats.push_back(0);
    } else {
        computeProbStatsSlices(sources1[0], sources2[0], result_stats);
    }

}

void 
computeVesicleFeatures(const PairSegment& pair, std::vector<double>& features){

	//in the vesicle image, see, which side has more vesicles
	std::vector<boost::shared_ptr<Slice> > sources1 = pair.getSegment1()->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > sources2 = pair.getSegment2()->getSourceSlices();
	std::vector<boost::shared_ptr<Slice> > targets1 = pair.getSegment1()->getTargetSlices();
	std::vector<boost::shared_ptr<Slice> > targets2 = pair.getSegment2()->getTargetSlices();

	//merge into one vector, we count all vesicles
	sources1.insert(sources1.end(), targets1.begin(), targets1.end());
	sources2.insert(sources2.end(), targets2.begin(), targets2.end());

	unsigned int nVesicles1 = 0;
	foreach (boost::shared_ptr<Slice> slice, sources1){
		nVesicles1 += countVesicles(slice);
	}
	unsigned int nVesicles2 = 0;
	foreach (boost::shared_ptr<Slice> slice, sources2){
		nVesicles2 += countVesicles(slice);
	}

	//TODO: now we should probably return std::pair<nVesicles1, nVesicles2> or something similar

}

unsigned int
countVesicles(boost::shared_ptr<Slice> slice){


	unsigned int section = slice->getSection();

	Image& vesicleImage = *(*_vesicleImages)[section];

	std::set<double> values;

	foreach (const util::point<unsigned int>& pixel, slice.getComponent()->getPixels()) {
		//FIXME: this should only be computed at a close enough distance from the synapse segment
		double value = vesicleImage(pixel.x, pixel.y); //FIXME: is it really double?
		values.insert(value);
	}
	return values.size();
}
*/
