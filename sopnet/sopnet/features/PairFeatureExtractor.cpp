#include "PairFeatureExtractor.h"

util::ProgramOption optionPairSegmentsRandomForest(
		util::_module           = "sopnet.features",
		util::_long_name        = "pairSegmentsRandomForest",
		util::_description_text = "A trained random forest file for computing unaries of pair segments.");

PairFeatureExtractor::PairFeatureExtractor() {

	registerInput(_segments, "segments");
	registerInput(_synapseImages, "synapse images");
	registerInput(_vesicleImages, "vesicle images");

	registerOutput(_features, "features");
}

void
PairFeatureExtractor::updateOutputs() {

	_features->clear();

	// end, continuation, branch
	_features->createSegmentIdsMap(*_segments);
	_features->resize(1);
	_features->addName("prob paired")

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
		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		//_features->get(segment->getId())[1] = Features::NoFeatureValue;
		//_features->get(segment->getId())[2] = Features::NoFeatureValue;

	}
}

void computeProbFeatures(const PairSegment& pair, std::vector<double>& features){

}

void computeVesicleFeatures(const PairSegment& pair, std::vector<double>& features){

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
