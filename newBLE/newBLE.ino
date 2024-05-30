///
///   ble_shining_kb.ino
///   
///   created: 2020-11
///
///  Simulate an evil writing machine in an isolated hotel on winter. 
///  No hardware required other than an Nano33 ble.
///

#include "Nano33BleHID.h"
#include "signal_utils.h"

/* -------------------------------------------------------------------------- */

Nano33BleKeyboard bleKb("Music ball");

// Tracking index for the end of the writing animation ticker.
int sTickerIndex = -1;

static const char kSentence[] = "      ";
static const int kNumTickerSteps = 4;

// How long it takes for the sentence to be written.
static const int kSentenceDurationMilliseconds = 4029;

// How long it takes before the sentence is rewritten.
static const int kSentenceDelayMilliseconds = 1977;

// How long one writing animation will run.
static const int kSentenceTotalTimeMilliseconds = kSentenceDurationMilliseconds + kSentenceDelayMilliseconds;

// Safeguard to terminate this mess of an app before going crazy.
static const int kTotalRuntime = 8 * kSentenceTotalTimeMilliseconds;


/* -------------------------------------------------------------------------- */

int before; 
void setup()
{
  letter = 'K'; 
  // General setup.
  pinMode(LED_BUILTIN, OUTPUT);

  // Initialize both BLE and the HID.
  bleKb.initialize();

  // Launch the event queue that will manage both BLE events and the loop. 
  // After this call the main thread will be halted.
  MbedBleHID_RunEventThread();
  before = millis(); 
}

void loop()
{
  // When disconnected, we animate the builtin LED to indicate the device state.
  if (bleKb.connected() == false) {
    return;
  }

  // Stop when we reach a certain runtime (better safe than sorry).
  if (bleKb.connection_time() > kTotalRuntime) {
    return;
  }

  // Retrieve the HIDService to update.
  auto *kb = bleKb.hid();

  // The animation is divided in two parts :
  if (before - millis() >= 3000)
  {
    // Write the sentence using the StringWriter object.
    &kb.sendCharacter(letter);
  }
}

/* -------------------------------------------------------------------------- */
