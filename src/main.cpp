#include <Arduino.h>

#define SAMPLES 100
#define PIN_SIGNAL A0
#define SERIAL_BAUD 9600
#define RMS_VOLTAGE 250.0
#define SAMPLING_FREQUENCY 1000.0
#define RESISTANCE 1.0

void setup() {
  Serial.begin(SERIAL_BAUD);
}

void loop() {
  float sample;
  float max_sample = 0;
  float sum = 0;
  float reactive_power = 0;
  float accumulated_energy = 0;
  float previous_sample = 0;
  for (int i = 0; i < SAMPLES; i++) {
    sample = analogRead(PIN_SIGNAL);
    if (sample > max_sample) {
      max_sample = sample;
    }
    sum += sample * sample;
    reactive_power += sample * previous_sample;
    previous_sample = sample;
  }

  float rms = sqrt(sum / SAMPLES);
  reactive_power /= SAMPLES;
  reactive_power /= RMS_VOLTAGE * RMS_VOLTAGE;
  reactive_power *= SAMPLING_FREQUENCY;
  accumulated_energy = reactive_power * (1.0 / SAMPLING_FREQUENCY) * RESISTANCE;

  Serial.print("r\t");
  Serial.print(RMS_VOLTAGE);
  Serial.print("\t");
  Serial.print(rms);
  Serial.print("\t");
  Serial.print(max_sample);
  Serial.print("\t");
  for (int i = 0; i < 10; i++) {
    Serial.print(random(0, 10000) / 100.0);
    Serial.print("\t");
  }
  Serial.print(reactive_power);
  Serial.print("\t");
  Serial.println(accumulated_energy);

  delay(1000);
}
