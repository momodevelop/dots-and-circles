/* date = June 27th 2021 1:36 pm */

#ifndef MOMO_RAYLINES_H
#define MOMO_RAYLINES_H
struct line2f {
    v2f min;
    v2f max;
    
    static inline line2f create(v2f min, v2f max);
    static inline line2f create(f32 min_x, f32 min_y, f32 max_x, f32 max_y);
};


struct ray2f {
    v2f origin;
    v2f dir;
    
    static inline ray2f create(line2f l);
};

struct Ray_Intersect_Times {
    f32 lhs;
    f32 rhs;
};


//~ functions
inline line2f
line2f::create(v2f min, v2f max) {
    return { min, max };
}

inline line2f
line2f::create(f32 MinX, f32 MinY, f32 MaxX, f32 MaxY) {
    v2f Min = { MinX, MinY };
    v2f Max = { MaxX, MaxY };
    
    return create(Min, Max);
    
}

inline ray2f
ray2f::create(line2f l) {
    ray2f ret = {};
    ret.origin = l.min;
    ret.dir = l.max -l.min;
    return ret;
}

static inline Ray_Intersect_Times 
get_intersection_times(ray2f lhs, ray2f rhs) {
    f32 t1;
    f32 t2;
    
    v2f p1 = lhs.origin;
    v2f p2 = rhs.origin;
    v2f v1 = lhs.dir;
    v2f v2 = rhs.dir;
    
    
    t2 = (v1.x*p2.y - v1.x*p1.y - v1.y*p2.x + v1.y*p1.x)/(v1.y*v2.x - v1.x*v2.y);
    t1 = (p2.x + t2*v2.x - p1.x)/v1.x;
    
    return { t1, t2 };
}

static inline v2f 
get_point(ray2f r, f32 time) {
    // O + D * T
    return r.origin + (r.dir * time);
}



#endif //MOMO_RAYLINES_H
