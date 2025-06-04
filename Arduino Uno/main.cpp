#include <Arduino.h>
#include <SPI.h>
#include <LiquidCrystal.h>
using namespace std;

char buff [255];
char prev [255];
volatile byte indx;
volatile boolean process;
LiquidCrystal lcd(9, 8, 7, 6, 5, 4);
bool checkSame (char buff[], char prev[]);
void assignPrev (char buff[], char prev[]);

void setup (void) {
  Serial.begin(115200);
  pinMode(MISO, OUTPUT); // have to send on master in so it set as output
  SPCR |= _BV(SPE);// turn on SPI in slave mode
  indx = 0; // buffer empty
  process = false;
  SPI.attachInterrupt(); // turn on interrupt
  lcd.begin(16,2);
}
 
ISR (SPI_STC_vect) // SPI interrupt routine 
{ 
   byte c = SPDR; // read byte from SPI Data Register
   if (indx < sizeof(buff)) {
      buff[indx++] = c; // save data in the next index in the array buff
      if (c == '\n') { 
        buff[indx - 1] = 0; // replace newline ('\n') with end of string (0)
        process = true;
      }
   }
}

bool checkSame (char buff[], char prev[]) {
  bool same = true;
  for (int i = 0; i < sizeof(buff); i++) {
    if (buff[i] != prev[i]) {
      same = false;
    }
  }
  return same;
}

void assignPrev (char buff[], char prev[]) {
  for (int i = 0; i < sizeof(buff); i++) {
    prev[i] = buff[i];
  }
}
 
void loop (void) {
   if (process) {
      process = false; //reset the process
      indx= 0; //reset button to zero
      Serial.println (buff); //print the array on serial monitor
      if (!checkSame(buff,prev)) {
        if ((strncmp(buff, "Score:", 6) == 0) || (strncmp(buff, "button", 6) == 0) || (strncmp(buff, "Your S", 6) == 0)) {
          lcd.setCursor(0,1);
        }
        else {
          lcd.clear();
          lcd.setCursor(0,0);
        }
        lcd.print(buff);
      }
      assignPrev(buff, prev);
   }
}