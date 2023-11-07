# What does it do?
Merge 3 PNG grey images each using a different RGB Channels.

# What is this for?
I use it for `personal` projects.

# How to build the build system using Cmake
```
cmake -S . -B build
cmake --build build --config=Release         for release
cmake --build build --config=Debug           for debug
```
```
You can check the cbuild.bat as an example.
```

If you don't want to get cmake, and you are on windows, and you have vcvars64.bat set to
`C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat`
you can use `build.bat`