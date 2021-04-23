#pragma once
#include <cassert>

class Vector3 {
public:
    double x, y, z;
    explicit Vector3(double x = 0, double y = 0, double z = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
    [[nodiscard]] Vector3 normalize() const{
        double r = sqrt(x * x + y * y + z * z);
        return Vector3(x / r, y / r, z / r);
    }
    double operator*(const Vector3& r) const{
        double ans = 0;
        ans += x * r.x + y * r.y + z * r.z;
        return ans;
    }
    Vector3 operator*(const double a) const{
        Vector3 ans(x * a,y * a,z * a);
        return ans;
    }
    Vector3 operator-(const Vector3& r) const{
        return Vector3(x - r.x,y - r.y,z - r.z);
    }
    Vector3 operator+(const Vector3& r) const{
        return Vector3(x + r.x, y + r.y, z + r.z);
    }
    [[nodiscard]] double length() const{
        return x * x + y * y + z * z;
    }
    bool operator>(const Vector3& r) const{
        if (length() > r.length()) {
            return true;
        }
        return false;
    }
    [[nodiscard]] Vector3 crossProduct(Vector3 b) const{
        Vector3 c;
        c.x = y * b.z - z * b.y;
        c.y = z * b.x - x * b.z;
        c.z = x * b.y - y * b.x;
        return c;
    }
    Vector3 operator-() const{
        return Vector3(-x, -y, -z);
    }

    double &operator[] (size_t i) {
        //return (reinterpret_cast<double*>(this))[i];
        assert(i >= 0 && i < 3);
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: ;//backroom
        }
    }

    const double &operator[] (size_t i) const {
        //return (reinterpret_cast<double*>(this))[i];
        assert(i >= 0 && i < 3);
        switch (i) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: ;//backroom
        }
    }

    [[maybe_unused]] friend Vector3& maxv(Vector3& l, Vector3& r) {
        return l > r ? l : r;
    }
    [[maybe_unused]] friend Vector3& minv(Vector3& l, Vector3& r) {
        return l > r ? r : l;
    }
};
