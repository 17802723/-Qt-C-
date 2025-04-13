#ifndef AUDIOHANDLER_H
#define AUDIOHANDLER_H

#include <QObject>
#include <QAudioSource>     // 音频输入类
#include <QAudioSink>       // 音频输出类
#include <QMediaDevices>    // 音频设备管理
#include <QByteArray>       // 字节数组容器
#include <QBuffer>          // 内存缓冲区

class AudioHandler : public QObject
{
    Q_OBJECT
public:

    explicit AudioHandler(QObject *parent = nullptr);

    ~AudioHandler();

    /**
     * @brief 初始化音频设备
     * @param inputDevice 输入设备，默认使用系统默认设备
     * @param outputDevice 输出设备，默认使用系统默认设备
     * @return bool 初始化是否成功
     */
    bool initialize(const QAudioDevice &inputDevice = QMediaDevices::defaultAudioInput(),
                    const QAudioDevice &outputDevice = QMediaDevices::defaultAudioOutput());

    /**
     * @brief 开始音频采集
     * @return bool 是否成功启动
     */
    bool startCapture();

    /**
     * @brief 停止音频采集
     */
    void stopCapture();

    /**
     * @brief 开始音频播放
     * @return bool 是否成功启动
     */
    bool startPlayback();

    /**
     * @brief 停止音频播放
     */
    void stopPlayback();

    /**
     * @brief 获取当前输入音频格式
     * @return QAudioFormat 当前输入格式
     */
    QAudioFormat inputFormat() const;

    /**
     * @brief 获取当前输出音频格式
     * @return QAudioFormat 当前输出格式
     */
    QAudioFormat outputFormat() const;

signals:
    /**
     * @brief 音频数据采集信号
     * @param data 采集到的音频数据
     */
    void audioDataCaptured(const QByteArray &data);

public slots:
    /**
     * @brief 接收并播放音频数据
     * @param data 要播放的音频数据
     */
    void receiveAudioData(const QByteArray &data);

private slots:
    /**
     * @brief 处理输入设备状态变化
     * @param state 新的状态
     */
    void handleInputStateChanged(QAudio::State state);

    /**
     * @brief 处理输出设备状态变化
     * @param state 新的状态
     */
    void handleOutputStateChanged(QAudio::State state);



private:
    /**
     * @brief 创建默认音频格式
     * @return QAudioFormat 配置好的音频格式
     */
    QAudioFormat defaultAudioFormat() const;

    /**
     * @brief 检查麦克风活动状态
     */
    void checkMicrophoneActivity();

    QAudioSource *m_audioInput = nullptr;    // 音频输入对象
    QAudioSink *m_audioOutput = nullptr;     // 音频输出对象
    QIODevice *m_inputDevice = nullptr;      // 输入设备接口
    QIODevice *m_outputDevice = nullptr;     // 输出设备接口

    QAudioFormat m_inputFormat;              // 输入音频格式
    QAudioFormat m_outputFormat;             // 输出音频格式
};

#endif // AUDIOHANDLER_H
