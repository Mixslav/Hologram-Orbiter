#include <hologram.h>   // library for holografic device
#include "icon.h"         // data base for letters and numbers
#include "scy_fi_block.h" // data base for animation
#include "equalizer.h"    // data base for animation
#include "my_gif.h"       // data base for animation
#include "guy_gif.h"       // data base for animation

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

gif();  // your sword man gif


}

/////////////////////// end of your code ////////////////////////////////////////

//gif function    
void gif (){
  lcd.block_load( picture_1,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide
  
  lcd.block_load( picture_2,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_3,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_4,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_5,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_6,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_7,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_8,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_9,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_10,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_11,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_12,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_13,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_14,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide

  lcd.block_load( picture_15,4 );      // load picture from guy_gif.h file on place 4
  lcd.print_timed_matrtx(200,0);      // project all blocks 200 ms without slide
  
}
