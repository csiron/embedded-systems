#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#define NUM_DIGITAL_SAMPLES 12
#define NUM_ANALOG_SAMPLES 4
#define PIN 6
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, PIN, NEO_GRB + NEO_KHZ800);

int tempC; 
int tempF;
int packet[32];
int digitalSamples[NUM_DIGITAL_SAMPLES];
int analogSamples[NUM_ANALOG_SAMPLES];
int volts;

void setup()
{
 Serial.begin(9600);
 strip.begin();
 strip.show();
}

void loop() {
 readPacket();
}

void readPacket() {
 if (Serial.available() > 0) {
   int b = Serial.read();
   if (b == 0x7E) {
     packet[0] = b;
     packet[1] = readByte();
     packet[2] = readByte();
     int dataLength = (packet[1] << 8) | packet[2];

     for(int i = 1; i <= dataLength; i++) {
       packet[2+i] = readByte();
     }
     int apiID = packet[3];
     packet[3+dataLength] = readByte(); // checksum

     printPacket(dataLength+4);

     if (apiID == 0x92) {
       int analogSampleIndex = 19;
       int digitalChannelMask = (packet[16] << 8) | packet[17];
       if (digitalChannelMask > 0) {
         int d = (packet[19] << 8) | packet[20];
         for(int i = 0; i < NUM_DIGITAL_SAMPLES; i++) {
           digitalSamples[i] = ((d >> i) & 1);
         }
         analogSampleIndex = 21;
       }

       int analogChannelMask = packet[18];
       for(int i = 0; i < 4; i++) {
         if ((analogChannelMask >> i) & 1) {
           analogSamples[i] = (packet[analogSampleIndex] << 8) | packet[analogSampleIndex+1];
           analogSampleIndex += 2;
         } else {
           analogSamples[i] = -1;
         }
       }
     }
   }

   int reading = analogSamples[0];  // pin 19 [0] is pin 20
   int volts = reading;
   Serial.print("Voltage:  ");
   Serial.println(volts);
  
   // convert reading to millivolts
   float v = ((float)reading / 1023.0) * 1200.0;

   // convert to Celcius.  10mv per Celcius degree with 500mV offset at 0 celcius
   float c = (v - 500.0) / 10.0;

   // round to nearest int
   tempC = (int)(c + 0.5);
   Serial.print("Temp: ");
   Serial.println(tempC);

   //convert to fahrenheit
   tempF = (((tempC * 9) / 5) + 32);
   Serial.print("Temp: ");
   Serial.println(tempF);

   if (volts > 0 && volts < 1500) {
   if (tempF > 0 && tempF <= 30) {
    colorWipe(strip.Color(255, 255, 255), 50);
   }
   else if (tempF > 30 && tempF <= 40) {
    colorWipe(strip.Color(160, 32, 240), 50);
   }
   else if (tempF > 40 && tempF <= 50) {
    colorWipe(strip.Color(0, 0, 255), 50);
   }
   else if (tempF > 50 && tempF <= 60) {
    colorWipe(strip.Color(0, 255, 0), 50);
   }
   else if (tempF > 60 && tempF <= 70) {
    colorWipe(strip.Color(255, 255, 0), 50);
   }
   else if (tempF > 70 && tempF <= 80) {
    colorWipe(strip.Color(255, 140, 0), 50);
   }
   else if (tempF > 80 && tempF <= 90) {
    colorWipe(strip.Color(255, 69, 0), 50);
   }
   else if (tempF > 90) {
    colorWipe(strip.Color(255, 0, 0), 50);
   }
   delay(1000);
 }
 }
}


void printPacket(int l) {
 for(int i = 0;i < l; i++) {
   if (packet[i] < 0xF) {
     // print leading zero for single digit values
     Serial.print(0);
   }
   Serial.print(packet[i], HEX);
   Serial.print(" ");
 }
 Serial.println("");
}


int readByte() {
   while (true) {
     if (Serial.available() > 0) {
     return Serial.read();
   }
 }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
 }
