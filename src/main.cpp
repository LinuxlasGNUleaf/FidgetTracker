#include <Arduino.h>
#include <main.h>

#define SERIAL_ENABLE true


//=====> hardware conf
const int buzzer_pin = 8;
const int transistor_pin = 2;

//=====> spinner conf
const int breaks_per_rev = 3;
// radius in mm
const float radius = 40e-03; 
const float circumfence = 2*radius*PI;

//=====> buffer conf
const int buf_length = 10;
volatile int buf_index = 0;
unsigned long time_buf[buf_length];
unsigned long last_trigger_time = 0;

//=====> evaluation conf
const unsigned long eval_interval = 1000;
unsigned long last_eval_time = 0;


void tick(){
  buf_index = (buf_index+1)%buf_length;
  time_buf[buf_index] = millis() - last_trigger_time;
  last_trigger_time = time_buf[buf_index];
}

void setup() {
  // pin setup
  pinMode(buzzer_pin, OUTPUT);
  pinMode(transistor_pin,INPUT);

  // serial setup
  if (SERIAL_ENABLE){
    Serial.begin(115200);
  }

  for (int i = 0; i < buf_index; i++){
    time_buf[i] = 0;
  }

  // attach interrupts
  attachInterrupt(digitalPinToInterrupt(2),tick, FALLING);
}

void loop() {
  check_eval();
}

void check_eval(){
  if (millis()-last_eval_time >= eval_interval){
    last_eval_time = millis();
  }
}