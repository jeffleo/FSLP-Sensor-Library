#include <FSLP.h>

// Object initialization
//FSLP::FSLP(int fslpSenseLine, int fslpDriveLine2, int fslpDriveLine1, int fslpBotR0, int Presgain, int filterTailTrim_Pres, int filterTailTrim_Pos)
FSLP fslp1(A3,A2,0,1, 40, 40, 14);  // Right sensor
int fslpSensor1[2]; // Analog reading storage array for fslp1

FSLP fslp2(A1,A0,4,8, 40, 40, 14);  // Right left sensor
int fslpSensor2[2]; // Analog reading storage array for fslp2


/****************************************************************/
void loop (void)
{
  

//---------------------Reads position and filtered pressure signals and stores data in given array if there is a pressure reading
  fslp1.GetData(fslpSensor1);
  fslp2.GetData(fslpSensor2);
  
// Use serial plotter
  Serial.print(fslpSensor2[0]); //position
  Serial.print(",");    
  Serial.print(fslp2.unfilteredPressure); //unfiltered pressure
  Serial.print(",");  
  Serial.println(fslpSensor2[1]);  //filterd pressure
  
}





