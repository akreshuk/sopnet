#include <sopnet/segments/EndSegment.h>
#include <sopnet/segments/ContinuationSegment.h>
#include <sopnet/segments/BranchSegment.h>
#include "TypeFeatureExtractor.h"

logger::LogChannel typefeatureextractorlog("typefeatureextractorlog", "[TypeFeatureExtractor] ");

TypeFeatureExtractor::TypeFeatureExtractor() :
	_features(new Features()) {

	registerInput(_segments, "segments");
	registerOutput(_features, "features");
}

void
TypeFeatureExtractor::updateOutputs() {

	LOG_DEBUG(typefeatureextractorlog) << "extracting features" << std::endl;

	_features->clear();

	// end, continuation, branch
	_features->createSegmentIdsMap(*_segments);
	_features->resize(4);

	_features->addName("is end");
	_features->addName("is continuation");
	_features->addName("is branch");
	_features->addName("is pair");


	foreach (boost::shared_ptr<EndSegment> segment, _segments->getEnds()) {

		_features->get(segment->getId())[0] = 1;
		_features->get(segment->getId())[1] = 0;
		_features->get(segment->getId())[2] = 0;
		_features->get(segment->getId())[3] = 0;
	}

	foreach (boost::shared_ptr<ContinuationSegment> segment, _segments->getContinuations()) {

		_features->get(segment->getId())[0] = 0;
		_features->get(segment->getId())[1] = 1;
		_features->get(segment->getId())[2] = 0;
		_features->get(segment->getId())[3] = 0;
	}

	foreach (boost::shared_ptr<BranchSegment> segment, _segments->getBranches()) {

		_features->get(segment->getId())[0] = 0;
		_features->get(segment->getId())[1] = 0;
		_features->get(segment->getId())[2] = 1;
		_features->get(segment->getId())[3] = 0;
	}

	foreach (boost::shared_ptr<PairSegment> segment, _segments->getPairs()) {

		_features->get(segment->getId())[0] = 0;
		_features->get(segment->getId())[1] = 0;
		_features->get(segment->getId())[2] = 0;
		_features->get(segment->getId())[3] = 1;
	}

	LOG_ALL(typefeatureextractorlog) << "found features: " << *_features << std::endl;

	LOG_DEBUG(typefeatureextractorlog) << "done" << std::endl;
}

