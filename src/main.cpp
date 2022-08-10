#include <Arduino.h>
#include <main.h>

#define ENABLE_SERIAL true
#define ENABLE_BUZZER false

//=====> hardware conf
const int buzzer_pin = 3;
const int transistor_pin = 2;

//=====> spinner conf
const int breaks_per_rev = 3;
// radius in mm
const float radius = 40e-03;
const float circumfence = 2 * radius * PI;

//=====> buffer conf
const int buf_length = 10;
volatile int buf_index = 0;
unsigned long time_buf[buf_length];
unsigned long last_trigger_time = 0;

//=====> evaluation conf
const unsigned long eval_interval = 1000;
unsigned long last_eval_time = 0;

//=====> idle conf
const unsigned long idle_timeout = 800;
volatile bool is_idle = false;
volatile bool clear_buf = false;

void tick() {
  // calculate delta_time and set last_trigger_time
  unsigned long delta_time = millis() - last_trigger_time;
  last_trigger_time = millis();

  if (delta_time >= idle_timeout) {
    // if idle condition was not met before, set clear_buf flag
    if (!is_idle)
      clear_buf = true;
    // idle condition is met, set flags accordingly
    is_idle = true;
  } else {
    if (!is_idle) {
      // idle condition wasn't met before, proceed normally
      buf_index = (buf_index + 1) % buf_length;
      time_buf[buf_index] = delta_time;
#if ENABLE_BUZZER
      tone(buzzer_pin, 440, 3);
#endif
    } else {
      // was idle before, reset idle flag
      is_idle = false;
    }
  }
}

void setup() {
// pin setup
#if ENABLE_BUZZER
  pinMode(buzzer_pin, OUTPUT);
#endif
  pinMode(transistor_pin, INPUT);

// serial setup
#if ENABLE_SERIAL
  Serial.begin(115200);
#endif

  // prep the buffer
  clear_time_buf();

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(transistor_pin), tick, FALLING);
}

void loop() {
  check_eval();
  check_idle();
}

void check_eval() {
  if (millis() - last_eval_time >= eval_interval) {
    last_eval_time = millis();
    float period = mean_time() * 3;
    float frequency = 1000 / period;
    float peripherial_speed = frequency * circumfence;
#if ENABLE_SERIAL
    Serial.print("frequency: ");
    Serial.print(frequency);
    Serial.print(" Hz\tperipheral speed: ");
    Serial.print(peripherial_speed);
    Serial.println(" m/s");
#endif
  }
}

float mean_time() {
  unsigned int count = 0;
  unsigned long sum = 0;

  if (is_idle)
    return INFINITY;

  for (int i = 0; i < buf_length; i++) {
    if (time_buf[i] != __LONG_MAX__) {
      count++;
      sum += time_buf[i];
    }
  }

  if (count != 0) {
    return float(sum) / count;
  }
  return INFINITY;
}

void check_idle() {
  if (clear_buf) {
    clear_buf = false;
    clear_time_buf();
  }
}

void clear_time_buf() {
  noInterrupts();
  for (int i = 0; i < buf_length; i++) {
    time_buf[i] = __LONG_MAX__;
  }
  interrupts();
}