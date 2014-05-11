#include "PairSegmentCreator.h"
#include "vigra/impex.hxx"

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
		_pmap{pmap},
		_distance{distance} {}

void
PairSegmentCreator::readPMap() {
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

void
PairSegmentCreator::groupBySynapse() {
	foreach(boost::shared_ptr<Image> im, _pmap) {
		/*
		Image bin(im.width(), im.height());
		vigra::transformImage(im.upperLeft(), im.lowerRight(), im.accessor(),
		                      bin.upperLeft(), bin.accessor(),
		                      vigra::Threshold<Image::PixelType, Image::PixelType>(0.5, 0.5, 0, 1));
        */

		std::cout<<"processed another image "<<im->width()<<" "<<im->height()<<std::endl;
	}
}
