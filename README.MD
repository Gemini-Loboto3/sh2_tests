# SH2TESTS
This is a stand alone collection of hacks for testing before getting integrated into [Silent Hill 2 Enhanced Edition](https://github.com/elishacloud/Silent-Hill-2-Enhancements).

## criware
A reimplementation of Criware's middleware library for audio streaming of ADX compressed data, with some support for AFS. This module implements the following features:
* Replicates all public ADX functions found in SH2;
* Reworked architecture to use only one thread to manage data transfers and to avoid desyncing issues;
* File I/O reworked for more practical modern data access (i.e. disregards DVD limitations in favor of better flow);
* ADX parsing and real time decoding;
* AIX real time demuxing and abstraction to ADX objects;
* Limited AFS support with one partition and numeric IDs;
* Implements the original DirectSound8 interface, but can be easily adapted to using any other sound API like XAudio2 or OpenAL.

## dxrinpit [work in progress]
A module to implement native DualShock 4 and DualSense support via Raw Input masking itself as a DInput device.
## xaudio [incomplete]

An early implementation of the criware module that uses an XAudio 2.9 interface.