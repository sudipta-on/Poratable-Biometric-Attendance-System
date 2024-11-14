#include <Adafruit_Fingerprint.h>

// Use hardware serial on ESP32 (change the pins if needed)
HardwareSerial mySerial(1); // Serial1 for ESP32

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200); // Set up Serial Monitor for debugging
  while (!Serial);  // Wait for Serial Monitor to connect
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // Initialize the fingerprint sensor at a baud rate of 57600
  mySerial.begin(57600, SERIAL_8N1, 16, 17); // RX=16, TX=17

  // Verify if the sensor is found
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // Read sensor parameters
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.print.ln(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: "))); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) { // ID #0 is not allowed, try again!
    return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!getFingerprintEnroll());
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);
  display.clearDisplay();
  display.drawBitmap( 34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        display.clearDisplay();
        display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        display.setTextSize(1);             // Normal 2:2 pixel scale
        display.setTextColor(WHITE);        // Draw white text
        display.setCursor(0,0);             // Start at top-left corner
        display.print(F("scanning"));
        display.display();
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        display.clearDisplay();
        display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
        display.display();
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  display.clearDisplay();
  display.setTextSize(2);             // Normal 2:2 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.print(F("Remove"));
  display.setCursor(0,20);
  display.print(F("finger"));
  display.display();
  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  display.clearDisplay();
  display.drawBitmap( 34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        display.clearDisplay();
        display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        display.setTextSize(1);             // Normal 2:2 pixel scale
        display.setTextColor(WHITE);        // Draw white text
        display.setCursor(0,0);             // Start at top-left corner
        display.print(F("scanning"));
        display.display();
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // Success!
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      display.clearDisplay();
      display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // Create 
  Serial.print("Creating model for #");  Serial.println(id);
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // Store model
  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    display.clearDisplay();
    display.drawBitmap( 34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else {
    Serial.println("Error storing fingerprint");
    return p;
  }

  return true;
}
