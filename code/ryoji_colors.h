
#ifndef RYOJI_COLORS_H
#define RYOJI_COLORS_H


union c4f {
    struct {
        f32 Red, Green, Blue, Alpha;
    };
    struct {
        f32 Hue, Saturation, Value, Alpha;
    };
    f32 Arr[4];
    
    inline const f32 operator[](usize index) const { return Arr[index]; }
    inline f32 operator[](usize index) { return Arr[index];}
};



#endif //RYOJI_COLORS_H
