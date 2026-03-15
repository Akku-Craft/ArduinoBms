#include <Arduino.h>
#include "BMS_Definitions.h"
#include "local_functions.cpp"
#include <SoftwareSerial.h>

// Anzahl der aktuell verbundehnen Einheiten
int currentUnitCount = 0;

struct SingleUnitData own_unit;

// Array in dehnen alle Units gespecihert sinddx
CellData Units[MAX_UNITS];

// hier werden die Pins zum Senden und Empfangen festgelegt
SoftwareSerial mySerial(10, 11);

// hier werden die Einheiten registriert und ausgelesen
void register_and_check_units() {
  // wird gebraucht fuer das Senden
  struct ScanPacket p;

  // empfangene Daten
  struct ScanPacket incoming;

  // status Variable ob ein Packet empfangen wurde
  bool success = false;

  p.activeUnits = 1; 
  p.units[0] = Units[0];
  p.type = TYPE_SCAN;

  // hier wird das Packet gesendet
  mySerial.write((byte*)&p, sizeof(ScanPacket));


  // empfangslogik
  unsigned long startWait = millis();
  while (millis() - startWait < 200) {

    if (mySerial.available() >= sizeof(ScanPacket)) {
    
      // Lies die exakte Anzahl an Bytes direkt in die Struktur
      mySerial.readBytes((byte*)&incoming, sizeof(ScanPacket));

      success = true;
      
      break;

    }

  }
  

  // ab hier werden die Zellen registriert und Ausgelesen

  if (success) {
    // Hier nutzt du incoming jetzt ganz sicher:
    currentUnitCount += incoming.activeUnits;

    for (int i = 0; i < currentUnitCount; i++) {
      // Wir kopieren die Daten in unser permanentes Units-Array
      Units[i] = incoming.units[i];
    }

    // "Löschen": Alle Einheiten, die früher da waren, aber jetzt fehlen
    for (int i = currentUnitCount; i < MAX_UNITS; i++) {
      Units[i].voltage_Cell1 = 0.0;
      Units[i].voltage_Cell2 = 0.0;
      Units[i].temperature_C = 0.0;
      Units[i].voltage_mV = 0;
      Units[i].internal_ID = i;
    }
  } else {

    // Wenn kein Paket ankam (Kabel abgezogen) -> ALLES löschen außer Head
    currentUnitCount = 1;

    for (int i = 1; i < MAX_UNITS; i++) {
      // Alles auf Null setzen!
      Units[i].voltage_Cell1 = 0.0;
      Units[i].voltage_Cell2 = 0.0;
      Units[i].voltage_mV = 0;
      Units[i].temperature_C = 0;
      Units[i].internal_ID = i;
    }

    digitalWrite(PIN_MAIN_SWITCH, LOW);
  }

}

// diese Funktion ueberprueft ob ein Fehler in den Zellen vorliegt und handelt entsprechend
void check_for_erros() {
  for(int i = 0; i < MAX_UNITS; i++) {
    // Abfrage ob eine zugrosse Spannungsdifferenz vorliegt
    if (abs(Units[i].voltage_Cell1 - Units[0].voltage_Cell2) > diffStart) {
      Units[i].error = voltage_difference;
    }

    // hier wird abgefragt ob die Zellen zu sehr entladen sind
    if (Units[0].voltage_Cell1 < 2.00 || Units[0].voltage_Cell2 < 2.00) {
      Units[0].error = underloading;
    }

    // hier wird abgefragt ob die Zellen zu voll sind
    if (Units[0].voltage_Cell1 < 4.00 || Units[0].voltage_Cell2 < 4.00) {
      Units[0].error = overloading;
    }

    // hier wird auf ueberhitzung geprueft
    if (Units[0].voltage_Cell1 < 4.00 || Units[0].voltage_Cell2 < 4.00) {
      Units[0].error = overloading;
    }


  }
}


void setup() {
  // wichtig fuer das debugging
  Serial.begin(115200);

  mySerial.begin(9600); // Kommunikation zu den anderen Arduinos (Kette)
  
  pinMode(10, INPUT);
  pinMode(11, OUTPUT);

  pinMode(8, INPUT);
  pinMode(9, OUTPUT);

  pinMode(pinCS_1, OUTPUT);
  pinMode(pinINC_1, OUTPUT);
  pinMode(pinUD_1, OUTPUT);

  pinMode(pinCS_2, OUTPUT);
  pinMode(pinINC_2, OUTPUT);
  pinMode(pinUD_2, OUTPUT);

  while(!Serial); // warten bis der Monitor offen ist
  
  Serial.print("Initialisierung des BMS ...\n");

  pinMode(PIN_MAIN_SWITCH, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);


}

// Zeitsteuerung
unsigned long lastScanTime = 0;
const unsigned long scanInterval = 5000; // 5000ms = 5 Sekunden

// Die Hauptschleife läuft unendlich.
void loop() {

  // alle 5 Sekunden wird eine Messung durchgefuehrt 
  if (currentTime - lastScanTime >= scanInterval) {
    // bevor man andere Einheiten regestrieren und einlesen kann muss ma erst die erste Einheit hinzufuegen
    read_Data_for_own_unit();

    // hier muss die erste einheit regestriert werden
    own_unit = read_Data_for_own_unit();

    Units[0].voltage_mV = first_unit.voltage_mV;
    Units[0].voltage_Cell1 = first_unit.voltage_Cell1;
    Units[0].voltage_Cell2 = first_unit.voltage_Cell2;
    Units[0].temperature_C = first_unit.temperature_C;

    Units[0].status = STATUS_IDLE;

    register_and_check_units();
  }

  balancing();

  // Status-LED blinken lassen (ohne delay, damit Messung schnell bleibt)
  digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));

  // Kurze Pause bis zur nächsten Messung
  delay(1000);
}