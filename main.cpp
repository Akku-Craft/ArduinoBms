#include <SPI.h>
#include <Wire.h>
#include <Arduino.h>
#include <LTC68041.h>
#include <ADS1X15.h> // steht fuer die Ardafruit Bibliotek


// Konstanten und Definitionen
#define NUM_CELLS 7 // Anzahl der Zellen
#define V_CELL_MAX 4200 // Maximale Spannung
#define V_CELL_MIN 3000 // Minimale Spannung
#define V_BALANCE_DIFF 50 // Differenz der Hoechsten und niedrigsspannung der Zellen

// Pin Definitionen
#define BALANCE_PIN_START 2 // Erster Pin zuer Steuerung der Mossfets
#define CHG_ENABLE_PIN 9    // Steuersignal an den CHG-Gate-Driver
#define DSG_ENABLE_PIN 8    // Steuersignal an den DSG-Gate-Driver
#define LTC_PIN 10

#define PD_CONTROLLER_I2C_ADDRESS  0x28 // Adresse des Moduls

// hier werden die Zustaende der Mosfets gespeichert
bool is_chg_mosfet_on = false;
bool is_dsg_mosfet_on = false;



// jede Zelle wird als eine Datenstruktur repraesentiert
struct CellData {
// Messwerte
  int voltage_mV;         // Aktuelle Zellspannung in Millivolt
  int temperature_C;      // Aktuelle Zelltemperatur in Grad Celsius
  // Status & Steuerung
  byte internal_ID;       // Die interne Kanal-ID im Monitor-IC
  bool is_balancing;      // TRUE, wenn der Balancierwiderstand dieser Zelle aktiv ist
};

// Arrya das alle Zellen speichert
CellData cell_array[NUM_CELLS];

// Wichtige Globale Variablen
bool system_error = false;
bool charging_status = false;

// Deklaration von Funktionen
int LTC_read_monitor_data();
int PD_Controller_get_Status();
void CHG_MOSFET_Control(bool enable);
void DSG_MOSFET_Control(bool enable);


// Funktion zum auslesen der Spannung, da noch kein
// Moudle vofhanden ist erstmal nur zufaellige zahlen
int LTC_read_monitor_data() {
  // array zum vorlaufigen Speichern der Daten
  uint16_t cell_codes[NUM_CELLS]; // Für einen IC, 7 Zellen

  // initialisierung des chips
  LTC6804_adcv();

  // kurze Pause fuer den Chip
  delay(15);

  // Daten auslesen und in das Array schreiben
  uint8_t status = LTC6804_rdcv(0, 1, cell_codes); // reg=0 für erste Zellgruppe, 1 IC

  // schreiben der Daten aus dem Array in die Datenstrukturen
  if (status != -1) {
    for (int i = 0; i < NUM_CELLS; i++) {
      // schreiben der Spannung in die Datenstruktur
      cell_array[i].voltage_mV = cell_codes[i]; 
      // schreiben der Temperatur in die Datenstruktur
      cell_array[i].temperature_C = 25; 
      cell_array[i].internal_ID = i + 1; // Setzt ID 1 bis 7
    }

    return 0;
  } else {
    Serial.println("Fehler: Keine Antwort vom LTC6804!");
    return -1; // Fehlerstatus
  }

  
}

// Funktion zum auslesen des PD Controllers
int PD_Controller_get_Status() {
  Wire.beginTransmission(PD_CONTROLLER_I2C_ADDRESS);
    Wire.write(0x00); // Beispiel-Register für Status
    if (Wire.endTransmission() != 0) {
        return -1; // Fehler: Chip nicht erreichbar
    }

    Wire.requestFrom(PD_CONTROLLER_I2C_ADDRESS, 1);
    if (Wire.available()) {
        byte status = Wire.read();
        // Bit 0 könnte z.B. aussagen: Ladegerät erkannt
        bool charger_plugged = bitRead(status, 0); 
        return charger_plugged ? 1 : 0;
    }
    return 0;
}

// Steuerung des Lade Mosfets
void CHG_MOSFET_Control(bool enable) {
  // steuerung des Mosfets
  digitalWrite(CHG_ENABLE_PIN, enable ? HIGH : LOW);

  // Status aendern
  charging_status = enable; 

  // Status ausgeben
  Serial.print(enable ? "-> LADE-MOSFET EIN" : "-> LADE-MOSFET AUS");
}

// Steuerung des Entlade Mosfets
void DSG_MOSFET_Control(bool enable) {
  // steuerung des Mosfets
  digitalWrite(DSG_ENABLE_PIN, enable ? HIGH : LOW);

  // Status aendern
  charging_status = enable; 

  // Status ausgeben
  Serial.print(enable ? "-> LADE-MOSFET EIN" : "-> LADE-MOSFET AUS");
}

void apply_balancing() {
  uint16_t discharge_mask = 0;

  for (int i = 0; i < NUM_CELLS; i++) {
    // Wenn Zelle > 4.15V UND (Zelle - Minimum) > 20mV
    if (cell_array[i].voltage_mV > 4150 && (cell_array[i].voltage_mV - min_voltage > 20)) {
      discharge_mask |= (1 << i); // Setze das Bit für diese Zelle
      cell_array[i].is_balancing = true;
    } else {
      cell_array[i].is_balancing = false;
    }
  }

  // der LTC Chip schaltet die entsprechenden Wiederstaende
  LTC6804_wrcfg(discharge_mask);
}

void setup() {
  // wichtig fuer das debugging
  Serial.begin(115200);

  while(!Serial); // warten bis der Monitor offen ist
  
  Serial.print("Initialisierung des BMS ...\n");

  // I2C Bus Starten
  Wire.begin();
  delay(100);

  // SPI Bus starten
  SPI.begin();

  // festlegung der Mosfetspins als Output pins
  pinMode(CHG_ENABLE_PIN, OUTPUT);
  pinMode(DSG_ENABLE_PIN, OUTPUT);
  pinMode(LTC_PIN, OUTPUT);

  // starten mit abgeschalteten Mosfets
  digitalWrite(CHG_ENABLE_PIN, LOW);
  digitalWrite(DSG_ENABLE_PIN, LOW);

  // initialisieren der LTC Bibliotek
  LTC6804_initialize(LTC_PIN);

  // ueberpruefen ob der USB Controller funktioniert
  Wire.beginTransmission(PD_CONTROLLER_I2C_ADDRESS);
  if (Wire.endTransmission() == 0) {
      Serial.println("USB-PD Controller gefunden.");
  } else {
      Serial.println("WARNUNG: USB-PD Controller (0x28) nicht erreichbar.");
  }

  // Initialisierung des Zellen
  Serial.println("Initialisiere Zell-Datenstruktur...");

  for (int i = 0; i < NUM_CELLS; i++) {
    cell_array[i].internal_ID = i + 1;
    cell_array[i].voltage_mV = 0;
    cell_array[i].is_balancing = false;
    cell_array[i].temperature_C = 25;
  }

  Serial.println("Setup abgeschlossen. Starte Überwachung...");
}


void loop() {

}


