/*
  This example has been modified to filter and average biometric data from
  the MAX30101 sensor.

  The code now performs the following steps:
  1. It continuously reads data from the sensor.
  2. It only considers readings where the 'confidence' value is greater than 50.
  3. It maintains a list of the top 5 readings with the highest confidence
     scores within a one-second window.
  4. Once per second, it calculates the average heart rate and blood oxygen
     from these top 5 readings and prints the result to the Serial Monitor.
  5. The list is then cleared to gather the best readings for the next second.

  This approach provides a more stable and reliable output by discarding
  low-confidence data and smoothing the results.

  Original Author: Elias Santistevan / SparkFun Electronics
  Modified By: Google's Gemini
  Date: 8/2025
*/

#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>

// Reset pin, MFIO pin
int resPin = 4;
int mfioPin = 5;

// Possible widths: 69, 118, 215, 411us
int width = 411;
// Possible samples: 50, 100, 200, 400, 800, 1000, 1600, 3200 samples/second
// Not every sample amount is possible with every width; check out our hookup
// guide for more information.
int samples = 400;
int pulseWidthVal;
int sampleVal;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin);

bioData body; // This struct will hold the most recent sensor reading.

// MODIFICATION: Create an array to store the top 5 readings based on confidence.
const int NUM_READINGS = 5;
bioData topReadings[NUM_READINGS];

// MODIFICATION: Variables for timing the one-second report.
unsigned long lastReportTime = 0;
const long reportInterval = 1000; // 1 second in milliseconds

void setup() {

  Serial.begin(115200);

  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");

  Serial.println("Configuring Sensor....");
  int error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode , MODE_TWO also available
  if (error == 0) { // Zero errors.
    Serial.println("Sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: ");
    Serial.println(error);
  }

  // Set pulse width.
  error = bioHub.setPulseWidth(width);
  if (error == 0) { // Zero errors.
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
  if (error == 0) { // Zero errors.
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

  // MODIFICATION: Initialize the topReadings array with 0 confidence.
  // This ensures the first valid readings will be stored.
  for (int i = 0; i < NUM_READINGS; i++) {
    topReadings[i].confidence = 0;
    topReadings[i].heartRate = 0;
    topReadings[i].oxygen = 0;
  }
  
  Serial.println("\nNow collecting data. Averaged report will be displayed every second.");
  Serial.println("Make sure your finger is placed firmly on the sensor.");
  
  // Data lags a bit behind the sensor, if your finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up.
  delay(2000);

}

void loop() {
  
  // Continuously read the sensor.
  body = bioHub.readSensorBpm();

  // MODIFICATION: Only process readings with confidence > 50.
  if (body.confidence > 50) {
    // Find the reading in our array with the lowest confidence.
    int minConfidenceIndex = 0;
    for (int i = 1; i < NUM_READINGS; i++) {
      if (topReadings[i].confidence < topReadings[minConfidenceIndex].confidence) {
        minConfidenceIndex = i;
      }
    }

    // If the new reading's confidence is higher than the lowest one in our
    // array, replace the lowest one.
    if (body.confidence > topReadings[minConfidenceIndex].confidence) {
      topReadings[minConfidenceIndex] = body;
    }
  }

  // MODIFICATION: Report and reset every second using millis().
  // This is non-blocking and allows the code above to run as fast as possible.
  if (millis() - lastReportTime >= reportInterval) {
    lastReportTime = millis(); // Update the time of the last report.

    float totalHeartRate = 0;
    float totalOxygen = 0;
    int validReadingsCount = 0;

    // Sum up the values from our top 5 array.
    for (int i = 0; i < NUM_READINGS; i++) {
      // Only include readings that have been populated (confidence > 0).
      if (topReadings[i].confidence > 0) {
        totalHeartRate += topReadings[i].heartRate;
        totalOxygen += topReadings[i].oxygen;
        validReadingsCount++;
      }
    }

    Serial.println("----------------------------------------");
    // Only display an average if we have collected enough valid data.
    if (validReadingsCount == NUM_READINGS) {
      float avgHeartRate = totalHeartRate / NUM_READINGS;
      float avgOxygen = totalOxygen / NUM_READINGS;

      Serial.print("Time: ");
      Serial.print(millis() / 1000);
      Serial.println("s");

      Serial.println("Averaged Report (from top 5 readings):");
      
      Serial.print("Average Heart Rate: ");
      Serial.println(avgHeartRate, 2); // Print with 2 decimal places.

      Serial.print("Average SpO2: ");
      Serial.println(avgOxygen, 2); // Print with 2 decimal places.

    }
    else {
      Serial.print("Collecting high-confidence data... (");
      Serial.print(validReadingsCount);
      Serial.print("/");
      Serial.print(NUM_READINGS);
      Serial.println(" collected)");
    }
     Serial.println("----------------------------------------");


    // Reset the array to start fresh for the next second.
    for (int i = 0; i < NUM_READINGS; i++) {
      topReadings[i].confidence = 0;
      topReadings[i].heartRate = 0;
      topReadings[i].oxygen = 0;
    }
  }
}
