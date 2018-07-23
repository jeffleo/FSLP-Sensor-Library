#pragma once

/* Library header for filtered data collection of FSLP sensors

Constructor
FSLP::FSLP(int fslpSenseLine, int fslpDriveLine2, int fslpDriveLine1, int fslpBotR0, int Presgain, int filterTailTrim_Pres, int filterTailTrim_Pos)

Methods
void FSLP::getData(int *fslpSensor)  puts in pressure then position into given array;
int FSLP::GetPressure()
int FSLP::GetPosition()

By: Jeffrey Leong, Tangio Printed Electronics

last modified: Apr. 9, 2017
*/


#ifndef FSLP_h
#define FSLP_h

class FILTER {
public:
	int filterTailTrimPercentage;  // larger the percentage, the more data trimmed from the final
								   // Optional averaging method
								   //RunningAverage runave;
								   //  int sensSmoothArray1[];   // array for holding raw sensor values for sensor1
								   //  int sensSmoothArray2[];   // array for holding raw sensor values for sensor2

	int* i = new int; //pointer to smootharray index, // make dynamic 
	int SmoothArray; // beginning of array for holding raw sensor values
	int* sensSmoothArray = &SmoothArray; // make pointer to Smootharray

	FILTER(int);
	~FILTER();


	// main filtering method
	int digitalSmooth(int, int *, int, int *);

};


class FSLP {
public:
	int sl; //senseline
	int dl2; //driveline1
	int dl1; ////driveline2 held high when reading
	int bRes;  //biasresistor
	float PressureGain;

	int unfilteredPressure;


	FILTER * filter_pos;
	FILTER * filter_force;



	FSLP(int, int, int, int, int, int, int);//constructor
	~FSLP();// deconstructor

	//methods
	void GetData(int*);
	int  GetPosition();
	int GetPressure();

private:

	// Optional averaging method
	//RunningAverage runave;
	//  int sensSmoothArray1[];   // array for holding raw sensor values for sensor1
	//  int sensSmoothArray2[];   // array for holding raw sensor values for sensor2

	//  int* i= new int; //pointer to smootharray index, // make dynamic 
	//  int SmoothArray; // beginning of array for holding raw sensor values
	//  int* sensSmoothArray=&SmoothArray; // make pointer to Smootharray
	//  int digitalSmooth(int, int[], int, int[]);
	void analogReset();
};





#endif











