/*
 This example displays a more manual method of adjusting the way in which the
 MAX30101 gathers data. Specifically we'll look at how to modify the pulse
 length of the LEDs within the MAX30101 which impacts the number of samples 
 that can be gathered, so we'll adjust this value as well. In addition we 
 gather additional data from the bioData type: LED samples. This data gives 
 the number of samples gathered by the MAX30101 for both the red and IR LEDs. 
 As a side note you can also choose MODE_ONE and MODE_TWO for configSensorBpm
 as well.
 
 Modified to collect only readings with confidence > 50, take 5 such readings,
 and display their averages every second.
 
 A summary of the hardware connections are as follows: 
 SDA -> SDA
 SCL -> SCL
 RESET -> PIN 4
 MFIO -> PIN 5

 Author: Elias Santistevan (Modified)
 Date: 8/2019
 SparkFun Electronics

 If you run into an error code check the following table to help diagnose your
 problem: 
 1 = Unavailable Command
 2 = Unavailable Function
 3 = Data Format Error
 4 = Input Value Error
 5 = Try Again
 255 = Error Unknown
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

bioData body; 

// Variables for averaging
const int TARGET_READINGS = 5;  // Number of valid readings to collect
int validReadingsCount = 0;
float heartRateSum = 0;
float oxygenSum = 0;
float irLedSum = 0;
float redLedSum = 0;
unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 1000; // Display every 1 second

void setup(){

  Serial.begin(115200);

  Wire.begin();
  int result = bioHub.begin();
  if (result == 0) // Zero errors!
    Serial.println("Sensor started!");

  Serial.println("Configuring Sensor...."); 
  int error = bioHub.configSensorBpm(MODE_ONE); // Configure Sensor and BPM mode , MODE_TWO also available
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

  // Set sample rate per second. Remember that not every sample rate is
  // available with every pulse width. Check hookup guide for more information.  
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
  
  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 
  Serial.println("Loading up the buffer with data....");
  delay(4000);
  
  // Additional stabilization - let sensor readings settle
  Serial.println("Allowing sensor to stabilize...");
  for(int i = 0; i < 20; i++) {
    body = bioHub.readSensorBpm();
    Serial.print("Stabilization reading - HR: ");
    Serial.print(body.heartrate);
    Serial.print(", Confidence: ");
    Serial.print(body.confidence);
    Serial.print(", Status: ");
    Serial.println(body.status);
    delay(100);
  }
  
  lastDisplayTime = millis();
  Serial.println("Starting data collection...");
  Serial.println("Only readings with confidence > 50 will be used for averaging.");
  Serial.println("Displaying averages of 5 valid readings every second.");
  Serial.println("----------------------------------------");

}

void loop(){

    // Information from the readSensor function will be saved to our "body"
    // variable.  
    body = bioHub.readSensorBpm();
    
    // Only process readings with confidence > 50 AND non-zero values
    if (body.confidence > 50 && body.heartrate > 0 && body.oxygen > 0) {
        // Add to running totals
        heartRateSum += body.heartrate;  // Fixed: lowercase 'r'
        oxygenSum += body.oxygen;
        irLedSum += body.irLed;
        redLedSum += body.redLed;
        validReadingsCount++;
        
        Serial.print("Valid reading #");
        Serial.print(validReadingsCount);
        Serial.print(" - HR: ");
        Serial.print(body.heartrate);  // Fixed: lowercase 'r'
        Serial.print(", O2: ");
        Serial.print(body.oxygen);
        Serial.print(", Confidence: ");
        Serial.println(body.confidence);
    } else {
        // Debug: Show why reading was rejected
        Serial.print("Rejected - HR: ");
        Serial.print(body.heartrate);
        Serial.print(", O2: ");
        Serial.print(body.oxygen);
        Serial.print(", Confidence: ");
        Serial.print(body.confidence);
        Serial.print(", Status: ");
        Serial.println(body.status);
    }
    
    // Check if we should display averages (every second OR when we have 5 readings)
    unsigned long currentTime = millis();
    if ((currentTime - lastDisplayTime >= DISPLAY_INTERVAL) || (validReadingsCount >= TARGET_READINGS)) {
        
        if (validReadingsCount > 0) {
            // Calculate averages
            float avgHeartRate = heartRateSum / validReadingsCount;
            float avgOxygen = oxygenSum / validReadingsCount;
            float avgIrLed = irLedSum / validReadingsCount;
            float avgRedLed = redLedSum / validReadingsCount;
            
            // Display averages
            Serial.println("========================================");
            Serial.print("AVERAGES (");
            Serial.print(validReadingsCount);
            Serial.println(" valid readings):");
            Serial.print("Average Heart Rate: ");
            Serial.print(avgHeartRate, 1);
            Serial.println(" BPM");
            Serial.print("Average Blood Oxygen: ");
            Serial.print(avgOxygen, 1);
            Serial.println("%");
            Serial.print("Average IR LED counts: ");
            Serial.println(avgIrLed, 0);
            Serial.print("Average Red LED counts: ");
            Serial.println(avgRedLed, 0);
            Serial.println("========================================");
        } else {
            Serial.println("No valid readings (confidence > 50) in the last second.");
        }
        
        // Reset for next collection period
        validReadingsCount = 0;
        heartRateSum = 0;
        oxygenSum = 0;
        irLedSum = 0;
        redLedSum = 0;
        lastDisplayTime = currentTime;
    }
    
    // Adjust delay to get approximately 5 readings per second when confidence > 50
    // This assumes roughly 50% of readings will have confidence > 50
    // You may need to adjust this based on your actual sensor performance
    delay(100); // 100ms delay = ~10 readings/second, hoping ~5 will have confidence > 50
}
