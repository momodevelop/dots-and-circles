/* date = April 26th 2021 5:36 pm */

#ifndef MOMO_BONK_H
#define MOMO_BONK_H

static inline b32
Bonk2f_IsCircleLineIntersect(circle2f C, line2f L) {
    // NOTE(Momo): Extend the ends of the lines based on radius of the circle, and use that to form a parametric equation of the line (ray)
    ray2f R = Ray2f_CreateFromLine2f(L);
    v2f NormalizedDir = V2f_Normalize(R.Dir);
    L.Min = V2f_Sub(L.Min, V2f_Mul(NormalizedDir, C.Radius));
    L.Max = V2f_Add(L.Max, V2f_Mul(NormalizedDir, C.Radius));
    R = Ray2f_CreateFromLine2f(L);
    
    v2f OriginToCircle = V2f_Sub(C.Origin, R.Origin);
    v2f OriginToClosestPtOnLine = V2f_Project(OriginToCircle, R.Dir); 
    v2f ClosestPtOnLine = V2f_Add(R.Origin, OriginToClosestPtOnLine);
    
    // NOTE(Momo): Find the time of intersection of the ClosestPtOnLine
    f32 Time = {}; 
    if (!F32_IsEqual(R.Dir.X, 0.f)) {
        Time = (ClosestPtOnLine.X - R.Origin.X)/R.Dir.X;
    }
    else if (!F32_IsEqual(R.Dir.Y, 0.f)) {
        Time = (ClosestPtOnLine.Y - R.Origin.Y)/R.Dir.Y;
    }
    else {
        return False;
    }
    
    if (Time < 0.f || Time > 1.f) {
        return False;
    }
    
    // NOTE(Momo): At this point, we are within range of the line segment, so we just have to check if the circle's radius is greater than its distance from the line.
    f32 CircleRadiusSq = C.Radius * C.Radius;
    f32 CircleDistFromLineSq = V2f_DistanceSq(OriginToCircle, 
                                              OriginToClosestPtOnLine);
    
    return CircleRadiusSq > CircleDistFromLineSq;
}

#endif //MOMO_BONK_H
