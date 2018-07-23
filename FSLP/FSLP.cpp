/* Library functions for filtered data collection of FSLP sensors

FSLP Sensor Connection Legend / Constructor Variables
- Sense line (SL) to Analog Input
- biased resistor between SL and to digital I/O (fslpbotR0)
- Drive line 1 (D1)
- Drive line 2 (D2)
 Connect one DL to Analog Input, other to digital I/O. Currently using dl2 to analog input.
If position is reversed, swap DL pins)

filterTailTrim = // percentage of data trimmed from upper and lower index of the sorted array (outliers) when calculating average;

Default Constructor values
filterTailTrim = 30;

Constructor
FSLP::FSLP(int fslpSenseLine, int fslpDriveLine2, int fslpDriveLine1, int fslpBotR0, int Presgain, int filterTailTrim_Pres, int filterTailTrim_Pos)

Methods
void FSLP::getData(int *fslpSensor)  puts in pressure then position into given array;
int FSLP::GetPressure()
int FSLP::GetPosition()

last modified: Apr. 9, 2017
Author: Jeffrey Leong

References:
Interlink FSLP Integration Guide
*/



#include "Arduino.h"
#include "FSLP.h"
 


/*
Filtering variables
*/

const int filterSamplesize = 15; // must use const to defines array size; filterSamples should  be an odd number, no smaller than 3 (default 13), cannot be changed after compiling
// filterSampleSize is directly proportional to sensor lag 

//  constructor
FSLP::FSLP(int fslpSenseLine, int fslpDriveLine2, int fslpDriveLine1, int fslpBotR0, int Presgain, int filterTailTrim_Pres, int filterTailTrim_Pos)

{
	PressureGain = Presgain;	
	sl = fslpSenseLine;
	dl2 = fslpDriveLine2;
	dl1 = fslpDriveLine1;
	bRes = fslpBotR0;


	/* Instantiate filter objects*/
	filter_pos = new FILTER(filterTailTrim_Pos);
	filter_force = new FILTER(filterTailTrim_Pres);

}

//   deconstructor
FSLP::~FSLP() {
	delete filter_pos;
	delete filter_force;
}


void FSLP::GetData(int *fslpSensor) {
	fslpSensor[0] = GetPressure();
	//There is no detectable pressure, so measuring the position does not make sense.
	if (fslpSensor[0] == 0) fslpSensor[1] = 0;
	else  fslpSensor[1] = GetPosition();  // Raw reading, from 0 to 1023.
}

// The return value of this function is proportional to the physical distance from drive line 2
int FSLP::GetPosition()
{
	// Step 1 - Clear the charge on the sensor.
	pinMode(sl, OUTPUT);
	digitalWrite(sl, LOW);

	pinMode(dl1, OUTPUT);
	digitalWrite(dl1, LOW);

	pinMode(dl2, OUTPUT);
	digitalWrite(dl2, LOW);

	pinMode(bRes, OUTPUT);
	digitalWrite(bRes, LOW);

	// Step 2 - Set up appropriate drive line voltages.
	digitalWrite(dl1, HIGH);
	pinMode(bRes, INPUT);
	pinMode(sl, INPUT);

	// Step 3 - Wait for the voltage to stabilize.
	delayMicroseconds(10);
	analogReset();

	// Step 4 - Take the measurement.
	int temp = analogRead(sl);
	temp = filter_pos->digitalSmooth(constrain((float)temp, 0.0, 1023.0), filter_pos->sensSmoothArray, filter_pos->filterTailTrimPercentage, filter_pos->i);	// filter raw data before writing to adcvoltinvert
	//return constrain(temp, 0, 1023);

	return constrain(temp, 0, 1023);
	// filter raw data before writing to returning
}


int FSLP::GetPressure() {

	// Step 1 - Set up the appropriate drive line voltages.
	pinMode(dl1, OUTPUT);
	digitalWrite(dl1, HIGH);

	pinMode(bRes, OUTPUT);
	digitalWrite(bRes, LOW);

	pinMode(sl, INPUT);
	pinMode(dl2, INPUT);

	// Step 2 - Wait for the voltage to stabilize.
	delayMicroseconds(10);

	// Step 3 - Take two measurements.
	analogReset();
	int v1 = analogRead(dl2);
	analogReset();
	delayMicroseconds(10);
	int v2 = analogRead(sl);

	// Inverts analog reading to calculate sensor conductance in order to produce a more linear response.
	int adcvoltinvert;
	// Avoids dividing by zero by giveing last reading instead
	if (v1 == v2)
	{
		adcvoltinvert = filter_force->sensSmoothArray[filterSamplesize - 1];
		unfilteredPressure = 1024;
	}
	else {
	
		unfilteredPressure = v2 / (float)(v1 - v2)*PressureGain;
		adcvoltinvert = filter_force->digitalSmooth(constrain((float)v2 / (float)(v1 - v2)*PressureGain, 0.0, 1023.0), filter_force->sensSmoothArray, filter_force->filterTailTrimPercentage, filter_force->i);	// filter raw data before writing to adcvoltinvert
	}
	
	//  Debugging
	//Serial.print(v2);
	//Serial.print("\t");
	//Serial.print(v1);
	//Serial.print("\t");
	//Serial.println(adcvoltinvert);


	return  adcvoltinvert;
}

// Performs an ADC reading on the internal GND channel in order
// to clear any voltage that might be leftover on the ADC.
// Only works on AVR boards and silently fails on others.
void FSLP::analogReset()
{
#if defined(ADMUX)
#if defined(ADCSRB) && defined(MUX5)
	// Code for the ATmega2560 and ATmega32U4
	ADCSRB |= (1 << MUX5);
#endif
	ADMUX = 0x1F;

	// Start the conversion and wait for it to finish.
	ADCSRA |= (1 << ADSC);
	loop_until_bit_is_clear(ADCSRA, ADSC);
#endif
}


//constructor
FILTER::FILTER(int filtertrimpercent)
{
	filterTailTrimPercentage = filtertrimpercent;

	*i = 0; // set index value at pointer to 0
	SmoothArray = 0; // set int to 0
	sensSmoothArray = new int[filterSamplesize]; // make dynamic array of size filterSamplesize with pointer sensSmoothArray
}

FILTER::~FILTER()
{//deconstructor
 //clear dynamic memory dangling pointers
	delete[] sensSmoothArray;
	delete  i;
}


//Digital FIR low-pass filter and averaging function used for removing sensor jitter and outlier values; smoothes the raw data.
//The amount of filtering is dependant on the size of the input data array sensSmoothArray.
/*	INPUTS:
int rawIn				// raw data
int *sensSmoothArray"	// pointer to head of array holding raw sensor data
int outlierpercentage	// percentage of data trimmed from upper and lower index of the sorted array when calculating average; (default 15%)
int *i					// index pointer for sensSmoothArray

OUTPUT:
Filtered raw data value
*/
int FILTER::digitalSmooth(int rawIn, int *sensSmoothArray, int outlierpercentage, int *i) {
	int j, k, temp, top, bottom;
	long total;
	static int sorted[filterSamplesize];
	boolean done;

	*i = (*i + 1) % filterSamplesize;    // increment counter and roll over if necc. -  % (modulo operator) rolls over variable

	sensSmoothArray[*i] = rawIn;                 // input new data into the oldest slot

	for (j = 0; j<filterSamplesize; j++) {     // transfer data array into anther array for sorting and averaging
		sorted[j] = sensSmoothArray[j];
	}


	done = 0;                // flag to know when we're done sorting              
	while (done != 1) {        // simple swap sort, sorts numbers from lowest to highest
		done = 1;
		for (j = 0; j < (filterSamplesize - 1); j++) {
			if (sorted[j] > sorted[j + 1]) {     // numbers are out of order - swap
				temp = sorted[j + 1];
				sorted[j + 1] = sorted[j];
				sorted[j] = temp;
				done = 0;
			}
		}
	}

	// throw out top and bottom outlierpercentage (default 15%) of samples - limit to throw out at least one from top and bottom
	bottom = max(((round(filterSamplesize * outlierpercentage) / 100)), 1);
	top = min((((round(filterSamplesize * (100 - outlierpercentage)) / 100))), (filterSamplesize - 1));   // + 1 is to make up for asymmetry caused by integer rounding
	k = 0;
	total = 0;
	for (j = bottom; j< top; j++) {
		total += sorted[j];  // total remaining indices
		k++;
	}

	return total / k;    // divide by number of samples for average
}


