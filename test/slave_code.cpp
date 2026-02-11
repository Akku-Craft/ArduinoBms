#include <Arduino.h>
#include "BMS_Definitions.h"
#include "head_local_functions.cpp"
#include <SoftwareSerial.h>

SoftwareSerial SerialDown(10, 11); // fuer die Kommunikation die Richtung Slaves geht
SoftwareSerial SerialUp(8, 9); // fuer die kommunikation die Richtung Head geht

bool forwardAndCheckIfLast(ScanPacket &p) {
    // 1. Buffer leeren, um keine alten ACKs fälschlicherweise zu lesen
    while(SerialDown.available()) SerialDown.read();

    // 2. Paket über SerialDown (Richtung Ende) senden
    SerialDown.write((byte*)&p, sizeof(ScanPacket));

    // 3. Auf ACK vom nächsten Slave warten (Timeout ca. 30-50ms)
    unsigned long startWait = millis();
    bool ackReceived = false;

    SerialUp.listen();

    while (millis() - startWait < 50) {
        if (SerialUp.available()) {
            if (SerialUp.read() == ACK_SIGNAL) {
                ackReceived = true;
                break;
            }
        }
    }

    // 4. Auswertung
    if (!ackReceived) {
        SerialUp.write((byte*)&p, sizeof(ScanPacket));
        return true; 
    }
}

// in dieser Funktion wird die Einehit in das Packet eingetragen und es wird sofort an den naechsten weitergeschickt
void handleScanPacket(Packet &p) {
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
    
    Serial.print("Paket bearbeitet an Position: ");
    Serial.println(myPos);
  }
}

void recv_data_from_serialdown() {
    SerialDown.listen();

    // Wir warten NUR auf die ersten 2 Bytes (Header)
    if (SerialDown.available() >= 2) {
    
        byte start = SerialDown.read(); // Byte 1: Startkennung (0xAA)
        byte type  = SerialDown.read(); // Byte 2: Der Typ-Entscheider

        if (start == 0xAA) {
            // bestaetigungsnachricht an den absaender schicken
            SerialUp.write(ACK_SIGNAL);
        
            // wenn es sich um ein Scan Packet handelt wrd das ausgefuehrt
            if (type == TYPE_SCAN) {
                // Jetzt wissen wir: Es ist ein ScanPacket!
                // Wir warten, bis der REST (sizeof(ScanPacket) - 2) da ist
                while(SerialDown.available() < (sizeof(ScanPacket) - 2));
                
                struct ScanPacket sp;
                sp.startByte = start;
                sp.type = type;

                // Jetzt lesen wir den Rest direkt in die Scan-Struktur
                SerialDown.readBytes((byte*)&sp.activeUnits, sizeof(ScanPacket) - 2);
                
                handleScanPacket(sp);

                delay(100);

                if (forwardAndCheckIfLast(sp) == true) {
                    unsigned long returnTimeout = millis();
                    bool packetReturned = false;

                    // Blockierende Schleife: Wir warten bis zu 2 Sekunden auf das Paket
                    while (millis() - returnTimeout < 2000) {
                        if (SerialUp.available() >= sizeof(ScanPacket)) {
                            struct ScanPacket returningPacket;
                            SerialUp.readBytes((byte*)&returningPacket, sizeof(ScanPacket));
                            
                            // Paket nach OBEN (Head) weiterreichen
                            SerialUp.write((byte*)&returningPacket, sizeof(ScanPacket));
                        }

                    }
                } else {
                    printf("Es handelt sich um die Letzte Einheit Packet wurde in die andere Richtung weitergegeben\n");
                }

            } 

            // wenn es sich um ein Balance Packet handelt wird das ausgefuehrt
            else if (type == TYPE_BALANCE) {
                break;
            }
        }
    }
}


void setup() {
    SerialUp.begin(9600);
    SerialDown.begin(9600);
}

void loop() {
    recv_data_from_serialdown();
    recv_data_from_serialup();
}