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

  void update() {
    if (millis() - previousMillis > repeatDuration) {
      previousMillis = millis();

      Serial.println("Playing Morse Code");
      digitalWrite(4,1);
      delay(1000);
      for (int i = 0; i < broadcast.length(); i++) {
        outputCharacterSequence(broadcast[i]);
      }
      delay(1000);
      digitalWrite(4,0);
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

int BUTTON_PIN = 2;
int LED_PIN = 4;
int MORSE_PIN = 3;
int INPUT_PIN = A0;

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

MorseCodeRepeater morseCodeRepeater(MORSE_PIN, 12000L , 700, "VV");

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Input value
  inval = digitalRead(BUTTON_PIN);

  int sensorValue = analogRead(INPUT_PIN);
  if (sensorValue > 400) {
//    Serial.println("ON");
  } else {
//    Serial.println("OFF");
  }
//  Serial.println(sensorValue);


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

  morseCodeRepeater.update();
}
