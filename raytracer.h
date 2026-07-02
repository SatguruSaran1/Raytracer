#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
#include <string>
#include <stdexcept>
#include <cmath>
#include <fstream>
#include <random>
#include <algorithm>

using namespace std;

const double PI = 3.1415926535897932385;

const int mat_ground = 0;
const int gold = 1;
const int blue = 2;
const int glass = 3;

struct Set {
    int w = 800;
    int h = 450;
    int samples = 10;
    int depth = 10;
    unsigned threads = 0;
    string out = "out.ppm";
    uint32_t seed = 42;
};

struct Vec {
    double x, y, z;
    Vec() : x(0), y(0), z(0) {
    }
    
    Vec(double x, double y, double z) : x(x), y(y), z(z) {
    }
    
    Vec operator+(const Vec& v) const {
        return Vec(x + v.x, y + v.y, z + v.z);
    }
    
    Vec operator-(const Vec& v) const {
        return Vec(x - v.x, y - v.y, z - v.z);
    }
    
    Vec operator*(const Vec& v) const {
        return Vec(x * v.x, y * v.y, z * v.z);
    }
    
    Vec operator*(double t) const {
        return Vec(x * t, y * t, z * t);
    }
    
    Vec operator/(double t) const {
        return Vec(x / t, y / t, z / t);
    }
    
    Vec& operator+=(const Vec& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    
    double dot(const Vec& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    
    Vec cross(const Vec& v) const {
        return Vec(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
    
    double len() const {
        return sqrt(x * x + y * y + z * z);
    }
    
    Vec unit() const {
        return *this / len();
    }
};

struct Ray {
    Vec orig, dir;
    Vec at(double t) const {
        return orig + dir * t;
    }
};

class Rng {
    mt19937 gen;
    uniform_real_distribution<double> dist;
public:
    Rng(uint32_t seed) : gen(seed), dist(0.0, 1.0) {
    }
    
    double next() {
        return dist(gen);
    }
};

class Img {
    int w, h;
    vector<Vec> pixels;
public:
    Img(int w, int h) : w(w), h(h), pixels(w * h) {
    }
    
    void set(int x, int y, Vec c) {
        pixels[y * w + x] = c;
    }
    
    void save(const string& filename, int samples) {
        ofstream f(filename);
        f << "P3\n" << w << " " << h << "\n255\n";
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                Vec c = pixels[y * w + x];
                double scale = 1.0 / samples;
                double r = sqrt(max(0.0, c.x * scale));
                double g = sqrt(max(0.0, c.y * scale));
                double b = sqrt(max(0.0, c.z * scale));
                f << static_cast<int>(256 * clamp(r, 0.0, 0.999)) << " "
                  << static_cast<int>(256 * clamp(g, 0.0, 0.999)) << " "
                  << static_cast<int>(256 * clamp(b, 0.0, 0.999)) << "\n";
            }
        }
    }
};

class Camera {
    Vec origin, lower_left_corner, horizontal, vertical, u, v, w;
    double lens_radius;
public:
    Camera(Vec look_from, Vec look_at, Vec vup, double vfov, double aspect, double aperture, double focus) {
        lens_radius = aperture / 2.0;
        double theta = vfov * PI / 180.0;
        double half_h = tan(theta / 2.0);
        double half_w = aspect * half_h;
        w = (look_from - look_at).unit();
        u = vup.cross(w).unit();
        v = w.cross(u);
        origin = look_from;
        horizontal = u * (2.0 * half_w * focus);
        vertical = v * (2.0 * half_h * focus);
        lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - w * focus;
    }
    
    Ray get(double s, double t, Rng& rng) const {
        double r1 = rng.next() * 2.0 - 1.0;
        double r2 = rng.next() * 2.0 - 1.0;
        while (r1 * r1 + r2 * r2 >= 1.0) {
            r1 = rng.next() * 2.0 - 1.0;
            r2 = rng.next() * 2.0 - 1.0;
        }
        Vec offset = (u * r1 + v * r2) * lens_radius;
        Ray r;
        r.orig = origin + offset;
        r.dir = lower_left_corner + horizontal * s + vertical * t - origin - offset;
        return r;
    }
};

struct Hit {
    double t;
    Vec p, n;
    int mat;
    bool hit;
};

struct Sphere {
    Vec c;
    double r;
    int mat;
};

class World {
    vector<Sphere> spheres;
public:
    void add_sphere(Vec c, double r, int mat) {
        Sphere s;
        s.c = c;
        s.r = r;
        s.mat = mat;
        spheres.push_back(s);
    }
    
    Hit hit(const Ray& r, double t_min, double t_max) const {
        Hit res;
        res.hit = false;
        double closest = t_max;
        for (const auto& s : spheres) {
            Vec oc = r.orig - s.c;
            double a = r.dir.dot(r.dir);
            double half_b = oc.dot(r.dir);
            double c = oc.dot(oc) - s.r * s.r;
            double disc = half_b * half_b - a * c;
            
            if (disc > 0) {
                double sqrtd = sqrt(disc);
                double root = (-half_b - sqrtd) / a;
                if (root < closest && root > t_min) {
                    closest = root;
                    res.hit = true;
                    res.t = root;
                    res.p = r.at(root);
                    res.n = (res.p - s.c) / s.r;
                    res.mat = s.mat;
                    continue;
                }
                root = (-half_b + sqrtd) / a;
                if (root < closest && root > t_min) {
                    closest = root;
                    res.hit = true;
                    res.t = root;
                    res.p = r.at(root);
                    res.n = (res.p - s.c) / s.r;
                    res.mat = s.mat;
                }
            }
        }
        return res;
    }
};