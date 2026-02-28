#include <HardwareSerial.h>
#include <Arduino.h>

#define I2C_DEV_ADDR 0x08

// LORA 1 AND 2 RESPECTIVELY
#define RXLORA_RX 3 
#define RXLORA_TX 2 

#define TXLORA_RX 5 
#define TXLORA_TX 4 

// --- NEW: TEST STAND HARDWARE DEFINITIONS ---
const int NUM_SOLENOIDS = 6;
const int SOLENOID_PINS[NUM_SOLENOIDS] = {25, 26, 27, 14, 12, 13}; // Change to your actual wiring

struct __attribute__((packed)) TelemetryPacket {
  uint32_t timestamp;
  uint32_t seq;
  uint8_t mask;
  uint8_t stat;
  uint16_t solenoid_state;
  uint16_t adc_values[6]; // 4 Pressure, 1 Temp, 1 Load Cell
};

TelemetryPacket packet;
uint16_t current_solenoid_state = 0;
unsigned long last_telemetry_time = 0;
// --------------------------------------------

String sendATcommand(const char *toSend, unsigned long ms, HardwareSerial &name);
void parseReceivedMessage(String msg);
void send_command(String lora_input, String address, HardwareSerial& name);

HardwareSerial RXLORA(1); 
HardwareSerial TXLORA(2); 

bool reporting_lock = false;
String lora_input = "";
String address = "";
unsigned long sendStartTime = 0;
bool waitingForReply = false;

String toJetson; 

void setup() {
  // 1. Start USB Serial for Python GUI
  Serial.begin(115200);
  delay(100);

  // 2. Initialize Physical Solenoid Pins
  for (int i = 0; i < NUM_SOLENOIDS; i++) {
    pinMode(SOLENOID_PINS[i], OUTPUT);
    digitalWrite(SOLENOID_PINS[i], LOW);
  }

  // 3. Initialize LoRa Modules
  RXLORA.begin(115200, SERIAL_8N1, RXLORA_RX, RXLORA_TX);
  TXLORA.begin(115200, SERIAL_8N1, TXLORA_RX, TXLORA_TX);
  delay(1000);

  // Note: Serial.println is okay in setup() because the Python GUI ignores 
  // data until it connects and establishes the struct format.
  Serial.println("Initializing LoRa's...");

  sendATcommand("AT", 100, RXLORA);
  sendATcommand("AT", 100, TXLORA);

  sendATcommand("AT+ADDRESS=4", 100, RXLORA);
  sendATcommand("AT+ADDRESS=1", 100, TXLORA);
  delay(100);
  sendATcommand("AT+BAND=915000000", 100, RXLORA);
  sendATcommand("AT+BAND=928000000", 100, TXLORA);
  delay(100);
  sendATcommand("AT+NETWORKID=18", 100, RXLORA);
  sendATcommand("AT+NETWORKID=18", 100, TXLORA);
  delay(100);
  sendATcommand("AT+PARAMETER=11,9,4,24", 100, RXLORA);
  sendATcommand("AT+PARAMETER=11,9,4,24", 100, TXLORA);
  delay(100);

  Serial.println("Setup complete. Awaiting GUI Connection...");
}

void loop() {
  // --- 1. HANDLE INCOMING LORA MESSAGES ---
  if (RXLORA.available()) {
    String incomingString = "";
    unsigned long startTime = millis();
    while (millis() - startTime < 1000) {
      if (RXLORA.available()) {
        char c = RXLORA.read();
        if (c == '\n') break;
        incomingString += c;
      }
    }
    
    if (incomingString.length() > 0) {
      if (incomingString.indexOf("+RCV=") != -1) {
        parseReceivedMessage(incomingString);
      }
    }
  }

  // --- 2. HANDLE INCOMING PYTHON GUI COMMANDS ---
  // Replaces the old text-based checkUserInput()
  if (Serial.available() >= 2) {
    uint8_t buffer[2];
    Serial.readBytes(buffer, 2);
    
    // Reconstruct the 16-bit integer sent by Python
    current_solenoid_state = buffer[0] | (buffer[1] << 8);

    // Apply the hardware changes
    for (int i = 0; i < NUM_SOLENOIDS; i++) {
      bool isOn = (current_solenoid_state & (1 << i)) != 0;
      digitalWrite(SOLENOID_PINS[i], isOn ? HIGH : LOW);
    }
  }

  // --- 3. TRANSMIT TELEMETRY TO PYTHON GUI ---
  // Send the binary struct at 10Hz
  if (millis() - last_telemetry_time >= 100) {
    last_telemetry_time = millis();

    packet.timestamp = millis();
    packet.seq++;
    packet.mask = 0xFF;
    packet.stat = 0x00;
    packet.solenoid_state = current_solenoid_state; 

    // Read physical sensors (Replace pins with your actual wiring)
    packet.adc_values[0] = analogRead(32); // P1
    packet.adc_values[1] = analogRead(33); // P2
    packet.adc_values[2] = analogRead(34); // P3
    packet.adc_values[3] = analogRead(35); // P4
    packet.adc_values[4] = analogRead(36); // T1
    packet.adc_values[5] = analogRead(39); // LC1

    // Write binary to USB so the GUI can plot it
    Serial.write((uint8_t*)&packet, sizeof(packet));
  }
}

// --- HELPER FUNCTIONS ---

void parseReceivedMessage(String rawMessage) {
  int startIdx = rawMessage.indexOf("+RCV=");
  if (startIdx == -1) return;
  
  String data = rawMessage.substring(startIdx + 5); 
  int firstComma = data.indexOf(',');
  int secondComma = data.indexOf(',', firstComma + 1);
  int thirdComma = data.indexOf(',', secondComma + 1);
  
  if (firstComma == -1 || secondComma == -1) return;
  
  String payload = data.substring(secondComma + 1, thirdComma != -1 ? thirdComma : data.length());
  toJetson = payload.c_str();

  // NOTE: Serial.print statements are removed here so they don't break the Python GUI telemetry stream!
}

void send_command(String inputString, String address, HardwareSerial &name) {
  int len = inputString.length();
  int addressInt = address.toInt();
  if (addressInt < 0 || addressInt > 65535) return;
  
  char atCommand[100];
  snprintf(atCommand, sizeof(atCommand), "AT+SEND=%d,%d,%s", addressInt, len, inputString.c_str());
  
  sendStartTime = millis();
  waitingForReply = true;
  sendATcommand(atCommand, 100, name);
  delay(100); 
}

String sendATcommand(const char *toSend, unsigned long ms, HardwareSerial &name) {
  name.println(toSend);
  unsigned long start = millis();
  String res = "";
  while (millis() - start < ms) {
    if (name.available()) res += (char)name.read();
  }
  // NOTE: Serial.println(res) removed to protect the Python GUI stream.
  return res;
}
