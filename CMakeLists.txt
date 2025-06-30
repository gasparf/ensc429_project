# Real-Time Voice Encoder Framework

This project provides a starting point for a real-time voice encoder. The code
is organized as a simple RTOS-like scheduler in C++ with placeholder audio
input and output. A small C# console application is included as a stand in for a
future GUI where parameters and effects can be adjusted.

The audio portion currently generates and plays silent frames. Actual DSP logic
and hardware integration will be added in later steps.

## Building

The C++ code uses CMake. To build from the `build` directory:

```bash
cmake ..
make
./voice_encoder
```

The C# application can be run with `dotnet run` from the `gui` folder.