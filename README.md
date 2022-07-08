# Minimalist Music Synthesizer for Qt

This project is a minimalist multiplatform music synth demo, using Qt Multimedia audio output. The synthesizer generates simple sine wave tones, which sound both dry and harsh, so they are not very useful for actual musical usage, except maybe for building a crude tuner.

![Screenshot](screenshot.png)

The synth is implemented in the `ToneSynthesizer` class, that expects an `AudioFormat` using floating point samples, a single channel (mono), and a sample rate of 44100 Hz. The 13 precomputed note frequencies reside on a table initialized with an [equal temperament](https://en.wikipedia.org/wiki/Equal_temperament#Twelve-tone_equal_temperament)
chromatic scale. The notes may be transposed from octave 0 to octave 9 in [scientific pitch notation](https://en.wikipedia.org/wiki/Scientific_pitch_notation) octave denomination.

The [Qt Multimedia](https://doc.qt.io/qt-6.2/multimediaoverview.html) [audio output](https://doc.qt.io/qt-6.2/audiooverview.html#low-level-audio-playback-and-recording) classes allow raw access to the system's audio output facilities, allowing applications to write raw data to speakers or other devices. This demo has a control to choose the audio device, the volume level and to request a buffer size (by default 50 milliseconds).

The applied buffer size sometimes is not the same as the requested buffer size, but the real latency is usually a smaller value anyway. See the debug output of the program for the real values achieved in your system.

An useful conclusion from this prototype is that the infrastructure may be usable across platforms and Qt versions, with only some exceptions: Qt versions between Qt 6.0 and Qt 6.3 are not suitable for Linux applications; see for instance [this bug report](https://bugreports.qt.io/browse/QTBUG-101169).
