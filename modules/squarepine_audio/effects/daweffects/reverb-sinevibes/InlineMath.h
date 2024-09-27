/*

InlineMath.h

(c) Artemiy Pavlov
Sinevibes (www.sinevibes.com)

*/

#ifndef InlineMath_h
#define InlineMath_h

inline float inlineTaylorSine( float phase )
{
    
    float p = 6.283185307179586f * phase;
    float p2 = p * p;
    
    return p - p2 * p * 0.166666666666667f + p2 * p2 * p * 0.008333333333333f;
    
}

inline float inlineInverseParabola( float phase )
{
    
    phase *= 4.f;
    
    return 1.f - ( 1.f - phase ) * ( 1.f - phase );
    
}

inline float inlineLinearInterpolation( float x0, float x1, float d )
{
        
    return d * ( x1 - x0 ) + x0;
        
}
        
inline float inlineSplineInterpolation( float x0, float x1, float x2, float x3, float d )
{
    
    float c0 = x1;
    float c1 = 0.5f * ( x2 - x0 );
    float c2 = x0 - 2.5f * x1 + 2.0f * x2 - 0.5f * x3;
    float c3 = 0.5f * ( x3 - x0 ) + 1.5f * ( x1 - x2 );
    
    return ( ( c3 * d + c2 ) * d + c1 ) * d + c0;
    
}

#endif /* InlineMath_h */
