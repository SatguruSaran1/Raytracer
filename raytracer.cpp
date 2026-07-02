#include "raytracer.h"

using namespace std;

Vec random_unit_vector(Rng& rng) {
    double a = rng.next() * 2.0 * PI;
    double z = rng.next() * 2.0 - 1.0;
    double r = sqrt(1.0 - z * z);
    return Vec(r * cos(a), r * sin(a), z);
}

Vec ray_color(const Ray& r, const World& world, int depth, Rng& rng) {
    if (depth <= 0) {
        return Vec(0, 0, 0);
    }

    Hit rec = world.hit(r, 0.001, 1e9);
    if (rec.hit) {
        Vec target = rec.p + rec.n + random_unit_vector(rng);
        
        if (rec.mat == gold) {
            Vec reflected = r.dir.unit() - rec.n * 2.0 * r.dir.unit().dot(rec.n);
            reflected = reflected + random_unit_vector(rng) * 0.2; 
            if (reflected.dot(rec.n) > 0) {
                Ray next_ray;
                next_ray.orig = rec.p;
                next_ray.dir = reflected;
                return ray_color(next_ray, world, depth - 1, rng) * Vec(0.9, 0.7, 0.1);
            }
            return Vec(0, 0, 0);
        }
        else if (rec.mat == blue) {
            Ray next_ray;
            next_ray.orig = rec.p;
            next_ray.dir = target - rec.p;
            return ray_color(next_ray, world, depth - 1, rng) * Vec(0.1, 0.2, 0.8);
        }
        else if (rec.mat == glass) {
            double refraction_ratio = r.dir.unit().dot(rec.n) < 0 ? (1.0 / 1.5) : 1.5;
            Vec unit_dir = r.dir.unit();
            double cos_theta = min(-unit_dir.dot(rec.n), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);
            
            Vec direction;
            if (refraction_ratio * sin_theta > 1.0 || (rng.next() < 0.1)) {
                direction = unit_dir - rec.n * 2.0 * unit_dir.dot(rec.n);
            }
            else {
                Vec r_out_perp = (unit_dir + rec.n * cos_theta) * refraction_ratio;
                Vec r_out_parallel = rec.n * -sqrt(abs(1.0 - r_out_perp.dot(r_out_perp)));
                direction = r_out_perp + r_out_parallel;
            }
            Ray next_ray;
            next_ray.orig = rec.p;
            next_ray.dir = direction;
            return ray_color(next_ray, world, depth - 1, rng);
        }
        else {
            Ray next_ray;
            next_ray.orig = rec.p;
            next_ray.dir = target - rec.p;
            return ray_color(next_ray, world, depth - 1, rng) * Vec(0.8, 0.8, 0.8);
        }
    }

    Vec unit_dir = r.dir.unit();
    double t = 0.5 * (unit_dir.y + 1.0);
    return Vec(1.0, 1.0, 1.0) * (1.0 - t) + Vec(0.5, 0.7, 1.0) * t;
}

World make_world() {
    World w;
    w.add_sphere(Vec(0.0, -1000.48, -1.0), 1000.0, mat_ground); 
    w.add_sphere(Vec(0.0, 0.52, -1.3), 1.0, glass);             

    for (int a = -6; a < 6; a++) {
        for (int b = -4; b < 4; b++) {
            double x = static_cast<double>(a) * 0.45;
            double z = -2.8 - static_cast<double>(b) * 0.45;
            int mat = gold;
            if ((a + b) % 2 == 0) {
                mat = blue;
            }
            w.add_sphere(Vec(x, -0.32, z), 0.16, mat);
        }
    }
    return w;
}

void render(const Set& cfg, const Camera& cam, const World& world, Img& img) {
    atomic<int> next_y(0);
    atomic<int> done(0);
    mutex lock;

    unsigned n = cfg.threads;
    if (n == 0) {
        n = thread::hardware_concurrency();
    }
    if (n == 0) {
        n = 4;
    }
    if (static_cast<unsigned>(cfg.h) < n) {
        n = static_cast<unsigned>(cfg.h);
    }
    if (n == 0) {
        n = 1;
    }

    vector<thread> pool;
    for (unsigned i = 0; i < n; i++) {
        pool.emplace_back([&]() {
            while (true) {
                int y = next_y.fetch_add(1);
                if (y >= cfg.h) {
                    break;
                }

                Rng rng(cfg.seed + static_cast<uint32_t>(y * 747796405));
                for (int x = 0; x < cfg.w; x++) {
                    Vec c;
                    for (int s = 0; s < cfg.samples; s++) {
                        double u = (static_cast<double>(x) + rng.next()) / static_cast<double>(cfg.w - 1);
                        double v = (static_cast<double>(cfg.h - 1 - y) + rng.next()) / static_cast<double>(cfg.h - 1);
                        Ray r = cam.get(u, v, rng);
                        c += ray_color(r, world, cfg.depth, rng);
                    }
                    img.set(x, y, c);
                }

                int rows = done.fetch_add(1) + 1;
                if (rows % 20 == 0 || rows == cfg.h) {
                    lock_guard<mutex> hold(lock);
                    cerr << "\rrows " << rows << "/" << cfg.h << flush;
                }
            }
        });
    }

    for (thread& t : pool) {
        t.join();
    }
    cerr << "\n";
}

void help() {
    cout << "Minimal C++ ray tracer\nOptions:\n"
         << "  --width N\n  --height N\n  --samples N\n"
         << "  --depth N\n  --threads N\n  --output FILE\n  --quick\n";
}

void need_val(int i, int argc, const string& arg) {
    if (i + 1 >= argc) {
        throw runtime_error(arg + " needs a value");
    }
}

int main(int argc, char** argv) {
    try {
        Set cfg;
        for (int i = 1; i < argc; i++) {
            string arg = argv[i];
            if (arg == "--help") {
                help();
                return 0;
            }
            else if (arg == "--width") {
                need_val(i, argc, arg);
                cfg.w = stoi(argv[++i]);
            }
            else if (arg == "--height") {
                need_val(i, argc, arg);
                cfg.h = stoi(argv[++i]);
            }
            else if (arg == "--samples") {
                need_val(i, argc, arg);
                cfg.samples = stoi(argv[++i]);
            }
            else if (arg == "--depth") {
                need_val(i, argc, arg);
                cfg.depth = stoi(argv[++i]);
            }
            else if (arg == "--threads") {
                need_val(i, argc, arg);
                int val = stoi(argv[++i]);
                cfg.threads = static_cast<unsigned>(max(0, val));
            }
            else if (arg == "--output") {
                need_val(i, argc, arg);
                cfg.out = argv[++i];
            }
            else if (arg == "--quick") {
                cfg.w = 320;
                cfg.h = 180;
                cfg.samples = 10;
                cfg.depth = 7;
            }
            else {
                throw runtime_error("unknown option: " + arg);
            }
        }

        cfg.w = max(2, cfg.w);
        cfg.h = max(2, cfg.h);
        cfg.samples = max(1, cfg.samples);
        cfg.depth = max(1, cfg.depth);

        double aspect = static_cast<double>(cfg.w) / static_cast<double>(cfg.h);
        Vec look_from(3.0, 1.7, 2.5);
        Vec look_at(0.0, -0.1, -1.3);
        double focus = (look_from - look_at).len();
        Camera cam(look_from, look_at, Vec(0.0, 1.0, 0.0), 35.0, aspect, 0.06, focus);
        World world = make_world();
        Img img(cfg.w, cfg.h);

        auto start = chrono::steady_clock::now();
        render(cfg, cam, world, img);
        img.save(cfg.out, cfg.samples);
        auto end = chrono::steady_clock::now();

        cout << "saved " << cfg.out << "\ntime " 
             << chrono::duration<double>(end - start).count() << " seconds\n";
    }
    catch (const exception& e) {
        cerr << "error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}