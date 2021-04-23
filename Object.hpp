#pragma once
#include <vector>
#include <algorithm>

constexpr double gammaCoef = 2.2;
constexpr double ambientLight = 0.00004;

class Object;

inline double xl = 0, yl = 0, zl = 0;
inline long long count1 = 0;
inline std::vector<Object*> objs;

class Object {
protected:
    double x0 = 0, y0 = 0, z0 = 0;
    double specularity = 0;

public:
    Color col;
    Vector3 bbox[2];

    virtual bool intersect(double, double, double, double, double, double, double&, bool&) = 0;
    virtual Color pixelColorCustoms(double, double, double) = 0;
    virtual void updateBoundingBox() = 0;
    Color pixelColor(double x, double y, double z, Vector3 n) {
        Vector3 a(xl - x, yl - y, zl - z);
        a = a.normalize();
        bool closed = false;
        double yt = 0;
        bool isKast = false;
        intersect(-a.x, -a.y, -a.z, xl, yl, zl, yt, isKast);
        Vector3 curs(yt * a.x / a.y, yt, yt * a.z / a.y);
        for (auto o : objs) {
            double yx;
            bool isKas;
            if (o != this && o->intersect(-a.x, -a.y, -a.z, xl, yl, zl, yx, isKas)) {
                Vector3 cur(yx * a.x / a.y, yx, yx * a.z / a.y);
                if (curs.length() > cur.length() && cur * curs > 0) {
                    closed = true;
                    count1++;
                    break;
                }
            }
        }
        Color theREDACTED = col;
        double osv = n * a;
        if (osv < 0) {
            osv = 0;
        }
        if (closed) {
            osv = 0;
        }
        if (a * Vector3(-x, -y, -z) < 0) {
            osv = 0;
        }
        Vector3 c((double)theREDACTED.red / 255, (double)theREDACTED.green / 255, (double)theREDACTED.blue / 255);
        c.x = pow(c.x, gammaCoef);
        c.y = pow(c.y, gammaCoef);
        c.z = pow(c.z, gammaCoef);
        c.x = ambientLight + c.x * osv;
        c.y = ambientLight + c.y * osv;
        c.z = ambientLight + c.z * osv;
        Vector3 spec = specularCount(n, a, Vector3(x, y, z), specularity, osv, closed);
        c = c + spec;
        c.x = pow(c.x, 1.0/gammaCoef);
        c.y = pow(c.y, 1.0/gammaCoef);
        c.z = pow(c.z, 1.0/gammaCoef);
        theREDACTED.red = (int)std::min(c.x * 255,255.0);
        theREDACTED.green = (int)std::min(c.y * 255, 255.0);
        theREDACTED.blue = (int)std::min(c.z * 255, 255.0);
        return theREDACTED;
    }
    friend bool sortO(Object*, Object*);
    static Vector3 specularCount(Vector3& n, Vector3& a, Vector3 cur, double specCoef, double& osv, bool& closed) {
        Vector3 glyanec = (n * (n * a)) * 2 - a;
        glyanec = glyanec.normalize();
        cur = cur.normalize();
        double diff = -(glyanec * cur);
        if (diff < 0) {
            diff = 0;
        }
        diff = pow(diff, specCoef);
        if (osv < 0) {
            osv = 0;
            diff = 0;
        }
        if (closed) {
            osv = 0;
            diff = 0;
        }
        return Vector3(diff, diff, diff);
    }
};
