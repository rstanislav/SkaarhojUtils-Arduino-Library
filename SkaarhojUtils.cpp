/*  SkaarhojUtils Arduino library with various utilities for products from SKAARHOJ.com
    Copyright (C) 2012 Kasper Skårhøj    <kasperskaarhoj@gmail.com> 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "SkaarhojUtils.h"


SkaarhojUtils::SkaarhojUtils(){}	// Empty constructor.



void SkaarhojUtils::uniDirectionalSlider_init()	{

		// Configuration constants, should have setter-methods:
	_uniDirectionalSlider_sliderTolerance = 10;  // >0. How much it should be moved before it counts as a change.
    _uniDirectionalSlider_sliderLowEndOffset = 20;  // >0. How far the slider is moved in the low end before we start registering the value range: The starting position.
    _uniDirectionalSlider_sliderHighEndOffset = 20;  // >0. How far the slider is moved in the high end before we start registering the value range: The ending position.

		// Internal variables during operation:
	_uniDirectionalSlider_previousSliderValue=-1;
	_uniDirectionalSlider_previousTransitionPosition=-1;
	_uniDirectionalSlider_sliderDirectionUp = true;
}

bool SkaarhojUtils::uniDirectionalSlider_hasMoved()	{
	int sliderValue = analogRead(0);

	if (sliderValue > _uniDirectionalSlider_previousSliderValue+_uniDirectionalSlider_sliderTolerance || sliderValue < _uniDirectionalSlider_previousSliderValue-_uniDirectionalSlider_sliderTolerance)  {

		// Find direction:
		if (sliderValue > _uniDirectionalSlider_previousSliderValue+_uniDirectionalSlider_sliderTolerance && (_uniDirectionalSlider_previousSliderValue==-1 || _uniDirectionalSlider_previousSliderValue<_uniDirectionalSlider_sliderLowEndOffset))  {
			_uniDirectionalSlider_sliderDirectionUp = true;
		}
		if (sliderValue < _uniDirectionalSlider_previousSliderValue-_uniDirectionalSlider_sliderTolerance && (_uniDirectionalSlider_previousSliderValue==-1 || _uniDirectionalSlider_previousSliderValue>1024-_uniDirectionalSlider_sliderHighEndOffset))  {
			_uniDirectionalSlider_sliderDirectionUp = false;
		}

		_uniDirectionalSlider_previousSliderValue = sliderValue;

		int transitionPosition = (long)1000*(long)(sliderValue-_uniDirectionalSlider_sliderLowEndOffset)/(long)(1024-_uniDirectionalSlider_sliderLowEndOffset-_uniDirectionalSlider_sliderHighEndOffset);
		if (transitionPosition>1000) transitionPosition=1000;
		if (transitionPosition<0) transitionPosition=0;
		if (!_uniDirectionalSlider_sliderDirectionUp)  transitionPosition = 1000-transitionPosition;
		if (_uniDirectionalSlider_previousTransitionPosition!=transitionPosition)  {
			bool returnValue = true;
			if ((_uniDirectionalSlider_previousTransitionPosition==0 || _uniDirectionalSlider_previousTransitionPosition==1000) && 
				(transitionPosition==0 || transitionPosition==1000)) {
					returnValue = false;
				}
			_uniDirectionalSlider_previousTransitionPosition=transitionPosition;
			return returnValue;
		}
	}
	return false;
}

int SkaarhojUtils::uniDirectionalSlider_position()	{
	return _uniDirectionalSlider_previousTransitionPosition;
}

bool SkaarhojUtils::uniDirectionalSlider_isAtEnd()	{
	return (_uniDirectionalSlider_previousTransitionPosition==1000 || _uniDirectionalSlider_previousTransitionPosition==0);
}






void SkaarhojUtils::encoders_init() {
	_encoders_countOn[0] = false;
	_encoders_countOn[1] = false;
	
	_encoders_pushOn[0] = false;
	_encoders_pushOn[1] = false;

	_encoders_pushOnTriggerTimeFired[0] = false;
	_encoders_pushOnTriggerTimeFired[1] = false;
	
	pinMode(3, INPUT);
	pinMode(5, INPUT);
	pinMode(6, INPUT);
	pinMode(7, INPUT);
	pinMode(8, INPUT);
	pinMode(9, INPUT);
}

int SkaarhojUtils::encoders_state(uint8_t encNum) {
	return encoders_state(encNum, 0);
}
int SkaarhojUtils::encoders_state(uint8_t encNum, unsigned int buttonPushTriggerDelay) {
	uint8_t trigger_pin;
	uint8_t direction_pin;
	uint8_t push_pin;

		// Check:
	if (encNum <2)	{
		if (encNum==1)	{
			trigger_pin = 7;
			direction_pin = 8;
			push_pin = 9;
		} else {	// encNum == 0
			trigger_pin = 3;
			direction_pin = 5;
			push_pin = 6;
		}

			// De-bouncing:
		bool isPushed = digitalRead(push_pin);
		bool isTriggered = digitalRead(trigger_pin);
		delay(1);
		isPushed = isPushed & digitalRead(push_pin);
		isTriggered = isTriggered & digitalRead(trigger_pin);

			// Rotation:
			// Notice: We detect the phase shift both on trigger and de-trigger - and return a value 1/-1 ONLY if they agree. 
			// This is a poor-mans way to raise the chance that a correct answer is given on the rotation now that we don't use interrupts
			// The general problem is this: When we look for a change on the trigger pin we might detect that somewhere in the last part of the period of the signal - and here the direction signal will be reverted again and we detect a rotation in the oppositve direction
			// In other words: The principle of detecting the rotation is based on the assumption that we detect the trigger and direction signals immediately as the trigger happens - which is only the case if we use interrupts and certainly a problem that becomes larger as we have slower and slower runloops.
		if (isTriggered)  {
			if (!_encoders_countOn[encNum]) {
				_encoders_countOn[encNum] = true; 
				if (digitalRead(direction_pin))  {
					_encoders_triggerCache[encNum]=1;
				} else {
					_encoders_triggerCache[encNum]=-1;
				}
			}
		} else {
			if (_encoders_countOn[encNum]) {
				_encoders_countOn[encNum] = false; 
				if (digitalRead(direction_pin))  {
					if (_encoders_triggerCache[encNum]==-1) return -1; 	// Must agree.
				} else {
					if (_encoders_triggerCache[encNum]==1) return 1; 	// Must agree.
				}
			}
		}

			// Push:
		if (isPushed)  {
			if (!_encoders_pushOn[encNum]) {
				_encoders_pushOn[encNum] = true;
				_encoders_pushOnTriggerTimeFired[encNum] = false;
				_encoders_pushOnMillis[encNum] = millis();
				
				return 2;
			}
			if (!_encoders_pushOnTriggerTimeFired[encNum] && buttonPushTriggerDelay>0 && (unsigned long)millis()-_encoders_pushOnMillis[encNum] > buttonPushTriggerDelay)	{
				_encoders_pushOnTriggerTimeFired[encNum] = true;
				return (unsigned long) millis()-_encoders_pushOnMillis[encNum];
			}
		} else {
			if (_encoders_pushOn[encNum])	{
				_encoders_pushOn[encNum] = false;
				if (!_encoders_pushOnTriggerTimeFired[encNum])	{
					return (unsigned long) millis()-_encoders_pushOnMillis[encNum];
				}
			}
		}
	}
		// Default return
	return 0;
}











void SkaarhojUtils::touch_init() {
		// Threshold for reading value that indicates a touch:
	_touch_Xthreshold = 1000;
	_touch_Ythreshold = 1000;
	
	_touch_touchTimeThreshold = 100;	// Lower limit (ms) for a touch being processed at all
	_touch_touchTimeTapAndHold = 800;	// After this time in ms it will return a hold event.
	_touch_touchPerimiterThreshold = 50;	// If the touch is released and dX and dY is outside this distance, the touch is cancelled
	_touch_gestureLength = 100;		// Length of one side in the gesture
	_touch_gestureRatio = 2;		// Ratio between end dX and end dY that will trigger a swipe gesture

	_touch_exitByTapAndHold = false;
	
	_touch_touchStartedTime3=0;
	_touch_touchStartedTime2=0;
	
	// Calibration: use touch_calibrationPointRawCoordinates !
	_touch_marginLeft = (71+72)/2;	
	_touch_marginRight = (976+974)/2;
	_touch_marginTop = (886+889)/2;
	_touch_marginBottom = (140+152)/2;
	_touch_snapToBorderZone = 50;
	_touch_scaleRangeX = 1280;
	_touch_scaleRangeY = 720;
}
void SkaarhojUtils::touch_calibrationPointRawCoordinates(int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, int p4x, int p4y) 	{
	_touch_marginLeft = float ((p1x-(p2x-p1x)/2)+(p4x-(p3x-p4x)/2))/2;
	_touch_marginRight = float ((p2x+(p2x-p1x)/2)+(p3x+(p3x-p4x)/2))/2;
	_touch_marginTop = float ((p1y+(p1y-p4y)/2)+(p2y+(p2y-p3y)/2))/2;
	_touch_marginBottom = float ((p4y-(p1y-p4y)/2)+(p3y-(p2y-p3y)/2))/2;	
}
bool SkaarhojUtils::touch_isTouched() {
	// Based on GNU/GPL code from 2009 Jonathan Oxer <jon@oxer.com.au>
	
	// Reads the touch values to internal memory and returns true if touch is detected

	// Set up the analog pins in preparation for reading the X value
	// from the touchscreen.
	pinMode(A1, INPUT_PULLUP);     // Analog pin 1
	pinMode(A3, INPUT_PULLUP);     // Analog pin 3
	pinMode(A0, OUTPUT);    // Analog pin 0
	pinMode(A2, OUTPUT);    // Analog pin 2
	digitalWrite(A0, LOW);  // Use analog pin 0 as a GND connection
	digitalWrite(A2, HIGH); // Use analog pin 2 as a +5V connection
	delay(1);
	_touch_rawXVal = analogRead(A1);   // Read the X value

	// Set up the analog pins in preparation for reading the Y value
	// from the touchscreen
	pinMode(A0, INPUT_PULLUP);     // Analog pin 0
	pinMode(A2, INPUT_PULLUP);     // Analog pin 2
	pinMode(A1, OUTPUT);    // Analog pin 1
	pinMode(A3, OUTPUT);    // Analog pin 3
	digitalWrite(A1, LOW);  // Use analog pin 1 as a GND connection
	digitalWrite(A3, HIGH); // Use analog pin 3 as a +5V connection
	delay(1);
	_touch_rawYVal = analogRead(A0);   // Read the Y value

	// Return true if touched:
	return (_touch_rawYVal < _touch_Ythreshold && _touch_rawXVal < _touch_Xthreshold);
}
bool SkaarhojUtils::touch_isInProgress()	{
	return _touch_isTouchedState && _touch_touchEndedRawXVal>0;
}
uint8_t SkaarhojUtils::touch_state() {
	
	bool isTouchedNow = touch_isTouched();
	if (_touch_exitByTapAndHold)	{
		//Serial.print("_touch_exitByTapAndHold is set: ");
		if (isTouchedNow)	{
			isTouchedNow = false;
			//Serial.println("keeps isTouchedNow low");
		} else {
			_touch_exitByTapAndHold = false;
			//Serial.println("resets _touch_exitByu....");
		}
	}
		
		// If screen is touched currently:
	if (isTouchedNow)	{
			// New touch registered:
		if (!_touch_isTouchedState)	{
			_touch_isTouchedState = true;
			_touch_checkingForTapAndHold = false;

			// Init variables:
			_touch_touchStartedRawXVal = 0;
			_touch_touchStartedRawYVal = 0;
			_touch_touchCoordRoundRobinIdx = 0;
			_touch_touchStartedTime = millis();	// long unsigned
			
			_touch_touchRawXValMax = 0;
			_touch_touchRawXValMin = 1024;
			_touch_touchRawYValMax = 0;
			_touch_touchRawYValMin = 1024;
			
			_touch_touchEndedRawXVal = 0;
			_touch_touchEndedRawYVal = 0;
		}
		
			// Round robin accumulation of the coordinates:
		_touch_touchRawXValRoundRobin[_touch_touchCoordRoundRobinIdx % 8] = _touch_rawXVal;
		_touch_touchRawYValRoundRobin[_touch_touchCoordRoundRobinIdx % 8] = _touch_rawYVal;
		_touch_touchCoordRoundRobinIdx++;
		
			// Set the start touch coordinate:
			// First, there must be 8 recent values in the round-robin base, second the start value must not already be set and thirdly, we  register it only AFTER the touchTimeThreshold has been reached. This is all about "stabilizing" the touch value since it fluctuates most when people put their finger to the screen first time.
		if (_touch_touchCoordRoundRobinIdx>=8 && _touch_touchStartedRawXVal==0 && (unsigned long)millis()-_touch_touchStartedTime>_touch_touchTimeThreshold)	{
			for(uint8_t i=0;i<8;i++)	{
				_touch_touchStartedRawXVal+=_touch_touchRawXValRoundRobin[i];
				_touch_touchStartedRawYVal+=_touch_touchRawYValRoundRobin[i];
			}
			_touch_touchStartedRawXVal/=8;
			_touch_touchStartedRawYVal/=8;
		}
		
			// Min / max:
		if (_touch_touchStartedRawXVal>0 && _touch_touchCoordRoundRobinIdx>=12)	{
			_touch_endedValueCalculation();
			
			if (_touch_touchEndedRawXVal > 	_touch_touchRawXValMax)	_touch_touchRawXValMax = _touch_touchEndedRawXVal;
			if (_touch_touchEndedRawXVal < 	_touch_touchRawXValMin)	_touch_touchRawXValMin = _touch_touchEndedRawXVal;
			if (_touch_touchEndedRawYVal > 	_touch_touchRawYValMax)	_touch_touchRawYValMax = _touch_touchEndedRawYVal;
			if (_touch_touchEndedRawYVal < 	_touch_touchRawYValMin)	_touch_touchRawYValMin = _touch_touchEndedRawYVal;

		}		

			// Checking if a single touch is the case and if so exits the active touch in case the tapAndHold threshold has been exceeded.
		if (!_touch_checkingForTapAndHold && (unsigned long)millis()-_touch_touchStartedTime > _touch_touchTimeTapAndHold)	{
			//Serial.println("Check for tapAndHold...");
			_touch_checkingForTapAndHold = true;
			_touch_endedValueCalculation();
			if (touch_type()==1)	{
				//Serial.println("Exceeded time, exit");
				_touch_exitByTapAndHold = true;
			} else {
				//Serial.println("Nope...!");
			}
		}
	} else {	// Touch ended:
		if (_touch_isTouchedState)	{
			_touch_isTouchedState = false;
			unsigned long touchTime = (unsigned long)millis()-_touch_touchStartedTime;
			
			if (touchTime > _touch_touchTimeThreshold && _touch_touchStartedRawXVal>0)	{
				_touch_endedValueCalculation();
				
				uint8_t touchType = touch_type();


/*
								Serial.print("Start: ");
								Serial.print(_touch_touchStartedRawXVal);
								Serial.print(",");
								Serial.println(_touch_touchStartedRawYVal);
								Serial.print("End: ");
								Serial.print(_touch_touchEndedRawXVal);
								Serial.print(",");
								Serial.println(_touch_touchEndedRawYVal);

								unsigned int dX = abs(_touch_touchStartedRawXVal-_touch_touchEndedRawXVal);
								unsigned int dY = abs(_touch_touchStartedRawYVal-_touch_touchEndedRawYVal);

								Serial.print("dX,dY: ");
								Serial.print(dX);
								Serial.print(",");
								Serial.println(dY);

								Serial.print("dX/dY, dY/dX: ");
								Serial.print((float)dX/dY);
								Serial.print(",");
								Serial.println((float)dY/dX);

								Serial.print("xMax,yMax: ");
								Serial.print(_touch_touchRawXValMax);
								Serial.print(",");
								Serial.println(_touch_touchRawYValMax);

								Serial.print("xMin,yMin: ");
								Serial.print(_touch_touchRawXValMin);
								Serial.print(",");
								Serial.println(_touch_touchRawYValMin);

								Serial.print("Bounding Box: ");
								Serial.print(_touch_touchRawXValMax-_touch_touchRawXValMin);
								Serial.print(",");
								Serial.println(_touch_touchRawYValMax-_touch_touchRawYValMin);

								Serial.print("Touch Type:");
								Serial.println(touch_type());

*/
				
					// Single:
				if (touchType==1)	{
					uint8_t retVal =0;
					
					if ((unsigned long)_touch_touchStartedTime-_touch_touchStartedTime2<500) {
						if ((unsigned long)_touch_touchStartedTime2-_touch_touchStartedTime3<500)	{
							_touch_touchStartedTime3=0;
							_touch_touchStartedTime2=0;
							_touch_touchStartedTime=0;
							retVal= 3;	// Triple
						} else {
							retVal= 2;	// Double
						}
					} else {
						if (touchTime > _touch_touchTimeTapAndHold)	{
							retVal= 9;	// Single + hold
						} else {
							retVal= 1;	// Single
						}
					}

					_touch_touchStartedTime3=_touch_touchStartedTime2;
					_touch_touchStartedTime2=_touch_touchStartedTime;
					
					return retVal;
				}
				
					// Gesture:
				if (touchType>=10 && touchType<=13)	{
					return touchType;
				}
				
				return 255;	// Something else touched...
			}
		}
	}
	
	return 0;
}
void SkaarhojUtils::_touch_endedValueCalculation()	{
	// This takes the oldest four values in the round-robin. This is in order to avoid the most recent values which in the case of a release of a finger will be contaminated by fluctuations
	_touch_touchEndedRawXVal = 0;
	_touch_touchEndedRawYVal = 0;

	for(uint8_t i=0;i<4;i++)	{
		uint8_t idx = ((_touch_touchCoordRoundRobinIdx % 8)+1+i)%8;
		_touch_touchEndedRawXVal+=_touch_touchRawXValRoundRobin[idx];
		_touch_touchEndedRawYVal+=_touch_touchRawYValRoundRobin[idx];
	}

	_touch_touchEndedRawXVal/=4;
	_touch_touchEndedRawYVal/=4;		
}
uint8_t SkaarhojUtils::touch_type()	{
	unsigned int dX = abs(_touch_touchStartedRawXVal-_touch_touchEndedRawXVal);
	unsigned int dY = abs(_touch_touchStartedRawYVal-_touch_touchEndedRawYVal);
	
		// Normal point touch:
	if (dX<_touch_touchPerimiterThreshold && dY<_touch_touchPerimiterThreshold)	{
		return 1;
	}
	
	if (dX>_touch_gestureLength || dY>_touch_gestureLength)	{
			// Horizonal gesture:
		if (dY==0 || (float) dX/dY>_touch_gestureRatio)	{
			return _touch_touchStartedRawXVal < _touch_touchEndedRawXVal ? 10 : 11;	// 10 = Left->Right; 11 = Right->Left
		}
			// Vertical gesture:
		if (dX==0 || (float) dY/dX>_touch_gestureRatio)	{
			return _touch_touchStartedRawYVal < _touch_touchEndedRawYVal ? 13 : 12;	// 12 = Up->Down; 13 = Down->Up
		}
	}
	return 0;	// Invalid touch type...
}
int SkaarhojUtils::touch_coordX(int rawCoordX)	{
	int coordX = rawCoordX-_touch_marginLeft;
	if (coordX < _touch_snapToBorderZone)	coordX=0;
	if (coordX > (_touch_marginRight-_touch_marginLeft)-_touch_snapToBorderZone)	coordX=(_touch_marginRight-_touch_marginLeft);
	coordX = (unsigned long) coordX  * _touch_scaleRangeX / (_touch_marginRight-_touch_marginLeft);
	return coordX;
}
int SkaarhojUtils::touch_coordY(int rawCoordY)	{
	int coordY = rawCoordY-_touch_marginBottom;
	if (coordY < _touch_snapToBorderZone)	coordY=0;
	if (coordY > (_touch_marginTop-_touch_marginBottom)-_touch_snapToBorderZone)	coordY=(_touch_marginTop-_touch_marginBottom);
	coordY = (unsigned long) coordY  * _touch_scaleRangeY / (_touch_marginTop-_touch_marginBottom);
	return coordY;
}

int SkaarhojUtils::touch_getRawXVal() {
	return _touch_rawXVal;
}
int SkaarhojUtils::touch_getRawYVal() {
	return _touch_rawYVal;
}

int SkaarhojUtils::touch_getEndedXCoord() {
	return touch_coordX(_touch_touchEndedRawXVal);
}
int SkaarhojUtils::touch_getEndedYCoord() {
	return touch_coordY(_touch_touchEndedRawYVal);
}

int SkaarhojUtils::touch_getRawXValMax() {
	return _touch_touchRawXValMax;
}
int SkaarhojUtils::touch_getRawXValMin() {
	return _touch_touchRawXValMin;
}
int SkaarhojUtils::touch_getRawYValMax() {
	return _touch_touchRawYValMax;
}
int SkaarhojUtils::touch_getRawYValMin() {
	return _touch_touchRawYValMin;
}
