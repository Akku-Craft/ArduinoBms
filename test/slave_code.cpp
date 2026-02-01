#include <Arduino.h>
#include "BMS_Definitions.h"
#include "head_local_functions.cpp"
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11);

void handlePacket(Packet &p) {
  // Meine Position im Paket ist der aktuelle Zählerstand
  // Wenn der Head activeUnits = 1 gesetzt hat, schreibt der erste Slave in Index 1
  int myPos = p.activeUnits;

  // Sicherheitscheck: Ist im Paket überhaupt noch Platz für mich?
  if (myPos < MAX_UNITS) {
    
    // Eigene Messwerte eintragen (hier die Variablen deiner Messfunktion)
    p.data[myPos].voltage_Cell1 = read_Cell1(); 
    p.data[myPos].voltage_Cell2 = read_Cell2();
    p.data[myPos].internal_ID = myPos;
    p.data[myPos].isConnected = true;

    // Den Zähler für den nächsten Nachbarn erhöhen
    p.activeUnits++;

    // 4. Das Paket sofort weiter an den nächsten Slave (oder zurück zum Head)
    mySerial.write((byte*)&p, sizeof(Packet));
    
    Serial.print("Paket bearbeitet an Position: ");
    Serial.println(myPos);
  }
}

void recv_data() {
// Wir warten NUR auf die ersten 2 Bytes (Header)
  if (mySerial.available() >= 2) {
    
    byte start = mySerial.read(); // Byte 1: Startkennung (0xAA)
    byte type  = mySerial.read(); // Byte 2: Der Typ-Entscheider

    if (start == 0xAA) {
      
        // wenn es sich um ein Scan Packet handelt wrd das ausgefuehrt
        if (type == TYPE_SCAN) {
            // Jetzt wissen wir: Es ist ein ScanPacket!
            // Wir warten, bis der REST (sizeof(ScanPacket) - 2) da ist
            while(mySerial.available() < (sizeof(ScanPacket) - 2));
            
            struct ScanPacket sp;
            sp.startByte = start;
            sp.type = type;
            // Jetzt lesen wir den Rest direkt in die Scan-Struktur
            mySerial.readBytes((byte*)&sp.activeUnits, sizeof(ScanPacket) - 2);
            
            handleScan(sp);
        } 

        // wenn es sich um ein Balance Packet handelt wird das ausgefuehrt
        else if (type == TYPE_BALANCE) {
            break;
        }
    }
  }
}

void setup() {
    
}

void loop() {
    recv_data();
}