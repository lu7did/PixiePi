#!/usr/bin/env python3
import wave,struct,math
waveFile = wave.open("/home/pi/PixiePi/bash/sampleaudio.wav", "rb")
frames   = waveFile.getnframes()   # total number of frames / samples
rate     = waveFile.getframerate() # number of frames / samples per second (should be 44100 Hz (44.1 kHz) for CD-quality audio)
length   = frames / int(rate)      # length in seconds
channels = waveFile.getnchannels() # number of channels (should be 2 channels (stereo) for CD-quality audio)
width    = waveFile.getsampwidth() # sample width / bit depth (should be 2 bytes (16 bits) for CD-quality audio)

print("Total Number of Frames:   " + str(frames))
print("Frames Per Second:        " + str(rate))
print("Total Number of Seconds:  " + str(length))
print("Total Number of Channels: " + str(channels))
print("Bytes Per Frame:          " + str(width))
