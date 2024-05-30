#include "Nano33BleHID.h"
#include "signal_utils.h"

/* -------------------------------------------------------------------------- */

Nano33BleKeyboard bleKb("Music ball");


char letter = ' '; 
// Safeguard to terminate this mess of an app before going crazy.

void setup()
{
  // Initialize both BLE and the HID.
  bleKb.initialize();

  // Launch the event queue that will manage both BLE events and the loop. 
  // After this call the main thread will be halted.
  MbedBleHID_RunEventThread();

}

void loop(){
  // When disconnected, we animate the builtin LED to indicate the device state.
  if (bleKb.connected() == false) {
    return;
  }

  // Retrieve the HIDService to update.
  auto *kb = bleKb.hid();

    // Write the sentence using the StringWriter object.
    kb->sendCharacter(letter);
  
}