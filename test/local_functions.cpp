#ifndef HEAD_LOCAL_FUNCTIONS_H
#define HEAD_LOCAL_FUNCTIONS_H
#include "head_code.cpp"

#include <Arduino.h>
#include "BMS_Definitions.h"

measure_Cell_Data read_Data_for_own_unit() {

  struct measure_Cell_Data cell;
  
  // 1. MESSUNG (Durchschnitt aus 20 Werten für Stabilität)
  float rawA1 = 0;
  float rawA2 = 0;
  for(int i = 0; i < 20; i++) {
    rawA1 += analogRead(PIN_ZELLE1);
    rawA2 += analogRead(PIN_GESAMT);
    delay(2);
  }
  rawA1 /= 20.0;
  rawA2 /= 20.0;

  // 2. UMRECHNUNG IN VOLT
  // Spannung von Zelle 1 (direkt oder via Schutzwiderstand)
  float vZelle1 = (rawA1 * V_REF) / 1024.0;
  
  // Gesamtspannung (Wert am Pin * 2, wegen Teiler)
  float vGesamt = ((rawA2 * V_REF) / 1024.0) * DIVIDER_FAKTOR;
  
  // Zelle 2 berechnen (Differenz)
  float vZelle2 = vGesamt - vZelle1;

  cell.voltage_mV = vGesamt;
  cell.voltage_Cell1 = vZelle1;
  cell.voltage_Cell2 = vZelle2;

  cell.temperature_C = 25;

  return cell;

}

// diese Funktion Balanced
void balancing(struct CellData own_cell) {
  // Nummer der Zelle mit der kleineren Spannung
  int lower_cell;

  // Spannung der niedrigeren Zelle
  float start_voltage;

  float new_voltage;

  // gewuenschte Spannung
  float desired_voltage;


  // Abfrage ob Zelle 1 zu wenug Spannung hat
  if (own_cell.voltage_Cell1 < own_cell.voltage_Cell2 && abs(own_cell.voltage_Cell1 - own_cell.voltage_Cell2) > 0.1) {
    lower_cell = 1;
    desired_voltage = own_cell.voltage_Cell1;
    current_voltage = own_cell.voltage_Cell2;

    // initialisierung des Potensiometers
    digitalWrite(pinINC_2, LOW);
  } 


  // Abfrage ob Zelle 2 zu wenig Spannung hat
  if (own_cell.voltage_Cell1 > own_cell.voltage_Cell2 && abs(own_cell.voltage_Cell1 - own_cell.voltage_Cell2) > 0.1) {
    lower_cell = 2;
    desired_voltage = own_cell.voltage_Cell2;
    current_voltage = own_cell.voltage_Cell1;

    // initialisierung des Potensiometers
    digitalWrite(pinINC_1, LOW);
  } 

  
  // schaltung des DC DC Wandlers wenn Zelle 1 zu wenig spannung hat
  if (lower_cell = 1) {

    // standart maessig wird die richtung auf Low gesetzt
    digitalWrite(pinUD_2, LOW);

    while (abs(desired_voltage - new_voltage) > 0.2) {

      // hier wird geguckt in welche Richtung das Poti arbeiten muss
      if (new_voltage > start_voltage) {
        digitalWrite(pinUD_2, HIGH);
      }

      // bewegung des Potis um einen Schritt
      digitalWrite(pinINC_2, LOW);
      delayMicroseconds(10);
    }

    digitalWrite(pinCS_2, HIGH);
  } 
  
  if (lower_cell = 2) {

    // standart maessig wird die Richtung auf Low gesetzt
    digitalWrite(pinUD_1, LOW);

    while (abs(desired_voltage - new_voltage) > 0.2) {

      // hier wird geguckt in welche Richtung das Poti arbeiten muss
      if (new_voltage > start_voltage) {
        digitalWrite(pinUD_1, HIGH);
      }

      // bewegung des Potis um einen Schritt
      digitalWrite(pinINC_1, LOW);
      delayMicroseconds(10);
    }

    digitalWrite(pinCS_1, HIGH);
  }



  

}


#endif