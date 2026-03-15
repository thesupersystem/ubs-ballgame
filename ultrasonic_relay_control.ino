/*
 * ============================================================================
 * ULTRASONIC SENSOR + DUAL RELAY CONTROL
 * ============================================================================
 * This sketch controls:
 * 1. Relay 1 that activates for 5 seconds when ultrasonic sensor detects object
 * 2. Relay 2 that activates for a set duration when button is pressed
 * ============================================================================
 */

// ============================================================================
// WIRING CONFIGURATION
// ============================================================================
/*
 * SHARED 5V POWER SUPPLY CONFIGURATION:
 * 
 * Power Supply Connections:
 *   - 5V+ → Sensor VCC, Relay Modules VCC
 *   - GND → Sensor GND, Relay Modules GND, Arduino GND
 * 
 * CRITICAL: Arduino GND MUST be connected to Power Supply GND for common reference!
 * 
 * Signal Connections to Arduino:
 *   
 *   Ultrasonic Sensor:
 *     - Trig → Arduino Pin 7
 *     - Echo → Arduino Pin 6
 *   
 *   Relay 1 (Ultrasonic triggered):
 *     - Signal → Arduino Pin 5
 *   
 *   Relay 2 (Button triggered):
 *     - Signal → Arduino Pin 4
 *   
 *   Push Button Switch:
 *     - One side → Arduino Pin 3
 *     - Other side → GND (using internal pullup resistor)
 * 
 * Power Supply Requirements:
 *   - Voltage: 5V DC (do not exceed 5.5V)
 *   - Current: Minimum 1A recommended
 *     * HC-SR04 sensor: ~15mA
 *     * Relay module: ~70-80mA when active (x2 = ~160mA)
 *     * Total: ~200mA (1A supply provides good headroom)
 */

// ============================================================================
// ULTRASONIC SENSOR CONFIGURATION
// ============================================================================
#define TRIG_PIN 7
#define ECHO_PIN 6

#define DISTANCE_THRESHOLD 10  // Distance in centimeters to trigger relay

// ============================================================================
// RELAY 1 CONFIGURATION (Ultrasonic triggered)
// ============================================================================
#define RELAY_1_PIN 5
#define RELAY_1_ON_TIME 5000  // Duration in milliseconds (5 seconds)

// ============================================================================
// RELAY 2 CONFIGURATION (Button triggered)
// ============================================================================
#define SWITCH_PIN 3
#define RELAY_2_PIN 4
#define RELAY_2_ON_TIME 1500  // Duration in milliseconds (1.5 seconds)

// ============================================================================
// GLOBAL VARIABLES
// ============================================================================
// Button state tracking for relay 2 control
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;

// Relay 1 state tracking (ultrasonic triggered)
unsigned long relay1OnStartTime = 0;
bool relay1Active = false;

// Relay 2 state tracking (button triggered)
unsigned long relay2OnStartTime = 0;
bool relay2Active = false;

// ============================================================================
// SETUP - Runs once at startup
// ============================================================================
void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);
  
  // ---- Ultrasonic Sensor Setup ----
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // ---- Relay 1 Setup (Ultrasonic triggered) ----
  pinMode(RELAY_1_PIN, OUTPUT);
  digitalWrite(RELAY_1_PIN, LOW);  // Ensure relay starts OFF
  
  // ---- Relay 2 Setup (Button triggered) ----
  pinMode(SWITCH_PIN, INPUT_PULLUP);  // Button with internal pullup
  pinMode(RELAY_2_PIN, OUTPUT);
  digitalWrite(RELAY_2_PIN, LOW);     // Ensure relay starts OFF
  
  Serial.println("System initialized - both relays OFF");
}

// ============================================================================
// MAIN LOOP - Runs continuously
// ============================================================================
void loop() {
  // ---- Handle Ultrasonic Sensor and Relay 1 Control ----
  long distance = getDistance(TRIG_PIN, ECHO_PIN);
  
  // Debug output for sensor readings
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm | Relay 1: ");
  Serial.print(relay1Active ? "ON" : "OFF");
  Serial.print(" | Relay 2: ");
  Serial.println(relay2Active ? "ON" : "OFF");
  
  // Check if sensor detects object within threshold
  if (distance <= DISTANCE_THRESHOLD) {
    // Object detected - activate relay 1 and start/reset timer
    if (!relay1Active) {
      digitalWrite(RELAY_1_PIN, HIGH);  // Turn relay ON
      relay1Active = true;
      relay1OnStartTime = millis();
      Serial.println("Object detected - Relay 1 ON");
    } else {
      // Already active, reset the timer (extends duration)
      relay1OnStartTime = millis();
    }
  }
  
  // Check if relay 1 duration has expired
  if (relay1Active && (millis() - relay1OnStartTime >= RELAY_1_ON_TIME)) {
    digitalWrite(RELAY_1_PIN, LOW);  // Turn relay OFF
    relay1Active = false;
    Serial.println("Relay 1 duration expired - OFF");
  }
  
  // ---- Handle Button Press and Relay 2 Control ----
  currentButtonState = digitalRead(SWITCH_PIN);
  
  // Detect button press (falling edge - button pressed)
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    digitalWrite(RELAY_2_PIN, HIGH);  // Turn relay ON
    relay2OnStartTime = millis();
    relay2Active = true;
    Serial.println("Button pressed - Relay 2 ON");
    delay(50);  // Debounce delay
  }
  
  // Auto turn off relay 2 after set duration
  if (relay2Active && (millis() - relay2OnStartTime >= RELAY_2_ON_TIME)) {
    digitalWrite(RELAY_2_PIN, LOW);  // Turn relay OFF
    relay2Active = false;
    Serial.println("Relay 2 auto OFF");
  }
  
  // Update button state for next iteration
  lastButtonState = currentButtonState;
  
  delay(100);  // Small delay between readings
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

// Function to get distance from ultrasonic sensor
long getDistance(int trigPin, int echoPin) {
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Send 10 microsecond pulse
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the echo pin
  long duration = pulseIn(echoPin, HIGH, 30000);  // 30ms timeout
  
  // Calculate distance in centimeters
  // Speed of sound is 343 m/s or 29.1 microseconds per centimeter
  // Divide by 2 because sound travels to object and back
  long distance = duration * 0.034 / 2;
  
  // Return 999 if no echo received (out of range)
  if (duration == 0) {
    return 999;
  }
  
  return distance;
}
