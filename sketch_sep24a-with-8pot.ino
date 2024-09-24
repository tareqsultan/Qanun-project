#include <Arduino.h>

// Define the pins connected to the 74HC4051 control inputs
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;

// Define the pin connected to the multiplexer's output
const int outputPin = A10;

void setup() {
  Serial.begin(9600);

  // Initialize the control input pins as outputs
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
}

void loop() {
  // Iterate through all 8 input channels
  for (int i = 0; i < 8; i++) {
    // Set the control signals to select the current input channel
    digitalWrite(S0, i & 0x01);
    digitalWrite(S1, (i >> 1) & 0x01);
    digitalWrite(S2, (i >> 2) & 0x01);

    // Read the value from the output pin
    int value = analogRead(outputPin);

    // Print the value to the serial monitor
    Serial.println(value);
  }

  // Add a delay between iterations
  delay(1000);
}