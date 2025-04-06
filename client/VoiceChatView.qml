import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import QtQuick.Effects
import QtMultimedia

Window {
    id: voiceCallWindow
    width: 360
    height: 640
    title: "语音通话"
    visible: false
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    // 主题颜色
    property color primaryColor: "#4F46E5"
    property color accentColor: "#6366F1"
    property color backgroundColor: "#111827"
    property color surfaceColor: "#1F2937"
    property color textColor: "#FFFFFF"
    property color secondaryTextColor: "#9CA3AF"
    property color successColor: "#10B981"
    property color errorColor: "#EF4444"

    // 通话状态
    property string callState: "incoming" // incoming, outgoing, connected, ended
    property string remoteName: "张三"
   // property string remoteAvatar: "https://randomuser.me/api/portraits/men/32.jpg"
    property int callDuration: 0
    property bool isMuted: false
    property bool isSpeaker: false


    //信号
    signal breakChat()
    signal responeVoiceChat(bool status)

    // 通话时长计时器
    Timer {
        id: durationTimer
        interval: 1000
        repeat: true
        running: callState === "connected"
        onTriggered: callDuration++
    }

    // 格式化通话时长
    function formatDuration(seconds) {
        var minutes = Math.floor(seconds / 60);
        var remainingSeconds = seconds % 60;
        return (minutes < 10 ? "0" : "") + minutes + ":" + (remainingSeconds < 10 ? "0" : "") + remainingSeconds;
    }

    // 接听电话
    function acceptCall() {
        responeVoiceChat(true)
        callState = "connected";
        callDuration = 0;
        durationTimer.start();
        callSound.stop();
        connectSound.play();
    }

    // 拒绝/结束电话
    function endCall() {
        if(callState === "incoming"){
            responeVoiceChat(false);
            callState = "ended";
            durationTimer.stop();
            endSound.play();
            endCallAnimation.start();
        }
        else if (callState !== "ended") {
            responeVoiceChat(false);
            breakChat();
            callState = "ended";
            durationTimer.stop();
            endSound.play();
            endCallAnimation.start();

        }

    }

    // 结束通话动画
    SequentialAnimation {
        id: endCallAnimation

        NumberAnimation {
            target: mainRect
            property: "opacity"
            to: 0
            duration: 300
            easing.type: Easing.InQuad
        }

        ScriptAction {
            script: {
                // 这里可以添加关闭窗口或重置状态的逻辑
                // voiceCallWindow.close();
                callState = "incoming"; // 为了演示，重置状态
                mainRect.opacity = 1;
            }
        }
    }

    // 音效
    // SoundEffect {
    //     id: callSound
    //     source: "qrc:/sounds/ringtone.wav"
    //     loops: SoundEffect.Infinite
    //     volume: 0.5
    //     playing: callState === "incoming" || callState === "outgoing"
    // }

    SoundEffect {
        id: connectSound
        source: "qrc:/sounds/connect.wav"
        volume: 0.5
    }

    SoundEffect {
        id: endSound
        source: "qrc:/sounds/end_call.wav"
        volume: 0.5
    }

    // 主容器
    Rectangle {
        id: mainRect
        anchors.fill: parent
        color: backgroundColor
        radius: 20

        // 添加渐变背景
        Rectangle {
            id: gradientBackground
            anchors.fill: parent
            radius: 20

            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(0.2, 0.2, 0.3, 1.0) }
                GradientStop { position: 1.0; color: Qt.rgba(0.1, 0.1, 0.15, 1.0) }
            }
        }

        // 添加模糊效果的装饰元素
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * 1.5
            height: width
            radius: width / 2
            rotation: -30

            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(primaryColor.r, primaryColor.g, primaryColor.b, 0.1) }
                GradientStop { position: 1.0; color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.0) }
            }

        }

        // 顶部状态栏
        Rectangle {
            id: statusBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            color: "transparent"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10

                Text {
                    text: "语音通话"
                    color: textColor
                    font.pixelSize: 16
                    font.bold: true
                }

                Item { Layout.fillWidth: true }
            }
        }

        // 用户头像和信息
        Item {
            id: userInfoSection
            anchors.top: statusBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: controlsSection.top

            // 头像
            Rectangle {
                id: avatarContainer
                width: 120
                height: 120
                radius: 60
                color: surfaceColor
                border.width: 4
                border.color: callState === "connected" ? successColor :
                              callState === "incoming" || callState === "outgoing" ? accentColor :
                              secondaryTextColor
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -50

                Image {
                    id: avatarImage
                    anchors.fill: parent
                    anchors.margins: 4
                    source: remoteAvatar
                    fillMode: Image.PreserveAspectCrop
                    layer.enabled: true
                }

                // 头像呼吸动画
                SequentialAnimation {
                    running: callState === "incoming" || callState === "outgoing"
                    loops: Animation.Infinite

                    NumberAnimation {
                        target: avatarContainer
                        property: "scale"
                        to: 1.05
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }

                    NumberAnimation {
                        target: avatarContainer
                        property: "scale"
                        to: 1.0
                        duration: 1000
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            // 用户名
            Text {
                id: nameText
                text: remoteName
                color: textColor
                font.pixelSize: 24
                font.bold: true
                anchors.top: avatarContainer.bottom
                anchors.topMargin: 20
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 通话状态
            Text {
                id: statusText
                text: {
                    if (callState === "incoming") return "来电..."
                    if (callState === "outgoing") return "呼叫中..."
                    if (callState === "connected") return "通话中 · " + formatDuration(callDuration)
                    return "通话结束"
                }
                color: secondaryTextColor
                font.pixelSize: 16
                anchors.top: nameText.bottom
                anchors.topMargin: 8
                anchors.horizontalCenter: parent.horizontalCenter
            }

            // 音频波形动画 (通话中显示)
            Row {
                id: audioWaveform
                anchors.top: statusText.bottom
                anchors.topMargin: 30
                anchors.horizontalCenter: parent.horizontalCenter
                spacing: 5
                visible: callState === "connected"

                Repeater {
                    model: 5

                    Rectangle {
                        width: 4
                        height: 20 + Math.random() * 20
                        radius: 2
                        color: accentColor

                        SequentialAnimation on height {
                            loops: Animation.Infinite
                            running: audioWaveform.visible

                            NumberAnimation {
                                to: 10 + Math.random() * 30
                                duration: 500 + index * 100
                                easing.type: Easing.InOutQuad
                            }

                            NumberAnimation {
                                to: 20 + Math.random() * 20
                                duration: 500 + index * 100
                                easing.type: Easing.InOutQuad
                            }
                        }
                    }
                }
            }
        }

        // 控制按钮区域
        Item {
            id: controlsSection
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 200

            // 来电控制按钮
            Row {
                id: incomingControls
                anchors.centerIn: parent
                spacing: 50
                visible: callState === "incoming"

                // 拒绝按钮
                RoundButton {
                    id: rejectButton
                    width: 70
                    height: 70

                    background: Rectangle {
                        radius: width / 2
                        color: errorColor

                        Rectangle {
                            anchors.centerIn: parent
                            width: 24
                            height: 4
                            radius: 2
                            color: "white"
                            rotation: 90
                        }
                    }

                    onClicked: endCall()

                    // 添加波纹效果
                    Rectangle {
                        id: rejectRipple
                        anchors.centerIn: parent
                        width: parent.width
                        height: parent.height
                        radius: width / 2
                        color: errorColor
                        opacity: 0

                        SequentialAnimation {
                            loops: Animation.Infinite
                            running: incomingControls.visible

                            NumberAnimation {
                                target: rejectRipple
                                property: "opacity"
                                to: 0.5
                                duration: 300
                            }

                            ParallelAnimation {
                                NumberAnimation {
                                    target: rejectRipple
                                    property: "opacity"
                                    to: 0
                                    duration: 1000
                                }

                                NumberAnimation {
                                    target: rejectRipple
                                    property: "scale"
                                    to: 1.5
                                    duration: 1000
                                }
                            }

                            ScriptAction {
                                script: rejectRipple.scale = 1.0
                            }
                        }
                    }
                }

                // 接听按钮
                RoundButton {
                    id: acceptButton
                    width: 70
                    height: 70

                    background: Rectangle {
                        radius: width / 2
                        color: successColor

                        Rectangle {
                            anchors.centerIn: parent
                            width: 24
                            height: 4
                            radius: 2
                            color: "white"
                        }
                    }

                    onClicked: acceptCall()

                    // 添加波纹效果
                    Rectangle {
                        id: acceptRipple
                        anchors.centerIn: parent
                        width: parent.width
                        height: parent.height
                        radius: width / 2
                        color: successColor
                        opacity: 0

                        SequentialAnimation {
                            loops: Animation.Infinite
                            running: incomingControls.visible

                            NumberAnimation {
                                target: acceptRipple
                                property: "opacity"
                                to: 0.5
                                duration: 300
                            }

                            ParallelAnimation {
                                NumberAnimation {
                                    target: acceptRipple
                                    property: "opacity"
                                    to: 0
                                    duration: 1000
                                }

                                NumberAnimation {
                                    target: acceptRipple
                                    property: "scale"
                                    to: 1.5
                                    duration: 1000
                                }
                            }

                            ScriptAction {
                                script: acceptRipple.scale = 1.0
                            }
                        }
                    }
                }
            }

            // 拨出电话控制按钮
            Item {
                id: outgoingControls
                anchors.centerIn: parent
                visible: callState === "outgoing"

                // 结束通话按钮
                RoundButton {
                    id: endOutgoingButton
                    width: 70
                    height: 70
                    anchors.centerIn: parent

                    background: Rectangle {
                        radius: width / 2
                        color: errorColor

                        Rectangle {
                            anchors.centerIn: parent
                            width: 24
                            height: 4
                            radius: 2
                            color: "white"
                        }
                    }

                    onClicked: endCall()
                }
            }

            // 通话中控制按钮
            Row {
                id: activeCallControls
                anchors.centerIn: parent
                spacing: 30
                visible: callState === "connected"

                // 静音按钮
                // RoundButton {
                //     id: muteButton
                //     width: 60
                //     height: 60

                //     background: Rectangle {
                //         radius: width / 2
                //         color: isMuted ? accentColor : surfaceColor
                //         border.width: 1
                //         border.color: isMuted ? "transparent" : secondaryTextColor

                //         Text {
                //             text: "🎤"
                //             color: isMuted ? textColor : secondaryTextColor
                //             font.pixelSize: 24
                //             anchors.centerIn: parent
                //         }
                //     }

                //     onClicked: isMuted = !isMuted
                // }

                // 结束通话按钮
                RoundButton {
                    id: endActiveButton
                    width: 70
                    height: 70

                    background: Rectangle {
                        radius: width / 2
                        color: errorColor

                        Rectangle {
                            anchors.centerIn: parent
                            width: 24
                            height: 4
                            radius: 2
                            color: "white"
                        }
                    }

                    onClicked: endCall()
                }

                // 扬声器按钮
                // RoundButton {
                //     id: speakerButton
                //     width: 60
                //     height: 60

                //     background: Rectangle {
                //         radius: width / 2
                //         color: isSpeaker ? accentColor : surfaceColor
                //         border.width: 1
                //         border.color: isSpeaker ? "transparent" : secondaryTextColor

                //         Text {
                //             text: "🔊"
                //             color: isSpeaker ? textColor : secondaryTextColor
                //             font.pixelSize: 24
                //             anchors.centerIn: parent
                //         }
                //     }

                //     onClicked: isSpeaker = !isSpeaker
                // }
            }
        }

        // 窗口拖动区域
        MouseArea {
            anchors.fill: parent
            property point clickPos: "0,0"

            onPressed: {
                clickPos = Qt.point(mouse.x, mouse.y)
            }

            onPositionChanged: {
                if (pressed) {
                    var delta = Qt.point(mouse.x - clickPos.x, mouse.y - clickPos.y)
                    voiceCallWindow.x += delta.x
                    voiceCallWindow.y += delta.y
                }
            }

            // 确保不干扰按钮点击
            z: -1
        }

        // 关闭按钮
        Rectangle {
            id: closeButton
            width: 30
            height: 30
            radius: 15
            color: closeArea.containsMouse ? Qt.rgba(1, 0, 0, 0.1) : "transparent"
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10

            Text {
                text: "✕"
                color: closeArea.containsMouse ? errorColor : secondaryTextColor
                font.pixelSize: 16
                anchors.centerIn: parent
            }

            MouseArea {
                id: closeArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    endCall()
                    voiceCallWindow.visible = false;

                }
            }
        }
    }


}
