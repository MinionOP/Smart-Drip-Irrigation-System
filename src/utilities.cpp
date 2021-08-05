#include "utilities.h"


int inRange(int num, int min, int max){
    if(num >= min && num <=max){
        return 1;
    }
    else if(num < min){
        return -1;
    }
    else{
        return 0;
    }
}

