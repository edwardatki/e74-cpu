
const int a12_pin = 2; // PD2
const int a13_pin = 3; // PD3
const int a14_pin = 4; // PD4
const int a15_pin = 5; // PD5

const int d0_pin = 6;  // PD6
const int d1_pin = 7;  // PD7
const int d2_pin = 8;  // PB0
const int d3_pin = 9;  // PB1
const int d4_pin = 10; // PB2
const int d5_pin = 11; // PB3
const int d6_pin = 12; // PB4
const int d7_pin = 13; // PB5

const int mem_read_pin = 14;    // Active low
const int mem_write_pin = 15;   // Active low
const int mem_inhibit_pin = 16; // Active low
const int interrupt_pin = 17;   // Active high

const int a0_pin = 18;
const int a1_pin = 19;

inline bool valid_address() {
  // Mapped from 0x7000 to 0x8000
  return ((PIND & 0b00111100) >> 2) == 0b0111;
}

inline void set_data_input() {
  DDRB &= 0b11000000;
  DDRD &= 0b00111111;
}

inline void set_data_output() {
  DDRB |= 0b00111111;
  DDRD |= 0b11000000;
}

inline char read_data() {
  return ((PINB & 0b00111111) << 2) | ((PIND & 0b11000000) >> 6);
}

inline void write_data(char data) {
  PORTD = (PORTD & 0b00111111) | ((data & 0b00000011) << 6);
  PORTB = (PORTB & 0b11000000) | ((data & 0b11111100) >> 2);
}

// This seems significantly faster than Serial.write()
inline void uart_write(char data) {
  while (UCSR0A & (1 << UDRE0) == 0); // Wait for any ongoing transmit to finish
  UDR0 = data;
}

void setup() {
  pinMode(a0_pin, INPUT);
  pinMode(a1_pin, INPUT);

  pinMode(a12_pin, INPUT);
  pinMode(a13_pin, INPUT);
  pinMode(a14_pin, INPUT);
  pinMode(a15_pin, INPUT);

  pinMode(d0_pin, INPUT);
  pinMode(d1_pin, INPUT);
  pinMode(d2_pin, INPUT);
  pinMode(d3_pin, INPUT);
  pinMode(d4_pin, INPUT);
  pinMode(d5_pin, INPUT);
  pinMode(d6_pin, INPUT);
  pinMode(d7_pin, INPUT);

  pinMode(mem_read_pin, INPUT);
  pinMode(mem_write_pin, INPUT);

  digitalWrite(mem_inhibit_pin, true);
  pinMode(mem_inhibit_pin, OUTPUT);

  pinMode(interrupt_pin, OUTPUT);
  digitalWrite(interrupt_pin, false);

  Serial.begin(115200);
  Serial.println("UART READY...");
}

void loop() {
  bool read_low = !digitalRead(mem_read_pin);
  static bool was_read_low = false;

  bool write_low = !digitalRead(mem_write_pin);
  static bool was_write_low = false;
  
  bool serial_available = Serial.available();
  digitalWrite(interrupt_pin, serial_available);

  if (valid_address()) {
    digitalWrite(mem_inhibit_pin, false);

    if (write_low && !was_write_low) {        // Read data from bus on falling edge
      uart_write(read_data());
    } else if (read_low && !was_read_low) {   // Write data to bus on falling edge
      if (serial_available) {
        if (digitalRead(a0_pin)) write_data(1);
        else write_data(Serial.read());
      } else {
        write_data(0);
      }
      set_data_output();
    } 

    if (!read_low) {                          // Otherwise get off the bus
      set_data_input();
    }

  } else {
    digitalWrite(mem_inhibit_pin, true);

    set_data_input();
  }

  was_read_low = read_low;
  was_write_low = write_low;
}
