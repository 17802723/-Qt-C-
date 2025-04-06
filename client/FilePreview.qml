import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    color: "transparent"
    radius: 8

    property string fileName: ""
    property int fileSize: 0
    property real progress: 0
    property bool isReceiving: false
    property bool isCompleted: false
    property bool isTranslate: false
    property color bgColor: "#666666"

    signal cancelTransfer()

    signal downloadRequested()
    signal openDownloadMenu()
    property bool isHovered: false




    // 文件预览卡片
    Rectangle {
        anchors.fill: parent
        color: "#F5F5F5"
        radius: parent.radius
        border.color: "#E0E0E0"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 12

            // 文件图标
            Rectangle {
                width: 40
                height: 40
                radius: 8
                color: "#E3F2FD"
                Layout.alignment: Qt.AlignVCenter

                Text {
                    text: "📄"
                    font.pixelSize: 24
                    anchors.centerIn: parent
                }
            }

            // 文件信息和进度条
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4

                // 文件名
                Text {
                    text: root.fileName
                    font.pixelSize: 14
                    color: "#333333"
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }

                // 文件大小
                Text {
                    text: (root.fileSize / 1024 / 1024).toFixed(2) + " MB"
                    font.pixelSize: 12
                    color: "#666666"
                }

                // 进度条（仅在传输时显示）
                Rectangle {
                    //visible: !root.isCompleted && (root.progress > 0 || root.isReceiving)
                    visible: isTranslate
                    Layout.fillWidth: true
                    height: 4
                    radius: 2
                    color: "#E0E0E0"

                    Rectangle {
                        width: parent.width * (root.progress / 100)
                        height: parent.height
                        radius: parent.radius
                        color: "#07C160"

                        // 进度条动画
                        Behavior on width {
                            NumberAnimation { duration: 200 }
                        }
                    }
                }

                // 状态文本
                Text {
                    //visible: !root.isCompleted && (root.progress > 0 || root.isReceiving)
                    visible: isTranslate
                    text: Math.round(root.progress) + "%"
                    font.pixelSize: 12
                    color: "#666666"
                }
            }

            // 下载按钮
                        Rectangle {
                            id:download
                            visible: !isTranslate && !isCompleted  // 当文件未在传输且未完成时显示
                            width: 24
                            height: 24
                            radius: 12
                            color: bgColor
                            Layout.alignment: Qt.AlignVCenter

                            Text {
                                text: "⭳"  // 下载图标
                                color: "white"
                                font.pixelSize: 16
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                acceptedButtons:Qt.LeftButton | Qt.RightButton
                                propagateComposedEvents: true  // 允许事件传播
                                hoverEnabled: true  // 启用悬停检测
                                onEntered: {
                                    console.log("鼠标进入")
                                    root.bgColor = "a9a9a9"
                                    isHovered = true
                                }
                                onExited: {
                                    console.log("鼠标离开")
                                    root.bgColor = "#666666"
                                    isHovered = false
                                }

                            }

                        }

            // 完成图标
            Rectangle {
                visible: root.isCompleted
                width: 24
                height: 24
                radius: 12
                color: "#07C160"
                Layout.alignment: Qt.AlignVCenter

                Text {
                    text: "✓"
                    color: "white"
                    font.pixelSize: 16
                    anchors.centerIn: parent
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        acceptedButtons:Qt.LeftButton | Qt.RightButton
        propagateComposedEvents: true  // 允许事件传播
            onClicked: {
                if (mouse.button === Qt.LeftButton) {
                    if(isHovered)
                    {
                        root.downloadRequested()  // 发送下载信号
                    }

                }
                else if (mouse.button === Qt.RightButton) {  // 右键点击时显示菜单
                    root.openDownloadMenu()
                }
            }
    }


    // 重置状态
    function reset() {
        progress = 0
        isCompleted = false
        isReceiving = false
    }
}
