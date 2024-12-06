const int ledPin = 2;
const int morseUnitTime = 100;
const char* text = "test 1234";

struct MorseMapping {
   char character;
   const char* morse;
};

const MorseMapping morseMap[] = {
   {'A', ".-"},
   {'B', "-..."},
   {'C', "-.-."},
   {'D', "-.."},
   {'E', "."},
   {'F', "..-."},
   {'G', "--."},
   {'H', "...."},
   {'I', ".."},
   {'J', ".---"},
   {'K', "-.-"},
   {'L', ".-.."},
   {'M', "--"},
   {'N', "-."},
   {'O', "---"},
   {'P', ".--."},
   {'Q', "--.-"},
   {'R', ".-."},
   {'S', "..."},
   {'T', "-"},
   {'U', "..-"},
   {'V', "...-"},
   {'W', ".--"},
   {'X', "-..-"},
   {'Y', "-.--"},
   {'Z', "--.."},
   {'0', "-----"},
   {'1', ".----"},
   {'2', "..---"},
   {'3', "...--"},
   {'4', "....-"},
   {'5', "....."},
   {'6', "-...."},
   {'7', "--..."},
   {'8', "---.."},
   {'9', "----."},
};

const int MORSE_MAP_SIZE = sizeof(morseMap) / sizeof(morseMap[0]);

const char* getMorseCode(char c) {
   for(int i = 0; i < MORSE_MAP_SIZE; i++) {
       if(morseMap[i].character == c) {
           return morseMap[i].morse;
       }
   }
   return NULL;
}

void setup() {
   pinMode(ledPin, OUTPUT);
}

void loop() {
   const char* currentChar = text;
   while(*currentChar) {
       char letter = toupper(*currentChar++);
       const char* signalInstructions = getMorseCode(letter);
       if(signalInstructions) {
           if(letter == ' ') {
               delay(morseUnitTime * 7);
           } else {
               executeMorseCode(signalInstructions);
               delay(morseUnitTime * 3);
           }
       }
   }
   delay(morseUnitTime * 50);
}

void executeMorseCode(const char* instructions) {
   while(*instructions) {
       char i = *instructions++;
       switch(i) {
           case '.': dot(); break;
           case '-': dash(); break;
       }
   }
}

void dot() {
   digitalWrite(ledPin, HIGH);
   delay(morseUnitTime);
   digitalWrite(ledPin, LOW);
   delay(morseUnitTime);
}

void dash() {
   digitalWrite(ledPin, HIGH);
   delay(morseUnitTime * 3);
   digitalWrite(ledPin, LOW);
   delay(morseUnitTime);
}
