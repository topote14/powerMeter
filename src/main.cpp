#include <Arduino.h>

#define SAMPLES 100
#define PIN_SIGNAL A0
#define SERIAL_BAUD 9600
#define RMS_VOLTAGE 250.0
#define SAMPLING_FREQUENCY 1000.0
#define RESISTANCE 1.0
#define DELAY_MS 1000

unsigned long previous_time, timer;
float accumulated_energy = 0;
float thd;
float max_current_ever = 0;

void setup()
{
  Serial.begin(SERIAL_BAUD);
  previous_time = millis();
  timer = millis();
}

void loop()
{
  float sample;
  float max_sample = 0;
  float sum = 0;
  float active_power = 0;
  float current_time = millis();
  float elapsed_time = (current_time - previous_time) / 1000.0;
  float previous_sample = 0;
  float harmonic_current[7] = {0, 0, 0, 0, 0, 0, 0};

  for (int i = 0; i < SAMPLES; i++)
  {
    sample = analogRead(PIN_SIGNAL);
    if (sample > max_sample)
    {
      max_sample = sample;
    }
    sum += sample * sample;
    active_power += sample * previous_sample;
    previous_sample = sample;
  }

  float RMS_CURRENT = sqrt(sum / SAMPLES);
  active_power /= SAMPLES;
  active_power /= RMS_VOLTAGE * RMS_VOLTAGE;
  active_power *= SAMPLING_FREQUENCY;
  accumulated_energy += active_power * (elapsed_time / 3600.0);

  if (RMS_CURRENT > max_current_ever)
  {
    max_current_ever = RMS_CURRENT;
  }
  Serial.print("r\t");
  Serial.print(RMS_VOLTAGE);
  Serial.print("\t");
  Serial.print(RMS_CURRENT);
  Serial.print("\t");
  Serial.print(max_sample);
  Serial.print("\t");
  Serial.print(max_current_ever);
  Serial.print("\t");
  float percentage = 10;
  for (int i = 0; i < 7; i++)
  {
    harmonic_current[i] = RMS_CURRENT * (percentage / 100.0);
    Serial.print(harmonic_current[i]);
    Serial.print("\t");
    percentage -= 1.5;
  }
  float thd = 0;
  if (RMS_CURRENT > 0)
  {
    thd = sqrt(pow(harmonic_current[1], 2) + pow(harmonic_current[2], 2) + pow(harmonic_current[3], 2) + pow(harmonic_current[4], 2) + pow(harmonic_current[5], 2) + pow(harmonic_current[6], 2)) / RMS_CURRENT;
    thd = thd;
  }
  Serial.print(thd);
  Serial.print("\t");
  Serial.print(active_power);
  Serial.print("\t");
  Serial.print(accumulated_energy);
  Serial.print("\t\t");

  //----------------------- imprimo la segunda trama -----------------------
  Serial.print("s\t");
  Serial.print(RMS_VOLTAGE);
  Serial.print("\t");
  Serial.print(RMS_CURRENT);
  Serial.print("\t");
  Serial.print(max_sample);
  Serial.print("\t");
  Serial.print(max_current_ever);
  Serial.print("\t");
  percentage = 15;
  for (int i = 0; i < 7; i++)
  {
    harmonic_current[i] = RMS_CURRENT * (percentage / 100.0);
    Serial.print(harmonic_current[i]);
    Serial.print("\t");
    percentage -= 1.5;
  }
  thd = 0;
  if (RMS_CURRENT > 0)
  {
    thd = sqrt(pow(harmonic_current[1], 2) + pow(harmonic_current[2], 2) + pow(harmonic_current[3], 2) + pow(harmonic_current[4], 2) + pow(harmonic_current[5], 2) + pow(harmonic_current[6], 2)) / RMS_CURRENT;
    thd = thd;
  }
  Serial.print(thd);
  Serial.print("\t");
  Serial.print(active_power);
  Serial.print("\t");
  Serial.println(accumulated_energy);

  previous_time = current_time;
  while (millis() - timer < DELAY_MS)
  {
    // espera hasta que haya pasado el tiempo especificado
  }
  timer = millis();
}