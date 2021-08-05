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
    switch(currentSet){
    case 0: return customSymbolTable[i];
    case 1: return customNumTable[i];
    }
    return customSymbolTable[0];
}

uint8_t CustomCharacters::getSize(void){
    return numberOfSymbol;
}