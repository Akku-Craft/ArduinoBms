#include <Arduino.h>
#include "BMS_Definitions.h"

// Anzahl der aktuell verbundehnen Einheiten
int currentUnitCount = 1;

// Variable fuer das System
SystemStatus currently_status;

// Array in dehnen alle Units gespecihert sind
CellData Units[MAX_UNITS];



void setup() {
  // wichtig fuer das debugging
  Serial.begin(115200);

  while(!Serial); // warten bis der Monitor offen ist
  
  Serial.print("Initialisierung des BMS ...\n");

  pinMode(PIN_MAIN_SWITCH, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);


}

// Die Hauptschleife läuft unendlich.
void loop() {
  // 3. Status-LED blinken lassen (ohne delay, damit Messung schnell bleibt)
  digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));

  // 4. Kurze Pause bis zur nächsten Messung
  delay(1000);
}