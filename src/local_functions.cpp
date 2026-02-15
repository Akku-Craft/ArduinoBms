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

// diese Funktion liest alle Regestrierten Zellen aus und legt fest was zu tun ist
// die ergebnisse werden dan an die jeweilige Einheit gesendet wo auch die umsetzung erfolgt
void update_System_Behavior_for_first_Unit() {
    
  // ueberpruefen ob balancing notwendig ist 
  // differenz beider Akkuzellen
  float diff = Units[i].voltage_Cell1 - Units[i].voltage_Cell2;
  
  // Frage 1: Ist Zelle 1 viel voller als Zelle 2?
  if (diff > diffStart) { 
    Units[i].is_balancing_Z1 = true;  // Merken: Z1 entladen
    Units[i].is_balancing_Z2 = false; // Z2 muss nichts tun
  } 
  // Frage 2: Ist Zelle 2 viel voller als Zelle 1?
  else if (diff < -diffStart) { 
    Units[i].is_balancing_Z1 = false; 
    Units[i].is_balancing_Z2 = true;  // Merken: Z2 entladen
  } 
  // Frage 3: Sind sie nah genug beieinander?
  else if (abs(diff) < diffStop) { 
    Units[i].is_balancing_Z1 = false;
    Units[i].is_balancing_Z2 = false;
  }
  
}


#endif