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
	void normalize() {
		double r = sqrt(x * x + y * y + z * z);
		x /= r;
		y /= r;
		z /= r;
	}
	double operator*(const Vector3& r) {
		double ans = 0;
		ans += x * r.x + y * r.y + z * r.z;
		return ans;
	}
	double length() {
		return x * x + y * y + z * z;
	}
};

class Object {
protected:
	double x0, y0, z0;
public:
	Color col;
	virtual bool intersect(double, double, double, double, double, double, double&, bool&) = 0;
	virtual Color pixelColor(double, double, double) = 0;
	friend bool sortO(Object*, Object*);
};

vector<Object*> objs;

class Sphere : public Object {
	double R;
public:
	Sphere(double x0, double y0, double z0, double R, Color col = Color(255,0,0)) {
		this->x0 = x0;
		this->y0 = y0;
		this->z0 = z0;
		this->R = R;
		this->col = col;
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
	Color pixelColor(double x, double y, double z) override {
		Vector3 n(2 * (x - x0), 2 * (y - y0), 2 * (z - z0));
		n.normalize();
		Vector3 a(xl - x, yl - y, zl - z);
		a.normalize();
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
		Color c = col;
		double osv = n * a;
		if (osv < 0) {
			osv = 0;
		}
		if (closed) {
			osv = 0;
		}
		c.red = static_cast<int>(min(c.red * osv + 10,255.0));
		c.green = static_cast<int>(min(c.green * osv + 10, 255.0));
		c.blue = static_cast<int>(min(c.blue * osv + 10, 255.0));
		return c;
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
	bitMapImage im(x, y);
	double ekrY = sqrt(static_cast<double>(x * x) / (2 * (1 - cos(fov))) - static_cast<double>(x * x) / 4);
	yl = ekrY - 200;
	objs.push_back(new Sphere(-800, ekrY + 100, 0, 200));
	objs.push_back(new Sphere(-400, ekrY - 32.5, 0, 75, Color(0,0,255)));
	objs.push_back(new Sphere(400, ekrY - 300, 0, 75, Color(0,255,0)));

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

					Color c = o->pixelColor(static_cast<double>(j) * yx / ekrY, yx, static_cast<double>(i) * yx / ekrY);
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