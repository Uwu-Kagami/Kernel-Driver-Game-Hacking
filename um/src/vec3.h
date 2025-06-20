#pragma once

struct Vec3 {
    float x, y, z;

    Vec3 operator-(const Vec3& o) const {
        return {x - o.x, y - o.y, z - o.z};
    }

    float Length2D() const {
        return std::sqrt(x * x + y * y);
    }

    bool IsZero() const {
        return x == 0.0f && y == 0.0f && z == 0.0f;
    }
};

