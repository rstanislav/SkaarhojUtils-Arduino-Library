/*****************
 * Calibration of touch screen.
 * Follow instructions printed to the Serial monitor. Notice t
 * - kasper
 *
 * This example code is in the public domain.
 */


#include "SkaarhojUtils.h"
SkaarhojUtils utils;



// Collects 16 samples, buffer:
int roundRobinX[16];
int roundRobinY[16];

// Stores the detected coordinates:
int calibrationCoordinatesX[5];
int calibrationCoordinatesY[5];


void setup() {
  Serial.begin(9600);
  Serial.println("Welcome to the SKAARHOJ utils-library touchscreen calibration process!");
  Serial.println("Please make sure your screen shows the SKAARHOJ monitor calibration chart edge to edge.");
  Serial.println("This chart is found in the same folder on your hard drive along with this example file (Monitor Calibration Chart.png).");
  Serial.println("Then, follow instructions below:");
  Serial.println("");

  utils.touch_init();


  for (int i=1; i<=4; i++)  {
    // Read point i:
    do {
      Serial.print("Press and hold crosshair point ");
      Serial.print(i);
      Serial.println(" until a confirmation appears.");
      while (!utils.touch_isTouched())  {
      }
    } 
    while (!readPoint(i));
    Serial.println("");
  }
  
  utils.touch_calibrationPointRawCoordinates(
    calibrationCoordinatesX[1], calibrationCoordinatesY[1], 
    calibrationCoordinatesX[2], calibrationCoordinatesY[2], 
    calibrationCoordinatesX[3], calibrationCoordinatesY[3], 
    calibrationCoordinatesX[4], calibrationCoordinatesY[4]);

    Serial.println("Screen is calibrated!");
    Serial.println("In your arduino sketch you need to insert this line (copy/paste it) right after 'utils.touch_init();':");
    Serial.println("");

    Serial.print("       utils.touch_calibrationPointRawCoordinates(");
    Serial.print(calibrationCoordinatesX[1]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesY[1]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesX[2]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesY[2]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesX[3]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesY[3]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesX[4]);    
    Serial.print(",");    
    Serial.print(calibrationCoordinatesY[4]);    
    Serial.println(");");
    Serial.println("");
    Serial.println("Before you do so, please touch the screen and verify the coordinates matches the calibration picture.");
}


void loop() {
  uint8_t touchState = utils.touch_state();
  switch(touchState)  {
    case 1:
      Serial.print("Single touch in: x=");
      Serial.print(utils.touch_getEndedXCoord());
      Serial.print("; y=");
      Serial.println(utils.touch_getEndedYCoord());
    break; 
    case 2:
      Serial.print("Double touch in: x=");
      Serial.print(utils.touch_getEndedXCoord());
      Serial.print("; y=");
      Serial.println(utils.touch_getEndedYCoord());
    break; 
    case 3:
      Serial.print("Triple touch in: x=");
      Serial.print(utils.touch_getEndedXCoord());
      Serial.print("; y=");
      Serial.println(utils.touch_getEndedYCoord());
    break; 
    case 9:
      Serial.print("Single touch - held down for a while: x=");
      Serial.print(utils.touch_getEndedXCoord());
      Serial.print("; y=");
      Serial.println(utils.touch_getEndedYCoord());
    break; 
    case 10:
      Serial.println("Swipe gesture Left -> Right");
    break; 
    case 11:
      Serial.println("Swipe gesture Right -> Left");
    break; 
    case 12:
      Serial.println("Swipe gesture Top -> Bottom");
    break; 
    case 13:
      Serial.println("Swipe gesture Bottom -> Top");
    break; 
    case 255:
      Serial.println("Some other type of touch. Bounding Box?");
          int xMax = utils.touch_coordX(utils.touch_getRawXValMax());
	  int xMin = utils.touch_coordX(utils.touch_getRawXValMin());
	  int yMax = utils.touch_coordY(utils.touch_getRawYValMax());
	  int yMin = utils.touch_coordY(utils.touch_getRawYValMin());

      Serial.print("From: x=");
      Serial.print(xMin);
      Serial.print("; y=");
      Serial.println(yMin);

      Serial.print("To: x=");
      Serial.print(xMax);
      Serial.print("; y=");
      Serial.println(yMax);
    break; 
  }
  delay(10);
}

// Reading the raw coordinates of a reference point:
bool readPoint(uint8_t pointNumber)  {
  Serial.println("Reading in progress...");

  // PointNumber between 1-4:
  int i=0;
  calibrationCoordinatesX[pointNumber] = 0;
  calibrationCoordinatesY[pointNumber] = 0;

  delay(1000);  // Wait for press to stabilize

  // Making 16 samples:
  while(utils.touch_isTouched() && i<16)  {
    roundRobinX[i] = utils.touch_getRawXVal();
    roundRobinY[i] = utils.touch_getRawYVal();
    i++;
    delay(50);
  }

  // Seeing if the pressure was released prematurely:
  if (i!=16)  {
    Serial.println("Error: You released your pressure too early, try again");
    return false; 
  } 
  else {
    // Ok reading, make calculations and return:
    Serial.print("Point reading OK: ");
    for(i=0; i<16;i++)  {
      calibrationCoordinatesX[pointNumber]+= roundRobinX[i];
      calibrationCoordinatesY[pointNumber]+= roundRobinY[i];
    }
    calibrationCoordinatesX[pointNumber]=calibrationCoordinatesX[pointNumber]>>4;
    calibrationCoordinatesY[pointNumber]=calibrationCoordinatesY[pointNumber]>>4;

    Serial.print("x=");
    Serial.print(calibrationCoordinatesX[pointNumber]);
    Serial.print("; y=");
    Serial.print(calibrationCoordinatesY[pointNumber]);

    // Waiting for touch to stop before we return:
    while (utils.touch_isTouched())  {
    }

    return true; 
  }
}


