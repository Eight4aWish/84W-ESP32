#include <Wire.h>
#include <Adafruit_MCP4728.h>
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>
#include <cmath>  // For sin() in sine wave generation

const char *ssid = "PLUSNET-KZC3HT";
const char *password = "FAn6mGRFgy6a9F";

AsyncWebServer server(80);

Adafruit_MCP4728 mcp;

const int potPin = 32;
const int cvPin = 33;
const int switchPinUp = 35;
const int switchPinDown = 34;
const int clock_high = 1500;
const int clock_low = 0;

// Update interval for ADC readings on the webpage (in milliseconds)
const int UPDATE_INTERVAL_MS = 2000;  // 2 seconds

// variable for storing the potentiometer value
int potValue = 0;
int cvValue = 2000;
int switch1 = 0;
int switch2 = 0;
int counterb = 1;
int counterc = 1;
int counterd = 1;
int delay_value = 125;
int value = 0;
int countera = 0;

// Define a structure to hold each range mapping
struct RangeMapping {
  int min_val_inclusive;  // The minimum value of the input range (inclusive)
  int max_val_exclusive;  // The maximum value of the input range (exclusive)
  float output_val;       // The output value if input falls in this range
};

// --- Define your lookup table here ---
// The ranges should ideally be sorted by min_val_inclusive and be non-overlapping
// for predictable behavior, though the function below will return the first match found.
RangeMapping lookupTable[] = {
  { 0, 47, 1479.98f },
  { 47, 113, 1396.91f },
  { 113, 149, 1318.51f },
  { 149, 186, 1244.51f },
  { 186, 222, 1174.66f },
  { 222, 257, 1108.73f },
  { 257, 290, 1046.5f },
  { 290, 326, 987.77f },
  { 326, 365, 932.33f },
  { 365, 402, 880.00f },
  { 402, 438, 830.61f },
  { 438, 474, 830.61f },
  { 474, 510, 783.99f },
  { 510, 546, 739.99f },
  { 546, 582, 698.46f },
  { 582, 620, 659.26f },
  { 620, 658, 622.25f },
  { 658, 696, 587.33f },
  { 696, 732, 554.37f },
  { 732, 768, 523.25f },
  { 768, 805, 493.88f },
  { 805, 841, 466.16f },
  { 841, 880, 440.00f },
  { 880, 915, 415.3f },
  { 915, 950, 392.00f },
  { 950, 989, 369.99f },
  { 989, 1027, 349.23f },
  { 1027, 1065, 329.63f },
  { 1065, 1101, 311.13f },
  { 1101, 1139, 293.66f },
  { 1139, 1178, 277.18f },
  { 1178, 1214, 261.63f },
  { 1214, 1249, 246.94f },
  { 1249, 1288, 233.08f },
  { 1288, 1326, 220.00f },
  { 1326, 1362, 207.65f },
  { 1362, 1399, 196.00f },
  { 1399, 1436, 185.00f },
  { 1436, 1472, 174.61f },
  { 1472, 1506, 164.81f },
  { 1506, 1546, 155.56f },
  { 1546, 1588, 146.83f },
  { 1588, 1624, 138.59f },
  { 1624, 1660, 130.81f },
  { 1660, 1698, 123.47f },
  { 1698, 1733, 116.54f },
  { 1733, 1768, 110.00f },
  { 1768, 1804, 103.83f },
  { 1804, 1842, 98.00f },
  { 1842, 1875, 92.5f },
  { 1875, 1911, 87.31f },
  { 1911, 1952, 82.41f },
  { 1952, 1985, 77.78f },
  { 1985, 2017, 73.42f },
  { 2017, 2059, 69.3f },
  { 2059, 2102, 65.41f },
  { 2102, 2140, 61.74f },
  { 2140, 2175, 58.27f },
  { 2175, 2207, 55.00f },
  { 2207, 2243, 51.91f },
  { 2243, 2280, 49.00f },
  { 2280, 2318, 46.25f },
  { 2318, 2354, 43.65f },
  { 2354, 2391, 41.2f },
  { 2391, 2429, 38.89f },
  { 2429, 2465, 36.71f },
  { 2465, 2505, 34.65f },
  { 2505, 2543, 32.7f },
  { 2543, 2579, 30.87f },
  { 2579, 2617, 29.14f },
  { 2617, 2653, 27.5f },
  { 2653, 2691, 25.96f },
  { 2691, 2728, 24.5f },
  { 2728, 2766, 23.12f },
  { 2766, 2804, 21.83f },
  { 2804, 2843, 20.6f },
  { 2843, 2883, 19.45f },
  { 2883, 2923, 18.35f },
  { 2923, 2965, 17.32f },
  { 2965, 3005, 16.35f },
  { 3005, 3051, 15.43f },
  { 3051, 3099, 14.57f },
  { 3099, 3143, 13.75f },
  { 3143, 3191, 12.98f },
  { 3191, 3243, 12.26f },
  { 3243, 3298, 11.57f },
  { 3298, 3351, 10.92f },
  { 3351, 3403, 10.31f },
  { 3403, 3462, 9.73f },
  { 3462, 3522, 9.19f },
  { 3522, 3585, 8.67f },
  { 3585, 3649, 8.19f },
  // {30.0f, 40.0f, 75}
};
const int lookupTableSize = sizeof(lookupTable) / sizeof(lookupTable[0]);

// --- Default value if no range is matched ---
const int NO_MATCH_DEFAULT_OUTPUT = 440.0f;  // Or any other suitable default/error value

// Function to perform the lookup
float getValueFromRange(int inputValue) {
  for (int i = 0; i < lookupTableSize; ++i) {
    if (inputValue >= lookupTable[i].min_val_inclusive && inputValue < lookupTable[i].max_val_exclusive) {
      return lookupTable[i].output_val;  // Input is within this range
    }
  }
  return NO_MATCH_DEFAULT_OUTPUT;  // Input did not fall into any defined range
}

// HTML page content with placeholders for two ADC values
const char ADC_VIEW_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Dual ADC Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, Helvetica, sans-serif; text-align: center; margin-top: 30px; background-color: #f4f4f4; color: #333; }
    h1 { color: #2c3e50; }
    .adc-container { background-color: #fff; margin: 20px auto; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); display: inline-block; min-width: 250px; }
    .adc-label { font-size: 1.1em; color: #555; margin-bottom: 5px; }
    .adc-value { font-size: 2.2em; color: #007bff; font-weight: bold; margin: 5px 0 15px 0; }
    a { color: #007bff; text-decoration: none; }
    a:hover { text-decoration: underline; }
    .nav-links { margin-bottom: 30px; }
  </style>
</head>
<body>
  <h1>ESP32 Dual ADC Monitor</h1>
  <div class="nav-links">
    <a href="/">Home</a> | <a href="/update">OTA Update</a>
  </div>

  <div class="adc-container">
    <div class="adc-label">ADC 1 (GPIO %ADC_PIN_1%):</div>
    <div id="adcValue1" class="adc-value">Loading...</div>
  </div>

  <div class="adc-container">
    <div class="adc-label">ADC 2 (GPIO %ADC_PIN_2%):</div>
    <div id="adcValue2" class="adc-value">Loading...</div>
  </div>

  <script>
    function getADCValue(valueElementId, endpoint) {
      var xhr = new XMLHttpRequest();
      xhr.onreadystatechange = function() {
        if (this.readyState == 4) { // Request finished
          if (this.status == 200) { // Successful response
            document.getElementById(valueElementId).innerText = this.responseText;
          } else {
            document.getElementById(valueElementId).innerText = "Error"; // Handle error
          }
        }
      };
      xhr.open("GET", endpoint, true);
      xhr.send();
    }

    const updateInterval = %UPDATE_INTERVAL%;

    // Get initial values and set intervals for updates
    getADCValue("adcValue1", "/adc1");
    setInterval(function() { getADCValue("adcValue1", "/adc1"); }, updateInterval);

    getADCValue("adcValue2", "/adc2");
    setInterval(function() { getADCValue("adcValue2", "/adc2"); }, updateInterval);
  </script>
</body>
</html>
)rawliteral";

// Function to replace placeholders in HTML
String processor(const String &var) {
  // Serial.println(var); // For debugging which placeholders are being processed
  if (var == "ADC_PIN_1") {
    return String(cvPin);
  }
  if (var == "ADC_PIN_2") {
    return String(potPin);
  }
  if (var == "UPDATE_INTERVAL") {
    return String(UPDATE_INTERVAL_MS);
  }
  return String();  // Return empty string for unhandled placeholders
}

int getStableAdc(int pin, int samples) {
  long sum = 0;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(pin);
    delayMicroseconds(50);  // Small delay between samples, can be adjusted
  }
  return sum / samples;
}

int getMedianAdc(int pin, int samples) {
  if (samples % 2 == 0) samples++;  // Ensure odd number of samples
  int readings[samples];
  for (int i = 0; i < samples; i++) {
    readings[i] = analogRead(pin);
    delayMicroseconds(50);
  }
  std::sort(readings, readings + samples);
  return readings[samples / 2];
}

const int WAVEFORM_TABLE_SIZE = 512;          // Number of points in the lookup table
const float TARGET_SAMPLE_RATE_HZ = 10250.0;  // How many DAC updates per second
const unsigned long SAMPLE_PERIOD_MICROS = (unsigned long)(1000000.0 / TARGET_SAMPLE_RATE_HZ);

// --- Waveform Types & Parameters ---
enum WaveformType {
  SINE,
  TRIANGLE,
  SAWTOOTH_RISING,  // Added Sawtooth
  SQUARE_50,        // Added Square
  NUM_WAVEFORMS     // Helper to get the count of waveforms
};
WaveformType currentWaveform = SINE;       // Default waveform
WaveformType lastSelectedWaveform = SINE;  // To track changes
float waveformFrequencyHz = 1.0;           // This will be updated by the ADC input

// --- Function Prototype (Forward Declaration) ---
void populateWaveformTable(WaveformType type);  // Fix for compiler error


uint16_t waveformTable[WAVEFORM_TABLE_SIZE];

float phase = 0.0;
float phaseIncrement = 0.0;
unsigned long lastUpdateMicros = 0;

// Function to populate the waveform lookup table
void populateWaveformTable(WaveformType type) {
  Serial.print("Populating table for: ");
  switch (type) {
    case SINE:
      Serial.println("SINE");
      for (int i = 0; i < WAVEFORM_TABLE_SIZE; i++) {
        waveformTable[i] = (uint16_t)(((sin(2.0 * PI * (double)i / WAVEFORM_TABLE_SIZE) + 1.0) / 2.0) * 4095.0);
      }
      break;
    case TRIANGLE:
      Serial.println("TRIANGLE");
      for (int i = 0; i < WAVEFORM_TABLE_SIZE; i++) {
        double val = 0;
        if (i < WAVEFORM_TABLE_SIZE / 2) {
          val = (double)i / (double)(WAVEFORM_TABLE_SIZE / 2);
        } else {
          val = 2.0 - ((double)i / (double)(WAVEFORM_TABLE_SIZE / 2));
        }
        waveformTable[i] = (uint16_t)(val * 4095.0);
        if (waveformTable[i] > 4095) waveformTable[i] = 4095;
      }
      waveformTable[0] = 0;
      break;
    case SAWTOOTH_RISING:  // New case for Sawtooth
      Serial.println("SAWTOOTH_RISING");
      for (int i = 0; i < WAVEFORM_TABLE_SIZE; i++) {
        waveformTable[i] = (uint16_t)(((double)i / (WAVEFORM_TABLE_SIZE - 1)) * 4095.0);
      }
      break;
    case SQUARE_50:  // New case for Square
      Serial.println("SQUARE_50");
      for (int i = 0; i < WAVEFORM_TABLE_SIZE; i++) {
        waveformTable[i] = (i < WAVEFORM_TABLE_SIZE / 2) ? 0 : 4095;
      }
      break;
    default:
      Serial.println("UNKNOWN");
      for (int i = 0; i < WAVEFORM_TABLE_SIZE; i++) {
        waveformTable[i] = 2048;
      }
      break;
  }
}

// --- Configuration Constants ---
const uint8_t MCP4728_ADDRESS = 0x60;  // Default I2C address, adjust if changed

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);  // Wait for serial monitor, if needed
  Serial.println("ESP32 MCP4728 Bass Oscillator Starting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // --- Web Server Handlers ---
  // Handler for the HTML page to display ADC values
  server.on("/adc-view", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", ADC_VIEW_HTML, processor);
  });

  // Handler to get ADC value for Pin 1
  server.on("/adc1", HTTP_GET, [](AsyncWebServerRequest *request) {
    int adcValue = cvValue;
    request->send(200, "text/plain", String(adcValue));
  });

  // Handler to get ADC value for Pin 2
  server.on("/adc2", HTTP_GET, [](AsyncWebServerRequest *request) {
    int adcValue = potValue;
    request->send(200, "text/plain", String(adcValue));
  });

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = "Hi! I am ESP32.<br>";
    html += "<a href=\"/update\">OTA Update</a><br>";
    html += "<a href=\"/adc-view\">ADC Monitor</a>";
    request->send(200, "text/html", html);
  });

  server.begin();

  // Initialize I2C
  Wire.begin();            // ESP32 default SDA, SCL pins
  Wire.setClock(1000000);  // Set I2C to 1MHz (Fast Mode Plus) if stable, or 400000 for 400kHz

  // Initialize MCP4728
  if (!mcp.begin(MCP4728_ADDRESS)) {
    Serial.println("Failed to find MCP4728 chip. Halting.");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MCP4728 Found!");

  ElegantOTA.begin(&server);
  delay(2000);
  mcp.setChannelValue(MCP4728_CHANNEL_A, clock_low);
  mcp.setChannelValue(MCP4728_CHANNEL_B, clock_low);
  mcp.setChannelValue(MCP4728_CHANNEL_C, clock_low);
  mcp.setChannelValue(MCP4728_CHANNEL_D, clock_low);
  Serial.println("Setup complete. Oscillator running.");
  pinMode(potPin, INPUT);
  pinMode(cvPin, INPUT);

  populateWaveformTable(currentWaveform);
  lastSelectedWaveform = currentWaveform;  // Initialize tracker

  // Initial calculation for phaseIncrement
  phaseIncrement = (waveformFrequencyHz * (float)WAVEFORM_TABLE_SIZE) / TARGET_SAMPLE_RATE_HZ;
}

void loop() {
  ElegantOTA.loop();
  potValue = analogRead(potPin);
  cvValue = analogRead(cvPin);
  delay_value = 250 - ((4096 - potValue) / 20);
  switch2 = analogRead(switchPinUp);
  switch1 = analogRead(switchPinDown);

  if (switch1 > 500) {
    // --- Check for waveform change ---
    int waveformCvAdcValue = potValue;
    // Map ADC value to a waveform type. Simple division is used here.
    int segmentSize = 4096 / (int)NUM_WAVEFORMS;
    WaveformType selectedWaveform = (WaveformType)(waveformCvAdcValue / segmentSize);
    if (selectedWaveform >= NUM_WAVEFORMS) {  // Safeguard for max ADC value
      selectedWaveform = (WaveformType)((int)NUM_WAVEFORMS - 1);
    }

    if (selectedWaveform != lastSelectedWaveform) {
      currentWaveform = selectedWaveform;
      populateWaveformTable(currentWaveform);
      lastSelectedWaveform = currentWaveform;
      phase = 0.0;  // Reset phase on waveform change to avoid glitches
    }

    waveformFrequencyHz = getValueFromRange(cvValue);

    // Recalculate phaseIncrement based on the new frequency
    phaseIncrement = (waveformFrequencyHz * (float)WAVEFORM_TABLE_SIZE) / TARGET_SAMPLE_RATE_HZ;

    // --- Timed DAC Update ---
    unsigned long currentTimeMicros = micros();
    if (currentTimeMicros - lastUpdateMicros >= SAMPLE_PERIOD_MICROS) {
      lastUpdateMicros += SAMPLE_PERIOD_MICROS;  // More stable timing

      // Update phase
      phase += phaseIncrement;

      // Wrap phase
      if (phase >= (float)WAVEFORM_TABLE_SIZE) {
        phase -= (float)WAVEFORM_TABLE_SIZE;
      }

      // Get value from lookup table
      uint16_t cvdacValue = waveformTable[(uint16_t)phase];

      // Send value to the specified MCP4728 channel
      if (!mcp.setChannelValue(MCP4728_CHANNEL_C, cvdacValue)) {
        // Serial.println("Failed to write to DAC channel"); // For debugging
      }
    }
  }

  if (switch2 > 500) {
    mcp.setChannelValue(MCP4728_CHANNEL_B, clock_high);
    if (counterb == 4) {
      //Serial.println(counterb);
      mcp.setChannelValue(MCP4728_CHANNEL_C, clock_high);
      counterb = 0;
    }
    if (counterc == 8) {
      //Serial.println(counterc);
      mcp.setChannelValue(MCP4728_CHANNEL_A, clock_high);
      counterc = 0;
    }
    if (counterd == 3) {
      //Serial.println(countercd
      mcp.setChannelValue(MCP4728_CHANNEL_D, clock_high);
      counterd = 0;
    }
    counterb = counterb + 1;
    counterc = counterc + 1;
    counterd = counterd + 1;
    delay(delay_value/8);
    mcp.setChannelValue(MCP4728_CHANNEL_A, clock_low);
    mcp.setChannelValue(MCP4728_CHANNEL_B, clock_low);
    mcp.setChannelValue(MCP4728_CHANNEL_C, clock_low);
    mcp.setChannelValue(MCP4728_CHANNEL_D, clock_low);
    delay(delay_value/8);
  }
}
