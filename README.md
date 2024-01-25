# Delay processor
This program was developed to run on a Huawei HG532s ADSL router. The firmware was modified in order to support ALSA and run this program.

## Build prerequisites
To build the program for the router you must save the toolchain under `/opt/trendchip/mips-linux-uclibc/` and also save the libraries and includes for alsa-lib (compiled for the router architecture) under `/opt/router/alsa-lib/`.

## Building
To compile the program for the router just run:
```
PROFILE=router make
```
If you want to build the program for run it on your PC:
```
make
```

## Running
Usage:
```
./build/delay_processor -p <playback_device> -c <capture_device> -t <delay_time in seconds> -f <delay_feedback 0 - 1> -w <delay_dry_wet 0 - 1>
```

For example, using the `plughw:0,0` interfaces, with a delay time of 100 milliseconds (0.1 seconds), 50% of feedback and 50% of dry wet:

```
./build/delay_processor -p plughw:0,0 -c plughw:0,0 -t 0.1 -f 0.5 -w 0.5
```

