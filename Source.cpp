#include "bmplib.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

#define PI 3.1415926

double xl = 0, yl = 0, zl = 0;
long long count1 = 0;

class Vector3 {
public:
	double x, y, z;
	Vector3(double x = 0, double y = 0, double z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
	}
	Vector3 normalize() {
		double r = sqrt(x * x + y * y + z * z);
		return Vector3(x / r, y / r, z / r);
	}
	double operator*(const Vector3& r) {
		double ans = 0;
		ans += x * r.x + y * r.y + z * r.z;
		return ans;
	}
	Vector3 operator*(const double a) {
		Vector3 ans(x * a,y * a,z * a);
		return ans;
	}
	Vector3 operator-(const Vector3& r) {
		return Vector3(x - r.x,y - r.y,z - r.z);
	}
	Vector3 operator+(const Vector3& r) {
		return Vector3(x + r.x, y + r.y, z + r.z);
	}
	double length() const{
		return x * x + y * y + z * z;
	}
	bool operator>(const Vector3& r) {
		if (length() > r.length()) {
			return true;
		}
		return false;
	}
	Vector3 crossProduct(Vector3 b) {
		Vector3 c;
		c.x = y * b.z - z * b.y;
		c.y = z * b.x - x * b.z;
		c.z = x * b.y - y * b.x;
		return c;
	}
	Vector3 operator-() {
		return Vector3(-x, -y, -z);
	}
	friend Vector3& maxv(Vector3& l, Vector3& r) {
		if (l > r) {
			return l;
		}
		else {
			return r;
		}
	}
	friend Vector3& minv(Vector3& l, Vector3& r) {
		if (l > r) {
			return r;
		}
		else {
			return l;
		}
	}
};

class Object;

vector<Object*> objs;

constexpr double gamma = 2.2;
constexpr double ambientLight = 0.00004;

class Object {
protected:
	double x0, y0, z0;
	double specularity;
public:
	Color col;
	virtual bool intersect(double, double, double, double, double, double, double&, bool&) = 0;
	virtual Color pixelColorCustoms(double, double, double) = 0;
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
		c.x = pow(c.x, gamma);
		c.y = pow(c.y, gamma);
		c.z = pow(c.z, gamma);
		c.x = ambientLight + c.x * osv;
		c.y = ambientLight + c.y * osv;
		c.z = ambientLight + c.z * osv;
		Vector3 spec = specularCount(n, a, Vector3(x, y, z), specularity, osv, closed);
		c = c + spec;
		c.x = pow(c.x, 1.0/gamma);
		c.y = pow(c.y, 1.0/gamma);
		c.z = pow(c.z, 1.0/gamma);
		theREDACTED.red = (int)min(c.x * 255,255.0);
		theREDACTED.green = (int)min(c.y * 255, 255.0);
		theREDACTED.blue = (int)min(c.z * 255, 255.0);
		return theREDACTED;
	}
	friend bool sortO(Object*, Object*);
	Vector3 specularCount(Vector3& n, Vector3& a, Vector3 cur, double specCoef, double& osv, bool& closed) {
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

class Sphere : public Object {
	double R;
public:
	Sphere(double x0, double y0, double z0, double R, Color col = Color(255,0,0), double spec = 5) {
		this->x0 = x0;
		this->y0 = y0;
		this->z0 = z0;
		this->R = R;
		this->col = col;
		this->specularity = spec;
	}
	bool intersect(double A, double B, double C, double xc, double yc, double zc, double& ans, bool& isKas) override {
		double d1 = ((x0-xc) * A / B + (y0-yc) + (z0-zc) * C / B) * ((x0-xc) * A / B + (y0-yc) + (z0-zc) * C / B) - (A * A / (B * B) + 1 + C * C / (B * B)) * ((x0-xc) * (x0-xc) + (y0-yc) * (y0-yc) + (z0-zc) * (z0-zc) - R * R);
		if (d1 >= 0) {
			double y1 = (((x0-xc) * A / B + (y0-yc) + (z0-zc) * C / B) + sqrt(d1)) / (A * A / (B * B) + 1 + C * C / (B * B));
			double y2 = (((x0-xc) * A / B + (y0-yc) + (z0-zc) * C / B) - sqrt(d1)) / (A * A / (B * B) + 1 + C * C / (B * B));
			if (d1 == 0) {
				isKas = true;
			}
			else {
				isKas = false;
			}
			ans = min(y1, y2);
			return true;
		}
		ans = 0;
		isKas = 0;
		return false;
	}
	Color pixelColorCustoms(double x, double y, double z) override {
		Vector3 n(2 * (x - x0), 2 * (y - y0), 2 * (z - z0));
		n = n.normalize();
		return pixelColor(x, y, z, n);
	}
};

class Polygon : public Object {
	double x1, y1, z1;
	double x2, y2, z2;
	double Ap, Bp, Cp, Dp;
public:
	Polygon(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2, Color col = Color(255, 0, 0), double spec = 5) {
		this->x0 = x0;
		this->y0 = y0;
		this->z0 = z0;
		this->x1 = x1;
		this->y1 = y1;
		this->z1 = z1;
		this->x2 = x2;
		this->y2 = y2;
		this->z2 = z2;
		this->col = col;
		Ap = (y1 - y0) * (z2 - z0) - (z1 - z0) * (y2 - y0);
		Bp = (z1 - z0) * (x2 - x0) - (x1 - x0) * (z2 - z0);
		Cp = (x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0);
		Dp = -Ap * x0 - Bp * y0 - Cp * z0;
		this->specularity = spec;
	}
	bool intersect(double A, double B, double C, double xc, double yc, double zc, double& ans, bool& isKas) override {
		double yo = (yc * (A * Ap / B + Bp + C * Cp / B) - xc * Ap - zc * Cp - Dp) / (A * Ap / B + Bp + C * Cp / B);
		double xo = (yo - yc) * A / B + xc;
		double zo = (yo - yc) * C / B + zc;
		Vector3 o(xo, yo, zo);
		Vector3 a(x0, y0, z0);
		Vector3 b(x1, y1, z1);
		Vector3 c(x2, y2, z2);
		Vector3 n1 = (o - a).crossProduct(b - a);
		Vector3 n2 = (o - b).crossProduct(c - b);
		Vector3 n3 = (o - c).crossProduct(a - c);
		if (n1 * n2 < 0 || n1 * n3 < 0 || n2 * n3 < 0) {
			ans = 0;
			isKas = false;
			return false;
		}
		ans = yo;
		isKas = false;
		return true;
	}
	Color pixelColorCustoms(double x, double y, double z) override {
		Vector3 n(Ap, Bp, Cp);
		n = n.normalize();
		Vector3 a(xl - x, yl - y, zl - z);
		if (a * n < 0) {
			n = -n;
		}
		return pixelColor(x, y, z, n);
	}
};

bool sortO(Object* a, Object* b) {
	return a->y0 > b->y0;
}

int main() {
	cout << "Please enter desired FOV>";
	double fov;
	int x, y;
	cin >> fov;
	cout << "Please enter Window size:>";
	cin >> x >> y;
	if (x % 2 != 0) {
		x++;
	}
	if (y % 2 != 0) {
		y++;
	}
	bitMapImage<24> im(x, y);
	double ekrY = sqrt(static_cast<double>((double)x * (double)x) / (2 * (1 - cos(fov / 180 * PI))) - static_cast<double>((double)x * (double)x) / 4);
	yl = ekrY - 200;
	objs.push_back(new Sphere(-800, ekrY + 100, 0, 200,Color(255),10));
	objs.push_back(new Sphere(-400, ekrY - 32.5, 0, 75, Color(0,0,255),10));
	objs.push_back(new Sphere(400, ekrY - 300, 0, 75, Color(0,255,0),10));
	objs.push_back(new Polygon(-200.0,ekrY + 400,-100.0, -300.0, ekrY + 400, 400.0, 100.0, ekrY + 400, 400.0, Color(255,0,0),10));
	objs.push_back(new Polygon(-200.0,ekrY + 10,0.0, -300.0, ekrY * 2 + 800, 700.0, 100.0, ekrY * 2 + 800, 700.0, Color(255,0,0),10));

	sort(objs.begin(), objs.end(), sortO);
	std::vector<std::vector<double>> depthBuffer(y, std::vector<double>(x, std::numeric_limits<double>::infinity()));

	for (int i = -y / 2; i < y / 2; i++) {
		for (int j = -x / 2; j < x / 2; j++) {
			bool pixelSet = false;
			for (auto o : objs) {
				double& depthNow = depthBuffer[i + y / 2][j + x / 2];
				double yx;
				bool isKas;
				if (o->intersect(j, ekrY, i, 0, 0, 0, yx, isKas)) {

					if (yx > depthNow) {
						continue;
					}

					depthNow = yx;

					Color c = o->pixelColorCustoms(static_cast<double>(j) * yx / ekrY, yx, static_cast<double>(i) * yx / ekrY);
					im.setPixel(x / 2 + j, y / 2 + i, c);
					pixelSet = true;
				}
			}
			if (!pixelSet) {
				im.setPixel(x / 2 + j, y / 2 + i, Color(0, 0, 0));
			}
		}
	}
	im.saveToFile("test.bmp");
	cout << count1 << endl;
}