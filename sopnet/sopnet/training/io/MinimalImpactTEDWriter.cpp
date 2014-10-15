#include <iostream>
#include <fstream>
#include <algorithm>
#include <stdio.h>

#include <boost/timer/timer.hpp>

#include <imageprocessing/SubStackSelector.h>
#include <util/ProgramOptions.h>
#include "MinimalImpactTEDWriter.h" 

util::ProgramOption optionNumAdjacentSections(
		util::_module           = "sopnet.training",
		util::_long_name        = "numAdjacentSections",
		util::_default_value    = 0,
		util::_description_text = "The number of adjacent sections to consider for the computation of the minimal impact TED coefficients. If set to 0 (default), all sections will be considered.");

static logger::LogChannel minimalImpactTEDlog("minimalImpactTEDlog", "[minimalImapctTED] ");

MinimalImpactTEDWriter::MinimalImpactTEDWriter() :
	_teDistance(boost::make_shared<TolerantEditDistance>()),
	_gsimCreator(boost::make_shared<IdMapCreator>()),
	_rimCreator(boost::make_shared<IdMapCreator>()),
	_rNeuronExtractor(boost::make_shared<NeuronExtractor>()),
	_gsNeuronExtractor(boost::make_shared<NeuronExtractor>()),
	_rReconstructor(boost::make_shared<Reconstructor>()),
	_linearSolver(boost::make_shared<LinearSolver>()),
	_objectiveGenerator(boost::make_shared<ObjectiveGenerator>()),
	_hammingCostFunction(boost::make_shared<HammingCostFunction>())
	{
	
	registerInput(_goldStandard, "gold standard");
	registerInput(_linearConstraints, "linear constraints");
	registerInput(_segments, "segments");
	registerInput(_reference, "reference");
	registerInput(_problemConfiguration, "problem configuration");

}

void
MinimalImpactTEDWriter::write(std::string filename) {

	updateInputs();

	initPipeline();

	// Remove the old file
	if( remove( filename.c_str() ) != 0 ) {
		LOG_DEBUG(minimalImpactTEDlog) << "Old file to output minimal impact TED approximation sucessfully deleted." << std::endl;
	} 
	else {
		LOG_DEBUG(minimalImpactTEDlog) << "Could not delete old file to output minimal impact TED approximation." << std::endl;
	}

	// Open file for writing
	std::ofstream outfile;
	outfile.open(filename.c_str(), std::ios_base::app);
	
	 // Get a vector with all gold standard segments.
        const std::vector<boost::shared_ptr<Segment> > goldStandard = _goldStandard->getSegments();

	int constant = 0;

	// Loop through variables
	std::set<unsigned int> variables = _problemConfiguration->getVariables();

	LOG_USER(minimalImpactTEDlog) << "computing ted coefficients for " << variables.size() << " variables" << std::endl;

	outfile << "numVar " << variables.size() << std::endl;

	outfile << "# var_num costs # value_in_gs fs fm fp fn ( <- when flipped )" << std::endl;

	for ( unsigned int varNum = 0 ; varNum < variables.size() ; varNum++ ) {

		std::string timerMessage = "MinimalImpactTEDWriter: variable " + boost::lexical_cast<std::string>(varNum) + ", %ws\n";
		boost::timer::auto_cpu_timer timer(timerMessage);

		unsigned int segmentId = _problemConfiguration->getSegmentId(varNum);

		int interSectionInterval = _problemConfiguration->getInterSectionInterval(varNum);

		// re-create the pipeline for the current segment and its inter-section 
		// interval
		updatePipeline(interSectionInterval, optionNumAdjacentSections.as<int>());
	
		// Is the segment that corresponds to the variable part of the gold standard?
		bool isContained = false;
		foreach (boost::shared_ptr<Segment> s, goldStandard) {
			if (s->getId() == segmentId) {
				isContained = true;
				break;
			}
		}

		// pin the value of the current variable to its opposite
		_linearSolver->pinVariable(varNum, (isContained ? 0 : 1));

		pipeline::Value<Errors> errors = _teDistance->getOutput("errors");
		int sumErrors = errors->getNumSplits() + errors->getNumMerges() + errors->getNumFalsePositives() + errors->getNumFalseNegatives();

		outfile << "c" << varNum << " ";
		outfile << (isContained ? -sumErrors : sumErrors) << " ";
		outfile << "# ";
		outfile << (isContained ? 1 : 0) << " ";
		outfile << errors->getNumSplits() << " ";
		outfile << errors->getNumMerges() << " ";
		outfile << errors->getNumFalsePositives() << " ";
		outfile << errors->getNumFalseNegatives() << std::endl;

		if (isContained) {

			// Forced segment to not be part of the reconstruction.
			// This resulted in a number of errors that are going to be stored in the constant.
			// To make net 0 errors when the variable is on, minus the number of errors will be written to the file.

			constant += sumErrors;
		}

		// Remove constraint
		_linearSolver->unpinVariable(varNum);
	}

	// Write constant to file
	outfile << "constant " << constant << std::endl;

	outfile.close();
}

void
MinimalImpactTEDWriter::initPipeline() {

	// Here we assemble the static part of the pipeline, i.e., the parts that 
	// don't change between iterations. Currently, these are all parts below the 
	// linear solver that creates new reconstructions based on pinned variables.

	_hammingCostFunction = boost::make_shared<HammingCostFunction>();
	_objectiveGenerator = boost::make_shared<ObjectiveGenerator>();
	_linearSolver = boost::make_shared<LinearSolver>();

	// -- Gold Standard --> Hamming Cost Function
	_hammingCostFunction->setInput("gold standard", _goldStandard);

	// -- Segments --> Objective Generator
	_objectiveGenerator->setInput("segments", _segments);
	// Hamming Cost Function ----> Objective Generator
	_objectiveGenerator->addInput("cost functions", _hammingCostFunction->getOutput());

	// -- Linear Constraints --> Linear Solver
	_linearSolver->setInput("linear constraints",_linearConstraints);
	// -- Parameters --> Linear Solver
	_linearSolver->setInput("parameters", boost::make_shared<LinearSolverParameters>(Binary));
	// Objective Generator ----> Linear Solver
	_linearSolver->setInput("objective", _objectiveGenerator->getOutput());
}

void
MinimalImpactTEDWriter::updatePipeline(int interSectionInterval, int numAdjacentSections) {

	// create new pipeline components
	_teDistance = boost::make_shared<TolerantEditDistance>();
	_gsimCreator = boost::make_shared<IdMapCreator>();
	_rimCreator = boost::make_shared<IdMapCreator>();
	_rNeuronExtractor = boost::make_shared<NeuronExtractor>();
	_gsNeuronExtractor = boost::make_shared<NeuronExtractor>();
	_rReconstructor = boost::make_shared<Reconstructor>();

	// find the range of sections to consider for the computation of the TED
	int minSection = interSectionInterval - numAdjacentSections;
	int maxSection = interSectionInterval + numAdjacentSections - 1;

	// pipeline values for the image stacks used to compute the TED
	pipeline::Value<ImageStack> goldStandard;
	pipeline::Value<ImageStack> reconstruction;

	// get image stacks for TED either as subsets (if numAdjacentSections is 
	// set) or as the original stacks
	if (numAdjacentSections > 0) {

		pipeline::Process<SubStackSelector> goldStandardSelector(minSection, maxSection);
		pipeline::Process<SubStackSelector> reconstructionSelector(minSection, maxSection);

		goldStandardSelector->setInput(_gsimCreator->getOutput("id map"));
		reconstructionSelector->setInput(_rimCreator->getOutput("id map"));

		goldStandard   = goldStandardSelector->getOutput();
		reconstruction = reconstructionSelector->getOutput();

	} else {

		goldStandard = _gsimCreator->getOutput("id map");
		reconstruction = _rimCreator->getOutput("id map");
	}

	// Set inputs

	// The name ground truth is slightly misleading here because really we are setting the gold standard as input,
	// but that is what the correct input in the TED is called.
	// IdMapCreator [gold standard] ----> TED
	_teDistance->setInput("ground truth", goldStandard);
	// -- gold standard --> NeuronExtractor [gold standard]
	_gsNeuronExtractor->setInput("segments",_goldStandard);
	// NeuronExtractor [gold standard] ----> IdMapCreator [gold standard]
	_gsimCreator->setInput("neurons", _gsNeuronExtractor->getOutput("neurons"));
	// reference image stack for width height and size of output image stacks
	// -- reference --> IdMapCreator
	_gsimCreator->setInput("reference",_reference);
	// IdMapCreator [reconstruction] ----> TED
	_teDistance->setInput("reconstruction", reconstruction);
	// Reconstructor ----> NeuronExtractor [reconstruction]
	_rNeuronExtractor->setInput("segments", _rReconstructor->getOutput("reconstruction"));
	// NeuronExtractor [reconstruction] ----> IdMapCreator [reconstruction]
	_rimCreator->setInput("neurons", _rNeuronExtractor->getOutput("neurons"));
	// reference image stack for width height and size of output image stack
	// -- reference --> IdMapCreator
	_rimCreator->setInput("reference",_reference);
	// Linear Solver ----> Reconstructor
	_rReconstructor->setInput("solution", _linearSolver->getOutput("solution"));
	// -- Segments --> Reconstructor
	_rReconstructor->setInput("segments",_segments);
}
