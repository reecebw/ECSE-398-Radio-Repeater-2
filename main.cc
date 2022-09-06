#include "limits.h"

int BUTTON_PIN = 2;  // analog pin used to connect the potentiometer
int LED_PIN = 4;

int inval;  // Raw on/off from LED
int cutoffval;  // Value processed to turn off after certain time
int bufferval;  // Value processed to stay on a bit after input turns off


int last_inval;
int last_bufferval;

long buffer_delay_time = 2000;  // ms
long cutoff_delay_time = 6000;


int outval;  // Output to transmitter

unsigned long start_time;
unsigned long cutoff_start_time; 

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Input value
  inval = digitalRead(BUTTON_PIN);

  // ---------- Buffer when turn off code here ----------

  // Detect falling edge
  if (!inval && last_inval) {
    Serial.println("Falling edge");
    start_time = millis();
    Serial.println(start_time);
  }

  // If we are less than the delay after a falling edge, keep the output high.
  // Otherwise, simply pass through the previous value unimpeded
  if (start_time != 0 && millis() - start_time <= buffer_delay_time) {
    bufferval = 1;
  } else {
    bufferval = inval;
  }

  // ------------------------------------------------------


  // ---------- On too long cutoff code here ----------
  cutoffval = bufferval;

if(!last_bufferval && bufferval) {
  Serial.println("Rising edge");
  cutoff_start_time = millis();
}

if (cutoff_start_time != 0 && cutoff_delay_time <= millis() - cutoff_start_time) {
  cutoffval = 0;

} else {
  cutoffval = bufferval; 
}



  // ------------------------------------------------------


  // Output here
  outval = cutoffval;

  // Output the final output to turn on transmitter
  digitalWrite(LED_PIN, outval);


  last_inval = inval;
  last_bufferval = bufferval; 
  delay(100);
}


