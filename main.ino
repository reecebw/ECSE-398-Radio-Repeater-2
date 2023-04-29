#include "MorseCodeRepeater.cpp"

// CONSTANTS
int OUTPUT_PIN = 12;
int MORSE_PIN = 3;
int INPUT_PIN = A1;

long BUFFER_DELAY_TIME = 500;  // 2 seconds, in ms
long CUTOFF_PERIOD_TIME = 3 * 60 * 1000L;  // 3 minutes, in ms
long DEBOUNCE_TIME = 350; // ms

long MORSE_REPEAT_TIME = 1 * 60 * 1000L; //ms
int MORSE_FREQUENCY = 700;
int MORSE_DOT_DURATION = 70;  // ms. Represents the base unit of time for morse code.
String MORSE_STRING = "W8EDU";

int RECEIVER_ON_THRESHOLD_HIGH = 230;  // Analog read out of 1024
int RECEIVER_ON_THRESHOLD_LOW = 100;
boolean RECIEVER_ON = false;


// Quindar tone for turning off https://en.m.wikipedia.org/wiki/Quindar_tones
long QUINDAR_OFF_DURATION = 250; // ms
int QUINDAR_OFF_FREQUENCY = 2475;
boolean QUINDAR_ENABLED = true;

int inval;  // Raw on/off from LED
int cutoffval;  // Value processed to turn off after certain time
int bufferval;  // Value processed to stay on a bit after input turns off
int debounceval;  // For disregarding a clacking input


int last_bufferval;
int last_debounceval;


int outval;  // Output to transmitter
int last_outval;

unsigned long start_time;
unsigned long cutoff_start_time; 
unsigned long debounce_start_time;

MorseCodeRepeater morseCodeRepeater(MORSE_PIN, MORSE_REPEAT_TIME, MORSE_FREQUENCY, MORSE_DOT_DURATION, MORSE_STRING);

void setup() {
  Serial.begin(115200);
  pinMode(OUTPUT_PIN, OUTPUT);
}

void loop() {
  // Input value from receiving baofeng
  int sensorValue = analogRead(INPUT_PIN);
  Serial.println(sensorValue);
  if (sensorValue > RECEIVER_ON_THRESHOLD_HIGH || (sensorValue > RECEIVER_ON_THRESHOLD_LOW && inval==1)) {
    inval = 1;
  } else {
    inval = 0;
  }

  // ---------- Debounce input here ----------
  if (!inval) {
    // Continously reset start time when input is off.
    // Then, when input is on, we can count the ms until we should turn on.
    debounce_start_time = millis();
    debounceval = 0;
  } else if (millis() - debounce_start_time >= DEBOUNCE_TIME) {
    debounceval = 1;
  } else {
    debounceval = 0;
  }


  // Morse code is a blocking call. Insert this here so it can simulate the 
  // inval being set to 1, and the start times can be set appropriatley.
  // We do many hacks with the other variables to make them behave properly.
  if (morseCodeRepeater.hasTimeElapsed()) {
      Serial.print("Playing Morse Code @ ");
      Serial.println(millis());
      morseCodeRepeater.resetTimer();
      digitalWrite(OUTPUT_PIN, 1);  // Manually turn on the output in case CUTOFF_PERIOD_TIME has turned it off
      delay(1000);
      morseCodeRepeater.playMorseCode();
      
      // Simulate the morse code as the reciever recieving, so it gets the adequate buffer delay.
      debounceval = 1;
  }

  // ---------- Buffer when turn off code here ----------

  // Detect falling edge
  if (!debounceval && last_debounceval) {
    start_time = millis();
    Serial.print("Falling edge @ ");
    Serial.println(start_time);
  }

  // If we are less than the delay after a falling edge, keep the output high.
  // Otherwise, simply pass through the previous value unimpeded
  if (start_time != 0 && millis() - start_time <= BUFFER_DELAY_TIME) {
    bufferval = 1;
  } else {
    bufferval = debounceval;
  }

  // ------------------------------------------------------


  // ---------- On too long cutoff code here ----------
  cutoffval = bufferval;

if(!last_bufferval && bufferval) {
  cutoff_start_time = millis();
  Serial.print("Rising edge @ ");
  Serial.println(cutoff_start_time);
}

if (cutoff_start_time != 0 && CUTOFF_PERIOD_TIME <= millis() - cutoff_start_time) {
  cutoffval = 0;
} else {
  cutoffval = bufferval; 
}

  // ------------------------------------------------------
  // Output here
  outval = cutoffval;

  // Output the final output to turn on transmitter push-to-talk
  // If we are turning off the transmitter, first play the Quindar tone if enabled
  if (QUINDAR_ENABLED && last_outval && !outval) {
    Serial.print("Playing Quindar Off Tone @ ");
    Serial.println(millis());
    tone(MORSE_PIN, QUINDAR_OFF_FREQUENCY);
    delay(QUINDAR_OFF_DURATION);
    noTone(MORSE_PIN);
  }

  digitalWrite(OUTPUT_PIN, outval);

  last_bufferval = bufferval;
  last_outval = outval;
  last_debounceval = debounceval;

  delay(100);  // Loop at 10Hz
}
