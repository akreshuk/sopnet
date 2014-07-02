#include "PairSegmentCreator.h"
#include "vigra/impex.hxx"
#include <vigra/multi_array.hxx>
#include <vigra/accumulator.hxx>
#include <vigra/labelimage.hxx>
#include <util/rect.hpp>

/*
PairSegmentCreator::PairSegmentCreator(
		std::vector<boost::shared_ptr<Segments> >& segments,
		std::string prob_map_directory,
		double distance): _neuron_segments{segments},
		_pmap_dir{prob_map_directory},
		_distance{distance} {
			readPMap();
		}

PairSegmentCreator::PairSegmentCreator(
		std::vector<boost::shared_ptr<Segments> >& segments,
		ImageStack& pmap,
		double distance): _neuron_segments{segments},
		_synapse_map{pmap},
		_distance{distance} {}
*/

PairSegmentCreator::PairSegmentCreator(
		Segments& segments,
		ImageStack& syn_map,
		double distance): _neuron_segments{segments},
		_synapse_map{syn_map},
		_distance{distance} {};

/*
void
PairSegmentCreator::readSynapseMap() {
	//FIXME: this has to become a proper pipeline element
	//Let's think of this later
	//So far, copied from ImageStackDirectoryReader

	boost::filesystem::path dir(_pmap_dir);

	if (!boost::filesystem::exists(dir))
		BOOST_THROW_EXCEPTION(IOError() << error_message(_pmap_dir + " does not exist"));

	if (!boost::filesystem::is_directory(dir))
		BOOST_THROW_EXCEPTION(IOError() << error_message(_pmap_dir + " is not a directory"));

	// get a sorted list of image files
	std::vector<boost::filesystem::path> sorted;
	std::copy(
			boost::filesystem::directory_iterator(dir),
			boost::filesystem::directory_iterator(),
			back_inserter(sorted));
	std::sort(sorted.begin(), sorted.end());

	//LOG_DEBUG(imagestackdirectoryreaderlog) << "directory contains " << sorted.size() << " entries" << std::endl;

	_pmap = ImageStack();
	// for every image file in the given directory
	foreach (boost::filesystem::path file, sorted) {

		if (boost::filesystem::is_regular_file(file)) {

			//LOG_INFO(imagestackdirectoryreaderlog) << "creating image for " << file << std::endl;

			//read the files, don't bother with creating the reader for now
			vigra::ImageImportInfo info(file.c_str());

			boost::shared_ptr<Image> im = boost::make_shared<Image>(info.width(), info.height());
			vigra::importImage(info, vigra::destImage(*im));
			_pmap.add(im);


		}
	}
}
*/


void
PairSegmentCreator::groupBySynapse() {

	using namespace vigra::acc;
	int interval_index = 0; //awful, isnt' it?
	foreach(boost::shared_ptr<Image> im, _synapse_map) {

		vigra::MultiArray<2, unsigned char> bin(vigra::Shape2(im->width(), im->height()));
		vigra::MultiArray<2, unsigned int> cc(vigra::Shape2(im->width(), im->height()));

		vigra::transformMultiArray(*im, bin, vigra::Threshold<Image::value_type, int>(0.5, 1, 0, 1));
		unsigned int total_synapses = vigra::labelImageWithBackground(bin, cc, false, 0);
		std::cout<<total_synapses<<" found by thresholding in image "<<interval_index<<std::endl;
		//find the segments of this image
		//FIXME: only continuations for now?

		//for each synapse: 1) take the mask, 2) dilate by 1 or 2, 3) create ConnectedComponent
		// 4) for each segment, check intersections on the first slice

		//How to store the groups? do we need one-on-one neighborhood relations as well?


		std::vector<boost::shared_ptr<ContinuationSegment> > cont_segments;
		cont_segments = _neuron_segments.getContinuations(interval_index);

		//find the synapse bounding boxes
		AccumulatorChainArray<vigra::CoupledArrays<2, Image::value_type, unsigned int>,
		                             Select<DataArg<1>, LabelArg<2>,
		                             Coord<Minimum>, Coord<Maximum> > > acc;
		acc.ignoreLabel(0);

		extractFeatures(*im, cc, acc);

		//unsigned int min = get<Coord<Minimum> >(acc, 1)[0];
		//std::cout<<"min for the first synapse:"<<min<<std::endl;

		std::vector<std::vector<boost::shared_ptr<Segment> > > synapse_groups(total_synapses);
		for (unsigned int isyn =0; isyn<total_synapses; isyn++) {
			util::rect<unsigned int> syn_bbox;
			syn_bbox.minX = get<Coord<Minimum> >(acc, isyn)[0];
			syn_bbox.minY = get<Coord<Minimum> >(acc, isyn)[1];
			syn_bbox.maxX = get<Coord<Maximum> >(acc, isyn)[0];
			syn_bbox.maxY = get<Coord<Maximum> >(acc, isyn)[1];
			foreach(boost::shared_ptr<Segment> segment, cont_segments) {
				boost::shared_ptr<ConnectedComponent> first_slice_cc = segment->getSourceSlices()[0]->getComponent();
				//check, if bounding boxes overlap
				util::rect<int> slice_bbox = first_slice_cc->getBoundingBox();
				if (syn_bbox.intersects(slice_bbox)){
					synapse_groups[isyn].push_back(segment);
				}
			}
			std::cout<<"synapse "<<isyn<<", bounding box: "<<syn_bbox.minX<<", "<<syn_bbox.minY<<", "<< syn_bbox.maxX<<", "<<syn_bbox.maxY<<std::endl;
			std::cout<<"synapse "<<isyn<<", "<<synapse_groups[isyn].size()<<" segments overlap in bounding boxes"<<std::endl;


		}




		std::cout<<"PAIR SEGMENT CREATOR: processed another image "<<im->width()<<" "<<im->height()<<std::endl;
		interval_index++;
	}
}
