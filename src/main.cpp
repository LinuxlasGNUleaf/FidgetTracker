#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <display_handler.h>
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

//======> filter conf
const int filter_min_delta_time = 10;

//=====> buffer conf
const int buf_length = 10;
volatile int buf_index = 0;
unsigned long time_buf[buf_length];
unsigned long last_trigger_time = 0;

//=====> idle conf
const unsigned long idle_timeout = 800;
volatile bool is_idle = false;
volatile bool clear_buf = false;

//=====> eval variables
float period, frequency, peripherial_speed;

//=====> display conf
const unsigned long display_update_interval = 250;
unsigned long last_display_update = 0;
DisplayHandler displayHandler = DisplayHandler();

void tick() {
  // calculate delta_time and set last_trigger_time
  unsigned long delta_time = millis() - last_trigger_time;
  if (delta_time < filter_min_delta_time) {
    #if ENABLE_SERIAL
      Serial.println("Datapoint discarded.");
    #endif
    return;
  }
  last_trigger_time = millis();

  if (delta_time >= idle_timeout) {
    #if ENABLE_SERIAL
    Serial.println("Entering Idle mode.");
    #endif
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

  // initialize display
  displayHandler.init();

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(transistor_pin), tick, FALLING);
}

void loop() {
  check_idle();
  update_display();
}

void eval_data() {
  period = mean_time() * 3;
  frequency = 1000 / period;
  peripherial_speed = frequency * circumfence;
#if ENABLE_SERIAL
  Serial.print("frequency: ");
  Serial.print(frequency);
  Serial.print(" Hz\tperipheral speed: ");
  Serial.print(peripherial_speed);
  Serial.println(" m/s");
#endif
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

  if (count != 0)
    return float(sum) / count;
  return INFINITY;
}

void check_idle() {
  if (millis()-last_trigger_time >= idle_timeout){
    #if ENABLE_SERIAL
    Serial.println("Enabling IDLE due to inactivity");
    #endif
    is_idle = true;
    clear_buf = true;
  }
  if (clear_buf) {
    clear_buf = false;
    clear_time_buf();
  }
}

void update_display() {
  if (millis() - last_display_update >= display_update_interval) {
    last_display_update = millis();
    eval_data();
    displayHandler.clearBuffer();
    displayHandler.drawGauge(2, 2, 61, 30, constrain(frequency / 30, 0.0f, 1.0f));
    displayHandler.sendBuffer();
  }
}

void clear_time_buf() {
  noInterrupts();
  for (int i = 0; i < buf_length; i++) {
    time_buf[i] = __LONG_MAX__;
  }
  interrupts();
}