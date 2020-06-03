#ifndef __RYOJI_MATH_VECTOR__
#define __RYOJI_MATH_VECTOR__

struct vf32 {
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
    
    
    inline vf32& operator+=(vf32 rhs)  {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    
    inline vf32& operator-=(vf32 rhs)  {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    
    
    inline vf32& operator*=(vf32 rhs)  {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    
    inline vf32& operator/=(vf32 rhs)  {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }
    
    inline vf32& operator*=(float rhs)  {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    
    inline vf32& operator/=(float rhs)  {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
    
};


pure vf32 operator+(vf32 lhs, vf32 rhs)  { 
    return vf32(lhs) += rhs; 
}

pure vf32 operator-(vf32 lhs, vf32 rhs)  { 
    return vf32(lhs) -= rhs;
}

pure vf32 operator*(vf32 lhs, float rhs)  { 
    return vf32(lhs) *= rhs; 
}

pure vf32 operator*(float lhs, vf32 rhs)  { 
    return vf32(rhs) *= lhs; 
}

pure vf32 operator/(vf32 lhs, float rhs)  { 
    return vf32(lhs) /= rhs; 
}

//NOTE(Momo): Dot product
pure float operator*(vf32 lhs, vf32 rhs) { 
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z; 
}


pure bool operator==(vf32 lhs, vf32 rhs)  { 
    return 
        IsFloatEq(lhs.x, rhs.x) && 
        IsFloatEq(lhs.y, rhs.y) && 
        IsFloatEq(lhs.z, rhs.z);
}

pure bool operator!=(vf32 lhs, vf32 rhs) { 
    return !(lhs == rhs); 
}

pure vf32 operator-(vf32 lhs)  {  
    return { -lhs.x, -lhs.y, -lhs.z }; 
}

pure vf32 Midpoint(vf32 lhs, vf32 rhs)  { 
    return (lhs + rhs)/2.f; 
}

pure f32 DistanceSq(vf32 lhs, vf32 rhs) { 
    return (rhs.x - lhs.x) * (rhs.y - lhs.y) * (rhs.z - lhs.z);
}

pure f32 LengthSq(vf32 lhs) { 
    return lhs * lhs;	
}

pure f32 Distance(vf32 lhs, vf32 rhs)  { 
    return Sqrt(DistanceSq(lhs, rhs)); 
}

pure f32 Length(vf32 lhs)  { 
    return Sqrt(LengthSq(lhs));
};

// TODO(Momo): Inverse Square root?
pure vf32 Normalize(vf32 lhs)  {
    vf32 ret = lhs;
    f32 len = Length(lhs);
    if (IsFloatEq(len, 1.f))
        return ret;
    ret /= len;
    return ret;
}

pure f32 AngleBetween(vf32 lhs, vf32 rhs) {
    return ACos((lhs * rhs) / (Length(lhs) * Length(rhs)));
}

pure f32 AngleBetweenNormalized(vf32 lhs, vf32 rhs) {
    Assert(IsFloatEqual(LengthSq(lhs), 1.f));
    Assert(IsFloatEqual(LengthSq(rhs), 1.f));
    return ACos(lhs * rhs);
}

pure bool IsPerpendicular(vf32 lhs, vf32 rhs) 
{ 
    return IsFloatEq((lhs * rhs), 0.f); 
}

pure bool IsSameDirection(vf32 lhs, vf32 rhs) { 
    return (lhs * rhs) > 0.f; 
}

pure bool IsOppositeDirection(vf32 lhs, vf32 rhs) { 
    return (lhs * rhs) < 0.f;
}

pure vf32 Project(vf32 from, vf32 to) { 
    return (to * from) / LengthSq(to) * to;
}


#endif
