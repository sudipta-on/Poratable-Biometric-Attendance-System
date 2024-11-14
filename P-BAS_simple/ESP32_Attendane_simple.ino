#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_Fingerprint.h>
#include <WiFiManager.h> 

// Fingerprint sensor connections
#define R307_TX_PIN 17   // ESP32 TX to R307 RX
#define R307_RX_PIN 16   // ESP32 RX to R307 TX

// WiFi credentials
const char* ssid = "POCO X4 Pro 5G";  // Your WiFi SSID
const char* password = "31415926";    // Your WiFi password

// Wifi Manager
WiFiManager wm;
bool res;
// Google script Web_App_URL.
String Web_App_URL = "https://script.google.com/macros/s/AKfycbwDrkpd4efwMO6Zl1Qn7XvHNW0f7PChd8liWFvz9bGOvjM2IkHV54r2A2AE68wtrrOf/exec";

// LED pin
#define LED_PIN 2  // GPIO2 for the LED

// Button pin
#define BTN_PIN 34 // GPIO34 for mode toggle

// Initialize fingerprint sensor and secure client
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
WiFiClientSecure client;

// Variable to store mode and fingerprint ID
String modes = "reg";  // Mode, either "reg" for register or "atc" for attendance
int fingerprintID = -1; // Fingerprint ID

// Function Prototypes
void http_Req(String str_modes, int fingerID);
void blinkLED(int times, int delayTime);

// HTTP request function
void http_Req(String str_modes, int fingerID) {
  if (WiFi.status() == WL_CONNECTED) {
    client.setInsecure();  // This bypasses SSL verification (use with caution)
    blinkLED(3, 200);      // Blink LED to indicate WiFi connection

    String http_req_url = Web_App_URL + "?sts=" + str_modes + "&uid=" + String(fingerID);

    Serial.println("Sending request to Google Sheets...");
    Serial.print("URL : ");
    Serial.println(http_req_url);

    HTTPClient http;
    http.begin(client, http_req_url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    int httpCode = http.GET();
    Serial.print("HTTP Status Code : ");
    Serial.println(httpCode);

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Payload : " + payload);
      String sts_Res = getValue(payload, ',', 0);
      Serial.println(sts_Res);
      if (sts_Res == "OK") {
        // Handle attendance logic
        if (str_modes == "atc") {
          blinkLED(5, 100); // Blink 5 times quickly for attendance confirmation
        }
      }
    }

    http.end();
  } else {
    blinkLED(1, 500);  // Blink once slowly if WiFi is disconnected
  }
}

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void blinkLED(int times, int delayTime) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH); // Turn on the LED
    delay(delayTime);
    digitalWrite(LED_PIN, LOW);  // Turn off the LED
    delay(delayTime);
  }
}

// Initialize the fingerprint sensor
// Initialize the fingerprint sensor
void initFingerprint() {
  Serial2.begin(57600, SERIAL_8N1, R307_RX_PIN, R307_TX_PIN);  // Start communication with the R307 sensor
  
  finger.begin(57600);  // No return value to check
  
  if (finger.verifyPassword()) {  // Use verifyPassword() to check if the sensor is responding correctly
    Serial.println("Fingerprint sensor found!");
    finger.getTemplateCount();  // Fetch the number of fingerprints stored
    Serial.print("Fingerprint templates: ");
    Serial.println(finger.templateCount);
  } else {
    Serial.println("Fingerprint sensor not found :(");
    while (1);  // Halt the program if the sensor is not found
  }
}


// Get fingerprint ID
int getFingerprintID() {
  int p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;  // If no fingerprint is detected
  
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;
  
  p = finger.fingerSearch();
  if (p != FINGERPRINT_OK) return -1;  // If the fingerprint is not recognized

  return finger.fingerID;
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);  // Set LED pin as OUTPUT
  pinMode(BTN_PIN, INPUT_PULLUP);  // Set button pin as INPUT with pullup

  initFingerprint();  // Initialize the fingerprint sensor

  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println("INNOVATION LAB Wifi");
  res = wm.autoConnect("INNOVATION LAB","password");
  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("Connected.:)");
  }
  

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  blinkLED(3, 200);  // Blink LED three times to indicate successful WiFi connection
  Serial.println("Connected to WiFi");
}

void loop() {
  int btnState = digitalRead(BTN_PIN);
  if (btnState == LOW) {
    delay(300); // Debounce delay
    if (modes == "atc") {
      modes = "reg";
      blinkLED(1, 500);  // Blink once to indicate mode change to "Register"
    } else {
      modes = "atc";
      blinkLED(2, 500);  // Blink twice to indicate mode change to "Attendance"
    }
  }

  // Get fingerprint ID and send it to the server if a valid fingerprint is detected
  fingerprintID = getFingerprintID();
  if (fingerprintID != -1) {
    Serial.print("Fingerprint ID: ");
    Serial.println(fingerprintID);
    http_Req(modes, fingerprintID);  // Send the fingerprint ID and mode to Google Sheets
  }
}
