// DO NOT CHANGE THIS FILE. IS IS USED AS AS A REFERENCE WITH FUNCTIONS 
// SO THAT YOU CAN SEE WHAT FUNCTIONS UNDER "PUBLIC" YOU CAN USE IN MAIN CODE

#ifndef hologram _h
#define hologram _h
#include <Arduino.h>


class hologram{                                 // read these comments for each function for help  ˇˇ
  public:                                                                 
    hologram(byte pinPomakni, byte pinDopusti, byte pinPomakni_2, byte pinDopusti_2);  // konstruktor
    void init(void);                                               /// pin initialization
    void split_screen(bool *on_off);                                // 1 is for "two side" projection, 0 is for "front" projection
    void text_load(char *str);                                     /// function for text projection (up to 60 leters/blocks per function)
    void synk_pix(unsigned int *synkL, unsigned int *synkR);        // function for synkronization of first LED board and other variable for the other LED board
    void block_load(byte *gif, int *pos);                          /// function for loading single blok to matrix, or one picture of a gif, other variable is used for defining block position that we are loading/changing (block 0 to 10 without "slide", and 0 to 60 for "slide")
    unsigned int lenght_pix(unsigned int *on_duration);             // function for controling pixel lenght (let this number be default, DO NOT CHANGE IT)
    void delete_m();                                                // delete whole matrix (recomended to put this function after changing whole matrix)
    void inverse_pixel();                                          /// inverz all pixels (1 for inverz, 0 for no inverz , default is 0). it will inverse text only
    void inverse_block_pixels(int *positionn);                      // inverz one block (you need to use this function when using gif, picture or block, after every picture to get that efect going trough whole gif)
    void number_load(double number, int num_width, int dec_num);   /// printing double number, second variable can be +/- and number should be as wide as number including dot, + is alining number on left, - is for alining number on right, last variable is number of decimal numbers after the dot
    void print_m(unsigned int *gif_duration, bool *slidee);         // after you load the data use this, first variable is in miliseconds, put 0 for non time dependent projection (optimal lenght of a gif picture is 50 ms to 1000 ms), second variable is for sliding the whole matrix (60 blocks is max)

    
  private:          // do not use these variables or functions below
    byte LATCH;
    byte enable;
    byte LATCH_2;
    byte enable_2;
    
    void print_matrix(bool *slide);
    unsigned int period();  // function for period duration 
};

#endif
