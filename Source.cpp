#include "bmplib.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <float.h>
#include "json.hpp"
#include "Vector3.hpp"
#include "kdTree.hpp"
#include "Object.hpp"
using namespace std;
using nlohmann::json;

#define PI 3.1415926

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
		updateBoundingBox();
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
		isKas = false;
		return false;
	}
	Color pixelColorCustoms(double x, double y, double z) override {
		Vector3 n(2 * (x - x0), 2 * (y - y0), 2 * (z - z0));
		n = n.normalize();
		return pixelColor(x, y, z, n, false);
	}
	void updateBoundingBox() override {
	    bbox[0].x = x0 - R;
        bbox[0].y = y0 - R;
        bbox[0].z = z0 - R;
        bbox[1].x = x0 + R;
        bbox[1].y = y0 + R;
        bbox[1].z = z0 + R;
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
		updateBoundingBox();
	}
	bool intersect(double A, double B, double C, double xc, double yc, double zc, double& ans, bool& isKas) override {
		//double yo = (yc * (A * Ap / B + Bp + C * Cp / B) - xc * Ap - zc * Cp - Dp) / (A * Ap / B + Bp + C * Cp / B);
		//double xo = (yo - yc) * A / B + xc;
		//double zo = (yo - yc) * C / B + zc;
		double aaa = (Ap * A + Bp * B + Cp * C);
		if (abs(aaa) < DBL_MIN) {
			return false;
		}
		double t = -Dp / aaa;
		double xo = A * t;
		double yo = B * t;
		double zo = C * t;
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
		return pixelColor(x, y, z, n, true);
	}

    void updateBoundingBox() override {
        bbox[0].x = std::min(std::min(x0, x1), x2);
        bbox[0].y = std::min(std::min(y0, y1), y2);
        bbox[0].z = std::min(std::min(z0, z1), z2);
        bbox[1].x = std::max(std::max(x0, x1), x2);
        bbox[1].y = std::max(std::max(y0, y1), y2);
        bbox[1].z = std::max(std::max(z0, z1), z2);
    }
};

bool sortO(Object* a, Object* b) {
	return a->y0 > b->y0;
}

int main() {
	double fov;
	int x, y;

	std::ifstream i("config.json");
	json js;
	i >> js;
	i.close();

	fov = js["fov"];
	x = js["resolution"][0];
	y = js["resolution"][1];

	if (x % 2 != 0) {
		x++;
	}
	if (y % 2 != 0) {
		y++;
	}
	bitMapImage<24> im(x, y);
	double ekrY = sqrt(static_cast<double>((double)x * (double)x) / (2 * (1 - cos(fov / 180 * PI))) - static_cast<double>((double)x * (double)x) / 4);

	for (auto it : js["lights"]) {
		lights.push_back(Light(it["lightcoords"][0], it["lightcoords"][1] + (it["relative"] ? ekrY : 0), it["lightcoords"][2],it["intensity"]));
	}

	for (auto it : js["objects"]) {
		if (it["name"] == "Sphere") {
			objs.push_back(new Sphere(it["coords"][0], it["coords"][1] + (it["relative"] ? ekrY : 0), it["coords"][2], it["r"], Color(it["col"][0], it["col"][1], it["col"][2]), it["spec"]));
		}
		else if (it["name"] == "Polygon") {
			objs.push_back(new Polygon(it["coords1"][0], it["coords1"][1] + (it["relative"] ? ekrY : 0), it["coords1"][2], it["coords2"][0], it["coords2"][1] + (it["relative"] ? ekrY : 0), it["coords2"][2], it["coords3"][0], it["coords3"][1] + (it["relative"] ? ekrY : 0), it["coords3"][2], Color(it["col"][0], it["col"][1], it["col"][2]), it["spec"]));
		}
		else {
			ifstream in(std::string(it["file"]).c_str());
			vector<Vector3> vertices;
			double p = it["euler"][0];
			double t = it["euler"][1];
			double s = it["euler"][2];
			double neo[3][3] = { {cos(p)*cos(s) - sin(p)*cos(t)*sin(s), -cos(p)*sin(s)-sin(p)*cos(t)*cos(s), sin(p)*sin(t)}, {sin(p) * cos(s) - cos(p) * cos(t) * sin(s), -sin(p) * sin(s) + cos(p) * cos(t) * cos(s), -cos(p) * sin(t)}, {sin(t)*sin(s), sin(t)*cos(s), cos(t)} };
			while (!in.eof()) {
				char sym;
				double t1;
				double t2;
				double t3;
				Vector3 beforeeu;
				Vector3 aftereu;
				in >> sym;
				if (sym == 'v') {
					in >> beforeeu[0] >> beforeeu[1] >> beforeeu[2];
					for (int i = 0; i < 3; i++) {
						for (int k = 0; k < 3; k++) {
							aftereu[i] += beforeeu[k] * neo[i][k];
						}
					}
					aftereu[0] += it["coords"][0];
					aftereu[1] += it["coords"][1] + (it["relative"] ? ekrY : 0);
					aftereu[2] += it["coords"][2];
					vertices.emplace_back(aftereu[0], aftereu[1], aftereu[2]);
				}
				else if (sym == 'f') {
					in >> t1 >> t2 >> t3;
					if (t1 == 0 || t2 == 0 || t3 == 0) {
						break;
					}
					objs.push_back(new Polygon(vertices[t1 - 1].x, vertices[t1 - 1].y, vertices[t1 - 1].z, vertices[t2 - 1].x, vertices[t2 - 1].y, vertices[t2 - 1].z, vertices[t3 - 1].x, vertices[t3 - 1].y, vertices[t3 - 1].z, Color(it["col"][0], it["col"][1], it["col"][2]), it["spec"]));
				}
			}
			vertices.clear();
		}
	}

	cout << "load complete" << endl;
	sort(objs.begin(), objs.end(), sortO);
	std::vector<std::vector<double>> depthBuffer(y, std::vector<double>(x, std::numeric_limits<double>::infinity()));

	Vector3 lower(DBL_MAX, DBL_MAX, DBL_MAX);
	Vector3 upper(-DBL_MAX, -DBL_MAX, -DBL_MAX);
	for (auto o : objs) {
		lower.x = std::min(o->bbox[0].x, lower.x);
		lower.y = std::min(o->bbox[0].y, lower.y);
		lower.z = std::min(o->bbox[0].z, lower.z);
		upper.x = std::max(o->bbox[1].x, upper.x);
		upper.y = std::max(o->bbox[1].y, upper.y);
		upper.z = std::max(o->bbox[1].z, upper.z);
	}

	//kdTree tree(lower,upper,Vector3(1,0,0),objs);
	
	//std::ofstream out("tree.txt");
	//tree.dump(out, 0);
	//out.close();

#pragma omp parallel for
	for (int i = -y / 2; i < y / 2; i++) {
#pragma omp parallel for
		for (int j = -x / 2; j < x / 2; j++) {
			bool pixelSet = false;
			//std::set<Object*> curStep;
			//tree.getObj(Vector3(j, ekrY, i), curStep);
			//for(auto o : curStep){
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

					//printf("%d %d %d\n",c.red,c.green,c.blue);

					im.setPixel(x / 2 + j, y / 2 + i, c);
					pixelSet = true;
				}
			}
			if (!pixelSet) {
				im.setPixel(x / 2 + j, y / 2 + i, Color(js["backgroundColor"][0], js["backgroundColor"][1], js["backgroundColor"][2]));
			}
		}
		//cout << (i + y / 2) << " / " << y << "\n";
	}
	for (auto i : objs) {
		free(i);
	}
	im.saveToFile(std::string(js["output"]).c_str());
	cout << count1 << endl;
}