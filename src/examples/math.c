#include <hc_memory.h>

typedef struct {
    float x, y, width, height;
} Rectangle; /*Pretend this is raylib Rectangle*/
#define HC_MATH_CUSTOM_RECTANGLE_OF_FLOAT_OVERLOAD\
    Rectangle raylib_rec;

#include <hc_math.h>

int main(void) {
    hc_RectangleF r = hc_recf_from_vf2(hc_vf2(100,100), hc_vf2(200, 200));
    printf("r.raylib_rec.(width, height) = %f, %f\n", r.raylib_rec.width, r.raylib_rec.height);

    printf("you can do limited form of 'component swizzling':\n"
            "\tr.xy = %s, r.wh = %s\n", 
            hc_vf2_format(r.xy), 
            hc_vf2_format(r.wh)
    );

    assert( hc_vf2_equal(
                hc_vf2(200,200), 
                hc_vf2_scale(hc_vf2(100,100), 2)
            ));

    printf("VF2_NAN = (%s), Is nan - %s ?\n",
            hc_vf2_format(HC_VF2_NAN),
            hc_vf2_is_nan(HC_VF2_NAN)? "true" : "false");
}
