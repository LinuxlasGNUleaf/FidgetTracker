#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <display_handler.h>
#include <main.h>

#define ENABLE_SERIAL true

//=====> hardware conf
const int transistor_pin = 2;

const int breaks_per_rev = 3;
// radius in mm
const float radius = 40e-03;
const float circumfence = 2 * radius * PI;

//=====> statemachine conf
#define RUNNING 0
#define IDLING 1
#define WAKING 2
volatile uint8_t state = IDLING;

//======> filter conf
// minimum delay between two interrupts
const int filter_min_time = 10;
// max deviation from calculated mean
const float filter_max_dev = 0.4f;

//=====> buffer conf
const int buf_length = 10;
volatile int buf_index = 0;
volatile unsigned long time_buf[buf_length];
volatile unsigned long last_trigger_time = 0;

//=====> idle conf
const unsigned long idle_min_time = 1600;
bool is_idle = false;

//=====> eval variables
float period, frequency, peripheral_speed;

//=====> display conf
const unsigned long display_update_interval = 250;
unsigned long last_display_update = 0;
DisplayHandler displayHandler = DisplayHandler();

void tick()
{
  // calculate delta_time and set last_trigger_time
  unsigned long delta_time = millis() - last_trigger_time;
  last_trigger_time = millis();

  if (delta_time < filter_min_time)
    return;

  switch (state)
  {
  case IDLING:
    state = WAKING;
    break;

  case WAKING:
    state = RUNNING;
  case RUNNING:
    buf_index = (buf_index + 1) % buf_length;
    time_buf[buf_index] = delta_time;
    break;
  }
}

void setup()
{
  // pin setup
  pinMode(transistor_pin, INPUT);

// serial setup
#if ENABLE_SERIAL
  Serial.begin(115200);
  Serial.println("Connected.");
#endif

  // prep the buffer
  clear_time_buf();

  // initialize display
  displayHandler.init();

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(transistor_pin), tick, RISING);
}

void loop()
{
  check_idle();
  update_display();
}

void eval_data()
{
  period = mean_period();
  frequency = 1000 / period;
  peripheral_speed = frequency * circumfence;
#if ENABLE_SERIAL
  Serial.print("frequency: ");
  Serial.print(frequency);
  Serial.print(" Hz\tperipheral speed: ");
  Serial.print(peripheral_speed);
  Serial.println(" m/s");
#endif
}

float mean_period()
{
  unsigned int count = 0;
  unsigned long sum = 0;

  if (is_idle)
    return INFINITY;

  for (int i = 0; i < buf_length; i++)
  {
    if (time_buf[i] != __LONG_MAX__)
    {
      count++;
      sum += time_buf[i];
    }
  }

  if (count == 0)
    return INFINITY;

  float aux_mean = float(sum) / count;

  // filter entries with calculated aux_mean
  count = 0;
  sum = 0;
  for (int i = 0; i < buf_length; i++)
  {
    if (time_buf[i] == __LONG_MAX__)
      continue;

    // only select entries that do not deviate more than the specified percentage from the aux_mean
    if (abs(1-time_buf[i]/aux_mean) < filter_max_dev){
      count++;
      sum += time_buf[i];
    }
  }

  if (count != 0)
    return float(sum * breaks_per_rev) / count;
  else
    return INFINITY;
}

void check_idle()
{
  if (state == IDLING)
    return;

  unsigned long current = millis();
  if (current - last_trigger_time < idle_min_time || current == last_trigger_time)
    return;

  if (state == RUNNING)
    clear_time_buf();

  state = IDLING;
}

void update_display()
{
  if (millis() - last_display_update >= display_update_interval)
  {
    last_display_update = millis();
    eval_data();
    displayHandler.clearBuffer();
    displayHandler.drawGauge(2, 2, 61, 30, constrain(frequency / 30, 0.0f, 1.0f));
    displayHandler.sendBuffer();
  }
}

void clear_time_buf()
{
  noInterrupts();
  for (int i = 0; i < buf_length; i++)
  {
    time_buf[i] = __LONG_MAX__;
  }
  interrupts();
}