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

#include <QDebug>
#include <QKeyEvent>
#include <QtMath>
#if !defined(Q_OS_WASM)
#include <QMessageBox>
#endif
#include <QTimer>
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QAudioDeviceInfo>
#include <QAudioOutput>
#else
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSink>
#endif

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
#if defined(Q_OS_WASM)
    , m_bufferTime(100)
#else
    , m_bufferTime(50)
#endif
    , m_running(false)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    , m_devices(new QMediaDevices(this))
#endif
{
    //qDebug() << Q_FUNC_INFO;
    m_ui->setupUi(this);
    initializeFormat();
    initializeAudio();
    initializeWindow();
    initializeDevice();
}

MainWindow::~MainWindow()
{
    //qDebug() << Q_FUNC_INFO;
#if !defined(Q_OS_WASM)
    m_stallDetector.stop();
#endif
    m_audioOutput->stop();
    if (m_synth) {
        m_synth->stop();
    }
    delete m_audioOutput;
    delete m_synth;
    delete m_ui;
}

void MainWindow::initializeFormat()
{
    //qDebug() << Q_FUNC_INFO;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_format.setChannelCount(1);
    m_format.setSampleSize(sizeof(float) * CHAR_BIT);
    m_format.setCodec("audio/pcm");
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setSampleType(QAudioFormat::Float);
    m_defaultDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
#else
    m_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    m_format.setSampleFormat(QAudioFormat::Float);
    m_defaultDeviceInfo = m_devices->defaultAudioOutput();
#endif
    m_format.setSampleRate(44100);
    m_currentDeviceInfo = m_defaultDeviceInfo;
   // qDebug() << Q_FUNC_INFO << m_format;
}

void MainWindow::initializeAudio()
{
    //qDebug() << Q_FUNC_INFO;
    m_running = false;
    
    m_synth = new ToneSynthesizer(m_format);
    qint64 bufferLength = m_format.bytesForDuration(m_bufferTime * 1000);
    // qDebug() << "requested buffer size:" << bufferLength << "bytes," << m_bufferTime
    //          << "milliseconds";
    m_synth->start();
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    m_audioOutput = new QAudioOutput(m_currentDeviceInfo, m_format);
    QObject::connect(m_audioOutput, &QAudioOutput::stateChanged, this, [=](QAudio::State state) {
#else
    m_audioOutput = new QAudioSink(m_currentDeviceInfo, m_format);
    connect(m_audioOutput, &QAudioSink::stateChanged, this, [=](QAudio::State state) {
#endif
        //qDebug() << "Audio Output state:" << state << "error:" << m_audioOutput->error();
        if (m_running && (m_audioOutput->error() == QAudio::UnderrunError)) {
            emit underrunDetected();
        }
    });
    m_audioOutput->setBufferSize(bufferLength);
    m_audioOutput->start(m_synth);
    auto bufferTime = m_format.durationForBytes(m_audioOutput->bufferSize()) / 1000;
    // qDebug() << "applied buffer size:" << m_audioOutput->bufferSize() << "bytes," << bufferTime
    //          << "milliseconds";
    volumeChanged(m_ui->volumeSlider->value());
    octaveChanged(m_ui->octaveSpin->value());
#if !defined(Q_OS_WASM)
    QTimer::singleShot(bufferTime * 2, this, [=]{
        m_running = true;
        m_stallDetector.start(bufferTime * 4);
     });
#endif
}

void MainWindow::updateDevices()
{
    qDebug() << Q_FUNC_INFO;
    m_ui->deviceBox->clear();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    m_ui->deviceBox->addItem(m_defaultDeviceInfo.deviceName(),
                             QVariant::fromValue(m_defaultDeviceInfo));
    for (auto &deviceInfo : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        qDebug() << Q_FUNC_INFO << deviceInfo.deviceName() << deviceInfo.isFormatSupported(m_format);
        if (deviceInfo != m_defaultDeviceInfo && deviceInfo.isFormatSupported(m_format))
            m_ui->deviceBox->addItem(deviceInfo.deviceName(), QVariant::fromValue(deviceInfo));
    }
    m_ui->deviceBox->setCurrentText(m_defaultDeviceInfo.deviceName());
#else
    m_ui->deviceBox->addItem(m_defaultDeviceInfo.description(),
                             QVariant::fromValue(m_defaultDeviceInfo));
    for (auto &deviceInfo : m_devices->audioOutputs()) {
        // see https://qt-project.atlassian.net/browse/QTBUG-136057
        qDebug() << Q_FUNC_INFO << deviceInfo.description() << deviceInfo.isFormatSupported(m_format);
        if (deviceInfo != m_defaultDeviceInfo && deviceInfo.isFormatSupported(m_format))
            m_ui->deviceBox->addItem(deviceInfo.description(), QVariant::fromValue(deviceInfo));
    }
    m_ui->deviceBox->setCurrentText(m_defaultDeviceInfo.description());
#endif
}

void MainWindow::initializeWindow()
{
    //qDebug() << Q_FUNC_INFO;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_devices, &QMediaDevices::audioOutputsChanged, this, &MainWindow::updateDevices);
#endif    
    updateDevices();

    m_ui->bufferSpin->setValue(m_bufferTime);
    connect(m_ui->deviceBox, SIGNAL(activated(int)), this, SLOT(deviceChanged(int)));
    connect(m_ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(volumeChanged(int)));
    connect(m_ui->bufferSpin, SIGNAL(valueChanged(int)), this, SLOT(bufferChanged(int)));
    connect(m_ui->octaveSpin, SIGNAL(valueChanged(int)), this, SLOT(octaveChanged(int)));
#if !defined(Q_OS_WASM)
    connect(this, &MainWindow::underrunDetected, this, &MainWindow::underrunMessage);
    connect(this, &MainWindow::stallDetected, this, &MainWindow::stallMessage);
    connect(&m_stallDetector, &QTimer::timeout, this, [=] {
        if (m_running) {
            if (m_synth->lastBufferSize() == 0) {
                emit stallDetected();
            }
            m_synth->resetLastBufferSize();
        }
    });
#endif
    auto buttons = findChildren<QPushButton *>();
    foreach (const auto btn, buttons) {
        connect(btn, &QPushButton::pressed, this, [=] { m_synth->noteOn(btn->text()); });
        connect(btn, &QPushButton::released, this, [=] { m_synth->noteOff(); });
    }
}

void MainWindow::initializeDevice()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QAudioDeviceInfo deviceInfo = m_ui->deviceBox->currentData().value<QAudioDeviceInfo>();
#else
    const QAudioDevice deviceInfo = m_ui->deviceBox->currentData().value<QAudioDevice>();
#endif
    if (m_currentDeviceInfo != deviceInfo) {
        if (!deviceInfo.isFormatSupported(m_format)) {
#if !defined(Q_OS_WASM)
            QMessageBox::warning(this,
                                 "Audio format not supported",
                                 "The selected audio device does not support the synth's audio format. "
                                 "Please select another device.");
#endif
            return;
        }
        m_currentDeviceInfo = deviceInfo;
    }
}

void MainWindow::deviceChanged(int index)
{
    // qDebug() << Q_FUNC_INFO << m_ui->deviceBox->itemText(index);
#if !defined(Q_OS_WASM)
    m_stallDetector.stop();
#endif
    m_audioOutput->stop();
    if (m_synth) {
        m_synth->stop();
    }
    delete m_synth;
    delete m_audioOutput;
    m_synth = nullptr;
    m_audioOutput = nullptr;

    initializeDevice();
    initializeAudio();
}

void MainWindow::volumeChanged(int value)
{
    //qDebug() << Q_FUNC_INFO << value;
    qreal linearVolume = QAudio::convertVolume(value / 100.0,
                                               QAudio::LogarithmicVolumeScale,
                                               QAudio::LinearVolumeScale);
    m_audioOutput->setVolume(linearVolume);
}

void MainWindow::bufferChanged(int value)
{
    if (m_bufferTime != value) {
        m_bufferTime = value;
        deviceChanged(m_ui->deviceBox->currentIndex());
        //qDebug() << Q_FUNC_INFO << value;
    }
}

void MainWindow::octaveChanged(int value)
{
    //qDebug() << Q_FUNC_INFO << value;
    m_synth->setOctave(value);
}

#if !defined(Q_OS_WASM)
void MainWindow::underrunMessage()
{
    m_running = false;
    QMessageBox::warning( this, "Underrun Error",
                          "Audio buffer underrun errors have been detected."
                          " Please increase the buffer time to avoid this problem.");
    m_running = true;
}

void MainWindow::stallMessage()
{
    QMessageBox::critical( this, "Audio Output Stalled",
                           "Audio output is stalled right now. Sound cannot be produced."
                           " Please increase the buffer time to avoid this problem.");
    m_running = false;
    m_stallDetector.stop();
}
#endif

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int k = static_cast<int>(event->key());
    if (m_keys.contains(k) && !event->isAutoRepeat() && !m_synth->isPlaying(m_keys[k])) {
        m_synth->noteOn(m_keys[k]);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    const auto k = event->key();
    if (m_keys.contains(k) && m_synth->isPlaying(m_keys[k])) {
        m_synth->noteOff();
    }
}
