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

#include <limits>
#include <QDebug>
#include <QtMath>
#include "tonesynth.h"

ToneSynthesizer::ToneSynthesizer(const QAudioFormat &format):
    QIODevice(),
    m_active(false),
    m_octave(3)
{
    //qDebug() << Q_FUNC_INFO;
    if (format.isValid()) {
        m_format = format;
    }
}

void ToneSynthesizer::start()
{
    qDebug() << Q_FUNC_INFO;
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
}

void ToneSynthesizer::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (isOpen()) {
        close();
    }
}

void ToneSynthesizer::noteOn(const QString &note)
{
    qDebug() << Q_FUNC_INFO << note;
    if (m_freq.contains(note)) {
        m_active = true;
        qreal noteFreq = qPow(2, m_octave - 3) * m_freq[note];
        qreal cyclesPerSample = noteFreq / m_format.sampleRate();
        m_angleDelta = cyclesPerSample * 2.0 * M_PI;
        m_currentAngle = 0.0;
    }
}

void ToneSynthesizer::noteOff()
{
    qDebug() << Q_FUNC_INFO
             << "last synth period:"
             << m_lastBufferSize << "bytes,"
             << m_format.durationForBytes(m_lastBufferSize) / 1000
             << "milliseconds";
    m_active = false;
}

qint64 ToneSynthesizer::lastBufferSize() const
{
    return m_lastBufferSize;
}

void ToneSynthesizer::resetLastBufferSize()
{
    m_lastBufferSize = 0;
}

void ToneSynthesizer::setOctave(int newOctave)
{
    m_octave = newOctave;
}

qint64 ToneSynthesizer::readData(char *data, qint64 maxlen)
{
    //qDebug() << Q_FUNC_INFO << maxlen;
    const qint64 channelBytes =
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            m_format.sampleSize() / CHAR_BIT;
#else
            m_format.bytesPerSample();
#endif
	Q_ASSERT(channelBytes > 0);
    qint64 length = (maxlen / channelBytes) * channelBytes;
    qint64 buflen = length;
    unsigned char *ptr = reinterpret_cast<unsigned char *>(data);
    while (length > 0){
        float currentSample = 0.0;
        if (m_active) {
            currentSample = qSin(m_currentAngle);
            m_currentAngle += m_angleDelta;
        }
        *reinterpret_cast<float *>(ptr) = currentSample;
        ptr += channelBytes;
        length -= channelBytes;
    }
    m_lastBufferSize = buflen;
    return buflen;
}

qint64 ToneSynthesizer::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    qDebug() << Q_FUNC_INFO;
	return 0;
}

qint64 ToneSynthesizer::size() const
{
    //qDebug() << Q_FUNC_INFO;
	return std::numeric_limits<qint64>::max();
}

qint64 ToneSynthesizer::bytesAvailable() const
{
    //qDebug() << Q_FUNC_INFO;
	return std::numeric_limits<qint64>::max();
}
