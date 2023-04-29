#include "Arduino.h" 

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

  // Dot duration defines how fast the Morse code is. Dash and space are 3x and 4x multiplies of dot duration.
  unsigned long dotDuration; // ms
  unsigned long dashDuration;
  unsigned long spaceDuration;

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
  MorseCodeRepeater(int outputPin, long repeatDuration, int toneFrequency, unsigned long dotDuration, String broadcast) {
    this->outputPin = outputPin;
    this->repeatDuration = repeatDuration;
    this->toneFrequency = toneFrequency;
    this->broadcast = broadcast;

    this->dotDuration = dotDuration;
    this->dashDuration = dotDuration * 3; 
    this->spaceDuration = dotDuration * 4;

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
