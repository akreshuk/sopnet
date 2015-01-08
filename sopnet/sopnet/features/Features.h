#ifndef SOPNET_FEATURES_H__
#define SOPNET_FEATURES_H__

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

#include <pipeline/all.h>
#include <sopnet/segments/Segments.h>

class Features : public pipeline::Data {

	typedef std::vector<std::vector<double> > features_type;

public:

	typedef std::map<unsigned int, unsigned int> segment_ids_map;

	Features();

	typedef features_type::iterator iterator;

	typedef features_type::const_iterator const_iterator;

	void addName(const std::string& name);

	const std::vector<std::string>& getNames() const;

	void clear();

	/**
	 * Create a mapping from segment ids to feature vector indices. Creates one empty feature vector for each segment.
	 */
	void createSegmentIdsMap(const Segments& segments);

	/**
	 * Resize each currently contained feature vector. New values will be set to NoFeatureValue.
	 */
	void resize(unsigned int numFeatures);

	unsigned int numFeatures();

	/**
	 * Get the feature vector associated to the given segment id.
	 */
	std::vector<double>& get(unsigned int segmentId);

	unsigned int size();

	iterator begin();

	iterator end();

	const_iterator begin() const;

	const_iterator end() const;

	/**
	 * Get the ith feature vector.
	 */
	std::vector<double>& operator[](unsigned int i);

	/**
     * Get the ith feature vector.
	 */
	const std::vector<double>& operator[](unsigned int i) const;

	void setSegmentIdsMap(const segment_ids_map& map);

	const segment_ids_map& getSegmentIdsMap() const;

	static double NoFeatureValue;

private:

	friend class boost::serialization::access;
	template <class Archive>
	void serialize(Archive& archive, const unsigned int version) {

		archive & _features;
		archive & _featureNames;
		archive & _segmentIdsMap;
	}

	features_type            _features;

	std::vector<std::string> _featureNames;

	// a map from segment ids to the corresponding feature
	segment_ids_map          _segmentIdsMap;
};

std::ostream&
operator<<(std::ostream& out, const Features& features);

#endif // SOPNET_FEATURES_H__

