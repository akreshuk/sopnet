#ifndef SOPNET_FEATURES_PAIR_FEATURE_EXTRACTOR_H__
#define SOPNET_FEATURES_PAIR_FEATURE_EXTRACTOR_H__

#include <sopnet/segments/Segments.h>
#include <pipeline/SimpleProcessNode.h>
#include <imageprocessing/ImageStack.h>
#include <sopnet/segments/PairSegment.h>
#include <sopnet/segments/Segments.h>


#include "Features.h"

class PairFeatureExtractor : public pipeline::SimpleProcessNode<> {

public:

	PairFeatureExtractor();


private:

	void updateOutputs();

	double computeSizeFeature(boost::shared_ptr<PairSegment> segment);
	unsigned int computeSize(std::vector<boost::shared_ptr<Slice> > slices);

	pipeline::Input<Segments>   _segments;
	//pipeline::Input<ImageStack> _synapseImages;
	//pipeline::Input<ImageStack> _vesicleImages; //segmented vesicles, connected components image
	pipeline::Output<Features>  _features;
};

#endif // SOPNET_FEATURES_PAIR_FEATURE_EXTRACTOR_H__
