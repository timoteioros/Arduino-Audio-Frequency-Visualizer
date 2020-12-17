#include <arduinoFFT.h>
#include <LedControl.h>


#define SAMPLES 64
#define SAMPLING_FREQUENCY 40000
#define ROWS 8
#define COLUMNS 32 

// we initialize a LedControl variable, pin 50 - datapin, 6 - clock, 7 - cs, 4 - how many matrices we have
LedControl lc = LedControl(50, 7, 6, 4);

double vReal[SAMPLES];
double vImg[SAMPLES];
arduinoFFT FFT = arduinoFFT(vReal, vImg, SAMPLES, SAMPLING_FREQUENCY);

unsigned long sampling_time;

void setup() {
  Serial.begin(2000000);

  // for each matrix, we have to initialize it, since they start in sleep mode
  // for each matrix, we set the intensity of the leds (value between 0 and 15);
  for (int i = 0; i < lc.getDeviceCount(); i++){
    lc.shutdown(i, false);
    lc.setIntensity(i, 1);
  }
  lc.setColumn(0, 0, B11111111);
  pinMode(A7, INPUT);
}

unsigned long newTime;
byte bandLevel;
double value;
byte x;

byte valueVector[8];
byte mask = 128;

float conversionTime;

void loop() { 
  newTime = micros();

  // Sampling the input data
  for (int i = 0; i < SAMPLES; i++){
    vReal[i] = analogRead(A7);
    Serial.println(vReal[i]);
    vImg[i] = 0.0;
  }

  // Fast Furier on that data
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();


  // printing on the matrix
  for (int k = 0; k < lc.getDeviceCount(); k++){

    for(int j = 0; j < 8; j++){
      valueVector[j] = 0;
    }

    // Taking the FFT and converting it to led bands
    for(int j = 0; j < 8; j++){
      value = vReal[k * 8 + j];
      if (value > 80) 
        value = 80;
      value /= 8.0;
      valueVector[j] = setBandLeds( (int) value);  
    } 

    //Making the data into row data
    mask = 128;
    for (int j = 0; j < 8; j++){
      x = 0;
      for(int p = 0; p < 8; p++){
        x |= !!(valueVector[p] & mask) << (7 - p);
      }
      mask >>= 1;
      lc.setRow(k, j, x);
    }
  } 
  
  Serial.println(micros() - newTime);
  
}

//converting a value between 1 and 8 into led data
byte setBandLeds(int value) {

  if (value == 0)
    return 0;
  if (value == 1)
    return 1;
  if (value == 2)
    return 3;
  if (value == 3)
    return 7;
  if (value == 4) 
    return 15;
  if (value == 5)
    return 31;
  if (value == 6)
    return 63;
  if (value == 7)
    return 127;
  if (value == 8) 
    return 255;
}
