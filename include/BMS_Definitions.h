// BMS_Definitions.h
#ifndef BMS_DEFINITIONS_H
#define BMS_DEFINITIONS_H

#include <Arduino.h>

// Pin Temperatursensor
const int tempPin = A0;

// Pin Definitionen fuer Potensiometer 1
const int pinCS_1  = 1; // Chip Select
const int pinINC_1 = 2;  // Increment (Der "Takt")
const int pinUD_1  = 3;  // Up / Down (Richtung)

// Pin Definitionen fuer Potensiometer 2
const int pinCS_2  = 4; // Chip Select
const int pinINC_2 = 5;  // Increment (Der "Takt")
const int pinUD_2  = 6;  // Up / Down (Richtung)

#define NUM_CELLS_PER_UNIT 2
#define MAX_UNITS          5
#define NUM_UNITS          5
#define ACK_SIGNAL         0x06

// Pin definierung
const int PIN_ZELLE1 = A1;
const int PIN_GESAMT = A2;
const int PIN_STATUS_LED = 13; // Onboard LED
const int Stromsensor_pin = A3;

// Konstanten fuer die Berechnungen und das Balancing 
const float V_REF = 5.0;         // Exakte Spannung am 5V Pin (bitte nachmessen!)
const float DIVIDER_FAKTOR = 2.0; // Da 10k/10k Teiler die Spannung halbiert
const float MIN_VOLT_ZELLE = 3.0; // Abschaltung bei 3.0V pro Zelle
const float MAX_VOLT_ZELLE = 4.2; // 100% bei 4.2V pro Zelle
const float diffStart = 0.05; // Wert der Differenz bei der das Balancing gestartet werden soll
const float diffStop = 0.01;  // Wert der Differenz bei der das Balancing gestoppy werden soll bzw. die Tolelranz die die beiden Zellen max. ahebn duerfen
const float gesamt_spannung = 5.00;

// die verschiedenen System moeglichkeiten
enum SystemStatus{
  STATUS_IDLE,      // Alles okay, keine Last
  STATUS_DISCHARGE, // Akku wird entladen
  STATUS_CHARGE,    // Akku wird geladen
  STATUS_CRITICAL,  // Not-Aus wegen Unterspannung
  STATUS_FULL       // Akku ist voll, Laden stoppenmeintest du vorhin nicht irgentwas mit ic2
}

enum PacketType {
  TYPE_SCAN
};

enum Error_Messages {
  overheating,
  overloading,
  underloading,
  voltage_difference
}
   
// diese Struktur representiert die einzelnen Einheiten in der Head Zelle
struct CellData {
// Messwerte
  int voltage_mV;         // Aktuelle Gesamtzellspannung in Millivolt
  int voltage_Cell1;      // Zellspannung der ersten Zelle
  int voltage_Cell2;      // Zellspannung der zweiten Zelle
  int temperature_C;      // Aktuelle Zelltemperatur in Grad Celsius
  // Fehlermeldungen der Zelle
  Error_Messages error;
  SystemStatus status;
};

// die Struktur ehthaelt alle werte fuer das ScanPacket
struct SingleUnitData {
  uint16_t vCell1_mV;    // z.B. 3750 für 3.75V
  uint16_t vCell2_mV;
  uint16_t voltage_mV;
  float temp_C;
};

// dieses Packet wird rumgeschickt um die Daten aller Einheiten zu erhalten
struct ScanPacket {
  uint8_t startByte;
  uint8_t type;
  uint8_t activeUnits;        // Wie viele haben sich eingetragen?
  SingleUnitData units[MAX_UNITS];
  uint8_t checksum;
};

#endif