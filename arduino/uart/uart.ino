
const int a12_pin = 2;
const int a13_pin = 3;
const int a14_pin = 4;
const int a15_pin = 5;

const int d0_pin = 6;
const int d1_pin = 7;
const int d2_pin = 8;
const int d3_pin = 9;
const int d4_pin = 10;
const int d5_pin = 11;
const int d6_pin = 12;
const int d7_pin = 13;

const int mem_read_pin = 14;    // Active low
const int mem_write_pin = 15;   // Active low
const int mem_inhibit_pin = 16; // Active low
const int interrupt_pin = 17;   // Active high

const int a0_pin = 18;
const int a1_pin = 19;

bool valid_address() {
  // Mapped from 0x7000 to 0x8000
  if (digitalRead(a15_pin) != false) return false;
  if (digitalRead(a14_pin) != true) return false;
  if (digitalRead(a13_pin) != true) return false;
  if (digitalRead(a12_pin) != true) return false;
  return true;
}

char read_data() {
  char data = 0;

  pinMode(d0_pin, INPUT);
  pinMode(d1_pin, INPUT);
  pinMode(d2_pin, INPUT);
  pinMode(d3_pin, INPUT);
  pinMode(d4_pin, INPUT);
  pinMode(d5_pin, INPUT);
  pinMode(d6_pin, INPUT);
  pinMode(d7_pin, INPUT);

  data |= digitalRead(d0_pin) << 0;
  data |= digitalRead(d1_pin) << 1;
  data |= digitalRead(d2_pin) << 2;
  data |= digitalRead(d3_pin) << 3;
  data |= digitalRead(d4_pin) << 4;
  data |= digitalRead(d5_pin) << 5;
  data |= digitalRead(d6_pin) << 6;
  data |= digitalRead(d7_pin) << 7;

  return data;
}

void write_data(char data) {
  digitalWrite(d0_pin, (data >> 0) & 1);
  digitalWrite(d1_pin, (data >> 1) & 1);
  digitalWrite(d2_pin, (data >> 2) & 1);
  digitalWrite(d3_pin, (data >> 3) & 1);
  digitalWrite(d4_pin, (data >> 4) & 1);
  digitalWrite(d5_pin, (data >> 5) & 1);
  digitalWrite(d6_pin, (data >> 6) & 1);
  digitalWrite(d7_pin, (data >> 7) & 1);

  pinMode(d0_pin, OUTPUT);
  pinMode(d1_pin, OUTPUT);
  pinMode(d2_pin, OUTPUT);
  pinMode(d3_pin, OUTPUT);
  pinMode(d4_pin, OUTPUT);
  pinMode(d5_pin, OUTPUT);
  pinMode(d6_pin, OUTPUT);
  pinMode(d7_pin, OUTPUT);
}

void setup() {
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

  digitalWrite(interrupt_pin, false);
  pinMode(interrupt_pin, OUTPUT);

  Serial.begin(115200);
  Serial.println("UART READY...");
}

void loop() {
  bool read_low = !digitalRead(mem_read_pin);
  static bool was_read_low = false;

  bool write_low = !digitalRead(mem_write_pin);
  static bool was_write_low = false;

  if (valid_address()) {
    digitalWrite(mem_inhibit_pin, false);

    if (write_low && !was_write_low) {        // Read data from bus on falling edge
      Serial.print(read_data());
    } else if (read_low && !was_read_low) {   // Write data to bus on falling edge
      if (Serial.available() > 0) {
        write_data(Serial.read());
      } else {
        write_data(0);
      }
    } 

    if (!read_low) {                          // Otherwise get off the bus
      read_data();
    }

    was_read_low = read_low;
    was_write_low = write_low;
  } else {
    digitalWrite(mem_inhibit_pin, true);

    read_data();

    was_read_low = false;
    was_write_low = false;
  }
}
