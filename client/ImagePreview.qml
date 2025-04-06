import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Window {
    id: imagePreviewWindow
    width: 800
    height: 600
    title: "图片预览"
    visible: false
    color: "transparent"
    flags: Qt.Window | Qt.FramelessWindowHint

    property string imageSource: ""
    property real zoomFactor: 1.0
    property bool isDragging: false
    property point dragStart
    property point windowStart

    // 添加窗口调整大小相关属性
    property int resizeBorderWidth: 5
    property bool isResizing: false
    property int resizeDirection: 0 // 0: 无, 1: 左, 2: 右, 3: 上, 4: 下, 5: 左上, 6: 右上, 7: 左下, 8: 右下
    property point resizeStartPos
    property size initialSize

    // 窗口关闭时重置缩放
    onVisibleChanged: {
        if (!visible) {
            zoomFactor = 1.0
        } else {
            showAnimation.start()
        }
    }

    // 显示动画
    NumberAnimation {
        id: showAnimation
        target: mainRect
        property: "opacity"
        from: 0
        to: 1
        duration: 200
        easing.type: Easing.OutCubic
    }

    // 关闭动画
    NumberAnimation {
        id: closeAnimation
        target: mainRect
        property: "opacity"
        from: 1
        to: 0
        duration: 200
        easing.type: Easing.InCubic
        onFinished: imagePreviewWindow.visible = false
    }

    // 关闭窗口的函数
    function closeWindow() {
        closeAnimation.start()
    }

    // 主容器
    Rectangle {
        id: mainRect
        anchors.fill: parent
        color: "#1A1A1A"
        radius: 12
        border.color: "#333333"
        border.width: 1
        opacity: 0 // 初始透明，通过动画显示

        // 窗口拖动区域
        // 窗口拖动区域
        MouseArea {
            id: dragArea
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40

            acceptedButtons: Qt.LeftButton //只处理鼠标左键
                   property point clickPos: "0,0"
                   onPressed: { //接收鼠标按下事件
                           clickPos  = Qt.point(mouse.x,mouse.y)
                   }
                   onPositionChanged: { //鼠标按下后改变位置
                           //鼠标偏移量
                           var delta = Qt.point(mouse.x-clickPos.x, mouse.y-clickPos.y)

                           //如果mainwindow继承自QWidget,用setPos
                           imagePreviewWindow.setX(imagePreviewWindow.x+delta.x)
                           imagePreviewWindow.setY(imagePreviewWindow.y+delta.y)
            //               rect1.x = rect1.x + delta.x
            //               rect1.y = rect1.y + delta.y
                   }
        }

        // 标题栏
        Rectangle {
            id: titleBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            color: "#2A2A2A"
            radius: 12

            // 只让顶部有圆角
            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                height: parent.height / 2
                color: parent.color
            }

            // Text {
            //     text: "图片预览"
            //     color: "#FFFFFF"
            //     font.pixelSize: 14
            //     font.bold: true
            //     anchors.centerIn: parent
            // }

            // 关闭按钮
            Rectangle {
                id: closeButton
                width: 30
                height: 30
                radius: 15
                color: "transparent"
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.verticalCenter: parent.verticalCenter

                Text {
                    text: "✕"
                    color: closeButtonArea.containsMouse ? "#FF6B6B" : "#CCCCCC"
                    font.pixelSize: 16
                    anchors.centerIn: parent
                }

                MouseArea {
                    id: closeButtonArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: closeWindow()
                }
            }
        }

        // 图片显示区域
        Flickable {
            id: imageFlickable
            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: controlBar.top
            anchors.margins: 20
            contentWidth: Math.max(imageItem.width * imagePreviewWindow.zoomFactor, width)
            contentHeight: Math.max(imageItem.height * imagePreviewWindow.zoomFactor, height)
            clip: true

            // 允许拖动图片
            interactive: contentWidth > width || contentHeight > height

            // 背景棋盘格（用于透明图片）
            // Grid {
            //     id: checkerboard
            //     anchors.fill: parent
            //     columns: Math.ceil(parent.width / 20)
            //     rows: Math.ceil(parent.height / 20)
            //     visible: true

            //     Repeater {
            //         model: checkerboard.columns * checkerboard.rows

            //         Rectangle {
            //             width: 20
            //             height: 20
            //             color: (Math.floor(index / checkerboard.columns) + index % checkerboard.columns) % 2 == 0 ? "#2A2A2A" : "#333333"
            //         }
            //     }
            // }

            Item {
                id: imageContainer
                width: Math.max(imageFlickable.width, imageItem.width * imagePreviewWindow.zoomFactor)
                height: Math.max(imageFlickable.height, imageItem.height * imagePreviewWindow.zoomFactor)

                Image {
                    id: imageItem
                    source: imagePreviewWindow.imageSource
                    width: sourceSize.width
                    height: sourceSize.height
                    scale: imagePreviewWindow.zoomFactor
                    smooth: true
                    antialiasing: true
                    asynchronous: true
                    cache: true
                    fillMode: Image.PreserveAspectFit

                    // 设置图片变换原点为中心
                    transformOrigin: Item.Center

                    // 居中显示
                    anchors.centerIn: parent

                    // 加载状态指示器
                    BusyIndicator {
                        anchors.centerIn: parent
                        running: parent.status === Image.Loading
                        width: 50
                        height: 50
                    }

                    // 加载失败显示
                    // Text {
                    //     anchors.centerIn: parent
                    //     text: "图片加载失败"
                    //     color: "#FF6B6B"
                    //     font.pixelSize: 16
                    //     visible: parent.status === Image.Error
                    // }
                }
            }

            // 添加鼠标滚轮缩放
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton

                onWheel: function(wheel) {
                    // 计算缩放因子
                    var oldZoom = imagePreviewWindow.zoomFactor

                    if (wheel.angleDelta.y > 0) {
                        // 放大
                        imagePreviewWindow.zoomFactor = Math.min(10.0, imagePreviewWindow.zoomFactor * 1.1)
                    } else {
                        // 缩小
                        imagePreviewWindow.zoomFactor = Math.max(0.1, imagePreviewWindow.zoomFactor / 1.1)
                    }

                    // 平滑动画
                    zoomAnimation.from = oldZoom
                    zoomAnimation.to = imagePreviewWindow.zoomFactor
                    zoomAnimation.start()
                }

                // 双击重置缩放
                onDoubleClicked: {
                    var oldZoom = imagePreviewWindow.zoomFactor
                    imagePreviewWindow.zoomFactor = 1.0

                    // 平滑动画
                    zoomAnimation.from = oldZoom
                    zoomAnimation.to = 1.0
                    zoomAnimation.start()
                }
            }

            // 缩放动画
            NumberAnimation {
                id: zoomAnimation
                target: imagePreviewWindow
                property: "zoomFactor"
                duration: 150
                easing.type: Easing.OutCubic
            }
        }

        // 控制按钮区域
        Rectangle {
            id: controlBar
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 70
            color: "#2A2A2A"
            radius: 12

            // 只让底部有圆角
            Rectangle {
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: parent.height / 2
                color: parent.color
            }

            RowLayout {
                anchors.centerIn: parent
                spacing: 20

                // 缩放信息显示
                Rectangle {
                    width: 80
                    height: 30
                    radius: 15
                    color: "#333333"

                    Text {
                        text: Math.round(imagePreviewWindow.zoomFactor * 100) + "%"
                        color: "white"
                        font.pixelSize: 12
                        anchors.centerIn: parent
                    }
                }

                // 缩小按钮
                Button {
                    id: zoomOutButton

                    background: Rectangle {
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 20
                        color: parent.pressed ? "#444444" : (parent.hovered ? "#3A3A3A" : "#333333")

                        Text {
                            text: "－"
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            anchors.centerIn: parent
                        }

                        // 添加过渡动画
                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }
                    }

                    onClicked: {
                        var oldZoom = imagePreviewWindow.zoomFactor
                        imagePreviewWindow.zoomFactor = Math.max(0.1, imagePreviewWindow.zoomFactor / 1.2)

                        // 平滑动画
                        zoomAnimation.from = oldZoom
                        zoomAnimation.to = imagePreviewWindow.zoomFactor
                        zoomAnimation.start()
                    }

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "缩小"
                    ToolTip.delay: 500
                }

                // 重置按钮
                Button {
                    id: resetButton

                    background: Rectangle {
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 20
                        color: parent.pressed ? "#444444" : (parent.hovered ? "#3A3A3A" : "#333333")

                        Text {
                            text: "⟲"
                            color: "white"
                            font.pixelSize: 16
                            anchors.centerIn: parent
                        }

                        // 添加过渡动画
                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }
                    }

                    onClicked: {
                        var oldZoom = imagePreviewWindow.zoomFactor
                        imagePreviewWindow.zoomFactor = 1.0

                        // 平滑动画
                        zoomAnimation.from = oldZoom
                        zoomAnimation.to = 1.0
                        zoomAnimation.start()
                    }

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "重置"
                    ToolTip.delay: 500
                }

                // 放大按钮
                Button {
                    id: zoomInButton

                    background: Rectangle {
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 20
                        color: parent.pressed ? "#444444" : (parent.hovered ? "#3A3A3A" : "#333333")

                        Text {
                            text: "＋"
                            color: "white"
                            font.pixelSize: 16
                            font.bold: true
                            anchors.centerIn: parent
                        }

                        // 添加过渡动画
                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }
                    }

                    onClicked: {
                        var oldZoom = imagePreviewWindow.zoomFactor
                        imagePreviewWindow.zoomFactor = Math.min(10.0, imagePreviewWindow.zoomFactor * 1.2)

                        // 平滑动画
                        zoomAnimation.from = oldZoom
                        zoomAnimation.to = imagePreviewWindow.zoomFactor
                        zoomAnimation.start()
                    }

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "放大"
                    ToolTip.delay: 500
                }

                // 保存按钮

                // 关闭按钮
                Button {
                    id: closeBtn

                    background: Rectangle {
                        implicitWidth: 40
                        implicitHeight: 40
                        radius: 20
                        color: parent.pressed ? "#AA4444" : (parent.hovered ? "#CC5555" : "#AA3333")

                        Text {
                            text: "✕"
                            color: "white"
                            font.pixelSize: 16
                            anchors.centerIn: parent
                        }

                        // 添加过渡动画
                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }
                    }

                    onClicked: {
                        closeWindow()
                    }

                    hoverEnabled: true
                    ToolTip.visible: hovered
                    ToolTip.text: "关闭"
                    ToolTip.delay: 500
                }
            }
        }

        // 键盘事件处理
        Keys.onPressed: function(event) {
            if (event.key === Qt.Key_Escape) {
                closeWindow()
                event.accepted = true
            } else if (event.key === Qt.Key_Plus || event.key === Qt.Key_Equal) {
                // 放大 (+/=)
                var oldZoom = imagePreviewWindow.zoomFactor
                imagePreviewWindow.zoomFactor = Math.min(10.0, imagePreviewWindow.zoomFactor * 1.2)
                zoomAnimation.from = oldZoom
                zoomAnimation.to = imagePreviewWindow.zoomFactor
                zoomAnimation.start()
                event.accepted = true
            } else if (event.key === Qt.Key_Minus) {
                // 缩小 (-)
                var oldZoom = imagePreviewWindow.zoomFactor
                imagePreviewWindow.zoomFactor = Math.max(0.1, imagePreviewWindow.zoomFactor / 1.2)
                zoomAnimation.from = oldZoom
                zoomAnimation.to = imagePreviewWindow.zoomFactor
                zoomAnimation.start()
                event.accepted = true
            } else if (event.key === Qt.Key_0) {
                // 重置 (0)
                var oldZoom = imagePreviewWindow.zoomFactor
                imagePreviewWindow.zoomFactor = 1.0
                zoomAnimation.from = oldZoom
                zoomAnimation.to = 1.0
                zoomAnimation.start()
                event.accepted = true
            }
        }

        // 图片信息显示
        Rectangle {
            id: imageInfoPanel
            anchors.top: titleBar.bottom
            anchors.right: parent.right
            anchors.margins: 20
            width: 180
            height: 80
            color: "#333333"
            radius: 8
            opacity: 0.8
            visible: imageItem.status === Image.Ready

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 5

                Text {
                    text: "图片尺寸: " + imageItem.sourceSize.width + " × " + imageItem.sourceSize.height
                    color: "white"
                    font.pixelSize: 12
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }

                Text {
                    text: "缩放: " + Math.round(imagePreviewWindow.zoomFactor * 100) + "%"
                    color: "white"
                    font.pixelSize: 12
                }

                Text {
                    text: "提示: 滚轮缩放, 拖动平移"
                    color: "#AAAAAA"
                    font.pixelSize: 11
                    font.italic: true
                }
            }

            // 淡入淡出动画
            NumberAnimation {
                id: infoFadeOut
                target: imageInfoPanel
                property: "opacity"
                to: 0
                duration: 500
                easing.type: Easing.InOutQuad
            }

            // 显示3秒后自动淡出
            Timer {
                interval: 3000
                running: imageInfoPanel.visible && imageInfoPanel.opacity > 0
                onTriggered: infoFadeOut.start()
            }

            // 鼠标悬停时保持显示
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: {
                    imageInfoPanel.opacity = 0.8
                    infoFadeOut.stop()
                }
                onExited: {
                    infoFadeOut.start()
                }
            }
        }

        // 加载指示器
        BusyIndicator {
            visible: false
            anchors.centerIn: parent
            width: 80
            height: 80
            running: imageItem.status === Image.Loading

            // 自定义样式
            contentItem: Item {
                implicitWidth: 64
                implicitHeight: 64

                Rectangle {
                    id: spinnerRect
                    width: parent.width
                    height: parent.height
                    radius: width / 2
                    border.width: 8
                    border.color: "#80FFFFFF"
                    color: "transparent"
                }

                // 旋转的指示器
                Item {
                    id: spinner
                    anchors.fill: parent

                    Rectangle {
                        width: parent.width
                        height: 8
                        radius: 4
                        color: "#FFFFFF"
                        anchors.top: parent.top
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    RotationAnimation on rotation {
                        from: 0
                        to: 360
                        duration: 1500
                        loops: Animation.Infinite
                        running: imageItem.status === Image.Loading
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        // 确保窗口可以接收键盘事件
        imagePreviewWindow.focus = true
    }

    onActiveChanged: {
        if (active) {
            imagePreviewWindow.focus = true
        }
    }
}
