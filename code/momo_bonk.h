/* date = April 26th 2021 5:36 pm */

#ifndef MOMO_BONK_H
#define MOMO_BONK_H

static inline b32
Bonk2_IsCircleXCircle(MM_Circle2f L, MM_Circle2f R) {
	f32 DistSq = MM_V2f_DistanceSq(L.origin, R.origin);
	f32 RSq = L.radius + R.radius;
    RSq *= RSq;
	return DistSq < RSq;
}

static inline b32
Bonk2_IsCircleXLine(MM_Circle2f C, MM_Line2f L) {
    // NOTE(Momo): Extend the ends of the lines based on radius of the circle, and use that to form a parametric equation of the line (ray)
    MM_Ray2f R = Ray2f_CreateFromLine2f(L);
    MM_V2f NormalizedDir = MM_V2f_Normalize(R.dir);
    L.min = MM_V2f_Sub(L.min, MM_V2f_Mul(NormalizedDir, C.radius));
    L.max = MM_V2f_Add(L.max, MM_V2f_Mul(NormalizedDir, C.radius));
    R = Ray2f_CreateFromLine2f(L);
    
    MM_V2f OriginToCircle = MM_V2f_Sub(C.origin, R.origin);
    MM_V2f OriginToClosestPtOnLine = MM_V2f_Project(OriginToCircle, R.dir); 
    MM_V2f ClosestPtOnLine = MM_V2f_Add(R.origin, OriginToClosestPtOnLine);
    
    // NOTE(Momo): Find the time of intersection of the ClosestPtOnLine
    f32 Time = {}; 
    if (!MM_Math_F32IsEqual(R.dir.x, 0.f)) {
        Time = (ClosestPtOnLine.x - R.origin.x)/R.dir.x;
    }
    else if (!MM_Math_F32IsEqual(R.dir.y, 0.f)) {
        Time = (ClosestPtOnLine.y - R.origin.y)/R.dir.y;
    }
    else {
        return False;
    }
    
    if (Time < 0.f || Time > 1.f) {
        return False;
    }
    
    // NOTE(Momo): At this point, we are within range of the line segment, so we just have to check if the circle's radius is greater than its distance from the line.
    f32 CircleRadiusSq = C.radius * C.radius;
    f32 CircleDistFromLineSq = MM_V2f_DistanceSq(OriginToCircle, 
                                              OriginToClosestPtOnLine);
    return CircleRadiusSq > CircleDistFromLineSq;
}

// NOTE(Momo): DynaCircle is short for 'dynamic circle'
static inline b32
Bonk2_IsDynaCircleXCircle(MM_Circle2f DynaCircle,
                          MM_V2f Velocity,
                          MM_Circle2f Circle) 
{
    MM_Line2f Line = Line2f_CreateFromV2f(DynaCircle.origin,
                                       MM_V2f_Add(DynaCircle.origin, Velocity));
    Circle.radius += DynaCircle.radius;
    return Bonk2_IsCircleXLine(Circle, Line);
}

static inline b32
Bonk2_IsDynaCircleXDynaCircle(MM_Circle2f CircleA,
                              MM_V2f VelocityA,
                              MM_Circle2f CircleB,
                              MM_V2f VelocityB) 
{
    
    f32 VelAMag = MM_V2f_LengthSq(VelocityA);
    f32 VelBMag = MM_V2f_LengthSq(VelocityB);
    b32 VelADead = MM_Math_F32IsEqual(VelAMag, 0.f);
    b32 VelBDead = MM_Math_F32IsEqual(VelBMag, 0.f);
    
    
    if (VelADead && VelBDead) {
        return Bonk2_IsCircleXCircle(CircleA, CircleB);
    }
    else if (VelADead) {
        return Bonk2_IsDynaCircleXCircle(CircleB, VelocityB, CircleA);
    }
    else if (VelBDead) {
        return Bonk2_IsDynaCircleXCircle(CircleA, VelocityA, CircleB);
    }
    else {
        MM_V2f RelativeVel = MM_V2f_Sub(VelocityB, VelocityA); 
        return Bonk2_IsDynaCircleXCircle(CircleB, RelativeVel, CircleA);
    }
}

#endif //MOMO_BONK_H
