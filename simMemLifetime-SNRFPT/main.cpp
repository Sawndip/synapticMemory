/*
 * main.cpp
 *
 *  Created on: Dec 15, 2011
 *      Author: kostasl
 */

#include "common.h"
#include "ContinuousTime/ContinuousTimeExperiments.h"
//#include "../synapseModels/common.h"

#include "InputVectorHandling.h"
#include "synapseAllocators.h"


#include <boost/program_options.hpp> //Located in usr/include --Using /usr/local/boost_1_42_0/stage/lib/libboost_program_options.a statically linked lib

namespace po = boost::program_options;
using namespace std;

//GLOBAL VARS
int g_FilterTh 			= 6; //Used for Single Filter Experiments
double g_FilterDecay 	= 0.0; //0.0916986;
uint g_AllocRefraction	= 0;//0.375*g_FilterTh*g_FilterTh;
uint g_timeToSampleMetaplasticity	= 10; //Used by Sim code as the time to sample the number of metaplastic transitions
uint g_MetaplasticitySampleSize		= 1;//Sim Code Stops saving to the distribution of same threshold crossings Once this number of samples has been gathered
double g_UpdaterQ 		= 1.0/(g_FilterTh*g_FilterTh); //The single Updater Transitions - Make sure its in double format
string g_outputTag;

double runContinuousMemoryRepetition(int modelType,double ts, long trials, int trackedMemIndex, int RepMemoryIndex, vector<double> vpReptimes, int CascadeSize, long synapsesPopulation, long lSimtimeSeconds, double dEncodingRate, string inputFile);

int main(int argc, char* argv[])
{

	clock_t start, finish;
	start = clock();
	map<uint,long> mapTrialToRepTime;
	map<string,unsigned int> mapSimType;
	map<string,int> mapSynapseAllocator; //An association of a the target object name With the allocation Function for the synapse Population

	po::options_description all("Allowed options"); //The group Of All options
	po::options_description basicSim("Simulation Averaging Options ");
	po::options_description singleFilterSim("Single Filter Simulation Options");
	po::options_description cascadeSim("Cascade simulation option");
	po::options_description AllocationOptions("Allocation Experiment simulation options");

	char file[200] = "\n";

	string inputFile,modelName;
	string simulationName = "simRepetition";
	int startIndex,endIndex,simulationType,modelType,synapsesPopulation,trackedMemIndex,initPeriod,iRepetitions;
	unsigned int trials;
	bool bUseCascadeParadigm;

	long lSimtimeSeconds = 250;
	int RepMemoryIndex =5;

	double ts = 1.000;//When Set to 1 simu. is in discrete Time
	double dEncodingRate = 1.0;
	vector<uint> pviTrackedIndex; //The List of Memories to Track - No Allocation Signal
	vector<uint> pviAllocIndex; //The list of Memories to Allocate - Allocation Signal Enabled
	vector< double > vdRepTime;//Vector of repetition times

	file[0] = 0; //Set to Null So we can detect if input has been given

	basicSim.add_options()
	    ("help", "produce help message")
	    ("model,M", po::value<string>(&modelName), "The model to run the simulation on")
		("simulation,S", po::value<string>(&simulationName)->default_value(simulationName), "The simulation name to run")
		("trials,T", po::value<unsigned int>(&trials)->default_value(10000), "Number of iteration to average over")
	    ("cSimTimeSecs,secs", po::value<long>(&lSimtimeSeconds)->default_value(lSimtimeSeconds), "Duration of continuous time simulation in seconds")
		("synapsesSize", po::value<int>(&synapsesPopulation)->default_value(10000), "The number of synapses to use - Has to match the vector file size where required")
		("inputFile,V", po::value<string>(&inputFile)->default_value("\n"), "The vector input file to use from directory MemoryInputVectors. If No file given then Random Vectors are used.")
		("UnAllocTID", po::value< vector<uint> >(&pviTrackedIndex)->multitoken(), "Pattern indexes to Track that will not be allocated. These patterns will signal No allocation")
		("AllocateTID", po::value< vector<uint> >(&pviAllocIndex)->multitoken(), "Pattern Index on which to enable the global Allocation Signal")
		("metaSampleTime", po::value<uint>(&g_timeToSampleMetaplasticity)->default_value(g_timeToSampleMetaplasticity), "Time to sample metaplasticity distribution");

	singleFilterSim.add_options()
	   ("FilterTh", po::value<int>(&g_FilterTh)->default_value(6), "The Single Filter's upper&Lower threshold Value")
	   ("FilterDecay,D", po::value<double>(&g_FilterDecay)->default_value(g_FilterDecay), "The Single Filter's Decay Value");

	cascadeSim.add_options()
	   ("startSize", po::value<int>(&startIndex)->default_value(1), "The first cascade size to begin testing")
	   ("endSize", po::value<int>(&endIndex)->default_value(1), "Simulate until this cascade size is reached")
	   ("initPeriod,I", po::value<int>(&initPeriod)->default_value(0), "Memories used to initialise Synapses")
	   ("UseCascade", po::value<bool>(&bUseCascadeParadigm)->default_value(true), "Should Construct cascade structure using model of choice Or single stochastic updater")
	   ("outFileTag,O", po::value<string>(&g_outputTag)->default_value(file), "A small string to attach to the outputFiles so they The experiment is identified")
	   ("StochUpdQ,Q", po::value<double>(&g_UpdaterQ)->default_value(g_UpdaterQ), "A small string to attach to the outputFiles so they The experiment is identified")
	   ("Timestep,ts", po::value<double>(&ts)->default_value(ts), "Sim. Timstep in seconds. Set to 1.0 for a discrete time simulation.")
	   ("cEncodingRate,R", po::value<double>(&dEncodingRate)->default_value(dEncodingRate), "Encoding rate - Or Rate of Incoming Patterns Default: 1sec");

	AllocationOptions.add_options()
		("repPatIndex,RI", po::value<int>(&RepMemoryIndex)->default_value(RepMemoryIndex), "The index of the pattern to repeat relative to the 1st tracked pattern")
		("repTimes,RT", po::value< vector<double> >(&vdRepTime)->multitoken(), "The relevant times a pattern is repeated after initial encoding")
		("AllocRefrac,RP", po::value<uint>(&g_AllocRefraction)->default_value(g_AllocRefraction), "The period a synapse needs to be stable before it is allocated");

	all.add(basicSim).add(singleFilterSim).add(cascadeSim).add(AllocationOptions);
	po::variables_map vm;
	po::store(po::parse_command_line(argc,  argv, all), vm);
	po::notify(vm);

	///Add List Of Simulation Types
	mapSimType["simMemSignalinTime"] = 1;
	mapSimType["simMemSignalsFromFile"] = 2;
	mapSimType["PerceptronTest"] = 3;
	mapSimType["HopfieldTest"] = 4;
	mapSimType["simMemSignalinContinuousTime"] = 5;
	mapSimType["simEscTime"] = 6;
	mapSimType["MeanMemoryLifetime"] = 7;
	mapSimType["simRepetition"] = 8;

	mapSynapseAllocator["synapseCascade"] = 1;// (pAllocationFunct)allocSynapseArrayCascade<synapseCascade>;
	mapSynapseAllocator["synapseCascadeFilterUnified"] = 2;
	mapSynapseAllocator["synapseCascadeFilterUnifiedWithDecay"] = 3;// (pAllocationFunct)allocSynapseArrayCascade<synapseCascadeFilterUnifiedWithDecay>;
	mapSynapseAllocator["synapseSingleFilterDual"] = 4;//(pAllocationFunct)allocSynapseArraySingleQ<synapseSingleFilterDual>;
	//mapSynapseAllocator["cascadeDelayed"]
	//mapSynapseAllocator["CascadeSamplingFilter"]
	//mapSynapseAllocator["synapseSingleFilterDual"]
	mapSynapseAllocator["synapseSingleFilterUnifiedWithDecay"] = 8;//(pAllocationFunct)allocSynapseArraySingleQ<synapseSingleFilterUnifiedWithDecay>;
	mapSynapseAllocator["synapseSingleUpdater"] = 9;//(pAllocationFunct)allocSynapseArraySingleQ<synapseSingleUpdater>;
	//mapSynapseAllocator["synapseSerialCascade"]
	mapSynapseAllocator["synapseSingleFilterUnifiedWithDecayRefl"] = 11;

	////A Map Of repetition Time Against Trial ID -Discrete Time- Number of Trials is used to distinquish the Repetition Time
	mapTrialToRepTime[10011] = g_FilterTh*g_FilterTh*0.375*1.0;
	mapTrialToRepTime[10012] = g_FilterTh*g_FilterTh*0.375*0.5;
	mapTrialToRepTime[10013] = g_FilterTh*g_FilterTh*0.375*2.0;
	mapTrialToRepTime[10014] = g_FilterTh*g_FilterTh*0.375*4.0;
	mapTrialToRepTime[10015] = g_FilterTh*g_FilterTh*0.375*5.0;
	mapTrialToRepTime[10016] = g_FilterTh*g_FilterTh*0.375*6.0;
	mapTrialToRepTime[10017] = g_FilterTh*g_FilterTh*0.375*0.25;
	mapTrialToRepTime[10018] = g_FilterTh*g_FilterTh*0.375*0.125;
	/////////

	trackedMemIndex = initPeriod;

	if (vm.count("simulation"))
		simulationType = mapSimType[simulationName];
	else
	{
		ERREXIT(1,"No simulation argument Specified");
	}

	if (simulationType==0)
		ERREXIT(2,"SimulationType not recognized");

	if (vm.count("model") == 0)
		ERREXIT(1,"No model argument Specified");

	if (mapSynapseAllocator.find(modelName) == mapSynapseAllocator.end()  )
	{	ERREXIT(2,"Model name not recognized");}
	else
	{
		modelType = mapSynapseAllocator[modelName];
	}

	////OPEN OUTPUT FILES To save The point When MEAN signal Drops below SNR=1
	string buffFilename(MFPTIMESSNR_OUTPUT_DIRECTORY);
	buffFilename.append(modelName);
	char buff[100];
	sprintf(buff,"_FPT-N%d_%d-%d_T%d_ts%.2f.dat",synapsesPopulation,startIndex,endIndex,trials,ts);
	buffFilename.append(buff);

	cout << "@ Simulation " << simulationName << " Output File:" << buffFilename.c_str() << endl;
	ofstream ofile(buffFilename.c_str(), ios::app ); //Open Data File for Appending So you dont Overwrite Previous Results

	if (!ofile.is_open())
		ERREXIT(errno,"Could not Open output file");

	//////LOG File Opened////
	ofile << "#First Passage Time Is where SNR=1 - That is the point where on Avg Signal crosses 0" << endl;
	ofile << "#Size\tMSFPT" << endl;

	double dMSFPT;
	//For Cascade Indexes
	for (int i=startIndex;i<=endIndex;i++)
	{
		 g_FilterTh =i;
		 g_UpdaterQ = 1.0/(g_FilterTh*g_FilterTh);

		 dMSFPT = runContinuousMemoryRepetition(modelType,ts,trials,trackedMemIndex,RepMemoryIndex,vdRepTime,i,synapsesPopulation,lSimtimeSeconds,dEncodingRate,inputFile);
		 ofile << i << "\t" << dMSFPT << endl;
		//Switch the Simulation Type
	 }//Loop For Each Cascade Index

	 ofile.close();
  ///Measure Duration
  finish = clock();
  ///Print Duration of Run - //TODO: This gives the wrong Time When using Threads!

  double duration = (double)(finish - start) / CLOCKS_PER_SEC;//Eclipse Reports Problem with  CLOCKS_PER_SEC But it compiles normally -Eclipse Bug
  printf( "\n Runtime was %2.1f seconds\n", duration );
 // std::exit(0);
return 0;
}



/*
 * This Is the Main Function that sets Up and Calls the Allocation Continuous Time Experiments
 * A repetition table is setup with the time of repetition and the memory index required.
 * Each Pattern added for repetition is automatically added to the list of tracked Pattern
 * RepMemoryIndex : Ignore The InitPatterns Just assume 0 is the first tracked Memory
 * Selects the appropriate Continuous Time Simulation To Run from the various objects and allocation Functions Available
 *
 * Returns the First Passage Time Of the Mean Signal under the noise
 */

double runContinuousMemoryRepetition(int modelType,double ts, long trials, int trackedMemIndex, int RepMemoryIndex, vector<double> vpReptimes, int CascadeSize, long synapsesPopulation, long lSimtimeSeconds, double dEncodingRate, string inputFile)
{
	double dMFPT; //The Time of Mean signal Crosses the noise
	double dRepIntervalsecs = 0;

cout << "MLT Continuous time of Repeated memory pattern:" << endl;
pAllocationFunct pF;
t_patt_reptbl repetitionTable;


//unsigned long lrepInterval = dRepIntervalsecs*(1.0/ts); //Normalize from Seconds to Timesteps

//repetitionTable[Timepoint of repetition] = RepMemoryIndex
///Rep Times Need to Account For the init period
for ( vector<double>::iterator it = vpReptimes.begin();it !=vpReptimes.end(); ++it)
{
	dRepIntervalsecs += *it;
	//Add the INdex of the memory with  A key Being the Time When it should be repeated
	repetitionTable[dRepIntervalsecs +(RepMemoryIndex)*(1.0/ts)*dEncodingRate] = trackedMemIndex+RepMemoryIndex; //Set Repetition Time At the peak of the tracked pattern
	cout << "ts:" << ts << " Repetition of Memory " << RepMemoryIndex << " at t:" << (dRepIntervalsecs+RepMemoryIndex) << endl;
}
cout << "#########" << endl;

/////////// LOG FILE INIT /////////////////////
//Add the File name as the 1st entry- Used by the makeLogFileNames
vector<string> slogFiles; //The list of output file names used
slogFiles.clear();
string fOutName(MFPTIMESSNR_OUTPUT_DIRECTORY);
slogFiles.push_back(fOutName);


/////////// END OF LOG FILE INIT //////////////////////////

switch (modelType)
{
case 1: //synapseCascade
	 pF =  (pAllocationFunct)allocSynapseArrayCascade<synapseCascade>;
	 makeLogFileNames<synapseCascade>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 //(pFunct pF, int iSynCount,int iCascadeSize,unsigned int iSimTime,unsigned int ciInitPeriod,double mdRate, double dFp=0.5)
	 //Also Available : simMemSignalinContinuousTime
	 dMFPT = simRepetitionAllocation<synapseCascade,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;
case 2: //Cascade Filter
	 pF =  (pAllocationFunct)allocSynapseArrayCascade<synapseCascadeFilterUnified>;
	 makeLogFileNames<synapseCascadeFilterUnified>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 //(pFunct pF, int iSynCount,int iCascadeSize,unsigned int iSimTime,unsigned int ciInitPeriod,double mdRate, double dFp=0.5)
	 //Also Available : simMemSignalinContinuousTime
	 dMFPT = simRepetitionAllocation<synapseCascadeFilterUnified,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;
case 3: //Cascade Filter with decay
	 pF =  (pAllocationFunct)allocSynapseArrayCascade<synapseCascadeFilterUnifiedWithDecay>;
	 makeLogFileNames<synapseCascadeFilterUnifiedWithDecay>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 //(pFunct pF, int iSynCount,int iCascadeSize,unsigned int iSimTime,unsigned int ciInitPeriod,double mdRate, double dFp=0.5)
	 //Also Available : simMemSignalinContinuousTime
	 dMFPT = simRepetitionAllocation<synapseCascadeFilterUnifiedWithDecay,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;
case 7:
	 pF =  (pAllocationFunct)allocSynapseArraySingleQ<synapseSingleFilterDual>;
	 makeLogFileNames<synapseSingleFilterDual>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 dMFPT = simRepetitionAllocation<synapseSingleFilterDual,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;
case 8: //A Single Filter Synapse
	 pF =  (pAllocationFunct)allocSynapseArraySingleQ<synapseSingleFilterUnifiedWithDecay>;
	 makeLogFileNames<synapseSingleFilterUnifiedWithDecay>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 dMFPT = simRepetitionAllocation<synapseSingleFilterUnifiedWithDecay,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;

case 9: //A Stochastic Updater Synapse
	 pF =  (pAllocationFunct)allocSynapseArraySingleQ<synapseSingleUpdater>;
	 makeLogFileNames<synapseSingleUpdater>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 //simMemSignalinContinuousTime<synapseSingleUpdater,pAllocationFunct>(pF, synapsesPopulation,i,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate);
	 dMFPT = simRepetitionAllocation<synapseSingleUpdater,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;
case 11: //U Filter Reflecting Boundary
	 pF =  (pAllocationFunct)allocSynapseArraySingleQ<synapseSingleFilterUnifiedWithDecayReflecting>;
	 makeLogFileNames<synapseSingleFilterUnifiedWithDecayReflecting>(slogFiles,trackedMemIndex,CascadeSize,dRepIntervalsecs, 0.5,trials, synapsesPopulation,pF);
	 dMFPT = simRepetitionAllocation<synapseSingleFilterUnifiedWithDecayReflecting,pAllocationFunct>(pF, synapsesPopulation,CascadeSize,trackedMemIndex,(char*)inputFile.c_str(), trials,lSimtimeSeconds,dEncodingRate,repetitionTable,ts,slogFiles);
break;

default:
	cerr << "Don't Have a repetition simulation associated with this type "<< modelType << " of object." << endl;
	ERREXIT(500,"Object not recognized for repetition simulation");
	break;
}

return dMFPT;
}




