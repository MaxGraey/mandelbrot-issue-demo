
# Mandelbrot set demo
Visually demonstrate some precision lost in **log(x)** / **log2(x)** math functions introduced in **libmusl 1.2.0**.

Perhaps other functions like **exp(x)** and **pow(x, y)** also affcted due to similar approach which uses lookup tables.

## [Live Demo](https://maxgraey.github.io/mandelbrot-issue-demo)

# Build Requirements

- emscripten 1.38.44 or later
- node.js v8.0.0 or later
