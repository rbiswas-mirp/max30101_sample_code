/*
 Modified example that filters readings by confidence (>50) and averages
 the top 5 readings by confidence every second.
 
 Hardware connections: 
 SDA -> SDA
 SCL -> SCL
 RESET -> PIN 4
 MFIO -> PIN 5

 Modified to include confidence filtering and averaging
*/

#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>

// Reset pin, MFIO pin
int resPin = 4;
int mfioPin = 5;

// Possible widths: 69, 118, 215, 411us
int width = 411; 
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
int samples = 400; 
int pulseWidthVal;
int sampleVal;

// Structure to store readings with confidence
struct Reading {
  int heartRate;
  int confidence;
  int oxygen;
  int irLed;
  int redLed;
  int status;
};

// Array to store readings for averaging
const int MAX_READINGS = 100;  // Buffer size for collecting readings
Reading readings[MAX_READINGS];
int readingCount = 0;
unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 1000; // Display every 1 second

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 

bioData body;

// Function to sort readings by confidence (descending)
void sortReadingsByConfidence(Reading arr[], int n) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[j].confidence < arr[j + 1].confidence) {
        Reading temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

void setup(){
  Serial.begin(115200);

  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");

  Serial.println("Configuring Sensor...."); 
  int error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode
  if (error == 0){// Zero errors.
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Set pulse width.
  error = bioHub.setPulseWidth(width);
  if (error == 0){// Zero errors.
    Serial.println("Pulse Width Set.");
  }
  else {
    Serial.println("Could not set Pulse Width.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check that the pulse width was set. 
  pulseWidthVal = bioHub.readPulseWidth();
  Serial.print("Pulse Width: ");
  Serial.println(pulseWidthVal);

  // Set sample rate per second.
  error = bioHub.setSampleRate(samples);
  if (error == 0){// Zero errors.
    Serial.println("Sample Rate Set.");
  }
  else {
    Serial.println("Could not set Sample Rate!");
    Serial.print("Error: "); 
    Serial.println(error); 
  }

  // Check sample rate.
  sampleVal = bioHub.readSampleRate();
  Serial.print("Sample rate is set to: ");
  Serial.println(sampleVal); 
  
  Serial.println("Loading up the buffer with data....");
  delay(4000);

  lastDisplayTime = millis();
}

void loop(){
  // Read sensor data
  body = bioHub.readSensorBpm();
  
  // Only store readings with confidence > 50
  if (body.confidence > 50 && readingCount < MAX_READINGS) {
    readings[readingCount].heartRate = body.heartRate;
    readings[readingCount].confidence = body.confidence;
    readings[readingCount].oxygen = body.oxygen;
    readings[readingCount].irLed = body.irLed;
    readings[readingCount].redLed = body.redLed;
    readings[readingCount].status = body.status;
    readingCount++;
  }
  
  // Check if it's time to display averaged results (every 1 second)
  unsigned long currentTime = millis();
  if (currentTime - lastDisplayTime >= DISPLAY_INTERVAL && readingCount > 0) {
    
    // Sort readings by confidence
    sortReadingsByConfidence(readings, readingCount);
    
    // Calculate averages from top 5 readings (or fewer if less than 5 available)
    int numToAverage = min(5, readingCount);
    float avgHeartRate = 0;
    float avgOxygen = 0;
    float avgConfidence = 0;
    long avgIrLed = 0;
    long avgRedLed = 0;
    
    for (int i = 0; i < numToAverage; i++) {
      avgHeartRate += readings[i].heartRate;
      avgOxygen += readings[i].oxygen;
      avgConfidence += readings[i].confidence;
      avgIrLed += readings[i].irLed;
      avgRedLed += readings[i].redLed;
    }
    
    avgHeartRate /= numToAverage;
    avgOxygen /= numToAverage;
    avgConfidence /= numToAverage;
    avgIrLed /= numToAverage;
    avgRedLed /= numToAverage;
    
    // Display averaged results
    Serial.println("\n=== AVERAGED RESULTS (Top " + String(numToAverage) + " readings) ===");
    Serial.print("Average Heartrate: ");
    Serial.println(avgHeartRate, 1);
    Serial.print("Average Blood Oxygen: ");
    Serial.println(avgOxygen, 1);
    Serial.print("Average Confidence: ");
    Serial.println(avgConfidence, 1);
    Serial.print("Average IR LED: ");
    Serial.println(avgIrLed);
    Serial.print("Average Red LED: ");
    Serial.println(avgRedLed);
    Serial.print("Total readings collected: ");
    Serial.println(readingCount);
    Serial.println("================================\n");
    
    // Reset for next second
    readingCount = 0;
    lastDisplayTime = currentTime;
  }
  
  // Small delay to prevent overwhelming the sensor
  // With 400 samples/second, we can afford a small delay
  delay(10);
}
