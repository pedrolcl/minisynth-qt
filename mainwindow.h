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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QtMath>
#include <QMap>
#include <QComboBox>
#include <QIODevice>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QScopedPointer>

#include <QAudioFormat>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#else
#include <QAudioSink>
#include <QAudioDevice>
#endif

#include "tonesynth.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void underrunDetected();

private:
    void initializeWindow();
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    void initializeAudio(const QAudioDeviceInfo &deviceInfo);
#else
    void initializeAudio(const QAudioDevice &deviceInfo);
#endif

private slots:
    void deviceChanged(int index);
    void volumeChanged(int value);
    void bufferChanged(int value);
    void octaveChanged(int value);
    void underrunMessage();

private:
    Ui::MainWindow *m_ui;
    QAudioFormat m_format;
    int m_bufferTime;
    bool m_running;
    QScopedPointer<ToneSynthesizer> m_synth;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QScopedPointer<QAudioOutput> m_audioOutput;
#else
    QScopedPointer<QAudioSink> m_audioOutput;
#endif
};

#endif // MAINWINDOW_H
