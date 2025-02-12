#include <MIDIUSB.h>

const int PL = 2;   // Parallel Load
const int CLK = 3;  // Clock
const int DATA = 4; // Data Output

uint32_t previousState = 0; // Previous state of the pedals

void setup()
{
  Serial.begin(115200);
  pinMode(PL, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(DATA, INPUT);

  digitalWrite(PL, HIGH);
  digitalWrite(CLK, LOW);
}

void loop()
{
  digitalWrite(PL, LOW);  // Load the shift register
  delayMicroseconds(5);   // Wait for the data to be ready
  digitalWrite(PL, HIGH); // Release the shift register

  uint32_t pedals = read_shift_registers(DATA, CLK, LSBFIRST, 4); // Read the shift registers

  Serial.print("Pedalboard state: "); // Print the state of the pedals
  for (int i = 0; i < 32; i++)
  {
    Serial.print((pedals >> i) & 1); // Print the state of each pedal
    Serial.print(" ");
  }
  Serial.println();

  for (uint8_t i = 0; i < 32; i++) // Check if a pedal has been pressed or released
  {
    bool previousPressed = bitRead(previousState, i); // Check if the pedal was pressed in the previous state
    bool currentPressed = bitRead(pedals, i);         // Check if the pedal is pressed in the current state

    if (currentPressed && !previousPressed) // If the pedal is pressed and was not pressed in the previous state
    {
      sendNoteOn(36 + i, 127);   // Send a Note On message
      Serial.print("Note ON: "); // Print the Note On message
      Serial.println(36 + i);    // Print the note number
    }
    else if (!currentPressed && previousPressed) // If the pedal is released and was pressed in the previous state
    {
      sendNoteOff(36 + i);        // Send a Note Off message
      Serial.print("Note OFF: "); // Print the Note Off message
      Serial.println(36 + i);     // Print the note number
    }
  }

  previousState = pedals; // Update the previous state

  MidiUSB.flush(); // Send the MIDI messages

  delay(50); // Wait for the next iteration of the loop to read the pedals again for debouncing
}

/**
 * Reads a byte from a shift register using the specified data and clock pins.
 *
 * @param dataPin The pin connected to the data line of the shift register.
 * @param clockPin The pin connected to the clock line of the shift register.
 * @param bitOrder The order in which the bits are shifted in (MSBFIRST or LSBFIRST).
 * @return The byte read from the shift register.
 */
uint8_t read_byte(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder)
{
  uint8_t value = 0;              // Initialize the value to 0
  for (uint8_t i = 0; i < 8; ++i) // Iterate over the 8 bits of the byte
  {
    if (bitOrder == LSBFIRST)                   // If the bit order is LSBFIRST
      value |= digitalRead(dataPin) << i;       // Read the bit and shift it to the right position
    else                                        // If the bit order is MSBFIRST
      value |= digitalRead(dataPin) << (7 - i); // Read the bit and shift it to the left position

    digitalWrite(clockPin, HIGH); // Pulse the clock line
    delayMicroseconds(2);         // Wait for the data to be ready
    digitalWrite(clockPin, LOW);  // Reset the clock line
  }
  return value; // Return the value read from the shift register
}

/**
 * Reads a 32-bit value from multiple shift registers using the specified data and clock pins.
 *
 * @param dataPin The pin connected to the data line of the shift registers.
 * @param clockPin The pin connected to the clock line of the shift registers.
 * @param bitOrder The order in which the bits are shifted in (MSBFIRST or LSBFIRST).
 * @param numRegisters The number of shift registers to read from.
 * @return The 32-bit value read from the shift registers.
 */
uint32_t read_shift_registers(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t numRegisters)
{
  uint32_t value = 0;                        // Initialize the value to 0
  for (uint8_t i = 0; i < numRegisters; i++) // Iterate over the number of shift registers
  {
    value |= ((uint32_t)read_byte(dataPin, clockPin, bitOrder)) << (i * 8); // Read a byte and shift it to the left position
  }
  return value; // Return the value read from the shift registers
}

/**
 * Sends a Note On message with the specified note and velocity.
 *
 * @param note The note number to play (0-127).
 * @param velocity The velocity of the note (0-127).
 */
void sendNoteOn(byte note, byte velocity)
{
  midiEventPacket_t noteOn = {0x09, 0x90, note, velocity}; // Create a Note On message with the specified note and velocity
  MidiUSB.sendMIDI(noteOn);                                // Send the Note On message
}

/**
 * Sends a Note Off message with the specified note.
 *
 * @param note The note number to stop playing (0-127).
 */
void sendNoteOff(byte note)
{
  midiEventPacket_t noteOff = {0x08, 0x80, note, 0}; // Velocity is set to 0 for Note Off
  MidiUSB.sendMIDI(noteOff);                         // Send the Note Off message
}
