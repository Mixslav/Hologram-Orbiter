#include <hologram.h>   // library for holografic device
#include "icon.h"         // data base for letters and numbers

// these pins are for data transfer
//#define SPICLOCK  13//sck
//#define DATAOUT 11//MOSI

// defining pins and sending them in library
// LATCH1 = 6, enable1 = 7
// LATCH2 = 5, enable2 = 4
hologram lcd(6,7,5,4);  

  
const int B_LED_1 = 9;      // blue leds, you can solder these leds on 4,4 V before diode nd arduino
const int B_LED_2 = 10;         


// here are numbers for manual synking both LED boards ( it is a number around 0 and 2000, 300 is the lenght of 1 pixel)

unsigned int br_sinkL = 0;   // variable for first LED board synchronization   (1500)
unsigned int br_sinkR = 50; // variable for second LED board synchronization  (350)


// variable for controlling the lenght of all pixels (it is usually around 300)
unsigned int pixel_lenght = 300; 


// setup fun, everything you put here will execute once
void setup() {

  Serial.begin(9600);   // baud rate, for usb comunication
  
  pinMode(B_LED_1, OUTPUT);   // blue leds as output
  pinMode(B_LED_2, OUTPUT);
  
  digitalWrite(B_LED_1, HIGH); // blue leds always ON
  digitalWrite(B_LED_2, HIGH);
  
  lcd.init();    // initialization function
 
  lcd.lenght_pix(pixel_lenght);       // pixel lenght setup
  lcd.synk_pix(br_sinkL , br_sinkR);  // function for synkronization
  lcd.split_screen(0);                // split screen or both side projection (still in test faze)
  lcd.delete_m();                     // delete all in matrix before you start your main code

}



//////////////////////////// put your code here  ///////////////////////////// 
void loop() {

lcd.delete_m(); // delete matrix to ensure there are no extra symbols
lcd.text_load("HOLOGRAM");            // write whatever
lcd.print_m(7000,0);       //project text for 7 seconds, 1 is for slide, 0 is without slide

lcd.delete_m(); // delete matrix to ensure there are no extra symbols
lcd.text_load("  ORBITER");   // write whatever (mind the spaces I left them so you can read the text from start)
lcd.inverse_pixel();        // inverse all pixels (all except gifs)
lcd.print_m(7000,1);       //project text for 7 seconds, 1 is for slide, 0 is without slide

}

/////////////////////// end of your code ////////////////////////////////////////
