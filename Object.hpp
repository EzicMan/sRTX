#pragma once
#include <vector>
#include <algorithm>

constexpr double gammaCoef = 2.2;
constexpr double ambientLight = 0.00004;

class Object;

struct Light {
    double xl = 0, yl = 0, zl = 0;
    double yarkost = 100000;
    Light(double xl = 0, double yl = 0, double zl = 0, double yarkost = 0) {
        this->xl = xl;
        this->yl = yl;
        this->zl = zl;
        this->yarkost = yarkost;
    }
};

inline std::vector<Light> lights;
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
    Color pixelColor(double x, double y, double z, Vector3 n, bool req) {
        Color ans;
#pragma omp parallel for shared(ans)
        for (int j = 0; j < lights.size(); j++) {
            Vector3 a(lights[j].xl - x, lights[j].yl - y, lights[j].zl - z);
            if (req && a * n < 0) {
                n = -n;
            }
            double r = a.length() / lights[j].yarkost;
            a = a.normalize();
            bool closed = false;
            double yt = 0;
            bool isKast = false;
            intersect(-a.x, -a.y, -a.z, lights[j].xl, lights[j].yl, lights[j].zl, yt, isKast);
            Vector3 curs(yt * a.x / a.y, yt, yt * a.z / a.y);
#pragma omp parallel for
            for (int i = 0; i < objs.size(); i++) {
                double yx;
                bool isKas;
                if (objs[i] != this && objs[i]->intersect(-a.x, -a.y, -a.z, lights[j].xl, lights[j].yl, lights[j].zl, yx, isKas)) {
                    Vector3 cur(yx * a.x / a.y, yx, yx * a.z / a.y);
                    if (curs.length() > cur.length() && cur * curs > 0) {
                        closed = true;
                        count1++;
                        break;
                    }
                }
            }
            Color theREDACTED = col;
            double osv = n * a * std::min(1.0 / r, 1.0);
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
            spec = spec * std::min(1.0 / r, 1.0);
            c = c + spec;
            c.x = pow(c.x, 1.0 / gammaCoef);
            c.y = pow(c.y, 1.0 / gammaCoef);
            c.z = pow(c.z, 1.0 / gammaCoef);
            theREDACTED.red = (int)std::min(c.x * 255, 255.0);
            theREDACTED.green = (int)std::min(c.y * 255, 255.0);
            theREDACTED.blue = (int)std::min(c.z * 255, 255.0);
            ans.red += theREDACTED.red;
            ans.blue += theREDACTED.blue;
            ans.green += theREDACTED.green;
        }
        ans.red = std::min(ans.red, 255);
        ans.green = std::min(ans.green, 255);
        ans.blue = std::min(ans.blue, 255);
        return ans;
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
