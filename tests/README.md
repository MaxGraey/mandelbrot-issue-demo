Build & Run with Clang:

```sh
clang -v && \
clang -O3 -o ./failed failed.cpp && \
./failed
```

Build & Run with GCC:
```sh
gcc -v && \
gcc -O3 -o ./failed failed.cpp && \
./failed
```

Build with Emscripten:
```sh
emcc -v && \
emcc -O3 -s WASM=1 failed.cpp -o index.html
```
Run: use `serve` or any other static server for /test folder. Open browser and http://localhost:"{some-port}"/index.html