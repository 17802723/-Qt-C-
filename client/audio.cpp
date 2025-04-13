#include "audio.h"
#include <QDebug>
#include <QThread>

AudioHandler::AudioHandler(QObject *parent)
    : QObject(parent)
{
    // 初始化默认音频格式
    m_inputFormat = defaultAudioFormat();
    m_outputFormat = defaultAudioFormat();

    qDebug() << "Audio 创建于线程:" << QThread::currentThreadId();
}

AudioHandler::~AudioHandler()
{
    // 停止所有音频活动
    stopCapture();
    stopPlayback();

    // 释放资源
    if (m_audioInput) {
        delete m_audioInput;
        m_audioInput = nullptr;
    }

    if (m_audioOutput) {
        delete m_audioOutput;
        m_audioOutput = nullptr;
    }

    qDebug() << "Audio 已销毁";
}

bool AudioHandler::initialize(const QAudioDevice &inputDevice, const QAudioDevice &outputDevice)
{
    // 检查设备是否可用
    if (!inputDevice.isFormatSupported(m_inputFormat)) {
        qWarning() << "默认输入格式不被支持";
    }

    if (!outputDevice.isFormatSupported(m_outputFormat)) {
        qWarning() << "默认输出格式不被支持";
    }
    qDebug() << "音频输入设备：" << inputDevice.description();
    qDebug() << "音频输出设备：" << outputDevice.description();

    // 打印最终使用的格式
    qDebug() << "输入格式:"
             << "采样率:" << m_inputFormat.sampleRate()
             << "格式:" << m_inputFormat.sampleFormat()
             << "声道:" << m_inputFormat.channelCount();

    qDebug() << "输出格式:"
             << "采样率:" << m_outputFormat.sampleRate()
             << "格式:" << m_outputFormat.sampleFormat()
             << "声道:" << m_outputFormat.channelCount();

    // 创建音频输入对象(QAudioSource)
    m_audioInput = new QAudioSource(inputDevice, m_inputFormat, this);
    if (!m_audioInput) {
        qCritical() << "无法创建音频输入对象";
        return false;
    }

    // 连接状态变化信号
    connect(m_audioInput, &QAudioSource::stateChanged,
            this, &AudioHandler::handleInputStateChanged);

    // 创建音频输出对象(QAudioSink)
    m_audioOutput = new QAudioSink(outputDevice, m_outputFormat, this);
    if (!m_audioOutput) {
        qCritical() << "无法创建音频输出对象";
        return false;
    }

    // 连接状态变化信号
    connect(m_audioOutput, &QAudioSink::stateChanged,
            this, &AudioHandler::handleOutputStateChanged);

    return true;
}

bool AudioHandler::startCapture()
{
    // 确保在主线程调用
    if (QThread::currentThread() != this->thread()) {
        qWarning() << "startCapture必须在对象所属线程调用";
        return false;
    }

    if (!m_audioInput) {
        qCritical() << "音频输入未初始化";
        return false;
    }

    // 启动音频输入设备
    m_inputDevice = m_audioInput->start();
    if (!m_inputDevice) {
        qCritical() << "无法启动音频输入设备";
        return false;
    }

    // 连接数据可读信号
    connect(m_inputDevice, &QIODevice::readyRead, [this]() {
        QByteArray data = m_inputDevice->readAll();
        if (!data.isEmpty()) {
            emit audioDataCaptured(data);
            //qDebug() << "采集到音频数据:" << data.size() << "字节";
        }
    });

    qDebug() << "音频采集已启动";
    return true;
}

void AudioHandler::stopCapture()
{
    if (m_audioInput) {
        m_audioInput->stop();
        if (m_inputDevice) {
            m_inputDevice->disconnect();
            m_inputDevice = nullptr;
        }
        qDebug() << "音频采集已停止";
    }
}

bool AudioHandler::startPlayback()
{
    if (!m_audioOutput) {
        qCritical() << "音频输出未初始化";
        return false;
    }

    m_outputDevice = m_audioOutput->start();
    if (!m_outputDevice) {
        qCritical() << "无法启动音频输出设备";
        return false;
    }

    qDebug() << "音频播放已启动";
    return true;
}

void AudioHandler::stopPlayback()
{
    if (m_audioOutput) {
        m_audioOutput->stop();
        if (m_outputDevice) {
            m_outputDevice = nullptr;
        }
        qDebug() << "音频播放已停止";
    }
}

QAudioFormat AudioHandler::inputFormat() const
{
    return m_inputFormat;
}

QAudioFormat AudioHandler::outputFormat() const
{
    return m_outputFormat;
}

void AudioHandler::receiveAudioData(const QByteArray &data)
{
    if (!m_outputDevice) {
        qWarning() << "输出设备未就绪";
        return;
    }

    qint64 written = m_outputDevice->write(data);
    if (written != data.size()) {
        qWarning() << "音频数据未完全写入，预期:" << data.size()
                   << "实际:" << written;
    }
}

void AudioHandler::handleInputStateChanged(QAudio::State state)
{
    switch (state) {
    case QAudio::ActiveState:
        qDebug() << "输入状态: 活跃(正在采集)";
        break;
    case QAudio::SuspendedState:
        qDebug() << "输入状态: 暂停";
        break;
    case QAudio::StoppedState:
        if (m_audioInput->error() != QAudio::NoError) {
            qCritical() << "输入错误:" << m_audioInput->error();
        } else {
            qDebug() << "输入正常停止";
        }
        break;
    case QAudio::IdleState:
        qDebug() << "输入状态: 空闲(无数据输入)";
        checkMicrophoneActivity();
        break;
    }
}

void AudioHandler::handleOutputStateChanged(QAudio::State state)
{
    switch (state) {
    case QAudio::ActiveState:
        qDebug() << "输出状态: 活跃(正在播放)";
        break;
    case QAudio::SuspendedState:
        qDebug() << "输出状态: 暂停";
        break;
    case QAudio::StoppedState:
        if (m_audioOutput->error() != QAudio::NoError) {
            qCritical() << "输出错误:" << m_audioOutput->error();
        } else {
            qDebug() << "输出正常停止";
        }
        break;
    case QAudio::IdleState:
        qDebug() << "输出状态: 空闲(无数据输出)";
        break;
    }
}

QAudioFormat AudioHandler::defaultAudioFormat() const
{
    QAudioFormat format;

    // 设置音频参数
    format.setSampleRate(48000);  // 48kHz采样率
    format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    format.setSampleFormat(QAudioFormat::Int16);  // 16位有符号整数

    return format;
}

void AudioHandler::checkMicrophoneActivity()
{
    // 实现麦克风活动检测逻辑
    static int silentCount = 0;
    const int maxSilentCount = 10;

    bool isSilent = true; // 这里需要实现实际的静音检测

    if (isSilent) {
        silentCount++;
        if (silentCount >= maxSilentCount) {
            qWarning() << "麦克风长时间无输入，尝试重新初始化";
            silentCount = 0;
            QMetaObject::invokeMethod(this, [this]() {
                stopCapture();
                startCapture();
            }, Qt::QueuedConnection);
        }
    } else {
        silentCount = 0;
    }
}
