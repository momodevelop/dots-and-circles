#ifndef __RYOJI_MATH_VECTOR__
#define __RYOJI_MATH_VECTOR__

struct Vec3f {
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
    
    
    inline Vec3f& operator+=(Vec3f rhs)  {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    
    inline Vec3f& operator-=(Vec3f rhs)  {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    
    
    inline Vec3f& operator*=(Vec3f rhs)  {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    
    inline Vec3f& operator/=(Vec3f rhs)  {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }
    
    inline Vec3f& operator*=(float rhs)  {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    
    inline Vec3f& operator/=(float rhs)  {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
    
};


pure Vec3f operator+(Vec3f lhs, Vec3f rhs)  { 
    return Vec3f(lhs) += rhs; 
}

pure Vec3f operator-(Vec3f lhs, Vec3f rhs)  { 
    return Vec3f(lhs) -= rhs;
}

pure Vec3f operator*(Vec3f lhs, float rhs)  { 
    return Vec3f(lhs) *= rhs; 
}

pure Vec3f operator*(float lhs, Vec3f rhs)  { 
    return Vec3f(rhs) *= lhs; 
}

pure Vec3f operator/(Vec3f lhs, float rhs)  { 
    return Vec3f(lhs) /= rhs; 
}

//NOTE(Momo): Dot product
pure float operator*(Vec3f lhs, Vec3f rhs) { 
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; 
}


pure bool operator==(Vec3f lhs, Vec3f rhs)  { 
    return 
        IsFloatEq(lhs.x, rhs.x) && 
        IsFloatEq(lhs.y, rhs.y) && 
        IsFloatEq(lhs.z, rhs.z);
}

pure bool operator!=(Vec3f lhs, Vec3f rhs) { 
    return !(lhs == rhs); 
}

pure Vec3f operator-(Vec3f lhs)  {  
    return { -lhs.x, -lhs.y, -lhs.z }; 
}

pure Vec3f Midpoint(Vec3f lhs, Vec3f rhs)  { 
    return (lhs + rhs)/2.f; 
}

pure float DistanceSq(Vec3f lhs, Vec3f rhs) { 
    return (rhs.x - lhs.x) * (rhs.y - lhs.y) * (rhs.z - lhs.z);
}

pure float LengthSq(Vec3f lhs) { 
    return lhs * lhs;	
}

pure float Distance(Vec3f lhs, Vec3f rhs)  { 
    return sqrt(DistanceSq(lhs, rhs)); 
}

pure float Length(Vec3f lhs)  { 
    return sqrt(LengthSq(lhs));
};

pure Vec3f Normalize(Vec3f lhs)  {
    Vec3f ret = lhs;
    f32 len = Length(lhs);
    if (IsFloatEq(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

pure float AngleBetween(Vec3f lhs, Vec3f rhs) {
    return acos((lhs * rhs) / (Length(lhs) * Length(rhs)));
}

pure float AngleBetweenNormalized(Vec3f lhs, Vec3f rhs) {
    Assert(IsFloatEqual(LengthSq(lhs), 1.f));
    Assert(IsFloatEqual(LengthSq(rhs), 1.f));
    return acos(lhs * rhs);
}

pure bool IsPerpendicular(Vec3f lhs, Vec3f rhs) 
{ 
    return IsFloatEq((lhs * rhs), 0.f); 
}

pure bool IsSameDirection(Vec3f lhs, Vec3f rhs) { 
    return (lhs * rhs) > 0.f; 
}

pure bool IsOppositeDirection(Vec3f lhs, Vec3f rhs) { 
    return (lhs * rhs) < 0.f;
}

pure Vec3f Project(Vec3f from, Vec3f to) { 
    return (to * from) / LengthSq(to) * to;
}


#endif
