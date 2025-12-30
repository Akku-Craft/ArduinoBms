#include <SPI.h> // Typisch für Monitor-ICs (LTC68xx)

// --- KONSTANTEN UND PIN-DEFINITIONEN ---
#define NUM_CELLS             7     // Ihre Anzahl an Zellen
#define V_CELL_MAX            4200  // Max. Ladeschlussspannung in mV (4.20 V)
#define V_CELL_MIN            3000  // Min. Entladeschlussspannung in mV (3.00 V)
#define V_BALANCE_DIFF        50    // Balancing-Schwelle in mV (0.05 V)
#define BALANCE_PIN_START     2     // Erster digitaler Pin zur Steuerung der Balancing-MOSFETs

// --- DATENSTRUKTUR FÜR EINE EINZELNE ZELLE ---
struct CellData {
  // Messwerte
  int voltage_mV;         // Aktuelle Zellspannung in Millivolt
  int temperature_C;      // Aktuelle Zelltemperatur in Grad Celsius
  
  // Status & Steuerung
  byte internal_ID;       // Die interne Kanal-ID im Monitor-IC
  bool is_balancing;      // TRUE, wenn der Balancierwiderstand dieser Zelle aktiv ist
};

// Array, das alle Zellen im System speichert
CellData cell_array[NUM_CELLS];

// --- FUNKTIONEN (Platzhalter für die Chip-Kommunikation) ---

// Liest die Spannungen und Temperaturen vom Monitor-IC (LTC/BQ)
// Die Daten werden direkt in das cell_array[] geschrieben.
int LTC_read_monitor_data() {
  // Hier würde die SPI-Kommunikation erfolgen.
  
  // Platzhalter: Simuliert das Auslesen (muss später ersetzt werden!)
  for (int i = 0; i < NUM_CELLS; i++) {
    // Lesen der Spannung (simuliert)
    cell_array[i].voltage_mV = random(3500, 4100); 
    // Lesen der Temperatur (simuliert)
    cell_array[i].temperature_C = random(20, 35); 
    cell_array[i].internal_ID = i + 1; // Setzt ID 1 bis 7
  }
  return 0; // Erfolg
}

// Steuert den Haupt-Lade-MOSFET (CHG)
void CHG_MOSFET_Control(bool enable) {
  // Steuerung des CHG-MOSFETs (Pin-Definition vorausgesetzt)
  // digitalWrite(CHG_PIN, enable ? HIGH : LOW);
  Serial.println(enable ? "-> LADE-MOSFET EIN" : "-> LADE-MOSFET AUS");
}

// Steuert den Haupt-Entlade-MOSFET (DSG)
void DSG_MOSFET_Control(bool enable) {
  // Steuerung des DSG-MOSFETs (Pin-Definition vorausgesetzt)
  // digitalWrite(DSG_PIN, enable ? HIGH : LOW);
  Serial.println(enable ? "-> ENTLADE-MOSFET EIN" : "-> ENTLADE-MOSFET AUS");
}

// Steuert den Lade-IC
void Charge_Controller_Configure(int voltage_mV, int current_mA) {
  Serial.print("Lade-IC konfiguriert: ");
  Serial.print(voltage_mV); Serial.print("mV bei ");
  Serial.print(current_mA); Serial.println("mA.");
}

// --- BALANCING LOGIK ---
void runBalancing() {
  int max_v = 0;
  int min_v = V_CELL_MAX + 1; // Startwert muss größer als max. mögl. Spannung sein
  int max_idx = -1;

  // 1. Minimum und Maximum finden und alle Balancer ausschalten
  for (int i = 0; i < NUM_CELLS; i++) {
    // Finden der höchsten und niedrigsten Spannung
    if (cell_array[i].voltage_mV > max_v) {
      max_v = cell_array[i].voltage_mV;
      max_idx = i;
    }
    if (cell_array[i].voltage_mV < min_v) {
      min_v = cell_array[i].voltage_mV;
    }
    
    // Status in der Datenstruktur und Pin zurücksetzen
    cell_array[i].is_balancing = false;
    digitalWrite(BALANCE_PIN_START + i, LOW);
  }

  // 2. Balancing-Bedingung prüfen
  if ((max_v - min_v) > V_BALANCE_DIFF) {
    Serial.print("! BALANCING NOTWENDIG: Max "); Serial.print(max_v);
    Serial.print(" mV, Min "); Serial.print(min_v); Serial.println(" mV");
    
    // 3. Widerstand der höchsten Zelle einschalten
    if (max_idx != -1) {
      // Balancing nur bei mittlerer bis hoher Spannung starten (z.B. > 3.8V)
      if (cell_array[max_idx].voltage_mV > 3800) { 
        digitalWrite(BALANCE_PIN_START + max_idx, HIGH); // Schaltet MOSFET EIN
        cell_array[max_idx].is_balancing = true; // Status in Datenstruktur speichern
        Serial.print("-> Zelle "); Serial.print(max_idx + 1); 
        Serial.println(" (Höchste) wird entladen.");
      }
    }
  } else {
    Serial.println("-> Zellspannungen ausgeglichen.");
  }
}

// --- SETUP ---
void setup() {
  Serial.begin(9600);
  SPI.begin();
  
  // Initialisiere Pins für Balancing-MOSFETs
  for (int i = 0; i < NUM_CELLS; i++) {
    pinMode(BALANCE_PIN_START + i, OUTPUT);
  }
  
  // Initialisiere Monitor-IC, USB-PD Controller und Lade-IC
  // LTC_init_sequence(); 
  // PD_Controller_init();
}

// --- HAUPTSCHLEIFE ---
void loop() {
  // 1. Daten auslesen und Fehlersicherheit
  if (LTC_read_monitor_data() != 0) {
    Serial.println("FEHLER: Konnte Messdaten nicht auslesen!");
    CHG_MOSFET_Control(false); // Failsafe
    DSG_MOSFET_Control(false);
    delay(2000);
    return;
  }
  
  // Variablen zur schnellen Logikprüfung
  int min_v = V_CELL_MAX;
  int max_v = V_CELL_MIN;
  int max_temp = 0;

  // 2. Grenzwerte und Sicherheitsprüfung ermitteln
  for (int i = 0; i < NUM_CELLS; i++) {
    if (cell_array[i].voltage_mV < min_v) min_v = cell_array[i].voltage_mV;
    if (cell_array[i].voltage_mV > max_v) max_v = cell_array[i].voltage_mV;
    if (cell_array[i].temperature_C > max_temp) max_temp = cell_array[i].temperature_C;

    // Individuelle Notabschaltung bei Überhitzung (z.B. > 50°C)
    if (cell_array[i].temperature_C > 50) {
      CHG_MOSFET_Control(false); 
      DSG_MOSFET_Control(false);
      Serial.print("!!! KRITISCHE TEMPERATUR (Zelle "); Serial.print(i + 1); Serial.println(") !!!");
      // Hier sollte eine kritische Fehlerbehandlung folgen
      delay(5000);
      return; 
    }
  }
  
  // --- ENTLADE-LOGIK ---
  if (min_v < V_CELL_MIN) {
    DSG_MOSFET_Control(false); // Entladen verbieten
    Serial.println("!!! UNTERSPANNUNG - ENTLADEN DEAKTIVIERT !!!");
  } else {
    DSG_MOSFET_Control(true); // Entladen erlauben
  }

  // --- LADE-LOGIK ---
  if (max_v > V_CELL_MAX) {
    DSG_MOSFET_Control(false); // Entladen verbieten (um Balancieren zu erleichtern)
    CHG_MOSFET_Control(false); // Laden verbieten
    Serial.println("!!! ÜBERSPANNUNG - LADEN DEAKTIVIERT !!!");
  } else {
    // Platzhalter: Hier PD_Controller_isConnected() und PD_Controller_getVoltage() aufrufen
    if (true /* Ladegerät angeschlossen */) { 
      CHG_MOSFET_Control(true); // Laden erlauben
      Charge_Controller_Configure(4200, 2000); // Beispiel: 4.2V, 2A
    } else {
      CHG_MOSFET_Control(false); // Kein Ladegerät angeschlossen
    }
  }

  // 3. Balancing ausführen
  runBalancing();
  
  Serial.print("Min V: "); Serial.print(min_v); Serial.print("mV, Max T: "); Serial.print(max_temp); Serial.println("°C");
  Serial.println("--- Nächster Zyklus ---");
  delay(1000); 
}