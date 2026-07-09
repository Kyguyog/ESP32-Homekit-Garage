#include "HomeSpan.h"

// Custom structure that teaches HomeKit how to talk to your physical relay
struct DEV_GarageDoor : Service::GarageDoorOpener {
  SpanCharacteristic *current;
  SpanCharacteristic *target;
  SpanCharacteristic *obstruction;
  int relayPin;

  // Constructor: Runs when the ESP32 boots up
  DEV_GarageDoor(int pin) : Service::GarageDoorOpener() {
    relayPin = pin;
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW); // Assumes Active High relay (starts OFF)

    // 0 = Open, 1 = Closed, 2 = Opening, 3 = Closing
    current = new Characteristic::CurrentDoorState(1); // Default to Closed
    target = new Characteristic::TargetDoorState(1);   // Default to Closed
    obstruction = new Characteristic::ObstructionDetected(false);
    
    Serial.println("HomeKit Garage Door Opener Ready!");
  }

  // Runs whenever you tap the button in the Apple Home App
  boolean update() override {
    Serial.println("HomeKit Command Received! Pulsing Relay...");
    
    // 1. Momentary Pulse (simulates a physical button press)
    digitalWrite(relayPin, HIGH); // Turn relay ON
    delay(300);                   // Wait 300ms
    digitalWrite(relayPin, LOW);  // Turn relay OFF

    // 2. Tell HomeKit to show the opening/closing animation
    if (target->getNewVal() == 0) {
      current->setVal(2); // Set status to "Opening..."
    } else {
      current->setVal(3); // Set status to "Closing..."
    }
    return true;
  }

  // Loops continuously in the background
  void loop() override {
    // Because we don't have door sensors yet, we simulate the door taking
    // 12 seconds to finish opening or closing.
    if (current->getVal() == 2 && target->timeVal() > 10000) {
      current->setVal(0); // Tell HomeKit the door is now fully OPEN
      Serial.println("Door is now open.");
    }
    if (current->getVal() == 3 && target->timeVal() > 10000) {
      current->setVal(1); // Tell HomeKit the door is now fully CLOSED
      Serial.println("Door is now closed.");
    }
  }
};

void setup() {
  Serial.begin(115200);
  
  // Set your custom 8-digit HomeKit pairing code
  homeSpan.setPairingCode("86359643"); 
  
  //Enable Captive Portal
  homeSpan.enableAutoStartAP();

  // Initialize HomeSpan under the Garage Door category
  homeSpan.begin(Category::GarageDoorOpeners, "Smart Garage");

  // Build the virtual Apple Accessory
  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Manufacturer("ESP32 Garage Door");
      new Characteristic::Model("ESP32");
    
    // Create the garage door object on GPIO pin 5
    new DEV_GarageDoor(5); 
}

void loop() {
  // Keeps HomeSpan running and listening for Apple devices
  homeSpan.poll();
}
