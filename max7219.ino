#include "LedControl.h"

// we initialize a LedControl variable, pin 50 - datapin, 6 - clock, 7 - cs, 4 - how many matrices we have
LedControl lc = LedControl(50, 7, 6, 4);


void setup() {
  Serial.begin(2000000);

  // for each matrix, we have to initialize it, since they start in sleep mode
  // for each matrix, we set the intensity of the leds (value between 0 and 15);
  for (int i = 0; i < lc.getDeviceCount(); i++){
    lc.shutdown(i, false);
    lc.setIntensity(i, 8);
  }

  // we test the led matrix to see if it works, and it works
  lc.setLed(0, 0, 0, true);

  pinMode(A7, INPUT);
}


void loop() { 
  Serial.println(analogRead(A7));
}
