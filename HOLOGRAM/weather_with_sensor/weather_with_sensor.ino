#include <hologram.h>   // library for holografic device
#include "icon.h"         // data base for letters and numbers
#include "cloud_animation.h"         // data base for letters and numbers

// these pins are for data transfer
//#define SPICLOCK  13//sck
//#define DATAOUT 11//MOSI

// defining pins and sending them in library
// LATCH1 = 6, enable1 = 7
// LATCH2 = 5, enable2 = 4
hologram lcd(6,7,5,4);  

  
const int B_LED_1 = 9;      // blue leds
const int B_LED_2 = 10;         


// here are numbers for manual synking both LED boards ( it is a number around 0 and 500)

unsigned int br_sinkL = 0;   // variable for first LED board synchronization
unsigned int br_sinkR = 110; // variable for second LED board synchronization


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

  
 double temp = analogRead(A0);    // read temperature from sensor that you soldered on pin A0
 temp = temp * 0.48828125;

  lcd.text_load("       C");     // write C for degrese on place 8 (spaces are gonna be overwritten)
  lcd.block_load( degres,7 );       // write "°" for degrees  on place 7
  lcd.number_load(temp,6,1);     // load temperature number with 1 decimal number and take 5 places (last 4 for the number and a dot, first two will be our cloud animation)
     lcd.block_load( p_1,0 );      // load picture from cloud_anumation.h on place 0
     lcd.block_load( p_2,1 );      // load picture from cloud_anumation.h on place 1
     
  lcd.print_m(200,0);  // project all 200 ms without slide

// now change cloud animation pictures 
     lcd.block_load( p_3,0 );     // load picture from cloud_anumation.h on place 0
     lcd.block_load( p_4,1 );     // load picture from cloud_anumation.h on place 1
 
  lcd.print_m(200,0);  // project all 200 ms without slide



}

/////////////////////// end of your code ////////////////////////////////////////
