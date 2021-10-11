#include "custom_character.h"


CustomCharacters::CustomCharacters(){

}


void CustomCharacters::setCharacterSet(uint8_t _currentSet){
    currentSet = _currentSet;
}

uint8_t CustomCharacters::getCharacterSet(void){
    return currentSet;
}

uint8_t* CustomCharacters::getCharacter(uint8_t i){
    return customNumTable[i];
}

uint8_t CustomCharacters::getSize(void){
    return numberOfSymbol;
}