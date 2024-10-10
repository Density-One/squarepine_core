/*
 
 SinCosTable.h
 
 (c) Artemiy Pavlov
 Sinevibes (www.sinevibes.com)
 
 */

#ifndef SinCosTable_h
#define SinCosTable_h

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <cstring>

#define SIN_COS_TABLE_SIZE 2048

class SinCosTable
{
    
private:
    
    int tableSize;
    float sinTable[SIN_COS_TABLE_SIZE];
    float cosTable[SIN_COS_TABLE_SIZE];
    float invTableSize;
    float range;
    float tableRange;
    
public:
    
    SinCosTable()
    {
        
        tableSize = SIN_COS_TABLE_SIZE;
        invTableSize = 1.f / (float)tableSize;
        range = 2.f * M_PI;
        tableRange = tableSize / range;
        
        for( int i = 0; i < tableSize; i++ )
        {
            
            sinTable[i] = sinf( range * i * invTableSize );
            cosTable[i] = cosf( range * i * invTableSize );
            
        }
        
    }
    
    float sin( float phase )
    {
        
        float address = phase * (float)tableRange;
        
        int index1 = (int)address;
        int index2 = index1 + 1;
        
        if( index1 >= tableSize ) index1 -= tableSize;
        if( index2 >= tableSize ) index2 -= tableSize;
        
        return sinTable[index1] + ( address - index1 ) * ( sinTable[index2] - sinTable[index1] );
        
    }
    
    float cos( float phase )
    {
        
        float address = phase * (float)tableRange;
        
        int index1 = (int)address;
        int index2 = index1 + 1;
        
        if( index1 >= tableSize ) index1 -= tableSize;
        if( index2 >= tableSize ) index2 -= tableSize;
        
        return cosTable[index1] + ( address - index1 ) * ( cosTable[index2] - cosTable[index1] );
        
    }
    
    void sinBlock( float *phase, float *sin, int blockSize )
    {
        
        for( int i = blockSize; i--; )
        {
            
            float address = phase[i] * (float)tableRange;
            
            int index1 = (int)address;
            int index2 = index1 + 1;
            
            if( index1 >= tableSize ) index1 -= tableSize;
            if( index2 >= tableSize ) index2 -= tableSize;
            
            sin[i] = sinTable[index1] + ( address - index1 ) * ( sinTable[index2] - sinTable[index1] );
            
        }
        
    }
    
    void sinCosBlock( float *phase, float *sin, float *cos, int blockSize )
    {
        
        for( int i = blockSize; i--; )
        {
            
            float address = phase[i] * (float)tableRange;
            
            int index1 = (int)address;
            int index2 = index1 + 1;
            
            if( index1 >= tableSize ) index1 -= tableSize;
            if( index2 >= tableSize ) index2 -= tableSize;
            
            sin[i] = sinTable[index1] + ( address - index1 ) * ( sinTable[index2] - sinTable[index1] );
            cos[i] = cosTable[index1] + ( address - index1 ) * ( cosTable[index2] - cosTable[index1] );
            
        }
        
    }
    
};

#endif /* SinCosTable_h */
