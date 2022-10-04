class MorseCodeRepeater {

  // Morse code related constants
  // Morse code letters
  const char* letters[26] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", // A-I
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", // J-R 
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.." // S-Z
  };

  // Morse code numbers
  const char* numbers[10] = {
    "-----", ".----", "..---", "...--", "....-", ".....",
  "-....", "--...", "---..", "----."
  };

  unsigned long dotDuration = 100; // ms
  unsigned long dashDuration = dotDuration * 3;
  unsigned long spaceDuration = dotDuration * 4;

  // Class Member Variables
  // These are initialized at startup
  int outputPin;  // the number of the audio signal pin
  unsigned long repeatDuration;  // milliseconds between signal repition
  int toneFrequency;  // Frequency in hertz of morse code tone
  String broadcast;  // The text to broadcast



  // These maintain the current state
  unsigned long previousMillis;   // Stores time last morse code was played

  // Constructor - creates a MorseCodeRepeater 
  // and initializes the member variables and state
  public:
  MorseCodeRepeater(int outputPin, long repeatDuration, int toneFrequency, String broadcast) {
    this->outputPin = outputPin;
    this->repeatDuration = repeatDuration;
    this->toneFrequency = toneFrequency;
    this->broadcast = broadcast;

    previousMillis = 0;

    pinMode(outputPin, OUTPUT);
  }

  boolean hasTimeElapsed() {
    return millis() - previousMillis > repeatDuration;
  }

  boolean resetTimer() {
      previousMillis = millis();
  }
  
  void playMorseCode() {
    for (int i = 0; i < broadcast.length(); i++) {
      outputCharacterSequence(broadcast[i]);
    }
  }

  private:
  void outputCharacterSequence(char ch) {
    // Convert the character to an index in the corresponding array by subtracting
    // the ascii value of 'a' or '0'. Spaces manifest as just a delay
    // Serial.println(ch);
    if (ch >= 'a' && ch <= 'z') {
      outputMorseSequence(letters[ch - 'a']);
    } else if (ch >= 'A' && ch <= 'Z') {
      outputMorseSequence(letters[ch - 'A']);
    } else if (ch >= '0' && ch <= '9') {
      outputMorseSequence(numbers[ch - '0']);
    } else if (ch == ' ') {
      delay(spaceDuration);
    } else {
      Serial.print("Unkown character ");
      Serial.println(ch);
    }
  }

  void outputMorseSequence(char* sequence) {
    int i = 0;
    while (sequence[i] != NULL) {
      outputDotOrDash(sequence[i]);
      i++;
    }

    // After each letter there is a dash delay
    delay(dashDuration);
  }

  void outputDotOrDash(char dotOrDash) {
    tone(outputPin, toneFrequency);
    // Serial.println(dotOrDash);
    if (dotOrDash == '.') {
      delay(dotDuration);
    } else { // must be a -
      delay(dashDuration);
    }
    noTone(outputPin);

    // After each dot or dash, there is a dot length delay
    delay(dotDuration);
  }
};

// CONSTANTS
int OUTPUT_PIN = 12;
int MORSE_PIN = 3;
int INPUT_PIN = A0;

long BUFFER_DELAY_TIME = 2000l;  // 2 seconds, in ms
long CUTOFF_PERIOD_TIME = 3 * 60 * 1000;  // 3 minutes, in ms

long MORSE_REPEAT_TIME = 8000L;
int MORSE_FREQUENCY = 700;
String MORSE_STRING = "VV";

int RECEIVER_ON_THRESHOLD = 400;  // Analog read out of 1024

// Quindar tone for turning off https://en.m.wikipedia.org/wiki/Quindar_tones
long QUINDAR_OFF_DURATION = 250; // ms
int QUINDAR_OFF_FREQUENCY = 2475;
boolean QUINDAR_ENABLED = true;

int inval;  // Raw on/off from LED
int cutoffval;  // Value processed to turn off after certain time
int bufferval;  // Value processed to stay on a bit after input turns off


int last_inval;
int last_bufferval;


int outval;  // Output to transmitter
int last_outval;

unsigned long start_time;
unsigned long cutoff_start_time; 

MorseCodeRepeater morseCodeRepeater(MORSE_PIN, MORSE_REPEAT_TIME , MORSE_FREQUENCY, MORSE_STRING);

void setup() {
  Serial.begin(9600);
  pinMode(OUTPUT_PIN, OUTPUT);
}

void loop() {
  // Input value from receiving baofeng
  int sensorValue = analogRead(INPUT_PIN);
  if (sensorValue > RECEIVER_ON_THRESHOLD) {
    inval = 1;
  } else {
    inval = 0;
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
      inval = 1;
  }

  // ---------- Buffer when turn off code here ----------

  // Detect falling edge
  if (!inval && last_inval) {
    start_time = millis();
    Serial.print("Falling edge @ ");
    Serial.println(start_time);
  }

  // If we are less than the delay after a falling edge, keep the output high.
  // Otherwise, simply pass through the previous value unimpeded
  if (start_time != 0 && millis() - start_time <= BUFFER_DELAY_TIME) {
    bufferval = 1;
  } else {
    bufferval = inval;
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

  last_inval = inval;
  last_bufferval = bufferval;
  last_outval = outval;

  delay(100);  // Loop at 10Hz
}
