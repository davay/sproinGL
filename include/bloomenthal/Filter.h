/* ===========================================================
   Filter.h - definitions for filter data types and operations
   Jules Bloomenthal, Unchained Geometry, Seattle, 1998.
   Copyright (c) 2004 Unchained Geometry Inc.
   =========================================================== */

#ifndef FILTER_HDR
#define FILTER_HDR

#include <float.h>
#include <math.h>
#include <vector>
#include <assert.h>

bool IsZero(float x) { return fabs(x) < FLT_EPSILON; }

#define F_PI 3.1415926535f
//#define _PI 3.1415926535897932384626433832795028

//****** Filter Class and Derived Filters

class Filter {
public:
    Filter() { };
    virtual float Function(float x) {
        // derived filters must provide: compute filter value at
        assert("not implemented" == NULL);
        return 0;
    };
};

class FilterPoint : public Filter {
public:
    // pulse, Fourier window
    virtual float Function(float x) {return IsZero(x)? 1 : 0;};
};


class FilterBox : public Filter {
public:
    // box, pulse, Fourier window (cardinal, unit-range, support .5)
    virtual float Function(float x) {
        // 1st order (constant) b-spline
        return x < -.5? 0 : x < .5? 1 : 0;
    };
};

class FilterTriangle : public Filter {
public:
    // triangle, Bartlett window (2nd order (linear) b-spline)
    // (cardinal, unit-range, support 1)
    virtual float Function(float x) {
          return x < -1? 0 : x < 0? 1+x : x < 1? 1-x : 0;
    };
};

class FilterQuadratic : public Filter {
public:
    // 3rd order (quadratic) b-spline (not cardinal, unit-range, support 1.5)
    virtual float Function(float x) {
        if (x < -1.5) return 0.;
        if (x <  -.5) {float t = x+1.5; return .5*t*t;}
        if (x <   .5) return .75-x*x;
        if (x <  1.5) {float t = x-1.5; return .5*t*t;}
        return 0.;
    };
};

class FilterCubic : public Filter {
public:
    // 4th order (cubic) b-spline (not carinal, unit-range, support 2)
    virtual float Function(float x) {
        if (x < -2.) return 0.;
        if (x < -1.) {float t = 2.+x; return t*t*t/6.;}
        if (x <  0.) return (4.+x*x*(-6.+x*-3.))/6.;
        if (x <  1.) return (4.+x*x*(-6.+x*3.))/6.;
        if (x <  2.) {float t = 2.-x; return t*t*t/6.;}
        return 0.;
    };
};

class FilterWyvill : public Filter {
public:
    // Geoff and Brian's 'blob' filter (cardinal, unit-rage, support 1)
    FilterWyvill() {
        halfArea = AreaFromZeroX(1);
    }
    virtual float Function(float x) {
        if (x < 0)
            x = -x;
        if (x < FLT_EPSILON)
            return 1;
        if (x >= 1)
            return 0;
        float x2 = x*x;
        float x4 = x2*x2;
        return (-4.0/9.0)*x2*x4+(17.0/9.0)*x4+(-22.0/9.0)*x2+1;
    }
private:
    float halfArea;
    float Area(float x) {
        return x < 0? halfArea-AreaFromZeroX(-x) : halfArea+AreaFromZeroX(x);
    }
    float AreaFromZeroX(float x) {
        if (x > 1)
            return halfArea;
        float x2 = x*x;
        float x3 = x2*x;
        float x5 = x3*x2;
        return -(1./7.)*(4./9.)*x2*x5+(1./5.)*(17./9.)*x5-(1./3.)*(22./9.)*x3+x;
    }
};

class FilterGauss : public Filter {
public:
    // Gaussian (not cardinal, unit-range, support 1.25, infinite impulse response)
    FilterGauss() {
        gaussTableXRange = 10.0;
        gaussTableSize = 10000;
        gaussTableScale = gaussTableSize/gaussTableXRange;
        gaussTableDx = 1.0/gaussTableScale;
        gaussTable = NULL;
    };
    ~FilterGauss() {
        delete [] gaussTable;
    };
    virtual float Function(float x) {
        static float sqrt2PI = sqrt(2./F_PI);
        return exp(-2.*x*x)*sqrt2PI;
    };
private:
    void MakeGaussTable() {
        gaussTable = (float *) new float[gaussTableSize];
        assert(gaussTable != NULL);
        float scale = gaussTableDx*0.5;
        float value1 = Function(0);
        gaussTable[0] = 0.5;                       // area from -inf to 0
        for (int i = 1; i < gaussTableSize; i++) { // create table from x = 0 to tableXRange
            float value2 = Function(i*gaussTableDx);
            gaussTable[i] = gaussTable[i-1]+scale*(value1+value2);
            value1 = value2;
        }
    };
    int Min(int a, int b) {return a < b? a : b;}
    float Area(float x) {return x < 0? 1.0-AreaForPosX(-x) : AreaForPosX(x);};
    float AreaForPosX(float x) {
        float scaledX = x*gaussTableScale;
        int index = Min(gaussTableSize-1, (int) floor(scaledX));
        float area = gaussTable[index];
        if (index < gaussTableSize-1)              // linear interpolation
            area += (scaledX-index)*(gaussTable[index+1]-area);
        return area;
    };
    float  gaussTableXRange;
    int     gaussTableSize;
    float  gaussTableScale;
    float  gaussTableDx;
    float *gaussTable;
};

class FilterSinc : public Filter {
public:
    // Sinc (perfect lowpass filter, cardinal, unit-range, support 4, infinite impulse response)
    virtual float Function(float x) {
        float pix = F_PI*x;
        return x==0. ? 1. : sin(pix)/(pix);
    };
};

#endif
