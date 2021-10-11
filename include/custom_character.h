#ifndef CUSTOM_CHARACTER_H
#define CUSTOM_CHARACTER_H

#include <Arduino.h>

class CustomCharacters
{
public:
  CustomCharacters();
  // Define our numbers 0 thru 9
  // 254 is blank and 255 is the "Full Block"
  uint8_t bigNums[10][6] = {
      {7, 0, 5, 4, 1, 6},         //0
      {0, 5, 254, 1, 255, 1},     //1
      {0, 2, 5, 7, 3, 1},         //2
      {0, 2, 5, 1, 3, 6},         //3
      {7, 3, 255, 254, 254, 255}, //4
      {7, 2, 0, 1, 3, 6},         //5
      {7, 2, 0, 4, 3, 6},         //6
      {0, 0, 5, 254, 7, 254},     //7
      {7, 2, 5, 4, 3, 6},         //8
      {7, 2, 5, 1, 3, 6},         //9
  };

  void setCharacterSet(uint8_t _currentSet);
  uint8_t *getCharacter(uint8_t i);
  uint8_t getCharacterSet(void);
  uint8_t getSize(void);

private:
  uint8_t numberOfSymbol = 3;
  uint8_t currentSet;
  uint8_t customNumTable[8][8] = {
      {31, 31, 31, 0, 0, 0, 0, 0},      // Small top line - 0
      {0, 0, 0, 0, 0, 31, 31, 31},      // Small bottom line - 1
      {31, 0, 0, 0, 0, 0, 0, 31},       // Small lines top and bottom -2
      {0, 0, 0, 0, 0, 0, 0, 31},        // Thin bottom line - 3
      {31, 31, 31, 31, 31, 31, 15, 7},  // Left bottom chamfer full - 4
      {28, 30, 31, 31, 31, 31, 31, 31}, // Right top chamfer full -5
      {31, 31, 31, 31, 31, 31, 30, 28}, // Right bottom chamfer full -6
      {7, 15, 31, 31, 31, 31, 31, 31},  // Left top chamfer full -7
  };


};

// //----------------------------------------------------------------------------
// //Credit to:

// uint8_t customNumTable[8][8] = {
//   {31, 31, 31, 0, 0, 0, 0, 0},      // Small top line - 0
//   {0, 0, 0, 0, 0, 31, 31, 31},      // Small bottom line - 1
//   {31,0, 0, 0, 0, 0, 0, 31},        // Small lines top and bottom -2
//   {0, 0, 0, 0, 0, 0,  0, 31},       // Thin bottom line - 3
//   {31, 31, 31, 31, 31, 31, 15, 7},  // Left bottom chamfer full - 4
//   {28, 30, 31, 31, 31, 31, 31, 31}, // Right top chamfer full -5
//   {31, 31, 31, 31, 31, 31, 30, 28}, // Right bottom chamfer full -6
//   {7, 15, 31, 31, 31, 31, 31, 31},  // Left top chamfer full -7
// };

// // Define our numbers 0 thru 9
// // 254 is blank and 255 is the "Full Block"
// uint8_t bigNums[10][6] = {
//   {7, 0, 5, 4, 1, 6},         //0
//   {0, 5, 254, 1, 255, 1},     //1
//   {0, 2, 5, 7, 3, 1},         //2
//   {0, 2, 5, 1, 3, 6},         //3
//   {7, 3, 255, 254, 254, 255}, //4
//   {7, 2, 0, 1, 3, 6},         //5
//   {7, 2, 0, 4, 3, 6},         //6
//   {0, 0, 5, 254, 7, 254},     //7
//   {7, 2, 5, 4, 3, 6},         //8
//   {7, 2, 5, 1, 3, 6},         //9
// };

// uint8_t *customSymbolTable[NUM_CUSTOM_CHAR] = {DownArrow, UpArrow, TemperatureSymbol, Battery100, Battery80};

// //----------------------------------------------------------------------------
#endif