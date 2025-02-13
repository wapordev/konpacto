#include <stdint.h>

int32_t frmult32(int32_t a,int32_t b){
    
    uint64_t a64 = (uint64_t)a;
    uint64_t b64 = (uint64_t)b;
    
    //stretch the range -1 to 1, so max*max == max
    //not sure if this is necesarry, number line is now non linear
    //but range is symettric
    if(a>=65536){
        a64+=1;
    }else if(b>=65536){
        b64+=1;
    }
    
    uint64_t r=a64*b64;
    
    return (int32_t)(r>>31);
}

//useless? we'll see
int32_t frdiv32(int32_t a,int32_t b){
    
    uint64_t a64 = (uint64_t)a;
    uint64_t b64 = (uint64_t)b;

    if(b>=65536){
        b64+=1;
    }else if(a>=65536){
        a64+=1;
    }
    
    uint64_t r=(a64<<15)*(b64>>16);
    
    return (int32_t)(r>>31);
}
