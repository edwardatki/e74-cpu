bool running = false;
bool clock_state = false;

const int RUN_SW_PIN = 2;
const int STEP_SW_PIN = 0;
const int POTENTIOMETER_PIN = 3;
const int CLOCK_PIN = 4;
const int RUN_LED_PIN = 1;

void setup() {
  pinMode(RUN_SW_PIN, INPUT_PULLUP);
  pinMode(STEP_SW_PIN, INPUT_PULLUP);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  digitalWrite(CLOCK_PIN, clock_state);
  pinMode(RUN_LED_PIN, OUTPUT);
  digitalWrite(RUN_LED_PIN, running);
}

void loop() {
  static unsigned long run_press_start = millis();
  static bool run_one_shot = false;
  if (digitalRead(RUN_SW_PIN)) {
    run_press_start = millis();
    run_one_shot = false;
  }
  
  if ((!run_one_shot) && ((millis() - run_press_start) > 20)) {
    running = !running;
    digitalWrite(RUN_LED_PIN, running);
    run_one_shot = true;
  }

  static unsigned long step_press_start = millis();
  static bool step_one_shot = false;
  if (digitalRead(STEP_SW_PIN)) {
    step_press_start = millis();
    step_one_shot = false;
  }
  
  if ((!step_one_shot) && ((millis() - step_press_start) > 20)) {
    clock_state = !clock_state;
    step_one_shot = true;
  }

  if (running) {
    // Read is 0 to 1023, need from 1023 to 1
    int period = 1024-analogRead(POTENTIOMETER_PIN);

    static unsigned long last_toggle = millis();
    unsigned long next_toggle = last_toggle + period;
    if (millis() > next_toggle) {
      clock_state = !clock_state;
      digitalWrite(CLOCK_PIN, clock_state);
      last_toggle = millis();
    }
  } else {
    digitalWrite(CLOCK_PIN, clock_state);
  }
}
