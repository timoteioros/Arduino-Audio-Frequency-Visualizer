#include <arduinoFFT.h>
#include <LedControl.h>


#define SAMPLES 64
#define SAMPLING_FREQUENCY 40000

// Initializam variabila LedControl, cu care vom comunica cu matricea 
// pin 50 - datapin, pin 7 - clock, pin 6 - cs, pin 4 - cate matrici de 8x8 avem cascadate
LedControl lc = LedControl(50, 7, 6, 4);

// Initializam variabila FFT, cu care vom face transformata fourier
// Este nevoie de 2 vectori, unul de indici reali, altul de indici imaginari
// Mai este nevoie sa precizam si cate sample-uri vom face pe loop si la ce frecventa dorim (40.000 / 2 = 20.000 kHz)
double vReal[SAMPLES];
double vImg[SAMPLES];
arduinoFFT FFT = arduinoFFT(vReal, vImg, SAMPLES, SAMPLING_FREQUENCY);

unsigned long sampling_time;

void setup() {
  Serial.begin(2000000);

  // Trebuie sa initalizam matricile, pentru ca acestea incep in modul sleep
  // Trebuie setata intensitatea led-urilor
  for (int i = 0; i < lc.getDeviceCount(); i++){
    lc.shutdown(i, false);
    lc.setIntensity(i, 1);
  }
  
  // De la pin-ul A7 vom primi input-ul in real-time
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

  // Facem sample pe datele de ce le primim (64 sample-uri)
  // Vectorul de coeficienti imaginari ramane pe 0
  for (int i = 0; i < SAMPLES; i++){
    vReal[i] = smooth();
    vImg[i] = 0.0;
  }

  // Facem Fast Fourier Transform pe vectorul de reali si imaginari
  FFT.DCRemoval();
  FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude();


  // Pentru fiecare matrice de 8x8, calculam care led-uri sa fie aprinse si care nu
  for (int k = 0; k < lc.getDeviceCount(); k++){

    for(int j = 0; j < 8; j++){
      valueVector[j] = 0;
    }

    // Luam datele prelucrate cu FFT din vReal, si le transformam in byte-uri care reprezinta
    // care led-uri vor fi pornite si care stinse pe fiecare coloana
    for(int j = 0; j < 8; j++){
      value = vReal[k * 8 + j];
      
      if (value > 80) 
        value = 80;
      value /= 8.0;
      
      valueVector[j] = setBandLeds( (int) value);  
    } 

    // Din date coloana, le transformam in date rand, deoarece setColumn este o functie foarte lenta
    // setRow este o functie cu mult mai rapida
    mask = 128; 
    for (int j = 0; j < 8; j++){
      x = 0;
      for(int p = 0; p < 8; p++){
        x |= !!(valueVector[7 - p] & mask) << (7 - p);
      }
      mask >>= 1;
      lc.setRow(k, j, x); 
    }
  } 
  
  Serial.println(micros() - newTime);
}

// Facem un average la datele de intrare, pentru a avea date mai coerente
int smooth(){
  int value = 0;
  int numReadings = 5;

  for (int i = 0; i < numReadings; i++){
    value = value + analogRead(A7);
  }

  value = value / numReadings;

  return value;
}


// Converteste o valoare intre 1 si 8 intr-un byte care reprezinta care led-uri sa fie aprinse pe fiecare coloana
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
