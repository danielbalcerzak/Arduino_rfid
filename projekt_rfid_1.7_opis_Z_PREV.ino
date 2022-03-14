/*
 ---------------------------------------------
 Szczecińska Szkoła Wyższa Collegium Balticum
 
 PROJEKT ELEKTRO-ZAMKA Z DOSTĘPEM DO RFID
 Autor: Daniel Balcerzak
 ---------------------------------------------
*/

// Dołączenie biobliotek użytych w projekcie

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Zdefiniowanie stałych 

#define SDA_PIN 10
#define RST_PIN 9
#define red_led 3 
#define green_led 5
#define blue_led 6
#define UID1 "77 1B 85 5F"
#define UID2 "12 2D AE 34"
#define UID3 "A8 65 AF 08"
#define buzzer 8
#define relay1 2
#define relay2 4

// Stworzenie obiektów zarządzanych przez biblioteki załączone do projektu

MFRC522 mfrc522(SDA_PIN, RST_PIN);                    
LiquidCrystal_I2C lcd(0x3F,16,2);

// Zadekladowanie zmiennych globalnych

int time_delay;
String int_in_string;

void setup() {
    pinMode(red_led, OUTPUT);
    pinMode(green_led, OUTPUT);
    pinMode(blue_led, OUTPUT);
    pinMode(buzzer, OUTPUT);
    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);
  
    // Inicjalizacja konsoli szeregowej, magistrali SPI, MFRC522 oraz LCD
    
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    lcd.init();
  
    // Jednorazowe wyświetlenie na konsoli szeregowej informacji pytania o czas otwarcia przekaźników oraz o dostępnym czasie ich wprowadzenia.
    // Wywołanie funkcji ustawiającej diodę RGB na kolor niebieski oraz ustawienie przekaźników w pozycji HIGH.
    
    Serial.println("Jaką wartość czasu otwarcia przekaźników ustawić? Dostępny czas to od 1 do 5 sek.");
    blue_led_light_on();
    lcd.backlight();
    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);

    // Utworzenie pętli while wraz ze zmienną boolean celem oczekiwania na wprowadzenie danych do monitora konsoli szeregowej
    
    boolean get_time = false;
    while (! get_time){
  
        // Zmienna int_in_string pobierze w postaci String dane wprowadzone do monitora konsoli szeregowej
        
        int_in_string=Serial.readStringUntil('\n');
  
        // Warunek czy zmienna ma długość równą 0, co oznacza, że nie wprowadzona została żadna wartość.
        // Jeżeli nie wpowadzona została wartość do konsoli szeregowej, wywoływana jest funkcja serial_monitor_lcd(),
        // która ma za zadanie wyświetlenie informacji o potrzebie jej wprowadzenia na wyświetlaczu LCD.
        // Dalsze instrukcje pętli nie są wykonywane, a pętla while rozpoczyna się od początku
        
        if (int_in_string.length() == 0){
            serial_monitor_lcd();
            continue;
        }
  
        // Zmiana wprowadzonej wartości do konsoli szeregowej ze String na int
        
        time_delay = int_in_string.toInt();
  
        // Warunek czy podana wartość do konsoli szeregowej jest większa lub równa 1 oraz mniejsza niż 6. W przypadku prawdy,
        // wyświetlona zostaje informacja o ustawionym czasie. W innym przypadku na monitorze konsoli szeregowej wyświetlona zostanie 
        // informacja o wprowadzonej nieprawidłowej wartości i domyślnym czasie 5 sek. otwarcia przekaźników.
        
        if (time_delay >=1 && time_delay <6){
            Serial.println("Ustawniono czas otwarcia drzwi na " + String(time_delay) + " sek.");
            Serial.println();
            time_delay *= 1000;
            get_time = true;    
        } 
        else{
            time_delay = 5000;
            Serial.println("Wprowadzono błędną wartość. Ustawiono domyślny czas 5 sek.");
            Serial.println();
            get_time = true;     
        } 
    }
    Serial.println("------------------------------");
}

void loop() {
  
    // Wywołanie funkcji lcd_read_rfid(), która wyświetla informację dla użytkownika o możliwości użycia karty RFID
    // z informacją wyświetloną na monitorze konsoli szeregowej

    lcd_read_rfid();
    Serial.println("Przyłóż kartę do czytnika RFID");
    Serial.println();

    // Utworzenie pętli while oraz zadekladowanie zmiennej typu boolean celem oczekiwania na dotknięcie karty do czytnika
    // i nie wykonywanie dalszego kodu
    
    boolean if_card_present = false;    
    while (! if_card_present){

        // Warunek czy nowa karta została wykryta na czytniku RFID i czy został odczytany jej numer, jeżeli prawda pętla 
        // zostaje przerwana przez zmianę if_card_present i wykonywana jest dalsza część kodu.
        
        if ( mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()){
            if_card_present = true;
        }
        else{
            continue;
        }
    }

    // Stworzenie zmiennej typu String oraz wyświetlenie jej na monitorze konsoli szeregowej.
    
    String UID= "";
    Serial.print("UID:");

    // Pętla for, w której w częściach sprawdzany i odczytywany jest zostaje RFID 
    for (int i = 0; i < mfrc522.uid.size; i++){

        // Wyświetlenie na monitorze konsoli szeregowej informacji o nr UID w formie szesnastkowej
        // Jeżeli sprawdzany uidByte jest mniejszy od 0x10 (16) do odczytanej liczby na zostać
        // dołączona liczba 0 np. DEC=15 to HEX=F. Zapis ma być 0F
                               
        Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "); 
        Serial.print(mfrc522.uid.uidByte[i], HEX);

        // Dodatnie - konkatenacja - do zmiennej UID nr karty w formie szesnastkowej
                                 
        UID.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
        UID.concat(String(mfrc522.uid.uidByte[i], HEX));                
    }

    // Wyświetlenie na monitorze konsoli szeregowej informacji zawartej w warunku oraz zmiana
    // wielkości z małych liter w zmiennej na duże
    
    Serial.println();
    Serial.print("Info: ");   
    UID.toUpperCase();

    // Warunek w którym sprawdzany jest odczytany UID zapisany w zmiennej (bez pierwszego znaku, ktorym jest spacja) ze
    // zdefiniowanymi stałymi UID1, UID2, UID3. W przypadku prawdy, wywoływana jest funkcja access_granted() przyjmująca 
    // argumenty w postaci String uruchamiająca odpowiedni przekażnik, informująca o stanie na: LCD, monitorze konsoli szeregowej,
    // zapalając opowiedni kolor diody RGB oraz modulując ton buzzera.
    
    // W przypadku błędnej karty uruchamiana jest funkcja access_denied() nie przyjmująca argumentów, nie uruchamiająca 
    // przekaźników i informująca o stanie w sposób tożsamy jak funkcja access_granted
    
    if ((UID.substring(1) == UID1) || (UID.substring(1) == UID2) || (UID.substring(1) == UID3)){
        access_granted(UID);
    }
    else{
        access_denied();
    }
    Serial.println("------------------------------");
}


// ----------------------------------------------------------------------------------------------------------
// ---------------------- FUNKCJE -------------------------------------------- FUNKCJE ----------------------
// ----------------------------------------------------------------------------------------------------------


// Funkcja acces_granted przyjmuje argumenty w postaci String. Docelowo jest to numer UID, który został 
// zweryfikowany w pętli loop(). Funkcja kolejny raz spradza numer UID i wykonuje przypisany do warunków kod
// związany z uruchomieniem przekaźników (relay1 i relay2) i poinformowaniem o stanie na LCD, monitorze konsoli 
// szeregowej oraz przez sygnał dźwiękowy

void access_granted(String nr){
    buzzer_tone_granted();
    green_led_light_on();
    if (nr.substring(1) == UID1){
        Serial.println("Otwarcie drzwi nr 1");                                   
        Serial.println("Uruchomienie przekaźnika nr 1");  
        Serial.println();
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("URUCHOMIONO");
        lcd.setCursor(0,1);
        lcd.print("PRZEKAZNIK NR 1");
        digitalWrite(relay1, LOW);                                                  
    }
    else if(nr.substring(1) == UID2){
        Serial.println("Otwarcie drzwi nr 2");                                   
        Serial.println("Uruchomienie przekaźnika nr 2");  
        Serial.println();     
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("URUCHOMIONO");
        lcd.setCursor(0,1);
        lcd.print("PRZEKAZNIK NR 2");
        digitalWrite(relay2, LOW);     
    }
    else{
        Serial.println("Otwarcie drzwi nr 1 i 2");                                   
        Serial.println("Uruchomienie przekaźnika nr 1 i 2");  
        Serial.println();
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("URUCHOMIONO");
        lcd.setCursor(0,1);
        lcd.print("PRZEKAZNIKI 1,2");
        digitalWrite(relay1, LOW);
        digitalWrite(relay2, LOW);   
    }
    
    // Czas opóżnienia nadany przez użytkownika w monitorze konsoli szeregowej
    
    delay(time_delay);

    // Powrót do stanu urządzeń z przed odczytu karty i zmiana stanu przekaźników

    digitalWrite(relay1, HIGH);
    digitalWrite(relay2, HIGH);
    lcd.clear();
    buzzer_tone_stop();
    blue_led_light_on();
}

// Funkcja access_denied nie przyjmuje żadnych argumentów. Wyświatlana jest w przypadku 
// Odczytu karty RFID nie zdefiniowanej w stałych

void access_denied(){
    lcd.clear();
    Serial.println("Błędna karta");
    Serial.println();
    lcd.setCursor(1,0);
    lcd.print("UZYTO BLEDNEJ");
    lcd.setCursor(5,1);
    lcd.print("KARTY");
    buzzer_tone_denied();
    red_led_light_on();
    delay(time_delay);
    buzzer_tone_stop();
    blue_led_light_on();
}

// FUNKCJE WYŚWIETLACZA LCD

void lcd_read_rfid(){
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("PRZYLOZ KARTE");
    lcd.setCursor(0,1);
    lcd.print("DO CZYTNIKA RFID");
    }

void serial_monitor_lcd(){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WPROWADZ DANE DO");
    lcd.setCursor(0,1);
    lcd.print(" MONITORA PORTU");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  SZEREGOWEGO  ");
    lcd.setCursor(0,1);
    lcd.print("   (1-5 sek)   ");
    delay(500);
    }

// FUNKCJE BUZZER

void buzzer_tone_denied(){
  tone(buzzer,300);
}

void buzzer_tone_granted(){
  tone(buzzer, 1000);
}

void buzzer_tone_stop(){
  noTone(buzzer);
}

// FUNKCJE LED RGB

void blue_led_light_on(){
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, LOW);
  digitalWrite(blue_led, HIGH);         
}

void green_led_light_on(){
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, HIGH);
  digitalWrite(blue_led, LOW);
}

void red_led_light_on(){
  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
  digitalWrite(blue_led, LOW);
}
