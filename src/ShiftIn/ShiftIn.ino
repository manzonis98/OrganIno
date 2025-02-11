const int PL = 2;    // Parallel Load
const int CLK = 3;   // Clock
const int DATA = 4;  // Data Output

void setup() {
  Serial.begin(115200);   /* Open the serial communication */
  pinMode(PL, OUTPUT);    /* Set the Parallel Load pin to Output */
  pinMode(CLK, OUTPUT);   /* Set the Cloack pin to Output */
  pinMode(DATA, INPUT);   /* Set the Data pin to Input*/
}

void loop() {
  digitalWrite(PL, HIGH);
  delay(1);
  digitalWrite(PL, LOW);
  delay(1);
  digitalWrite(PL, HIGH);

  uint16_t buttons = read_values(DATA, CLK, LSBFIRST, 2);

  for (int i = 0; i < 16; i++) {
    Serial.print((buttons >> i) & 1);
    Serial.print(" ");
  }
  Serial.println();
  delay(100);
}

/* Read values function, reading one bit and then shifting pulling high and low the clock pin. 
 */
uint16_t read_values(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t n_shiftRegisters) {
	uint16_t value = 0;
	uint8_t i;

	for (i = 0; i < 8 * n_shiftRegisters; ++i) {
		if (bitOrder == LSBFIRST)
			value |= digitalRead(dataPin) << i;
		else
			value |= digitalRead(dataPin) << (n_shiftRegisters - 1 - i);
		digitalWrite(clockPin, HIGH);
		digitalWrite(clockPin, LOW);
	}
	return value;
}