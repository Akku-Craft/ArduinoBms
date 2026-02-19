#include <Arduino.h>

// Pins für die Zellspannung
const int pinZelle1 = A0; // Direkt verbunden (0 - 4.2V)
const int pinZelle2 = A1; // Über Spannungsteiler 1:2 verbunden (max 8.4V -> 4.2V)

// Faktoren zur Umrechnung (Kalibrierung)
const float refVoltage = 5.0;     // Standard Nano Spannung
const float dividerRatio = 2.0;   // Faktor für den Spannungsteiler an Zelle 2

void setup() {
  Serial.begin(9600);
  Serial.println("BMS Messung gestartet...");
}

void loop() {
  // --- Zelle 1 messen ---
  int raw1 = analogRead(pinZelle1);
  float volt1 = (raw1 * refVoltage) / 1023.0;

  // --- Zelle 2 messen ---
  int raw2 = analogRead(pinZelle2);
  float voltRaw2 = (raw2 * refVoltage) / 1023.0 + 2.5;
  float voltGesamt = voltRaw2; // Gesamte Pack-Spannung
  float volt2 = voltGesamt - volt1;           // Differenz ergibt Zelle 2

  // --- Ausgabe ---
  Serial.print("Zelle 1: ");
  Serial.print(volt1);
  Serial.print(" V | ");
  Serial.print("Zelle 2: ");
  Serial.print(volt2);
  Serial.println(" V");

  delay(1000); // 1 Sekunde warten
}