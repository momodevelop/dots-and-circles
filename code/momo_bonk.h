/* date = April 26th 2021 5:36 pm */

#ifndef MOMO_BONK_H
#define MOMO_BONK_H

static inline b8
Bonk2_IsCircleXCircle(circle2f L, circle2f R) {
	f32 DistSq = DistanceSq(L.origin, R.origin);
	f32 RSq = L.radius + R.radius;
    RSq *= RSq;
	return DistSq < RSq;
}

static inline b8
Bonk2_IsCircleXLine(circle2f C, line2f L) {
    // NOTE(Momo): Extend the ends of the lines based on radius of the circle, and use that to form a parametric equation of the line (ray)
    ray2f R = ray2f::create(L);
    v2f NormalizedDir = Normalize(R.dir);
    L.min = Sub(L.min, Mul(NormalizedDir, C.radius));
    L.max = Add(L.max, Mul(NormalizedDir, C.radius));
    R = ray2f::create(L);
    
    v2f OriginToCircle = Sub(C.origin, R.origin);
    v2f OriginToClosestPtOnLine = Project(OriginToCircle, R.dir); 
    v2f ClosestPtOnLine = Add(R.origin, OriginToClosestPtOnLine);
    
    // NOTE(Momo): Find the time of intersection of the ClosestPtOnLine
    f32 Time = {}; 
    if (!IsEqual(R.dir.x, 0.f)) {
        Time = (ClosestPtOnLine.x - R.origin.x)/R.dir.x;
    }
    else if (!IsEqual(R.dir.y, 0.f)) {
        Time = (ClosestPtOnLine.y - R.origin.y)/R.dir.y;
    }
    else {
        return false;
    }
    
    if (Time < 0.f || Time > 1.f) {
        return false;
    }
    
    // NOTE(Momo): At this point, we are within range of the line segment, so we just have to check if the circle's radius is greater than its distance from the line.
    f32 CircleRadiusSq = C.radius * C.radius;
    f32 CircleDistFromLineSq = DistanceSq(OriginToCircle, 
                                           OriginToClosestPtOnLine);
    return CircleRadiusSq > CircleDistFromLineSq;
}

// NOTE(Momo): DynaCircle is short for 'dynamic circle'
static inline b8
Bonk2_IsDynaCircleXCircle(circle2f DynaCircle,
                          v2f Velocity,
                          circle2f Circle) 
{
    line2f Line = line2f::create(DynaCircle.origin,
                                 DynaCircle.origin + Velocity);
    Circle.radius += DynaCircle.radius;
    return Bonk2_IsCircleXLine(Circle, Line);
}

static inline b8
Bonk2_IsDynaCircleXDynaCircle(circle2f CircleA,
                              v2f VelocityA,
                              circle2f CircleB,
                              v2f VelocityB) 
{
    
    f32 VelAMag = LengthSq(VelocityA);
    f32 VelBMag = LengthSq(VelocityB);
    b8 VelADead = IsEqual(VelAMag, 0.f);
    b8 VelBDead = IsEqual(VelBMag, 0.f);
    
    
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
        v2f RelativeVel = Sub(VelocityB, VelocityA); 
        return Bonk2_IsDynaCircleXCircle(CircleB, RelativeVel, CircleA);
    }
}

#endif //MOMO_BONK_H
