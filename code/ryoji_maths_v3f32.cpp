#ifndef __RYOJI_MATH_V3F32__
#define __RYOJI_MATH_V3F32__

struct v3f32 {
    union {
        f32 arr[3];
        struct {
            float x;
            float y;
            float z;
        };	
    };
    
    inline const f32& operator[](usize index) const { 
        return arr[index]; 
    } 
    
    inline f32& operator[](usize index)  { 
        return arr[index]; 
    }
    
    
    inline v3f32& operator+=(v3f32 rhs)  {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    
    inline v3f32& operator-=(v3f32 rhs)  {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    
    
    inline v3f32& operator*=(v3f32 rhs)  {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    
    inline v3f32& operator/=(v3f32 rhs)  {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }
    
    inline v3f32& operator*=(float rhs)  {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    
    inline v3f32& operator/=(float rhs)  {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
    
};


pure v3f32 operator+(v3f32 lhs, v3f32 rhs)  { 
    return v3f32(lhs) += rhs; 
}

pure v3f32 operator-(v3f32 lhs, v3f32 rhs)  { 
    return v3f32(lhs) -= rhs;
}

pure v3f32 operator*(v3f32 lhs, float rhs)  { 
    return v3f32(lhs) *= rhs; 
}

pure v3f32 operator*(float lhs, v3f32 rhs)  { 
    return v3f32(rhs) *= lhs; 
}

pure v3f32 operator/(v3f32 lhs, float rhs)  { 
    return v3f32(lhs) /= rhs; 
}

//NOTE(Momo): Dot product
pure float operator*(v3f32 lhs, v3f32 rhs) { 
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; 
}


pure bool operator==(v3f32 lhs, v3f32 rhs)  { 
    return 
        IsFloatEq(lhs.x, rhs.x) && 
        IsFloatEq(lhs.y, rhs.y) && 
        IsFloatEq(lhs.z, rhs.z);
}

pure bool operator!=(v3f32 lhs, v3f32 rhs) { 
    return !(lhs == rhs); 
}

pure v3f32 operator-(v3f32 lhs)  {  
    return { -lhs.x, -lhs.y, -lhs.z }; 
}

pure v3f32 Midpoint(v3f32 lhs, v3f32 rhs)  { 
    return (lhs + rhs)/2.f; 
}

pure f32 DistanceSq(v3f32 lhs, v3f32 rhs) { 
    return (rhs.x - lhs.x) * (rhs.y - lhs.y) * (rhs.z - lhs.z);
}

pure f32 LengthSq(v3f32 lhs) { 
    return lhs * lhs;	
}

pure f32 Distance(v3f32 lhs, v3f32 rhs)  { 
    return Sqrt(DistanceSq(lhs, rhs)); 
}

pure f32 Length(v3f32 lhs)  { 
    return Sqrt(LengthSq(lhs));
};

pure v3f32 Normalize(v3f32 lhs)  {
    v3f32 ret = lhs;
    f32 len = Length(lhs);
    if (IsFloatEq(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

pure f32 AngleBetween(v3f32 lhs, v3f32 rhs) {
    return ACos((lhs * rhs) / (Length(lhs) * Length(rhs)));
}

pure f32 AngleBetweenNormalized(v3f32 lhs, v3f32 rhs) {
    Assert(IsFloatEqual(LengthSq(lhs), 1.f));
    Assert(IsFloatEqual(LengthSq(rhs), 1.f));
    return ACos(lhs * rhs);
}

pure bool IsPerpendicular(v3f32 lhs, v3f32 rhs) 
{ 
    return IsFloatEq((lhs * rhs), 0.f); 
}

pure bool IsSameDirection(v3f32 lhs, v3f32 rhs) { 
    return (lhs * rhs) > 0.f; 
}

pure bool IsOppositeDirection(v3f32 lhs, v3f32 rhs) { 
    return (lhs * rhs) < 0.f;
}

pure v3f32 Project(v3f32 from, v3f32 to) { 
    return (to * from) / LengthSq(to) * to;
}


#endif
