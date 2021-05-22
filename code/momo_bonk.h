/* date = April 26th 2021 5:36 pm */

#ifndef MOMO_BONK_H
#define MOMO_BONK_H

static inline b32
Bonk2_IsCircleXCircle(circle2f L, circle2f R) {
	f32 DistSq = V2f_DistanceSq(L.Origin, R.Origin);
	f32 RSq = L.Radius + R.Radius;
    RSq *= RSq;
	return DistSq < RSq;
}

static inline b32
Bonk2_IsCircleXLine(circle2f C, line2f L) {
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
        return false;
    }
    
    if (Time < 0.f || Time > 1.f) {
        return false;
    }
    
    // NOTE(Momo): At this point, we are within range of the line segment, so we just have to check if the circle's radius is greater than its distance from the line.
    f32 CircleRadiusSq = C.Radius * C.Radius;
    f32 CircleDistFromLineSq = V2f_DistanceSq(OriginToCircle, 
                                              OriginToClosestPtOnLine);
    return CircleRadiusSq > CircleDistFromLineSq;
}

// NOTE(Momo): DynaCircle is short for 'dynamic circle'
static inline b32
Bonk2_IsDynaCircleXCircle(circle2f DynaCircle,
                          v2f Velocity,
                          circle2f Circle) 
{
    line2f Line = Line2f_CreateFromV2f(DynaCircle.Origin,
                                       V2f_Add(DynaCircle.Origin, Velocity));
    Circle.Radius += DynaCircle.Radius;
    return Bonk2_IsCircleXLine(Circle, Line);
}

static inline b32
Bonk2_IsDynaCircleXDynaCircle(circle2f CircleA,
                              v2f VelocityA,
                              circle2f CircleB,
                              v2f VelocityB) 
{
    
    f32 VelAMag = V2f_LengthSq(VelocityA);
    f32 VelBMag = V2f_LengthSq(VelocityB);
    b32 VelADead = F32_IsEqual(VelAMag, 0.f);
    b32 VelBDead = F32_IsEqual(VelBMag, 0.f);
    
    
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
        v2f RelativeVel = V2f_Sub(VelocityB, VelocityA); 
        return Bonk2_IsDynaCircleXCircle(CircleB, RelativeVel, CircleA);
    }
}

#endif //MOMO_BONK_H
