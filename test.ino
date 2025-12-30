#include <Arduino.h>

// Die interne LED wird hier initialisiert. Bei den meisten Arduinos ist das Pin 13.
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
}

// Die Hauptschleife läuft unendlich.
void loop() {
  digitalWrite(LED_BUILTIN, HIGH);   // LED einschalten
  delay(5000);                       // Eine Sekunde warten
  digitalWrite(LED_BUILTIN, LOW);    // LED ausschalten
  delay(5000);                       // Eine Sekunde warten
}