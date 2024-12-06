#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "WIFI_NETWORK_NAME";
const char* password = "WIFI_PASSWORD";

WebServer server(80);

const int ledPin = 2;
const int morseUnitTime = 100;
String currentMessage = "default";

unsigned long previousMillis = 0;
int messageIndex = 0;
int morseIndex = 0;
bool isDisplaying = false;
const char* currentMorse = NULL;
int ledState = LOW;
unsigned long nextChangeTime = 0;

enum MorseState {
    IDLE,
    DOT_ON,
    DOT_OFF,
    DASH_ON,
    DASH_OFF,
    LETTER_SPACE,
    WORD_SPACE
};
MorseState morseState = IDLE;

struct MorseMapping {
   char character;
   const char* morse;
};

const MorseMapping morseMap[] = {
    // Letters
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
    
    // Numbers
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
    
    // Punctuation
    {'.', ".-.-.-"},
    {',', "--..--"},
    {'?', "..--.."},
    {'!', "-.-.--"},
    {'"', ".-..-."},
    {'(', "-.--."},
    {')', "-.--.-"},
    {'&', ".-..."},
    {':', "---..."},
    {';', "-.-.-."},
    {'=', "-...-"},
    {'+', ".-.-."},
    {'-', "-....-"},
    {'_', "..--.-"},
    {'$', "...-..-"},
    {'@', ".--.-."},
};

const int MORSE_MAP_SIZE = sizeof(morseMap) / sizeof(morseMap[0]);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>Morse Encoder</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin: 20px; }
    .content { max-width: 600px; margin: auto; }
    input[type=text] { width: 80%; padding: 12px 20px; margin: 8px 0; }
    button { background-color: #4CAF50; color: white; padding: 14px 20px; border: none; cursor: pointer; }
    button:hover { opacity: 0.8; }
  </style>
</head>
<body>
  <div class="content">
    <h1>Morse Encoder</h1>
    <form action="/send" method="get">
      <input type="text" name="message" placeholder="Enter your message">
      <button type="submit">Send Message</button>
    </form>
    <p>Current Message: <span id="current">%MESSAGE%</span></p>
  </div>
</body>
</html>
)rawliteral";

const char* getMorseCode(char c) {
   for(int i = 0; i < MORSE_MAP_SIZE; i++) {
       if(morseMap[i].character == c) {
           return morseMap[i].morse;
       }
   }
   return NULL;
}

void startDot() {
    digitalWrite(ledPin, HIGH);
    nextChangeTime = millis() + morseUnitTime;
    morseState = DOT_ON;
    Serial.print(".");
}

void startDash() {
    digitalWrite(ledPin, HIGH);
    nextChangeTime = millis() + (morseUnitTime * 3);
    morseState = DASH_ON;
    Serial.print("-");
}

void finishDot() {
    digitalWrite(ledPin, LOW);
    nextChangeTime = millis() + morseUnitTime;
    morseState = DOT_OFF;
}

void finishDash() {
    digitalWrite(ledPin, LOW);
    nextChangeTime = millis() + morseUnitTime;
    morseState = DASH_OFF;
}

void handleRoot() {
    String html = String(index_html);
    html.replace("%MESSAGE%", currentMessage);
    server.send(200, "text/html", html);
}

void handleSend() {
    if (server.hasArg("message")) {
        currentMessage = server.arg("message");
        Serial.print("\nNew message: ");
        Serial.println(currentMessage);
        messageIndex = 0;
        morseIndex = 0;
        isDisplaying = false;
        currentMorse = NULL;
        morseState = IDLE;
        server.sendHeader("Location", "/");
        server.send(303);
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(ledPin, OUTPUT);
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.on("/", handleRoot);
    server.on("/send", handleSend);
    server.begin();
}

void updateMorse() {
    unsigned long currentMillis = millis();
    char letter;
    
    if (currentMillis < nextChangeTime) {
        return;
    }

    switch(morseState) {
        case IDLE:
            if (messageIndex >= currentMessage.length()) {
                messageIndex = 0;
                nextChangeTime = currentMillis + morseUnitTime * 50;
                Serial.println("\n--- Message End ---");
                return;
            }
            
            letter = toupper(currentMessage[messageIndex]);
            if (letter == ' ') {
                morseState = WORD_SPACE;
                nextChangeTime = currentMillis + morseUnitTime * 7;
                messageIndex++;
                Serial.print("   ");
                return;
            }
            
            currentMorse = getMorseCode(letter);
            if (currentMorse) {
                Serial.print("\nLetter '");
                Serial.print(letter);
                Serial.print("' (");
                if (currentMorse[morseIndex] == '.') {
                    startDot();
                } else if (currentMorse[morseIndex] == '-') {
                    startDash();
                }
            }
            messageIndex++;
            break;
            
        case DOT_ON:
            finishDot();
            break;
            
        case DOT_OFF:
            morseIndex++;
            if (morseIndex >= strlen(currentMorse)) {
                morseState = LETTER_SPACE;
                nextChangeTime = currentMillis + morseUnitTime * 3;
                morseIndex = 0;
                currentMorse = NULL;
                Serial.print(") ");
            } else if (currentMorse[morseIndex] == '.') {
                startDot();
            } else if (currentMorse[morseIndex] == '-') {
                startDash();
            }
            break;
            
        case DASH_ON:
            finishDash();
            break;
            
        case DASH_OFF:
            morseIndex++;
            if (morseIndex >= strlen(currentMorse)) {
                morseState = LETTER_SPACE;
                nextChangeTime = currentMillis + morseUnitTime * 3;
                morseIndex = 0;
                currentMorse = NULL;
                Serial.print(") ");
            } else if (currentMorse[morseIndex] == '.') {
                startDot();
            } else if (currentMorse[morseIndex] == '-') {
                startDash();
            }
            break;
            
        case LETTER_SPACE:
        case WORD_SPACE:
            morseState = IDLE;
            break;
    }
}

void loop() {
    server.handleClient();
    updateMorse();
}
