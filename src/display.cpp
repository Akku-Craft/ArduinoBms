#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>  // <--- DAS MUSS REIN

// Display-Größe definieren
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// Das Objekt "display" erstellen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  // Startet das Display. Wenn es nicht klappt, hält das Programm an.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); // Endlosschleife bei Fehler
  }
  
  display.clearDisplay(); // Wichtig: Den Grafikspeicher einmal leeren
}

void loop() {
  display.clearDisplay();      // 1. Altes Bild weg
  
  display.setTextSize(2);      // Textgröße (1 = klein, 2 = mittel)
  display.setTextColor(WHITE); // Farbe (da OLED, gibt es nur an/aus)
  display.setCursor(0, 10);    // Wo soll der Text starten? (X, Y)
  
  display.print("Akku: ");
  display.print("Hallo\n"); // Dein Wert aus dem BMS
  display.println("V");

  display.display();           // 3. Alles auf den Bildschirm schieben
  delay(2000);
}

