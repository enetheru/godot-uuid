# Benchmarks

## Upstream
There exists a benchmarking folder inside the flatbuffers repository which performs some benchmark tests using [google benchmark](https://github.com/google/benchmark)
It is my goal to replicate this setup so that I may see the difference in performance against the raw c++, the flatbuffers c++
my flatbuffers gdextension, and default godot serialisation.
### Results
Existing bench results look like this:
```
C:\git\flatbuffers\cmake-build-release-llvm\flatbenchmark.exe
2026-03-01T17:28:14+10:30
Running C:\git\flatbuffers\cmake-build-release-llvm\flatbenchmark.exe
Run on (16 X 3194 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 512 KiB (x8)
  L3 Unified 16384 KiB (x1)
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
BM_Flatbuffers_Encode        304 ns          265 ns      2240000
BM_Flatbuffers_Decode       1.55 ns         1.50 ns    407272727
BM_Flatbuffers_Use          31.6 ns         31.1 ns     23578947
BM_Raw_Encode               41.6 ns         40.5 ns     16592593
BM_Raw_Decode               1.55 ns         1.53 ns    407272727
BM_Raw_Use                  8.39 ns         8.20 ns     89600000

Process finished with exit code 0
```
## Replication
Is it weird that I want to implement google/benchmark now?
Anyway, I am replicating the setup somewhat hackishly to look similar to the existing setup.
Defining classes and functions to appear like google benchmark.

**Update 2026-03-03**: Because I am a weirdo, and my mind is struggling due to life difficulty, I ended up just manually transcoding the necessary functions straight from google benchmarks until I have something somewhat resembling google test, but in gdscript.
It's an entirely stupid premise, but since it meditative it's soothing to keep going.

Current Status:
```
family: <RefCounted#-9223350130122318134>
family._args: [[]]
instance.name = 'BM_Flatbuffers_Encode///////'
2026-03-03T13:24:23
Running C:/build/godot/w64.scons.msvc.x86_64.windows.editor.dev_build.4.5/bin/godot.windows.editor.dev.x86_64.exe
Run on (16 X 0.000000 MHz CPUs)
***WARNING*** Library was built as DEBUG. Timings may be affected.
BM_Flatbuffers_Encode
STUB: StartStopBarrier
STUB State.ResumeTiming
STUB State.PauseTiming
STUB: StartStopBarrier
STUB: NotifyThreadComplete
#STUB GetMinTimeToApply
-----------------------------
@GlobalScope::str
-----------------------------

                             
     0.000 ns        0.000 ns   
         1


1 benchmarks ran
```

**Update 2026-03-04**: I guess its been cathartic to copy and paste code because i kept doing it. I honesty tried to quit three times.
```
2026-03-04T01:22:48
Running C:/build/godot/w64.scons.msvc.x86_64.windows.editor.dev_build.4.5/bin/godot.windows.editor.dev.x86_64.exe
Run on (16 X 0.000000 MHz CPUs)
***WARNING*** Library was built as DEBUG. Timings may be affected.
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
BM_Flatbuffers_Encode     368707 ns       368708 ns         2003
BM_Flatbuffers_Decode       5183 ns         5183 ns       128036
BM_Flatbuffers_Use        322207 ns       322208 ns         2160
BM_Raw_Encode             339345 ns       339346 ns         2045
BM_Raw_Decode             340005 ns       340006 ns         2064
BM_Raw_Use            ERROR OCCURRED: 'Checksum did not match: 2524655701620245727 != 2524655701620275757'
6 benchmarks
```
so I have some things to look at, I think the times are wrong. I think I need to brush off my tracy binary to have a look at the numbers.

**Update 2026-03-05**: I compiled godot 4.7-dev with default options, I was using debug builds before, and I looked into tracy logs but didn't see anything specific I could point to. either way, I'm mostly unhappy with these scenarios. I don't think they are a very good match and they don't provide nice enough data right now.
```
2026-03-05T17:13:07
Running C:/build/godot/w64.mingw64.x86_64.windows.editor.default/bin/godot.windows.editor.x86_64.exe
Run on (16 X 0.000000 MHz CPUs)
***WARNING*** Library was built as DEBUG. Timings may be affected.
----------------------------------------------------------------
Benchmark                      Time             CPU   Iterations
----------------------------------------------------------------
BM_Flatbuffers_Encode      93531 ns        93531 ns         7035
BM_Flatbuffers_Decode       1001 ns         1001 ns       664521
BM_Flatbuffers_Use         85863 ns        85863 ns         7258
BM_Raw_Encode              69667 ns        69667 ns        10128
BM_Raw_Decode              77387 ns        77387 ns         8901
BM_Raw_Use                  8762 ns         8762 ns        76893
6 benchmarks
```