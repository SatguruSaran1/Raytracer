# Raytracer
A multi-threaded 3D path tracer written in C++ from scratch. Features diffuse, metallic, and glass materials, anti-aliasing, depth-of-field blur, and direct PPM image rendering.
FeaturesMulti-Threaded CPU Rendering: Automatically scales to use all available CPU cores to render images significantly faster.  Realistic Materials: Includes Lambertian diffuse (matte blue/ground), polished metal with adjustable roughness (gold), and dielectric glass featuring total internal reflection and refraction.  Camera Mechanics: Configurable Field of View (FOV) and true depth-of-field (lens aperture blur).  Zero Dependencies: Requires nothing but a standard C++ compiler.  
## How to Download
You can download this project to your local machine using one of the following methods:
### Clone the Repository (Using Git)
If you have Git installed, run the following command in your terminal or command prompt:
```bash
git clone https://github.com/SatguruSaran1/Raytracer.git
```
### Download the ZIP File
1. On the GitHub repository page, click the green **Code** button.
2. Click **Download ZIP**.
3. Extract the downloaded `.zip` file to a folder on your computer.

## Prerequisites
To compile the code, you will need a C++ compiler that supports the C++17 standard (e.g., `g++` or `clang++`). 

## Compilation
Open your terminal, navigate to the directory where you saved and run the following command to compile:
```bash
g++ -std=c++17 -O3 raytracer.cpp -o raytracer -pthread
```
The `-pthread` flag is required because the code utilizes multithreading.

## Execution
Once compiled, you can run the raytracer from your terminal or command prompt.

### Open your Terminal or Command Prompt
- Open **Command Prompt** or **PowerShell** (on Windows) or your **Terminal** (on macOS/Linux).
- Navigate to the folder where your compiled executable (`raytracer` or `raytracer.exe`) is located using the `cd` command. Example:
  ```bash
  cd path/to/your/folder
  ```
  
### Step 2: Run the Executable
You can run the program using default fast settings or customize the resolution and quality using command-line arguments.

#### For a customized / high-quality render:
- **Windows (Command Prompt):**
  ```cmd
  raytracer.exe --width 1200 --height 675 --samples 100 --depth 50 --output scene.ppm
  ```
- **Windows (PowerShell):**
  ```powershell
  .\raytracer.exe --width 1200 --height 675 --samples 100 --depth 50 --output scene.ppm
  ```
- **macOS / Linux:**
  ```bash
  ./raytracer --width 1200 --height 675 --samples 100 --depth 50 --output scene.ppm
  ```

#### For a quick test render (low resolution, completes in seconds):
- **Windows (Command Prompt):**
  ```cmd
  raytracer.exe --quick
  ```
- **Windows (PowerShell):**
  ```powershell
  .\raytracer.exe --quick
  ```
- **macOS / Linux:**
  ```bash
  ./raytracer --quick
  ```
*(This forces a $320 \times 180$ resolution with 10 samples per pixel, which completes in just a few seconds).*

## Viewing the Output

The raytracer outputs a standard Netpbm color image format (`.ppm`). Because this is raw RGB data, it cannot natively be opened in all generic picture viewers.

Here is how you can view the `.ppm` file on different operating systems:

- **Online:** You can convert or view the image directly in your browser using online `.ppm` to `.png` converters (e.g., [Convertio](https://convertio.co/)).
- **Windows:** You can use free tools like IrfanView, GIMP, or Photoshop.
- **macOS:** You can open `.ppm` files natively using the built-in **Preview** app.
- **Linux:** Many image viewers support `.ppm` directly, such as the default GNOME Image Viewer, Feh, or ImageMagick (`display out.ppm`).

### Command-Line Options Overview

You can customize the rendering settings using the following command-line arguments:

| Option | Description |
| :--- | :--- |
| `--width N` | Set image width. |
| `--height N` | Set image height. |
| `--samples N` | Anti-aliasing samples per pixel. |
| `--depth N` | Maximum number of light bounces. |
| `--threads N` | Number of threads to use (defaults to your CPU core count). |
| `--output FILE` | Output filename (defaults to `out.ppm`). |
| `--quick` | Overrides values to 320x180, 10 samples, and 7 light bounces for a rapid test. |
| `--help` | Displays the help menu. |

**Example customization:**
```bash
./raytracer --width 1920 --height 1080 --samples 100 --depth 50 --threads 8 --output render.ppm
```
