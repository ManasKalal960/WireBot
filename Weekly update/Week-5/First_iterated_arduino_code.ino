#include <AFMotor.h>
#include <NewPing.h>

// Define motors connected to the motor shield
AF_DCMotor weightBalanceMotor(1);   // M1 for weight shifting
AF_DCMotor arm1Motor(2);            // M2 for arm 1 engage/disengage
AF_DCMotor arm2Motor(3);            // M3 for arm 2 engage/disengage
AF_DCMotor arm3Motor(4);            // M4 for arm 3 engage/disengage

// Relay pin for main driving motor
const int relayPin = 20;

// IR Sensors for wire detection (close range, 1-2 cm)
const int irSensor1 = A10;
const int irSensor2 = A9;
const int irSensor3 = A11;
const int threshold = 500;  // Define IR sensor threshold for detection

// Ultrasonic sensors for junction detection (4-5 cm away)
#define TRIGGER_PIN_1 14
#define ECHO_PIN_1 15
#define TRIGGER_PIN_2 16
#define ECHO_PIN_2 17
#define TRIGGER_PIN_3 18
#define ECHO_PIN_3 19

// Ultrasonic sensor range
#define MAX_DISTANCE 50 // Maximum distance for junction detection (5 cm)

// Create NewPing objects for each ultrasonic sensor
NewPing sonar1(TRIGGER_PIN_1, ECHO_PIN_1, MAX_DISTANCE);
NewPing sonar2(TRIGGER_PIN_2, ECHO_PIN_2, MAX_DISTANCE);
NewPing sonar3(TRIGGER_PIN_3, ECHO_PIN_3, MAX_DISTANCE);

void setup() {
  Serial.begin(9600);

  pinMode(relayPin, OUTPUT);  // Relay for main driving motor
  digitalWrite(relayPin, HIGH); // Start main driving motor initially

  pinMode(irSensor1, INPUT);
  pinMode(irSensor2, INPUT);
  pinMode(irSensor3, INPUT);
}

void loop() {
  // Read ultrasonic sensors for junction detection
  int distance1 = sonar1.ping_cm();
  int distance2 = sonar2.ping_cm();
  int distance3 = sonar3.ping_cm();

  // Print ultrasonic sensor distances to Serial Monitor
  Serial.print("Ultrasonic Sensor 1 Distance: ");
  Serial.print(distance1);
  Serial.println(" cm");
  Serial.print("Ultrasonic Sensor 2 Distance: ");
  Serial.print(distance2);
  Serial.println(" cm");
  Serial.print("Ultrasonic Sensor 3 Distance: ");
  Serial.print(distance3);
  Serial.println(" cm");

  // Read IR sensors and print values to Serial Monitor
  int irReading1 = analogRead(irSensor1);
  int irReading2 = analogRead(irSensor2);
  int irReading3 = analogRead(irSensor3);

  Serial.print("IR Sensor 1 Reading: ");
  Serial.println(irReading1);
  Serial.print("IR Sensor 2 Reading: ");
  Serial.println(irReading2);
  Serial.print("IR Sensor 3 Reading: ");
  Serial.println(irReading3);

  // Default case: Move forward if no junction detected by any ultrasonic sensor
  if (distance1 > 5 && distance2 > 5 && distance3 > 5) {
    digitalWrite(relayPin, HIGH);  // Ensure main driving motor is running
  }

  // If Ultrasonic Sensor 1 detects a junction
  if (distance1 > 0 && distance1 <= 5) { 
    digitalWrite(relayPin, LOW);  // Stop main driving motor
    
    // Step 1: Shift weight between Arm 2 and Arm 3
    weightBalanceMotor.setSpeed(100);
    weightBalanceMotor.run(FORWARD);
    delay(2000);  // Adjust weight shift time

    // Step 2: Disengage Arm 1
    arm1Motor.setSpeed(100);
    arm1Motor.run(FORWARD);
    delay(1000);  // Adjust disengagement time for Arm 1

    // Step 3: Move forward to cross the junction
    digitalWrite(relayPin, HIGH);  // Start main driving motor again
    delay(2000);  // Time to cross the junction

    // Step 4: Re-engage Arm 1 when Ultrasonic Sensor 2 detects the junction
    if (distance2 > 0 && distance2 <= 5) {
      arm1Motor.run(BACKWARD);  // Re-engage Arm 1
      delay(1000);  // Adjust re-engagement time

      // Check IR sensor on Arm 1 for correct alignment
      if (irReading1 < threshold) {
        // Arm 1 aligned; proceed to shift weight back to Arm 1 and Arm 3
        weightBalanceMotor.run(BACKWARD);
        delay(2000);  // Time to shift weight back to initial position
      }

      // Step 5: Disengage Arm 2
      arm2Motor.setSpeed(100);
      arm2Motor.run(FORWARD);
      delay(1000);  // Adjust disengagement time for Arm 2

      // Step 6: Continue moving forward to cross the junction
      digitalWrite(relayPin, HIGH);
      delay(2000);

      // Step 7: Re-engage Arm 2 when Ultrasonic Sensor 3 detects the junction
      if (distance3 > 0 && distance3 <= 5) {
        arm2Motor.run(BACKWARD);  // Re-engage Arm 2
        delay(1000);

        // Check IR sensor on Arm 2 for alignment
        if (irReading2 < threshold) {
          // Arm 2 aligned; shift weight to Arm 1 and Arm 2
          weightBalanceMotor.run(FORWARD);
          delay(2000);
        }

        // Step 8: Disengage Arm 3
        arm3Motor.setSpeed(100);
        arm3Motor.run(FORWARD);
        delay(1000);

        // Step 9: Move forward to cross the junction
        digitalWrite(relayPin, HIGH);
        delay(2000);

        // Step 10: Re-engage Arm 3 after crossing the junction
        arm3Motor.run(BACKWARD);  // Re-engage Arm 3
        delay(1000);

        // Check IR sensor on Arm 3 for alignment
        if (irReading3 < threshold) {
          // Arm 3 aligned; ready for next cycle
          weightBalanceMotor.run(BACKWARD);
          delay(2000); // Shift weight back to center
        }
      }
    }
  }

  // Delay for readability in Serial Monitor
  delay(500);
}
