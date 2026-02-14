#include <Arduino.h>
#include "BMS_Definitions.h"
#include "local_functions.cpp"
#include <SoftwareSerial.h>

// Anzahl der aktuell verbundehnen Einheiten
int currentUnitCount = 0;

// Variable fuer das System
SystemStatus currently_status;

// Array in dehnen alle Units gespecihert sind
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
      Units[i].isConnected = true;
    }

    // "Löschen": Alle Einheiten, die früher da waren, aber jetzt fehlen
    for (int i = currentUnitCount; i < MAX_UNITS; i++) {
      Units[i].voltage_Cell1 = 0.0;
      Units[i].voltage_Cell2 = 0.0;
      Units[i].voltage_mV = 0;
      Units[i].isConnected = false;
      Units[i].is_balancing_Z1 = false;
      Units[i].is_balancing_Z2 = false;
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
      Units[i].isConnected = false;
      Units[i].is_balancing_Z1 = false;
      Units[i].is_balancing_Z2 = false;
      Units[i].internal_ID = i;
    }

    digitalWrite(PIN_MAIN_SWITCH, LOW);
  }

}


void print_BMS_Status() {
  Serial.println("\n--- Aktueller BMS Status ---");
  Serial.println("ID\tStatus\tZelle 1\tZelle 2\tGesamt");
  Serial.println("------------------------------------------");

  for (int i = 0; i < currentUnitCount; i++) {
    Serial.print(i == 0 ? "HEAD" : String(i)); // Markiert die ID 0 als Head
    Serial.print("\t");

    if (Units[i].isConnected) {
      Serial.print("OK\t");
      Serial.print(Units[i].voltage_Cell1, 2); // 2 Nachkommastellen
      Serial.print("V\t");
      Serial.print(Units[i].voltage_Cell2, 2);
      Serial.print("V\t");
      
      float total = Units[i].voltage_Cell1 + Units[i].voltage_Cell2;
      Serial.print(total, 2);
      Serial.println("V");
    } else {
      Serial.println("DISCONNECTED");
    }
  }
  Serial.println("------------------------------------------");
}


void setup() {
  // wichtig fuer das debugging
  Serial.begin(115200);

  mySerial.begin(9600); // Kommunikation zu den anderen Arduinos (Kette)
  
  pinMode(10, INPUT);
  pinMode(11, OUTPUT);

  while(!Serial); // warten bis der Monitor offen ist
  
  Serial.print("Initialisierung des BMS ...\n");

  pinMode(PIN_MAIN_SWITCH, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);


}

// Die Hauptschleife läuft unendlich.
void loop() {

  // bevor man andere Einheiten regestrieren und einlesen kann muss ma erst die erste Einheit hinzufuegen
  read_Data_for_first_unit();

  register_and_check_units();

  print_BMS_Status();

  // Status-LED blinken lassen (ohne delay, damit Messung schnell bleibt)
  digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));

  // Kurze Pause bis zur nächsten Messung
  delay(1000);
}