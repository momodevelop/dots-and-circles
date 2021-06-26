/* date = April 26th 2021 5:36 pm */

#ifndef MOMO_BONK_H
#define MOMO_BONK_H

static inline b8
Bonk2_IsCircleXCircle(circle2f L, circle2f R) {
	f32 DistSq = distance_sq(L.Origin, R.Origin);
	f32 RSq = L.Radius + R.Radius;
    RSq *= RSq;
	return DistSq < RSq;
}

static inline b8
Bonk2_IsCircleXLine(circle2f C, line2f L) {
    // NOTE(Momo): Extend the ends of the lines based on radius of the circle, and use that to form a parametric equation of the line (ray)
    ray2f R = Ray2f_CreateFromLine2f(L);
    v2f NormalizedDir = normalize(R.Dir);
    L.Min = sub(L.Min, mul(NormalizedDir, C.Radius));
    L.Max = add(L.Max, mul(NormalizedDir, C.Radius));
    R = Ray2f_CreateFromLine2f(L);
    
    v2f OriginToCircle = sub(C.Origin, R.Origin);
    v2f OriginToClosestPtOnLine = project(OriginToCircle, R.Dir); 
    v2f ClosestPtOnLine = add(R.Origin, OriginToClosestPtOnLine);
    
    // NOTE(Momo): Find the time of intersection of the ClosestPtOnLine
    f32 Time = {}; 
    if (!is_equal(R.Dir.x, 0.f)) {
        Time = (ClosestPtOnLine.x - R.Origin.x)/R.Dir.x;
    }
    else if (!is_equal(R.Dir.y, 0.f)) {
        Time = (ClosestPtOnLine.y - R.Origin.y)/R.Dir.y;
    }
    else {
        return false;
    }
    
    if (Time < 0.f || Time > 1.f) {
        return false;
    }
    
    // NOTE(Momo): At this point, we are within range of the line segment, so we just have to check if the circle's radius is greater than its distance from the line.
    f32 CircleRadiusSq = C.Radius * C.Radius;
    f32 CircleDistFromLineSq = distance_sq(OriginToCircle, 
                                           OriginToClosestPtOnLine);
    return CircleRadiusSq > CircleDistFromLineSq;
}

// NOTE(Momo): DynaCircle is short for 'dynamic circle'
static inline b8
Bonk2_IsDynaCircleXCircle(circle2f DynaCircle,
                          v2f Velocity,
                          circle2f Circle) 
{
    line2f Line = Line2f_CreateFromV2f(DynaCircle.Origin,
                                       add(DynaCircle.Origin, Velocity));
    Circle.Radius += DynaCircle.Radius;
    return Bonk2_IsCircleXLine(Circle, Line);
}

static inline b8
Bonk2_IsDynaCircleXDynaCircle(circle2f CircleA,
                              v2f VelocityA,
                              circle2f CircleB,
                              v2f VelocityB) 
{
    
    f32 VelAMag = length_sq(VelocityA);
    f32 VelBMag = length_sq(VelocityB);
    b8 VelADead = is_equal(VelAMag, 0.f);
    b8 VelBDead = is_equal(VelBMag, 0.f);
    
    
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
        v2f RelativeVel = sub(VelocityB, VelocityA); 
        return Bonk2_IsDynaCircleXCircle(CircleB, RelativeVel, CircleA);
    }
}

#endif //MOMO_BONK_H
