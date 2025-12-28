/*
    Minimal Synthesizer for Qt applications
    Copyright (C) 2022-2025 Pedro Lopez-Cabanillas <plcl@users.sf.net>

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
#include <QTimer>

#include <QAudioFormat>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QAudioOutput>
#else
#include <QMediaDevices>
#include <QAudioSink>
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

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void initializeFormat();
    void initializeAudio();
    void initializeWindow();
    void initializeDevice();

signals:
    void underrunDetected();
    void stallDetected();

private slots:
    void deviceChanged(int index);
    void volumeChanged(int value);
    void bufferChanged(int value);
    void octaveChanged(int value);
#if !defined(Q_OS_WASM)
    void underrunMessage();
    void stallMessage();
#endif

    void updateDevices();

private:
    Ui::MainWindow *m_ui;
    QAudioFormat m_format;
    int m_bufferTime;
    bool m_running;
    ToneSynthesizer *m_synth;

    // clang-format off
    const QMap<int, QString> m_keys{
        {Qt::Key_K, "C'"},
        {Qt::Key_J, "B"},
        {Qt::Key_U, "A#"},
        {Qt::Key_H, "A"},
        {Qt::Key_Y, "G#"},
        {Qt::Key_G, "G"},
        {Qt::Key_T, "F#"},
        {Qt::Key_F, "F"},
        {Qt::Key_D, "E"},
        {Qt::Key_E, "D#"},
        {Qt::Key_S, "D"},
        {Qt::Key_W, "C#"},
        {Qt::Key_A, "C"}
    };
    // clang-format on

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QAudioDeviceInfo m_defaultDeviceInfo, m_currentDeviceInfo;
    QAudioOutput *m_audioOutput;
#else
    QAudioDevice m_defaultDeviceInfo, m_currentDeviceInfo;
    QAudioSink *m_audioOutput{nullptr};
    QMediaDevices *m_devices{nullptr};
#endif

#if !defined(Q_OS_WASM)
    QTimer m_stallDetector;
#endif
};

#endif // MAINWINDOW_H
