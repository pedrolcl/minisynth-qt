/*
    Minimal Synthesizer for Qt applications
    Copyright (C) 2022, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TONESYNTH_H
#define TONESYNTH_H

#include <QIODevice>
#include <QObject>
#include <QString>
#include <QMap>
#include <QAudioFormat>

class ToneSynthesizer : public QIODevice
{
    Q_OBJECT

public:
    ToneSynthesizer(const QAudioFormat &format);
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;
    void setOctave(int newOctave);

public slots:
    void start();
    void stop();
    void noteOn(const QString& note);
    void noteOff();

private:
    QAudioFormat m_format;
    bool m_active;
    int m_octave; /* octave 3 */
    /* Equal temperament scale */
    const QMap<QString, qreal> m_freq{
        {"C'", 	261.63},
        {"B",  	246.94},
        {"A#",  233.08},
        {"A",  	220.00},
        {"G#", 	207.65},
        {"G",  	196.00},
        {"F#",  185.00},
        {"F",   174.61},
        {"E",   164.81},
        {"D#", 	155.56},
        {"D",  	146.83},
        {"C#", 	138.59},
        {"C",  	130.81}
    };
    qreal m_angleDelta;
    qreal m_currentAngle;
    qint64 m_lastBufferSize;
};

#endif // TONESYNTH_H
