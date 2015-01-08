#include "PairFeatureExtractor.h"

util::ProgramOption optionPairSegmentsRandomForest(
		util::_module           = "sopnet.features",
		util::_long_name        = "pairSegmentsRandomForest",
		util::_description_text = "A trained random forest file for computing unaries of pair segments.");

PairFeatureExtractor::PairFeatureExtractor() {

	registerInput(_segments, "segments");
	registerInput(_synapseImages, "synapse images");

	registerOutput(_features, "features");
}

void
PairFeatureExtractor::updateOutputs() {

	_features->clear();

	// end, continuation, branch
	_features->createSegmentIdsMap(*_segments);
	_features->resize(3);

	_features->addName("p prob pre-post");
	_features->addName("p prob post-pre");
	_features->addName("p prob no-connection");

	foreach (boost::shared_ptr<EndSegment> segment, _segments->getEnds()) {

		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		_features->get(segment->getId())[1] = Features::NoFeatureValue;
		_features->get(segment->getId())[2] = Features::NoFeatureValue;
	}

	foreach (boost::shared_ptr<ContinuationSegment> segment, _segments->getContinuations()) {

		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		_features->get(segment->getId())[1] = Features::NoFeatureValue;
		_features->get(segment->getId())[2] = Features::NoFeatureValue;
	}

	foreach (boost::shared_ptr<BranchSegment> segment, _segments->getBranches()) {

		_features->get(segment->getId())[0] = Features::NoFeatureValue;
		_features->get(segment->getId())[1] = Features::NoFeatureValue;
		_features->get(segment->getId())[2] = Features::NoFeatureValue;
	}

	foreach (boost::shared_ptr<PairSegment> segment, _segments->getPairs()) {

		// TODO: ...
	}
}
