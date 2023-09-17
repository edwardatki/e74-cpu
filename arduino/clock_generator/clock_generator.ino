bool running = false;
bool single_step = false;

const int RUN_SW_PIN = 2;
const int STEP_SW_PIN = 0;

const int CLOCK_PIN = 4;
const int RUN_LED_PIN = 1;

void setup() {
  pinMode(RUN_SW_PIN, INPUT_PULLUP);
  pinMode(STEP_SW_PIN, INPUT_PULLUP);

  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(RUN_LED_PIN, OUTPUT);
}

void loop() {
  static unsigned long run_press_start = millis();
  static bool run_one_shot = false;
  if (digitalRead(RUN_SW_PIN)) {
    run_press_start = millis();
    run_one_shot = false;
  }
  
  if ((!run_one_shot) && ((millis() - run_press_start) > 20)) {
    if (running) single_step = (millis() / 50) % 2;
    running = !running;
    run_one_shot = true;
  }

  static unsigned long step_press_start = millis();
  static bool step_one_shot = false;
  if (digitalRead(STEP_SW_PIN)) {
    step_press_start = millis();
    step_one_shot = false;
  }
  
  if ((!step_one_shot) && ((millis() - step_press_start) > 20)) {
    single_step = !single_step;
    step_one_shot = true;
  }

  digitalWrite(RUN_LED_PIN, running);

  if (running) {
    digitalWrite(CLOCK_PIN, (millis() / 50) % 2);  // 10 Hz
  } else {
    digitalWrite(CLOCK_PIN, single_step);
  }
}
