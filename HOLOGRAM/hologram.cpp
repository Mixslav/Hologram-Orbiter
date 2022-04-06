#include <SPI.h>               // biblioteka za SPI protokol za prijenos podataka na registre
#include <Arduino.h>           // opća biblioteka uključena u arduino IDE, sadrži mnoštvo pomagala ta rad programa
#include <hologram.h>   // biblioteka koja služi za prikaz slike na hologramu
#include <avr/pgmspace.h>      // biblioteka koja služi za brzi pristup programskoj memoriji (icon.h)
//#include <digitalWriteFast.h>  // biblioteka za brzo upravljanje digitalnim ulazima i izlazima mikrokontorlera
#include "icon.h"              // baza podataka već napravljenih simbola

SPISettings tpic6c595(16000000, MSBFIRST, SPI_MODE0);  // postavi rad SPI protokola

const int IRpin = 3; // pin za infracrveni senzor vrtnje

unsigned int synk_varL;          // var za sinkronizaciju obe strane ledica 180
unsigned int synk_varR;          // var za sinkronizaciju obe strane ledica 180

const int br_led = 16 * ( 10 );  // ukupan broj piksela 128 = 16 (bajtova) * 8 (8 slova)
int br_bajt = 0;  // redni broj bita
volatile int br_intrpt = 0; // brojač interupta(volatile je zbog interupta, nužno za manipulaciju ove varijable)

//računanje perioda vrtnje
unsigned int currentMicros = 0;
unsigned int previousMicros = 0;    
unsigned int interval = 0; 


  //vremena potrebna za oslobađanje prostora od delay() za upravljanje trajanja piksela
unsigned int vrijeme_sada = 0;
unsigned int vrijeme_pocetak = 0;
unsigned int trajanje_stupa;   // mikrosekunde (trajanje jednog piksela)

// za kontrolu trajanja svake slike gifa
unsigned long predd = 0;        // will store last time LED was updated
unsigned long sadd = 0;         // will store last time LED was updated


/*
  //vremena potrebna za gifove i dinamičke slike mozda
unsigned int v_sada = 0;
unsigned int v_pocetak = 0;
unsigned int trajanje_slike;   // mikrosekunde (trajanje jedne slike)
*/

int strPos;         // redni broj očitanog slova koje se sprema u matricu ( 16(stupova) * 9(redno slovo riječi)) 0-144 byte-a

int shift = 0;
bool enable_split = 0;  // pomoćna varijabla za split screen


byte matrix[16 * ( 60 )] = {};   // matrica u kojoj se spemaju simboli 16 * 60_slova
char text_ch[] = {};            // tekst koji nam služi za provjeru promjene teksta prikaza

char str_i[10]; // varijabla za spemanje teksta za primanje double vrijednosti iz maina
char data[50];


// definicije za pomicanje i izbacivanje sadržaja registara
#define LATCH_ON       digitalWrite(LATCH, HIGH);    // pin za pomicanje unesenih podataka na izlaz "latch"
#define LATCH_OFF      digitalWrite(LATCH, LOW); 
#define enable_ON      digitalWrite(enable, LOW);    //pin za dopuštanje prikaza
#define enable_OFF     digitalWrite(enable, HIGH);         // isključi izlaz

#define LATCH_2_ON     digitalWrite(LATCH_2, HIGH);    // pin za pomicanje unesenih podataka na izlaz "latch"
#define LATCH_2_OFF    digitalWrite(LATCH_2, LOW);  
#define enable_2_ON    digitalWrite(enable_2, LOW);    //pin za dopuštanje prikaza
#define enable_2_OFF   digitalWrite(enable_2, HIGH);




/*
#define LATCH_ON       digitalWriteFast(LATCH, HIGH);    // pin za pomicanje unesenih podataka na izlaz "latch"
#define LATCH_OFF      digitalWriteFast(LATCH, LOW); 
#define enable_ON      digitalWriteFast(enable, LOW);    //pin za dopuštanje prikaza
#define enable_OFF     digitalWriteFast(enable, HIGH);         // isključi izlaz

#define LATCH_2_ON     digitalWriteFast(LATCH_2, HIGH);    // pin za pomicanje unesenih podataka na izlaz "latch"
#define LATCH_2_OFF    digitalWriteFast(LATCH_2, LOW);  
#define enable_2_ON    digitalWriteFast(enable_2, LOW);    //pin za dopuštanje prikaza
#define enable_2_OFF   digitalWriteFast(enable_2, HIGH);
*/


/* // ovo je za esp 8266 ili drugi mikrokontroler
#define LATCH_ON       PORTD = B11000100;    // pin za pomicanje unesenih podataka na izlaz "latch"
#define LATCH_OFF      PORTD = B10000100; 
#define enable_ON      PORTD = B00000100;    //pin za dopuštanje prikaza
#define enable_OFF     PORTD = B10000100;         // isključi izlaz

#define LATCH_2_ON     PORTD = B10100100;    // pin za pomicanje unesenih podataka na izlaz "latch"
#define LATCH_2_OFF    PORTD = B10000100; 
#define enable_2_ON    PORTD = B10000000;    //pin za dopuštanje prikaza
#define enable_2_OFF   PORTD = B10000100;
*/




//Constructor
hologram::hologram(byte pinPomakni, byte pinDopusti, byte pinPomakni_2, byte pinDopusti_2) {
  this->LATCH    = pinPomakni;
  this->enable   = pinDopusti;
  this->LATCH_2  = pinPomakni_2;
  this->enable_2 = pinDopusti_2;
}

/**
 * Initialize the LCD.
 */

void pin3Interrupt(void){ // interupt se izvršava svako pola perioda pa kad računamo period stavljamo *2
 
 
 br_intrpt ++;

}


void hologram::init(void) {   // u ovoj funkciji program se izvršava samo jednom (poziva se u setup funkciji u glavnom kodu)
  
  Serial.begin(9600);
  
  SPI.begin();
  SPI.beginTransaction(tpic6c595);
  pinMode(LATCH, OUTPUT);       // pin za kontrolu prenošenja bitova na izlate registra, definiran kao izlaz
  pinMode(enable, OUTPUT);      // pin za upravljanje  paljenja svih izlaza na registrima, definiran kao izlaz
  pinMode(LATCH_2, OUTPUT);     // pin za kontrolu prenošenja bitova na izlate registra, definiran kao izlaz
  pinMode(enable_2, OUTPUT);    // pin za upravljanje  paljenja svih izlaza na registrima, definiran kao izlaz
  pinMode(IRpin, INPUT_PULLUP); // pin za interupt sa infracrvenog senzora dekodera

  // aktiviraj interupte (dodijeli pin, funkciju i način izvršavanja interupta)
  attachInterrupt(digitalPinToInterrupt(IRpin), pin3Interrupt, FALLING); //0 je pin2, 1 je pin3 na pro mini
  
  
  LATCH_OFF;      // pripremi izlaze za program (ugasi sve prije početka)
  enable_OFF;
  LATCH_2_OFF;
  enable_2_OFF;

  //noInterrupts (); // nedozvoljeni interrupti
  interrupts (); // dozvoljeni interrupti
}



void hologram::synk_pix(unsigned int *synkL, unsigned int *synkR){
  
  synk_varL = synkL; // dobavi sinkronizacijski broj za vremensko namještanje poklapanja obiju strana ledica
  synk_varR = synkR; // dobavi sinkronizacijski broj za vremensko namještanje poklapanja obiju strana ledica
}


unsigned int hologram::period(){  // funkcija za dobavljanje perioda vrtnje holograma
  
  previousMicros = currentMicros;  // spremi trenutno vrijeme za određivanje paljenja drugih ledica 
  currentMicros = micros();        // spemaj trenutno vrijeme da možemo odrediti period
  
  return interval = (currentMicros - previousMicros);  // računaj period 
 
}



unsigned int hologram::lenght_pix(unsigned int *on_duration){    // mijenjanje dužine piksela
  
  trajanje_stupa = on_duration;
}


void hologram::split_screen(bool *on_off){    // dopusti inverz u funkciji "printaj_matricu(bool *slide)"

  enable_split = on_off; // ako smo primili 1, tada podijeli ekran

}




void hologram::print_matrix(bool *slide){  // funkcija za printanje matrice

/////////////////////////////////////////////////////////// PRVE LEDICE ///////////////////////////////////////////////////////////////////////////

   if(br_intrpt == 1)  {   // ako je dopušten prikaz prikaži prve simbole

/*
        if( br_bajt == 0){   
           period();    // kontinuirano traži period na početku svakog prikaza prvih led dioda
      }
*/

      if( br_bajt == 0)   
          delayMicroseconds(synk_varL);   // služi za sinkronizaciju sa drugom stranom ledica
 
       SPI.transfer( matrix [br_bajt + shift] );    // izbaci jedan bajt
       br_bajt++;   
       SPI.transfer( matrix [br_bajt + shift] );    // izbaci drugi bajt
       br_bajt++;
    
    //SPI.endTransaction();                   // završi prijenos
    
	enable_OFF;         // isključi izlaz 1   // ovo je proba za maknit "ghosting"
    enable_2_OFF;       // isključi izlaz 2
   
   LATCH_ON;                                // izbaci bitove na izlaz (prve ledice)
   LATCH_OFF;  
   LATCH_2_ON;                              // izbaci bitove na izlaz (druge ledice)
   LATCH_2_OFF;  

     vrijeme_pocetak = micros();        // spremi početno vrijeme
     vrijeme_sada = vrijeme_pocetak;    // spremi sadašnje vrijeme
     
    while(vrijeme_sada - vrijeme_pocetak < trajanje_stupa){   // upali piksel određeno vrijeme
      
    vrijeme_sada = micros();  // kontrola trajanja while petlje za trajanje i svjetlinu piksela 
    
      enable_ON;            // prikaz ledica 1 
    
      if(enable_split == 1) // ako je dopušten split screen
      enable_2_ON;          // prikaz ledica 2
       
    }

   enable_OFF;         // isključi izlaz 1
   enable_2_OFF;       // isključi izlaz 2

   if(br_bajt == br_led){ // resetiraj brojač kako bi mogli počet paliti druge ledice
	
	delayMicroseconds(300);   // OVO FIXA BUGOVE NA RAZNIM PLOČAMA
	SPI.transfer(B00000000);		
	SPI.transfer(B00000000);
	LATCH_ON;                                // izbaci bitove na izlaz (prve ledice)
	LATCH_OFF;  
	LATCH_2_ON;                              // izbaci bitove na izlaz (druge ledice)
	LATCH_2_OFF; 
	enable_ON;            // prikaz ledica 1 
	enable_2_ON;          // prikaz ledica 2
	enable_OFF;         // isključi izlaz 1
    enable_2_OFF;       // isključi izlaz 2 
    
	br_bajt = 0;          // resetiraj brojač bajtova
    br_intrpt = 2;        // promjeni brojač kako bi prestao prikaz

   }
} //izvrši prikaz stupa ledica



/////////////////////////////////////////////////////////// DRUGE LEDICE ///////////////////////////////////////////////////////////////////////////
      
   if(br_intrpt == 3){    // ako je prošlo pola perioda od paljenja prvih ledica upali druge za 2x veći fps

      if( br_bajt == 0)   
          delayMicroseconds(synk_varR);   // služi za sinkronizaciju sa drugom stranom ledica  
               
       SPI.transfer( matrix [br_bajt + shift] );    // izbaci jedan bajt
       br_bajt++;   
       SPI.transfer( matrix [br_bajt + shift] );    // izbaci drugi bajt
       br_bajt++;  
   
    //SPI.endTransaction();                   // završi prijenos 
 
    enable_OFF;         // isključi izlaz 1   // ovo je proba za maknit "ghosting"
    enable_2_OFF;       // isključi izlaz 2
   
   LATCH_ON;                                // izbaci bitove na izlaz (prve ledice)
   LATCH_OFF;  
   LATCH_2_ON;                              // izbaci bitove na izlaz (druge ledice)
   LATCH_2_OFF;  

     vrijeme_pocetak = micros();        // spremi početno vrijeme
     vrijeme_sada = vrijeme_pocetak;    // spremi sadašnje vrijeme
     
    while(vrijeme_sada - vrijeme_pocetak < trajanje_stupa){   // upali piksel određeno vrijeme
    
     vrijeme_sada = micros();  // kontrola trajanja while petlje za trajanje i svjetlinu piksela  
      
      enable_2_ON;          // prikaz ledica 2

      if(enable_split == 1) // ako je dopušten split screen
      enable_ON;            // prikaz ledica 1  
    }

   enable_OFF;         // isključi izlaz 1
   enable_2_OFF;       // isključi izlaz 2

    if(br_bajt == br_led){
		
     delayMicroseconds(300);	// OVO FIXA BUGOVE NA RAZNIM PLOČAMA
	 SPI.transfer(B00000000);    
	 SPI.transfer(B00000000);
	 LATCH_ON;                                // izbaci bitove na izlaz (prve ledice)
	 LATCH_OFF;  
	 LATCH_2_ON;                              // izbaci bitove na izlaz (druge ledice)
	 LATCH_2_OFF;
	 enable_ON;            // prikaz ledica 1 
	 enable_2_ON;          // prikaz ledica 2
	 enable_OFF;         // isključi izlaz 1
     enable_2_OFF;       // isključi izlaz 2
   
	 br_bajt = 0;        // resetiraj brojač bajtova simbola
     br_intrpt = 0;      // nakon prikaza oba stupa svih piksela simbola, resetiraj brojač interuptova

  if(shift == 16*strPos) // koliko piksela će biti prikazano
    shift = 0;
  if(slide == 1)  // ako je omogućen slide onda uvećavaj za dva za pomak stupova
    shift += 2;
 
   }
  }      //izvrši prikaz stupa ledica druge strane (suprotnih ledica)

}

void hologram::delete_m(){  // funkcija za brisanje cijele matrice

//    if(br_intrpt == 0 || br_intrpt == 2){
      memset(matrix, 0, sizeof(matrix));
      br_bajt = 0;
      shift = 0;
      
//   }	// od if 

if(br_intrpt == 1)   // popravljanje interupt brojača da ne minja u krivo
br_intrpt = 2;
if(br_intrpt == 3)
br_intrpt = 0;

}


// funkcija za promjenu vrijednosti svih piksela  iz 0 u 1 te 1 u 0
// osim slika preko funkcije lcd.gif_print( picture_1,11 );
// i osim nepopunjenih blokova

void hologram::inverse_pixel(){ 

//  if(br_intrpt == 0 || br_intrpt == 2){
  
   for(int i = 0; i < 16*strPos ; i++){  
   
      matrix[i] = ((byte)~matrix[i]);
  
    }
//  } // od if
if(br_intrpt == 1)    // popravljanje interupt brojača da ne minja u krivo
br_intrpt = 2;
if(br_intrpt == 3)
br_intrpt = 0;
}


void hologram::inverse_block_pixels(int *positionn){    // za inverzanje bloka pixela

          int pozicija = positionn;

        for(int i = 0; i < 16 ; i++){
   
           matrix[ i + 16 * pozicija] = ((byte)~matrix[i + 16 * pozicija]);
  
          }
}


void hologram::block_load(byte *gif, int *pos){   // funkcija za printanje gifa

          int pozicija = pos;

        for(int i = 0; i < 16 ; i++){
   
           matrix[ i + 16 * pozicija] = pgm_read_byte( gif+i );
  
     }
}

void hologram::print_timed_matrtx(unsigned int *gif_lenght, bool *slidee){

        sadd = millis();
        predd = sadd;
      while(sadd - predd < gif_lenght){
          sadd = millis();
        print_matrix(slidee);
     }

}

void hologram::number_load(double number, int num_width, int dec_num){

    dtostrf(number, num_width, dec_num, str_i );
  sprintf(data, "%s", str_i);
   text_load(data);
  
}


void hologram::text_load(char *str){   // funkcija za printanje teksta
  
//usporedi testni tekst i tekst koji smo dobili od korisnika if(ako su različiti) i ako su 
//oba statusa za paljenje ledica LOW  tek tada radi s promjenom teksta jer se sve zbagira
//  if(strcmp(str, text_ch) != 0 && (br_intrpt == 0 || br_intrpt == 2)){ 

//  if(br_intrpt == 0 || br_intrpt == 2){
   
    strPos = 0;   // resetiraj brojač slova kako bi mogli unijeti novi tekst ako postoji 
  
  while(str[strPos] != 0){  // provjeri svako slovo pa ga dekodiraj, zatim ubaci u matricu
 
      text_ch[strPos] = str[strPos];
      
    switch (str[strPos]) {
                 
    case ('A'): 
          for(int i = 0; i < 16 ; i++){    // i je broj bajta određenog slova iz icon.h, 16 je broj bitova
                                           // u jednom stupu, strPos je pozicija slova iz primljenog teksta
            matrix[ i+16*strPos ] = pgm_read_byte( A+i );  //spremi slovo u matricu
          }
      break;
      
    case ('B'): 
          for(int i = 0; i < 16 ; i++){    
            matrix[ i+16*strPos ] = pgm_read_byte( B+i );  //spremi slovo u matricu
          }
      break;
      
    case ('C'):   
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( C+i );  //spremi slovo u matricu
          }
      break;
      
    case ('D'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( D+i );  //spremi slovo u matricu
          }
      break;

    case ('E'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( E+i );  //spremi slovo u matricu
          }
      break;
      
    case ('F'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( F+i );  //spremi slovo u matricu
          }
      break;
      
    case ('G'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( G+i );  //spremi slovo u matricu
          }
      break;

    case ('H'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( H+i );  //spremi slovo u matricu
          }
      break;

    case ('I'):  
          for(int i = 0; i < 16 ; i++){    
            matrix[ i+16*strPos ] = pgm_read_byte( I+i );  //spremi slovo u matricu
          }
      break;
      
    case ('J'):  
          for(int i = 0; i < 16 ; i++){    
            matrix[ i+16*strPos ] = pgm_read_byte( J+i );  //spremi slovo u matricu
          }
      break;

    case ('K'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( K+i );  //spremi slovo u matricu
          }
      break;

    case ('L'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( L+i );  //spremi slovo u matricu
          }
      break;
      
    case ('M'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( M+i );  //spremi slovo u matricu
          }
      break;

    case ('N'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( N+i );  //spremi slovo u matricu
          }
      break;

    case ('O'):  
          for(int i = 0; i < 16 ; i++){    
            matrix[ i+16*strPos ] = pgm_read_byte( O+i );  //spremi slovo u matricu
          }
      break;

    case ('P'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( P+i );  //spremi slovo u matricu
          }
      break;

    case ('R'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( R+i );  //spremi slovo u matricu
          }
      break;

    case ('S'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( S+i );  //spremi slovo u matricu
          }
      break;

    case ('T'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( T+i );  //spremi slovo u matricu
          }
      break;

    case ('U'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( U+i );  //spremi slovo u matricu
          }
      break;
      
    case ('V'):  
          for(int i = 0; i < 16 ; i++){    
            matrix[ i+16*strPos ] = pgm_read_byte( V+i );  //spremi slovo u matricu
          }
      break;

     case ('W'):  
          for(int i = 0; i < 16 ; i++){    
            matrix[ i+16*strPos ] = pgm_read_byte( W+i );  //spremi slovo u matricu
          }
      break;

    case ('X'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( X+i );  //spremi slovo u matricu
          }
      break;

    case ('Y'):  
          for(int i = 0; i < 16 ; i++){   
            matrix[ i+16*strPos ] = pgm_read_byte( Y+i );  //spremi slovo u matricu
          }
      break;

    case ('Z'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( Z+i );  //spremi slovo u matricu
          }
      break;

    case (' '):  //SPACE
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( space+i );  //spremi slovo u matricu
          }
      break;

    case ('1'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( one+i );  //spremi slovo u matricu
          }
      break;

    case ('2'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( two+i );  //spremi slovo u matricu
          }
      break;
      
    case ('3'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( three+i );  //spremi slovo u matricu
          }
      break;

    case ('4'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( four+i );  //spremi slovo u matricu
          }
      break;

    case ('5'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( five+i );  //spremi slovo u matricu
          }
      break;

    case ('6'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( six+i );  //spremi slovo u matricu
          }
      break;

    case ('7'): 
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( seven+i );  //spremi slovo u matricu
          }
      break;

    case ('8'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( eight+i );  //spremi slovo u matricu
          }
      break;

    case ('9'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( nine+i );  //spremi slovo u matricu
          }
      break;

    case ('0'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( zero+i );  //spremi slovo u matricu
          }
      break;
      
/// POSEBNI ZNAKOVI


    case ('d'):  // ne može "°" ovaj znak kao karakter
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( degres+i );  //spremi slovo u matricu
          }
      break;

    case ('%'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( percent+i );  //spremi slovo u matricu
          }
      break;

    case (':'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( two_dots+i );  //spremi slovo u matricu
          }
      break;

    case ('.'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( dot+i );  //spremi slovo u matricu
          }
      break;


    case (','):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( coma+i );  //spremi slovo u matricu
          }
      break;

    case ('!'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( exclamation+i );  //spremi slovo u matricu
          }
      break;

    case ('?'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( question+i );  //spremi slovo u matricu
          }
      break;

    case ('-'):  
          for(int i = 0; i < 16 ; i++){     
            matrix[ i+16*strPos ] = pgm_read_byte( dash+i );  //spremi slovo u matricu
          }
      break;
     
   }
   
   strPos++; // povećaj poziciju slova kojeg obrađujemo

  } // while
 
// } // od if

if(br_intrpt == 1)    // popravljanje interupt brojača da ne minja u krivo
br_intrpt = 2;
if(br_intrpt == 3)
br_intrpt = 0;
}// funkcija



//
