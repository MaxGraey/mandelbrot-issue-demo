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