/* Raymath-like math module for general use math, such as:
 * - Scalars
 * - Vectors
 * - Geomertic figures: Circles, Rectangles, Triangles, etc.
 *
 *   Difference between raymath and hc_math in:
 *   - being single-header library, no weird linking errors when including raymath without raylib.
 *   - use of anon union, struct types for "swizzling" (REMOVE VIA FLAG).
 *   - support for interger version of all types. 
 */
#ifndef __HC_MATH_H
#define __HC_MATH_H

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>


/************************|Prerequisites, dependencies.|************************/

#ifdef HC_MATH_PEDANTIC_C89
#   if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#       define HC_MATH_LIMITED_FEATURES
#   endif
#endif

#ifndef HC_STATIC_CONST
#   define HC_CONST const
#else
#   define HC_CONST static const
#endif

#ifndef HC_HINT_INLINE
#   if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)
#       define HC_HINT_INLINE static inline
#   else 
#       define HC_HINT_INLINE /**/
#   endif
#endif

#ifndef __HC_FORMAT
#include <stdarg.h>
#ifndef FORMAT_MAX_BUFFERS  
#   define FORMAT_MAX_BUFFERS  16
#endif

#ifndef FORMAT_MAX_CHARS
#   define FORMAT_MAX_CHARS    (1024 * 2)
#endif

/*This function works since GNU C89(90,ANSI), 
 * I HATE REGULAR C89 ISO. NOT SUPPORTED. PERIOD.*/
const char* hc_format(const char* fmt, ...) {
    va_list     args, args_len;
    static int  n;
    static char memory[FORMAT_MAX_BUFFERS][FORMAT_MAX_CHARS];
    int current = n;
    size_t size = 0;

    va_start(args, fmt);

    va_copy(args_len, args);
    size = vsnprintf(NULL,0,fmt,args_len);
    va_end(args_len);
    size++; /* vsnprintf is weird. */

    assert(size <= FORMAT_MAX_CHARS);
    vsnprintf((void*)(memory[n]), size, fmt, args);
    memory[n][size-1] = 0; /* vsnprintf is weird. */
    n = (n + 1) % FORMAT_MAX_BUFFERS;

    return memory[current];
}
#endif/*__HC_FORMAT*/

#ifndef __HC_STRING_H
typedef const char* hc_CString;
#endif

/***********************************|Types|************************************/

/*Vector 2*/
#ifdef HC_MATH_LIMITED_FEATURES
typedef struct { int    x, y; } hc_VectorI2;
typedef struct { float  x, y; } hc_VectorF2;
#else
typedef struct { int    x, y; } hc_VectorI2;
typedef union { 
    struct {float  x, y;}; 
    struct {int    x_as_int, y_as_int;}; 
} hc_VectorF2;
#endif

/*Vector 3*/
#ifdef HC_MATH_LIMITED_FEATURES
typedef struct { int    x, y, z; } hc_VectorI3;
typedef struct { float  x, y, z; } hc_VectorF3;
#else
typedef union { 
    struct { int    x, y, z;            };
    struct { hc_VectorI2    xy; int _z; };
    struct { int _x; hc_VectorI2   yz;  };
} hc_VectorI3;

typedef union { 
    struct { float  x, y, z;            };
    struct { hc_VectorI2    xy;float _z;};
    struct { float _x; hc_VectorI2   yz;};
} hc_VectorF3;
#endif

/*Axis Aligned Rectangle */
#ifdef HC_MATH_LIMITED_FEATURES

/*Pedantic ANSI, no anon unions, structs. */
typedef struct { 
    int     x, y, w, h; 
} hc_RectangleI;

typedef struct { 
    float   x, y, w, h; 
} hc_RectangleF;

#else/*!HC_MATH_LIMITED_FEATURES*/

/*Union trick is supported */
typedef union {
    struct { int x, y, w, h; };
    struct { hc_VectorI2 xy;  hc_VectorI2 wh; };

#   ifdef HC_MATH_CUSTOM_RECTANGLE_OF_INT_OVERLOAD
    HC_MATH_CUSTOM_RECTANGLE_OF_INT_OVERLOAD
#   endif
} hc_RectangleI;


typedef union {
    struct { float x, y, w, h; };
    struct { hc_VectorF2 xy;  hc_VectorF2 wh; };

#   ifdef HC_MATH_CUSTOM_RECTANGLE_OF_FLOAT_OVERLOAD
    HC_MATH_CUSTOM_RECTANGLE_OF_FLOAT_OVERLOAD
#   endif
} hc_RectangleF;

#endif/*HC_MATH_LIMITED_FEATURES*/


/*Formatting*/


/*Constants*/

#define HC_QUIET_NAN    (0x7ff80000)
#define HC_NAN          HC_QUIET_NAN

HC_CONST hc_VectorI2    HC_VI2_ZERO     = {0           , 0            };
HC_CONST hc_VectorI2    HC_VI2_UNIT     = {1           , 1            };

HC_CONST hc_VectorF2    HC_VF2_ZERO     = {.x=0.0f        , .y=0.0f         };
HC_CONST hc_VectorF2    HC_VF2_UNIT     = {.x=1.0f        , .y=1.0f         };

#ifndef HC_MATH_LIMITED_FEATURES
#define                 HC_VF2_NAN      HC_VF2_QNAN
HC_CONST hc_VectorF2    HC_VF2_QNAN     = {.x_as_int=HC_QUIET_NAN, .y_as_int=HC_QUIET_NAN };
#endif

HC_CONST hc_RectangleF  HC_RECF_ZERO    = {.x=0.0f, .y=0.0f, .w=0.0f, .h=0.0f};
HC_CONST hc_RectangleF  HC_RECF_UNIT    = {.x=1.0f, .y=1.0f, .w=1.0f, .h=1.0f};

#ifndef HC_MATH_LIMITED_FEATURES
#define                 HC_RECF_NAN      HC_RECF_QNAN
HC_CONST hc_RectangleF  HC_RECF_QNAN    = {.x=HC_QUIET_NAN, .y=HC_QUIET_NAN, .w=HC_QUIET_NAN, .h=HC_QUIET_NAN};
#endif

HC_CONST hc_RectangleI  HC_RECI_ZERO    = {.x=0, .y=0, .w=0, .h=0};
HC_CONST hc_RectangleI  HC_RECI_UNIT    = {.x=1, .y=1, .w=1, .h=1};

/**********************************|Macro's|***********************************/

/*Taken from raymath.h*/
#ifndef HC_PI
    #define HC_PI 3.14159265358979323846f
#endif

#ifndef HC_EPSILON
    #define HC_EPSILON 0.00001f /*was 0.000001f*/ 
#endif

#ifndef HC_DEG2RAD
    #define HC_DEG2RAD (HC_PI/180.0f)
#endif

#ifndef HC_RAD2DEG
    #define HC_RAD2DEG (180.0f/HC_PI)
#endif

#ifndef hc_min
#   define hc_min(a,b) ( ((a) < (b)) ? (a) : (b) )
#endif

#ifndef hc_max
#   define hc_max(a,b) ( ((a) > (b)) ? (a) : (b) )
#endif 

/*THE min-max function :O*/
#ifndef hc_clamp
#   define hc_clamp(v,a,b) hc_min((b), hc_max((a), (v)))
#endif

#ifndef hc_cast_int
#   define hc_cast_int(V) ((int)(V))
#endif

#ifndef hc_cast_unsigned
#   define hc_cast_unsigned(V) ((unsigned)(V))
#endif

#ifndef hc_cast_float
#   define hc_cast_float(V) ((float)(V))
#endif

#ifndef hc_cast_double
#   define hc_cast_double(V) ((double)(V))
#endif

#ifndef hc_cast_size
#   define hc_cast_size(V) ((size_t)(V))
#endif

#ifndef hc_cast_ssize
#   define hc_cast_ssize(V) ((ssize_t)(V))
#endif


HC_HINT_INLINE ssize_t  hc_isqrt(ssize_t n);
HC_HINT_INLINE float    hc_fsqrt(float n);
HC_HINT_INLINE float    hc_atan2f(float det, float dot);
HC_HINT_INLINE float    hc_lerp(float begin, float end, float scale);

/**************************|Vector2 (Int)|***************************/
HC_HINT_INLINE hc_VectorI2  hc_vi2(int x, int y);
HC_HINT_INLINE hc_CString   hc_vi2_format(hc_VectorI2);

hc_VectorI2 hc_vi2_add(hc_VectorI2 v1, hc_VectorI2 v2);
hc_VectorI2 hc_vi2_sub(hc_VectorI2 v1, hc_VectorI2 v2);
hc_VectorI2 hc_vi2_mul(hc_VectorI2 v1, hc_VectorI2 v2);
hc_VectorI2 hc_vi2_sub(hc_VectorI2 v1, hc_VectorI2 v2);

hc_VectorI2 hc_vi2_addval(hc_VectorI2 v1, int v);
hc_VectorI2 hc_vi2_subval(hc_VectorI2 v1, int v);
hc_VectorI2 hc_vi2_mulval(hc_VectorI2 v1, int v);
hc_VectorI2 hc_vi2_subval(hc_VectorI2 v1, int v);

int         hc_vi2_length(hc_VectorI2 v);
int         hc_vi2_dot(hc_VectorI2 v1, hc_VectorI2 v2);
int         hc_vi2_cross(hc_VectorI2 v1, hc_VectorI2 v2);
int         hc_vi2_distance(hc_VectorI2 v1, hc_VectorI2 v2);
float       hc_vi2_angle(hc_VectorI2 v1, hc_VectorI2 v2);
unsigned    hc_vi2_angle_deg(hc_VectorI2 v1, hc_VectorI2 v2);


/*************************|Vector2 (Float)|**************************/
HC_HINT_INLINE hc_VectorF2  hc_vf2(float x, float y);
HC_HINT_INLINE hc_CString   hc_vf2_format(hc_VectorF2);
HC_HINT_INLINE hc_CString   hc_vf2_format_short(hc_VectorF2 v);

hc_VectorF2 hc_vf2_add(hc_VectorF2 v1, hc_VectorF2 v2);
hc_VectorF2 hc_vf2_sub(hc_VectorF2 v1, hc_VectorF2 v2);
hc_VectorF2 hc_vf2_mul(hc_VectorF2 v1, hc_VectorF2 v2);
hc_VectorF2 hc_vf2_sub(hc_VectorF2 v1, hc_VectorF2 v2);

hc_VectorF2 hc_vf2_addval(hc_VectorF2 v1, float v);
hc_VectorF2 hc_vf2_subval(hc_VectorF2 v1, float v);
hc_VectorF2 hc_vf2_mulval(hc_VectorF2 v1, float v);
hc_VectorF2 hc_vf2_subval(hc_VectorF2 v1, float v);

HC_HINT_INLINE hc_VectorF2  hc_vf2_neg(hc_VectorF2 v1);
HC_HINT_INLINE hc_VectorF2  hc_vf2_normal(hc_VectorF2 v);
hc_VectorF2                 hc_vf2_normalize(hc_VectorF2 v);

float       hc_vf2_length(hc_VectorF2 v);
float       hc_vf2_dot(hc_VectorF2 v1, hc_VectorF2 v2);
float       hc_vf2_cross(hc_VectorF2 v1, hc_VectorF2 v2);
float       hc_vf2_distance(hc_VectorF2 v1, hc_VectorF2 v2);
float       hc_vf2_angle(hc_VectorF2 v1, hc_VectorF2 v2);
unsigned    hc_vf2_angle_deg(hc_VectorF2 v1, hc_VectorF2 v2);


/****************************|Implementation block|****************************/
#ifndef HC_MATH_HEADER_ONLY

HC_HINT_INLINE ssize_t hc_isqrt(ssize_t n) {
    return sqrt(n);
}

HC_HINT_INLINE float hc_fsqrt(float n) {
    return sqrt(n);
}

HC_HINT_INLINE float hc_atan2f(float det, float dot) {
    return atan2f(det, dot);
}

HC_HINT_INLINE float hc_lerp(float begin, float end, float scale) {
    return begin + scale*(end-begin);
}

HC_HINT_INLINE float hc_norm(float value, float begin, float end) {
    return (value - begin)/(end - begin);
}

/***************************|Vector of Integers [2]|***************************/
HC_HINT_INLINE hc_VectorI2 hc_vi2(int x, int y) {
    hc_VectorI2 v = {x,y}; return v;
}

HC_HINT_INLINE hc_CString hc_vi2_format(hc_VectorI2 v) {
    return hc_format("%i, %i", v.x, v.y);
}

HC_HINT_INLINE bool hc_vi2_equal(hc_VectorI2 v1, hc_VectorI2 v2) {
    return v1.x==v2.x && v1.y==v2.y;
}

HC_HINT_INLINE bool hc_vi2_equalval(hc_VectorI2 v1, int value) {
    return v1.x==value && v1.y==value;
}

hc_VectorI2 hc_vi2_add(hc_VectorI2 v1, hc_VectorI2 v2) {
    hc_VectorI2 v = { v1.x + v2.x, v1.y + v2.y }; return v;
}

hc_VectorI2 hc_vi2_sub(hc_VectorI2 v1, hc_VectorI2 v2) {
    hc_VectorI2 v = { v1.x - v2.x, v1.y - v2.y }; return v;
}

hc_VectorI2 hc_vi2_mul(hc_VectorI2 v1, hc_VectorI2 v2) {
    hc_VectorI2 v = { v1.x * v2.x, v1.y * v2.y }; return v;
}

hc_VectorI2 hc_vi2_div(hc_VectorI2 v1, hc_VectorI2 v2) {
    hc_VectorI2 v = { v1.x / v2.x, v1.y / v2.y }; return v;
}

hc_VectorI2 hc_vi2_addval(hc_VectorI2 v1, int val) {
    hc_VectorI2 v = { v1.x + val, v1.y + val }; return v;
}

hc_VectorI2 hc_vi2_subval(hc_VectorI2 v1, int val) {
    hc_VectorI2 v = { v1.x - val, v1.y - val }; return v;
}

hc_VectorI2 hc_vi2_mulval(hc_VectorI2 v1, int val) {
    hc_VectorI2 v = { v1.x * val, v1.y * val }; return v;
}

hc_VectorI2 hc_vi2_divval(hc_VectorI2 v1, int val) {
    hc_VectorI2 v = { v1.x / val, v1.y / val }; return v;
}

int hc_vi2_length(hc_VectorI2 v) {
    return hc_isqrt(v.x*v.x + v.y*v.y); 
}

int hc_vi2_dot(hc_VectorI2 v1, hc_VectorI2 v2) {
    return hc_isqrt(v1.x*v2.x + v1.y*v2.y); 
}

int hc_vi2_cross(hc_VectorI2 v1, hc_VectorI2 v2) {
    return hc_isqrt(v1.x*v2.y + v1.y*v2.x); 
}

int hc_vi2_distance(hc_VectorI2 v1, hc_VectorI2 v2) {
    return hc_isqrt((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y)); 
}

float hc_vi2_angle(hc_VectorI2 v1, hc_VectorI2 v2) {
    return hc_vf2_angle(hc_vf2(v1.x,v1.y),hc_vf2(v2.x,v2.y));
}

unsigned hc_vi2_angle_deg(hc_VectorI2 v1, hc_VectorI2 v2) {
    return hc_cast_unsigned(hc_vi2_angle(v1,v2) * HC_RAD2DEG);
}


/****************************|Vector of Floats [2]|****************************/

/*aliases*/
#define hc_vf2_scale hc_vf2_mulval

HC_HINT_INLINE hc_VectorF2 hc_vf2(float x, float y) {
    hc_VectorF2 v = {.x=x,.y=y}; return v;
}

HC_HINT_INLINE hc_CString hc_vf2_format_short(hc_VectorF2 v) {
    return hc_format("%.2f, %.2f", v.x, v.y);
}

HC_HINT_INLINE hc_CString hc_vf2_format(hc_VectorF2 v) {
    return hc_format("%.f, %.f", v.x, v.y);
}

HC_HINT_INLINE bool hc_vf2_is_nan(hc_VectorF2 v) {
    return !(v.x == v.x && v.y == v.y);
}

HC_HINT_INLINE bool hc_vf2_equal(hc_VectorF2 v1, hc_VectorF2 v2) {
    return (hc_max(v1.x,v2.x) - hc_min(v1.x,v2.x)) <= HC_EPSILON 
        && (hc_max(v1.y,v2.y) - hc_min(v1.y,v2.y)) <= HC_EPSILON;
}

HC_HINT_INLINE bool hc_vf2_equalval(hc_VectorF2 v1, float value) {
    return (hc_max(v1.x,value) - hc_min(v1.x,value)) <= HC_EPSILON 
        && (hc_max(v1.y,value) - hc_min(v1.y,value)) <= HC_EPSILON;
}

hc_VectorF2 hc_vf2_add(hc_VectorF2 v1, hc_VectorF2 v2) {
    hc_VectorF2 v ;  v.x=v1.x + v2.x; v.y=v1.y + v2.y; return v;
}

hc_VectorF2 hc_vf2_sub(hc_VectorF2 v1, hc_VectorF2 v2) {
    hc_VectorF2 v ; v.x= v1.x - v2.x;v.y = v1.y - v2.y; return v;
}

hc_VectorF2 hc_vf2_mul(hc_VectorF2 v1, hc_VectorF2 v2) {
    hc_VectorF2 v ; v.x= v1.x * v2.x;v.y = v1.y * v2.y; return v;
}

hc_VectorF2 hc_vf2_div(hc_VectorF2 v1, hc_VectorF2 v2) {
    hc_VectorF2 v ; v.x= v1.x / v2.x;v.y = v1.y / v2.y; return v;
}

hc_VectorF2 hc_vf2_addval(hc_VectorF2 v1, float val) {
    hc_VectorF2 v ; v.x= v1.x + val;v.y = v1.y + val; return v;
}

hc_VectorF2 hc_vf2_subval(hc_VectorF2 v1, float val) {
    hc_VectorF2 v ; v.x= v1.x - val;v.y = v1.y - val; return v;
}

hc_VectorF2 hc_vf2_mulval(hc_VectorF2 v1, float val) {
    hc_VectorF2 v ; v.x= v1.x * val;v.y = v1.y * val; return v;
}

hc_VectorF2 hc_vf2_divval(hc_VectorF2 v1, float val) {
    hc_VectorF2 v; v.x=v1.x / val; v.y = v1.y / val; return v;
}

HC_HINT_INLINE hc_VectorF2 hc_vf2_neg(hc_VectorF2 v1) {
    v1.x *= -1; v1.y *= -1; return v1; 
}

HC_HINT_INLINE hc_VectorF2 hc_vf2_normal(hc_VectorF2 v) {
    return hc_vf2(-v.y, v.x);
}

hc_VectorF2 hc_vf2_normalize(hc_VectorF2 v) {
    hc_VectorF2 r = {0};
    float length = hc_fsqrt((v.x*v.x) + (v.y*v.y));
    if (length > 0)
    {
        float ilength = 1.0f/length;
        r.x = v.x*ilength;
        r.y = v.y*ilength;
    }
    return r;
}


float hc_vf2_length(hc_VectorF2 v) {
    return hc_fsqrt(v.x*v.x + v.y*v.y); 
}

float hc_vf2_dot(hc_VectorF2 v1, hc_VectorF2 v2) {
    return hc_fsqrt(v1.x*v2.x + v1.y*v2.y); 
}

float hc_vf2_cross(hc_VectorF2 v1, hc_VectorF2 v2) {
    return hc_fsqrt(v1.x*v2.y + v1.y*v2.x); 
}

float hc_vf2_distance(hc_VectorF2 v1, hc_VectorF2 v2) {
    return hc_fsqrt((v1.x - v2.x)*(v1.x - v2.x) + (v1.y - v2.y)*(v1.y - v2.y)); 
}

float hc_vf2_angle(hc_VectorF2 v1, hc_VectorF2 v2) {
    float result = 0.0f;
    float dot = v1.x*v2.x + v1.y*v2.y;
    float det = v1.x*v2.y - v1.y*v2.x;
    result = atan2f(det, dot);
    return result;
}

/****************************|Rectangle of Integer|****************************/
HC_HINT_INLINE hc_RectangleI hc_reci(int x, int y, int w, int h) {
    hc_RectangleI r = {.x=x,.y=y,.w=w,.h=h}; return r;
}

HC_HINT_INLINE hc_RectangleI hc_reci_from_vi2(hc_VectorI2 xy, hc_VectorI2 wh) {
    hc_RectangleI r = {.x=xy.x,.y=xy.y,.w=wh.x,.h=wh.y}; return r;
}

/*****************************|Rectangle of Float|*****************************/
HC_HINT_INLINE hc_RectangleF hc_recf(float x, float y, float w, float h) {
    hc_RectangleF r = {.x=x,.y=y,.w=w,.h=h}; return r;
}

HC_HINT_INLINE hc_RectangleF hc_recf_from_vf2(hc_VectorF2 xy, hc_VectorF2 wh) {
    hc_RectangleF r = {.x=xy.x,.y=xy.y,.w=wh.x,.h=wh.y}; return r;
}

HC_HINT_INLINE bool hc_recf_is_nan(hc_RectangleF r) {
    return !(r.x == r.x && r.y == r.y && r.w == r.w && r.h == r.h);
}

#endif/*HC_MATH_HEADER_ONLY*/
#endif/*__HC_MATH_H*/
