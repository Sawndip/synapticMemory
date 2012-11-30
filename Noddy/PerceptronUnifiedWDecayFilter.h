/*
 * PerceptronUnifiedWDecayFilter.h
 *
 *  Created on: 14 Mar 2011
 *      Author: kostasl
 */

#ifndef PERCEPTRONUNIFIEDWDECAYFILTER_H_
#define PERCEPTRONUNIFIEDWDECAYFILTER_H_

//<Lookup Tables For Parameters>

///RE-INjection At ZERO - PDFs
const double mdPDF_r100[][7] = {
		 {0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0}, //n=1
		 {0.0, 0.0 ,0.170, 0.659, 0.170, 0.0, 0.0}, //n=2 - Sim i0
		 {0.0, 0.0 ,0.256193, 0.480363, 0.256193, 0.0, 0.0}, //n=3
		 {0.0, 0.0, 0.368089, 0.262159, 0.368089, 0.0, 0.0}, //n=4 i0

	     {0.06250000000000000000, 0.1250000000000000000, 0.187500000000000000, 0.25000000000000000,  0.1875000000000000, 0.12500000000000, 0.0625000000000000000}, //n=5 i0
	     {0.0517318430596683797, 0.120569855949104312, 0.207128445042122995, 0.241139711898208625, 0.207128445042122995, 0.120569855949104312, 0.05173184305966838},//n=6 0i
	     {0.04178090320605485768, 0.11991970205845851808,0.22669658910714549203,0.22320561125668226443,0.2266965891071454920,0.1199197020584585181,0.0417809032060548577}, //n=7
	     {0.03033980324165782516, 0.11375865233380612532, 0.24617387244966487268, 0.21945534394974235366, 0.2461738724496648727, 0.1137586523338061253, 0.030339803241657825},//n=8
	     {0.02207650931777412286,0.10759264781989243877, 0.26477172580743867559 , 0.21111823410978952554, 0.264771725807438676, 0.1075926478198924388, 0.022076509317774123},//n=9
	     {0.01551202508390696529,0.10039869969193056834, 0.28466441487178715927, 0.19884972070475061420, 0.284664414871787159, 0.1003986996919305683, 0.015512025083906965},//n=10
	     {0.01107689622611658533, 0.09328862481463681768, 0.30283461593474448079, 0.18559972604900423240, 0.302834615934744481, 0.0932886248146368177, 0.011076896226116585},//n=11
	     {0.00794007027933235005, 0.08620047067753524242, 0.31990807418748821973, 0.17190276971128837562, 0.319908074187488220, 0.086200470677535242, 0.007940070279332350},//n=12
	     {0.00561104924088536600, 0.07892128935747792290, 0.33667028676943866612, 0.15759474926439608994, 0.336670286769438666, 0.078921289357477923, 0.005611049240885366}, //n=13
	     {0.00390308244102358892, 0.07158541208445645002, 0.35298599021679465694, 0.14305103051545060824, 0.352985990216794657, 0.071585412084456450, 0.00390308244102359}, //n=14
	     {0.00279223634637394967, 0.06516152558458873724, 0.36691538387921754998, 0.13026170837963952624, 0.36691538387921755, 0.065161525584588737,  0.00279223634637395} //n=15
 };

//With the View that the lower theshold is the Reflecting one
//		//Reflecting								//Absorbing
const double mdPDFTerminal_r100[][7] = {
						{0.0, 0.0, 0.0,		1.0, 	  0.0, 		0.0,	0.0}, //n=1
						{0.0, 0.0 ,0.0,		1.0, 	  0.0,		0.0, 	0.0}, //n=2 - Sim i0
						{0.0, 0.0, 0.5, 	0.50,	  0.0, 		0.0, 	0.0}, //n=3
						{0.0, 0.0, 0.3333, 0.33333,  0.2222, 	0.1111,	0.0}, //n=4 i0 //Thres 1,3

						{0.0, 0.09237, 0.3521,   0.2083,  0.34723, 0.0, 0.0}, //n=5 i0 Thres 2,2
						{0.0, 0.03616, 0.4329,   0.09836, 0.4325, 0.0, 0.0},//n=6 0i
						{0.0, 0.01737, 0.4664,   0.04973, 0.4664, 0.0, 0.0}, //n=7 Thres 2,2
						//Thres 3,3
					    {0.01020117410199211142, 0.0913429598129562639, 0.313347967390033711, 0.186262213680185469, 	  0.3114098711587613,  0.0874358138560711, 	0.0},//n=8
						{0.00464808937484574594, 0.0748993804197276717, 0.347784022265603529, 	  0.152133069779678630,   0.347192837188120,   0.073342600972023,	0.0},//n=9
						{0.0023538540965529977,  0.061932317432758907,   0.37469405428023903, 	  0.125284389548946356,   0.374489542593762,   0.0612458420477402, 	0.0},//n=10
						{0.00141733266077160678, 0.053316276325584176,   0.39241757230475938, 	  0.107574555123429597,   0.39232617721386379, 0.05294808637159145,	0.0},//n=11
						{0.00051651803985112326, 0.039029305280931016,   0.42154776460888664, 	  0.078451179945527004,   0.42152979889170465, 0.03892543323309956,	0.0},//n=12
						{0.00025185174365122620, 0.031032426527322594,   0.43773065851853435,	  0.062269089548515900,   0.43772508641419692, 0.03099088724777900,	0.0}, //n=13
						{0.00012365502526760665, 0.024638552656549110,   0.45061763483271835,    0.049382304279847994,   0.45061589936851583, 0.02462195383710111,	0.0}, //n=14
						{0.00006935118818222132, 0.020388589671107832,   0.45916198988909251,    0.040837990932378369,   0.45916132011373434, 0.0203807582055047,	0.0} //n=15
				 };


//alpha = 0.5
//Array of Upper and Lower Threshold values used by strong and weak cascade, 1st column is p escape and 2nd is q
///Matched Escape times to 1/q+p
//N=15 Double Decay Filter
 const int miThreshold_r100[][2] ={ {1,1},{1,2},
									{2,2},{2,2},
									{4,4},{4,4}, //n=6
									{4,4},{4,4},
									{4,4},{4,4}, //n=10
									{4,4},{4,4}, //n=12
									{4,4},{4,4}, //n=14
									{4,4} }; ////n=15

 //TODO: UNIFIED FILTER WITH DECAY: NEED TO MAKE THE REFLECTING VALUES FOR R=0.1 and r=0.01
 const int miThresholdTerminal_r100[][2] ={ {0,1},{0,1},
											{1,1},{3,1},
											{2,2},{2,2}, //n=6
											{2,2},{3,3},
											{3,3},{3,3}, //n=10
											{3,3},{3,3}, //n=12
											{3,3},{3,3}, //n=14
											{3,3} }; ////n=15


//COnformed to TE values accuracy
 const double mdDecay_r100[][2] =
 { {0.0000000000,0.0000000000},{0.0000000000,0.0000000000},//n=2
   {0.0000000000,0.0000000000},{1.0986122887,1.0986122887},//n=4
   {0.0000000000,0.0000000000},{0.1675660078,0.1675660078}, //n=6
   {0.3121871366,0.3121871366},{0.4443631040,0.4443631040},//n=8
   {0.5694798840,0.5694798840},{0.6904734513,0.6904734513}, //n=10
   {0.8090210052,0.8090210052},{0.9261117805,0.9261117805},//n=12
   {1.0423410960,1.0423410960},{1.1580711338,1.1580711338},//n=14
   {1.2735229248,1.2735229248} }; //n=15 - Its always reflecting ANYWAY so this wont be used


 const double mdDecayTerminal_r100[][2] ={ {0.0,0.0},{0.0,0.0},//n=2
 												{0.0,0.0},{0.0,0.0},//n=4
 												{1.022,1.022},{1.9347,1.9347}, //n=6
 												{2.7,2.7},{0.8601,0.8601},//n=8
 												{1.11733,1.11733},{1.36333,1.36333}, //n=10
 												{1.60326,1.60326},{1.83968,1.83968},//n=12
 												{2.07402,2.07402},{2.3120,2.3120},//n=14
 												{2.53942,2.53942} }; //Reflecting Boundary Values






//Prototypes
 int drawDecaySteps(int _miRFilterValue,double _dDecayRate,int _iTimeSinceLastInduction,gsl_rng* _rng);
 void set_thresholds(int _iCascadeIndex,int _iStrength,int* _piFLHThres,double& _dDecayRate,int _iTerminalIndex,gsl_rng* _rng);
 int set_injection_filter_state(int& _iFilterState,int _iCascadeIndex,
 									  int _iStrength,int* _piFLHThres,
 									  double& _dDecayRate,int _iTerminalIndex,
 									  gsl_rng* _rng);

 int addSampleToFilter(int& _iFilterState,int _InductionSignal,int& _iCascadeIndex,
 		  int& _iStrength,int* _piFLHThres,
 		  double& _dDecayRate,int _iTerminalIndex,
 		  gsl_rng* _rng);

 inline void switchResetSynapse(int& _iFilterState,int& _iCascadeIndex,
 							  int& _iStrength,int* _piFLHThres,
 							  double& _dDecayRate,int _iTerminalIndex,
 							  gsl_rng* _rng);


int testPerceptronRecallOfStoredPattern(double& _Signal,float* W,int** X,uint _uiSynCount,uint _iStoredPatIndex);

#endif /* PERCEPTRONUNIFIEDWDECAYFILTER_H_ */
