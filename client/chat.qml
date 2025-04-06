import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs   // Qt6 文件对话框
import QtCore 6.4        // 剪贴板功能
import QtQuick.Window
import Qt.labs.platform
import QtQuick.Controls.Material 2.15  // 添加Material样式支持

ApplicationWindow {
    id: root
    width: 800
    height: 600
    visible: true
    title: "聊天应用"  // 添加应用标题

    // 应用全局样式
    property color primaryColor: "#07C160"       // 主色调
    property color secondaryColor: "#FFB6C1"     // 次要色调
    property color bgColor: "#F5F5F5"            // 背景色
    property color sidebarColor: "#a9a9a9"       // 侧边栏颜色
    property color chatListColor: "#404040"      // 聊天列表背景色
    property color messageOutColor: "#95EC69"    // 发出消息气泡颜色
    property color messageInColor: "#FFFFFF"     // 收到消息气泡颜色
    property int defaultRadius: 8                // 默认圆角大小
    property int defaultMargin: 10               // 默认边距




    // 消息存储结构
    property var chatMessages: ({})

    property var messageAtt: ({})

    // 用户信息
    property string username: server.getUserName()
    // 当前选中会话
    property string currentChat
    // 待处理好友请求数量
    property int beAddedFriendNum: 0

    // 加载指定聊天的消息
    function loadMessages(chatName) {
        if(chatName in chatMessages) {
            messageModel.clear()
            chatMessages[chatName].forEach(msg => {
                //messageModel.append(msg)
                messageModel.append({
                            sender: msg.sender,
                            content: msg.content,
                            time: msg.time,
                            fileType: msg.fileType,
                            fileSize: msg.fileSize || 0,
                            progress: msg.fileprogress || 0,
                            isReceiving: msg.sender !== "me",
                            isCompleted: msg.fileprogress >= 100,
                            isTrans: msg.isTrans || false
                    })
            })
            listView.positionViewAtEnd()
        } else {
            chatMessages[chatName] = []
            messageModel.clear()
            chatMessages[chatName].forEach(msg => {
                //messageModel.append(msg)
                messageModel.append({
                            sender: msg.sender,
                            content: msg.content,
                            time: msg.time,
                            fileType: msg.fileType,
                            fileSize: msg.fileSize || 0,
                            progress: msg.fileprogress || 0,
                            isReceiving: msg.sender !== "me",
                            isCompleted: msg.fileprogress >= 100,
                            isTrans: msg.isTrans || false
                })
            })
            listView.positionViewAtEnd()
        }
    }

    //刷新指定用户的最新消息
    function refrashUserNewMessage(friendName){
        if (!(friendName in chatMessages) || chatMessages[friendName].length === 0) {
                return;
            }

            const latestMessage = chatMessages[friendName][chatMessages[friendName].length - 1];

            if (latestMessage.fileType === "file") {

                for (let i = 0; i < chatList.count; i++) {
                    const item = chatList.get(i);

                    if (item.name === friendName) {
                        chatList.setProperty(i, "newMessage",  (latestMessage.sender === "me" ? username : latestMessage.sender) + ":[文件]" + latestMessage.content);
                        return;
                    }
                }
                console.log("未找到匹配的用户:", friendName);
            }
            else{
                for (let i = 0; i < chatList.count; i++) {
                    const item = chatList.get(i);

                    if (item.name === friendName) {
                        chatList.setProperty(i, "newMessage", (latestMessage.sender === "me" ? username : latestMessage.sender)  + "：" + server.QmlHtmlPro(latestMessage.content));
                        return;
                    }
                }

            }
    }

    // 搜索用户连接
    Connections{
        target: server
        onSerachFriendRes: (successStatus) => {
            var existingIndex = -1
            for(var i = 0; i < searchResultModel.count; ++i) {
                if(searchResultModel.get(i).username === searchField.text) {
                    existingIndex = i
                    break
                }
            }
            console.log(successStatus)
            // 更新或添加记录
            if(existingIndex !== -1) {
                // 存在则更新状态
                searchResultModel.setProperty(existingIndex, "status",
                                           successStatus ? "found" : "notfound")
            } else {
                // 不存在则添加新记录
                searchResultModel.append({
                    username: searchField.text,
                    status: successStatus ? "found" : "notfound",
                    ApplyFriend : "未进行添加好友" ,
                    applyOrAccept : "加好友"
                })
            }
        }
    }

    //新的电话通讯接入
    Connections{
        target: server
        onAcceptNewVoiceChat: (sender , receiver) =>{
                                  if(chatVoice.visible === false)
                                  {
                                      chatVoice.visible = true
                                      chatVoice.remoteName = sender
                                      chatVoice.callState = "incoming"
                                  }
                              }
    }

    //当从客户端发送一个文件出去成功后接受到的信号
    Connections{
        target: server
        onNewFileMessage_send: (fileName  , sender, receiver , fileSize) => {
                                   console.log("新文件消息" ,fileName , sender , receiver , fileSize)
                                   chatMessages[receiver].push({
                                       sender: "me",
                                       content: fileName,
                                       fileType: "file",
                                       fileSize: fileSize,
                                       fileprogress:0,
                                       time: new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm"),
                                       isTrans: false

                                   })
                                   loadMessages(receiver)
                                   refrashUserNewMessage(receiver)


                               }
    }

    //服务器发来的文件预览
    Connections{
        target:server
        onNewFileMessage: (SenderName,ReceiverName,fileName,messageTime,filesize) =>{
                              console.log("新文件消息" ,fileName , SenderName , ReceiverName , ReceiverName , messageTime)
                              chatMessages[ReceiverName === username ? SenderName : ReceiverName].push({
                                  sender: SenderName === username ? "me" : SenderName,
                                  content: fileName,
                                  fileType: "file",
                                  fileSize: filesize,
                                  fileprogress:0,
                                  time: messageTime,
                                  isTrans: false
                              })
                              if(currentChat === (ReceiverName === username ? SenderName : ReceiverName))
                                  loadMessages(ReceiverName === username ? SenderName : ReceiverName)
                              refrashUserNewMessage(ReceiverName === username ? SenderName : ReceiverName)

                          }
    }

    Connections{
        target: server
        onNewFileMessage_send_TransProgress: (fileName ,  senderName , receiverName , progress , fileindex) => {
                //console.log(fileName ,  senderName , receiverName , progress , fileindex)
                                                     for(var i = 0; i < chatMessages[receiverName].length; i++)
                                                    {
                                                        if(chatMessages[receiverName][i].content === fileName) {
                                                            chatMessages[receiverName][i].fileprogress = progress
                                                            chatMessages[receiverName][i].isCompleted = progress >= 100
                                                            chatMessages[receiverName][i].isTrans = progress >= 100 ? false : true


                                                        }
                                                    }
                                                 if(currentChat === receiverName)
                                                 {
                                                    messageModel.setProperty(fileindex - 1, "progress", progress)
                                                    messageModel.setProperty(fileindex - 1, "isCompleted", progress >= 100)
                                                    messageModel.setProperty(fileindex - 1, "isTrans",progress >= 100 ? false : true)
                                                 }
                                             }
    }

    //下载文件传输进度
    Connections{
        target: server
        onDownloadFileTransProgress: (fileName ,  senderName , receiverName , progress , fileindex) => {
                                         console.log("文件：" , fileName , "发送者：" ,  senderName , "接受者：" , receiverName , "下载进度:" , progress , fileindex)
                                        chatMessages[receiverName === username ? senderName : receiverName][fileindex - 1].fileprogress = progress
                                        chatMessages[receiverName === username ? senderName : receiverName][fileindex - 1].isCompleted = progress >= 100
                                        chatMessages[receiverName === username ? senderName : receiverName][fileindex - 1].isTrans = progress >= 100 ? false : true


                                         if(currentChat === (receiverName === username ? senderName : receiverName))
                                         {
                                            messageModel.setProperty(fileindex - 1, "progress", progress)
                                            messageModel.setProperty(fileindex - 1, "isCompleted", progress >= 100)
                                            messageModel.setProperty(fileindex - 1, "isTrans",progress >= 100 ? false : true)
                                         }

                                     }
    }


    // 接收新文本图片消息连接
    Connections{
        target: server
        onNewTextMessage: (sender, receiver, text , time) => {
            //console.log("qml: ", sender, receiver, text)
            let targetUser = (sender === username) ? receiver : sender;

            if (!chatMessages.hasOwnProperty(targetUser)) {
                chatMessages[targetUser] = [];
            }

            if(sender === username) {
                console.log(username)
                chatMessages[receiver].push({
                    sender: "me",
                    content: text,
                    time: time,
                    fileType: "text"
                })
                refrashUserNewMessage(receiver)
                if(currentChat === receiver) {
                    loadMessages(receiver)
                    refrashUserNewMessage(receiver)

                }
            } else {
                console.log(sender)
                chatMessages[sender].push({
                    sender: sender,
                    content: text,
                    time: time,
                    fileType: "text"

                })
                refrashUserNewMessage(sender)
                if(currentChat === sender) {
                    loadMessages(sender)
                    refrashUserNewMessage(sender)

                }
            }
        }
    }

    //接受图片文件
    Connections{
        target:server
        onNewPictureMessage: (base64String) => {
                                 // 将图片插入到输入框
                                         inputField.insert(inputField.cursorPosition, `<img src="${base64String}" />`)
                             }
    }

    // 好友请求列表连接
    Connections{
        target: server
        function onSendFriendAddList(sender, receiver, status){
            console.log(sender, receiver, status)
            var existingIndex = -1
            for(var i = 0; i < searchResultModel.count; ++i) {
                if(searchResultModel.get(i).username === sender || searchResultModel.get(i).username === receiver) {
                    existingIndex = i
                    break
                }
            }
            if(existingIndex !== -1) {
                // 存在则更新状态
                searchResultModel.setProperty(existingIndex, "status","found")
                searchResultModel.setProperty(existingIndex, "applyOrAccept", (sender === username) ? "加好友" : "被加好友" )
                if(sender === username) {
                    if(status === 0)
                        searchResultModel.setProperty(existingIndex, "ApplyFriend","已申请，等待对方同意")
                    else if(status === 1)
                        searchResultModel.setProperty(existingIndex, "ApplyFriend","对方已同意")
                    else if(status === 2)
                        searchResultModel.setProperty(existingIndex, "ApplyFriend","对方拒绝")
                } else if(receiver === username) {
                    if(status === 0)
                        searchResultModel.setProperty(existingIndex, "ApplyFriend","对方申请添加你为好友")
                    else if(status === 1)
                        searchResultModel.setProperty(existingIndex, "ApplyFriend","已同意")
                    else if(status === 2)
                        searchResultModel.setProperty(existingIndex, "ApplyFriend","已拒绝")
                }
            } else {
                if(sender === username) {
                    if(status === 0)
                        searchResultModel.append({
                            username: receiver,
                            status: "found",
                            ApplyFriend: "已申请，等待对方同意",
                            applyOrAccept: "加好友"
                        })
                    else if(status === 1)
                        searchResultModel.append({
                            username: receiver,
                            status: "found",
                            ApplyFriend: "对方已同意",
                            applyOrAccept: "加好友"
                        })
                    else if(status === 2)
                        searchResultModel.append({
                            username: receiver,
                            status: "found",
                            ApplyFriend: "对方已拒绝",
                            applyOrAccept: "加好友"
                        })
                } else if(receiver === username) {
                    if(status === 0) {
                        searchResultModel.append({
                            username: sender,
                            status: "found",
                            ApplyFriend: "对方申请添加你为好友",
                            applyOrAccept: "被加好友"
                        })
                        beAddedFriendNum++
                    } else if(status === 1)
                        searchResultModel.append({
                            username: sender,
                            status: "found",
                            ApplyFriend: "已接受",
                            applyOrAccept: "被加好友"
                        })
                    else if(status === 2)
                        searchResultModel.append({
                            username: sender,
                            status: "found",
                            ApplyFriend: "已拒绝",
                            applyOrAccept: "被加好友"
                        })
                }
            }
        }
    }

    // 搜索用户函数
    function findUser(searchText) {
        var searchResult = server.searchUser(searchText)
    }

    // 添加好友函数
    function addFriend(username, applyOrAccept) {
        if(applyOrAccept === "添加")
            server.sendAddFriend(username)
        else if(applyOrAccept === "接受") {
            beAddedFriendNum--
            server.respondFriendRequest(username, 1)
        }
    }

    // 添加好友结果连接
    Connections{
        target: server
        onAddFriendRes: (successStatus) => {
            // 处理添加好友结果
        }
    }

    // 新好友通知连接
    Connections{
        target: server
        onNewFriend: (FriendName) => {
            console.log(FriendName)
            // 防止重复添加
            if (!isFriendExist(FriendName)) {
                // 添加到好友列表
                friend.append({
                    "name": FriendName,
                    "Color": chatListColor
                });
                // 添加到聊天列表
                chatList.append({
                    "name": FriendName,
                    "Color": chatListColor,
                    "ischeckble": "false",
                    "newMessage": ""
                });
            }
        }
    }

    // 数据模型
    ListModel { id: messageModel
        dynamicRoles: true
    }
    ListModel { id: chatList }
    ListModel { id: friend }

    // 检查好友是否已存在
    function isFriendExist(name) {
        for (var i = 0; i < friend.count; ++i) {
            if (friend.get(i).name === name) return true
        }
        return false
    }

    // 主界面布局
    RowLayout {
        id: rootLayout
        anchors.fill: parent
        spacing: 0

        // 左侧导航栏
        Rectangle {
            width: 60  // 增加宽度使图标更明显
            Layout.fillHeight: true
            color: sidebarColor

            // 添加渐变效果
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.darker(sidebarColor, 1.2) }
                GradientStop { position: 1.0; color: sidebarColor }
            }

            Column {
                spacing: 20  // 增加间距
                anchors.top: parent.top
                anchors.topMargin: 20  // 顶部边距
                anchors.horizontalCenter: parent.horizontalCenter

                // 用户头像
                Rectangle {
                    width: 45
                    height: 45
                    radius: width/2
                    color: primaryColor
                    anchors.horizontalCenter: parent.horizontalCenter

                    Text {
                        font.pixelSize: 22
                        text: username[0]
                        anchors.centerIn: parent
                        color: "white"
                        font.bold: true
                    }
                }

                // 聊天切换按钮
                Rectangle {
                    id: changeChat
                    width: 45
                    height: 45
                    radius: 8
                    color: "transparent"
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        id: chatImage
                        anchors.centerIn: parent
                        width: 30
                        height: 30
                        source: "qrc:/picture./checkChat.png"
                        fillMode: Image.PreserveAspectFit
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            chatImage.source = "qrc:/picture./checkChat.png"
                            friendimage.source = "qrc:/picture./unCheckUser.png"
                            toChat.visible = true
                            toFriend.visible = false
                        }
                        hoverEnabled: true
                        onEntered: {
                            changeChat.color = Qt.lighter(sidebarColor, 1.3)
                        }
                        onExited: {
                            changeChat.color = "transparent"
                        }
                    }
                }

                // 好友切换按钮
                Rectangle {
                    id: changeFriend
                    width: 45
                    height: 45
                    radius: 8
                    color: "transparent"
                    anchors.horizontalCenter: parent.horizontalCenter

                    Image {
                        id: friendimage
                        anchors.centerIn: parent
                        width: 30
                        height: 30
                        source: "qrc:/picture./unCheckUser.png"
                        fillMode: Image.PreserveAspectFit

                        Rectangle {
                            id: redDot
                            width: 18
                            height: 18
                            radius: width/2
                            color: "#FF4444"
                            visible: beAddedFriendNum > 0

                            // 添加动画效果
                            SequentialAnimation on opacity {
                                running: redDot.visible
                                loops: Animation.Infinite
                                NumberAnimation { to: 0.6; duration: 1000 }
                                NumberAnimation { to: 1.0; duration: 1000 }
                            }

                            anchors {
                                right: parent.right
                                top: parent.top
                                margins: -5
                            }

                            Text {
                                text: beAddedFriendNum
                                color: "white"
                                anchors.centerIn: parent
                                font.pixelSize: 10
                                font.bold: true
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            chatImage.source = "qrc:/picture./uncheckChat.png"
                            friendimage.source = "qrc:/picture./CheckUser.png"
                            toChat.visible = false
                            toFriend.visible = true
                        }
                        hoverEnabled: true
                        onEntered: {
                            changeFriend.color = Qt.lighter(sidebarColor, 1.3)
                        }
                        onExited: {
                            changeFriend.color = "transparent"
                        }
                    }
                }
            }
        }

        // 中间联系人列表
        Rectangle {
            id: chatFriend
            width: 220  // 略微增加宽度
            Layout.fillHeight: true
            color: "#3A3A3A"  // 更深的背景色

            // 添加边框效果
            Rectangle {
                width: 1
                height: parent.height
                anchors.right: parent.right
                color: "#555555"  // 边框颜色
            }

            ColumnLayout {
                id: toChat
                anchors.fill: parent
                spacing: 0

                // 添加标题栏
                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#2C2C2C"

                    Text {
                        text: "聊天列表"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        anchors.centerIn: parent
                    }
                }

                // 聊天会话列表
                ListView {
                    id: chatListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: chatList
                    clip: true
                    spacing: 2  // 添加间距

                    // 添加滚动条样式
                    ScrollBar.vertical: ScrollBar {
                        active: true
                        policy: ScrollBar.AsNeeded
                    }

                    delegate: Rectangle {
                        id: chatDelegate
                        width: chatListView.width
                        height: 50  // 增加高度
                        color: model.Color
                        radius: 4  // 添加圆角

                        // 添加边距
                        anchors {
                            left: parent.left
                            right: parent.right
                            leftMargin: 5
                            rightMargin: 5
                        }

                        Row {
                            spacing: 10
                            anchors {
                                fill: parent
                                leftMargin: 10
                                rightMargin: 10
                            }

                            Rectangle {
                                width: 36
                                height: 36
                                radius: 18
                                color: "#FFB6C1"
                                anchors.verticalCenter: parent.verticalCenter

                                Text {
                                    text: name[0]
                                    anchors.centerIn: parent
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: 16
                                }
                            }

                            Column {
                                anchors.verticalCenter: parent.verticalCenter
                                spacing: 2

                                Text {
                                    text: name
                                    color: "white"
                                    font.pixelSize: 14
                                }

                                Text {
                                    text: newMessage
                                    color: "#AAAAAA"
                                    font.pixelSize: 10
                                }
                            }
                        }


                        MouseArea {
                            hoverEnabled: true
                            anchors.fill: parent
                            onClicked: {
                                // 更新选中状态
                                console.log(index)
                                // 先重置所有项目的颜色和选中状态
                                for(var i=0; i<chatList.count; i++){
                                    chatList.setProperty(i, "Color", "#404040")
                                    chatList.setProperty(i, "ischeckble", "false")
                                    // 确保视图中的项目立即更新颜色
                                    var item = chatListView.itemAtIndex(i)
                                    if(item) item.color = "#404040"
                                }
                                // 然后设置当前选中项的颜色和状态
                                chatList.setProperty(index, "Color", "#808080")
                                chatList.setProperty(index, "ischeckble", "true")
                                parent.color = "#808080"

                                // 加载消息
                                currentChat = name
                                loadMessages(name)
                            }
                            onEntered: {
                                if(model.ischeckble !== "true") {
                                    parent.color = "#606060" // 悬停颜色，比选中颜色浅一些
                                }
                            }
                            onExited: {
                                parent.color = (model.ischeckble === "true") ? "#808080" : "#404040"
                            }
                        }
                    }
                }
            }



            // 添加好友界面
            ColumnLayout {
                id: toFriend
                visible: false
                anchors.fill: parent
                spacing: 5

                // 添加标题栏
                Rectangle {
                    Layout.fillWidth: true
                    height: 40
                    color: "#2C2C2C"

                    Text {
                        text: "好友添加"
                        color: "white"
                        font.pixelSize: 14
                        font.bold: true
                        anchors.centerIn: parent
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.margins: 10
                    height: 40
                    color: "#FFFFFF"
                    radius: 20  // 圆角搜索框



                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 8  // 增加内边距
                        spacing: 8  // 增加元素间距

                        // 搜索图标
                        Item {
                            width: 24
                            height: 24
                            Layout.alignment: Qt.AlignVCenter  // 垂直居中对齐

                            Text {
                                text: "🔍"
                                anchors.centerIn: parent
                                font.pixelSize: 14
                                color: "#888888"
                            }
                        }

                        TextField {
                            id: searchField
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter  // 垂直居中对齐
                            placeholderText: "输入用户ID..."
                            font.pixelSize: 12
                            selectByMouse: true
                            verticalAlignment: TextInput.AlignVCenter  // 文本垂直居中
                            leftPadding: 5  // 添加左侧内边距
                            rightPadding: 5  // 添加右侧内边距

                            background: Rectangle {
                                color: "transparent"
                            }

                            // 添加回车键搜索功能
                            Keys.onReturnPressed: {
                                searchButton.clicked()
                            }
                        }

                        Button {
                            id: searchButton
                            text: "搜索"
                            implicitWidth: 60  // 固定宽度
                            implicitHeight: 30  // 固定高度
                            Layout.alignment: Qt.AlignVCenter  // 垂直居中对齐

                            background: Rectangle {
                                radius: 15
                                color: searchButton.pressed ? "#DDDDDD" :
                                       searchButton.hovered ? "#EEEEEE" : "#F5F5F5"
                                border.color: "#DDDDDD"
                                border.width: 1

                                // 添加过渡动画
                                Behavior on color {
                                    ColorAnimation { duration: 100 }
                                }
                            }

                            contentItem: Text {
                                text: searchButton.text
                                font.pixelSize: 12
                                color: "#333333"
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }

                            // 添加鼠标悬停效果
                            hoverEnabled: true

                            onClicked: {
                                const searchText = searchField.text.trim()
                                if(searchText.length >= 1) {
                                    console.log(searchText)
                                    console.log(username)
                                    if(username !== searchText)
                                        findUser(searchText) // 传递正确的文本内容
                                }
                            }
                        }
                    }
                }






                ListView {
                    id: searchResultView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 10
                    clip: true
                    spacing: 8  // 增加间距
                    model: ListModel { id: searchResultModel }

                    // 添加滚动条样式
                    ScrollBar.vertical: ScrollBar {
                        active: true
                        policy: ScrollBar.AsNeeded
                    }

                    delegate: Rectangle {
                        width: parent.width
                        height: 70  // 增加高度以容纳更多内容
                        color: "#F5F5F5"
                        radius: 8  // 增加圆角

                        // 添加鼠标悬停效果
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onEntered: parent.color = "#EAEAEA"
                            onExited: parent.color = "#F5F5F5"
                        }


                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 12

                            // 用户头像
                            Rectangle {
                                width: 45
                                height: 45
                                radius: width/2
                                color: "#FFB6C1"
                                Layout.alignment: Qt.AlignVCenter

                                // 添加渐变效果使头像更美观
                                gradient: Gradient {
                                    GradientStop { position: 0.0; color: Qt.lighter("#FFB6C1", 1.1) }
                                    GradientStop { position: 1.0; color: Qt.darker("#FFB6C1", 1.1) }
                                }

                                Text {
                                    text: username[0]
                                    anchors.centerIn: parent
                                    color: "white"
                                    font.bold: true
                                    font.pixelSize: 18
                                }
                            }

                            // 用户信息 - 使用Layout.fillWidth确保它占据所有可用空间
                            Column {
                                spacing: 6
                                Layout.fillWidth: true
                                Layout.alignment: Qt.AlignVCenter

                                Text {
                                    text: username
                                    font.bold: true
                                    font.pixelSize: 14
                                    color: "#333333"
                                    elide: Text.ElideRight
                                    width: parent.width
                                }

                                Text {
                                    text: ApplyFriend === "未进行添加好友" ?
                                          (status === "found" ? "已注册用户" : "未找到用户") :
                                          ApplyFriend
                                    color: status === "found" ? "#07C160" : "#FF4444"
                                    font.pixelSize: 12
                                    width: parent.width
                                    elide: Text.ElideRight
                                    wrapMode: Text.Wrap
                                    maximumLineCount: 2
                                }
                            }

                            // 添加按钮 - 使用Layout确保正确定位
                            Button {
                                id: addFriendButton
                                visible: (status === "found") ?
                                        ((ApplyFriend === "未进行添加好友" || ApplyFriend === "对方申请添加你为好友") ? true : false) :
                                        false
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignRight

                                text: (applyOrAccept === "加好友") ? "添加" : "接受"

                                // 美化按钮样式
                                background: Rectangle {
                                    radius: 15
                                    color: addFriendButton.pressed ? "#059E4F" :
                                           addFriendButton.hovered ? "#06B055" : "#07C160"
                                    implicitWidth: 70
                                    implicitHeight: 30

                                    // 添加过渡动画
                                    Behavior on color {
                                        ColorAnimation { duration: 100 }
                                    }
                                }

                                contentItem: Text {
                                    text: addFriendButton.text
                                    color: "white"
                                    font.pixelSize: 12
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                // 添加鼠标悬停效果
                                hoverEnabled: true

                                onClicked: {
                                    addFriend(username, addFriendButton.text)
                                    ApplyFriend = (applyOrAccept === "加好友") ?
                                                  "已申请，等待对方同意" : "已接受"
                                    visible = false
                                }
                            }
                        }
                    }

                    // 空状态提示
                    Item {
                        anchors.centerIn: parent
                        width: emptyStateText.width + 40
                        height: emptyStateIcon.height + emptyStateText.height + 20
                        visible: searchResultModel.count === 0

                        Text {
                            id: emptyStateIcon
                            text: "🔍"
                            font.pixelSize: 32
                            color: "#CCCCCC"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }

                        Text {
                            id: emptyStateText
                            anchors.top: emptyStateIcon.bottom
                            anchors.topMargin: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: "输入用户ID开始搜索"
                            color: "#999999"
                            font.pixelSize: 14
                        }
                    }
                }


            }



        }

        // 右侧消息区域
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // 添加聊天标题栏
            Rectangle {
                Layout.fillWidth: true
                height: 50
                color: "#F5F5F5"

                // 添加底部边框
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#DDDDDD"
                    anchors.bottom: parent.bottom
                }

                Text {
                    text: currentChat ? currentChat : ""
                    font.pixelSize: 16
                    font.bold: true
                    color: "#333333"
                    anchors.centerIn: parent
                }



                Button {
                    id: voiceCallButton
                    width: 40
                    height: 40
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    visible: currentChat ? true : false

                    background: Rectangle {
                        radius: 20
                        color: voiceCallButton.pressed ? Qt.darker(primaryColor, 1.2) :
                               voiceCallButton.hovered ? primaryColor : Qt.lighter(primaryColor, 1.1)

                        // 添加过渡动画
                        Behavior on color {
                            ColorAnimation { duration: 100 }
                        }
                    }

                    contentItem: Text {
                        text: "📞"
                        font.pixelSize: 18
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        color: "white"
                    }

                    ToolTip.visible: hovered
                    ToolTip.text: "发起语音通话"
                    ToolTip.delay: 500

                    onClicked: {
                        // 打开语音通话窗口
                        if(chatVoice.visible == false)
                        {
                            chatVoice.visible = true
                            chatVoice.remoteName = currentChat
                            chatVoice.callState = "outgoing"
                            server.sendVoiceChat(username , currentChat)
                        }
                    }


                    hoverEnabled: !chatVoice.visible
                }



            }

            // 消息列表
            ListView {
                id: listView
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: messageModel
                spacing: 15  // 增加间距
                clip: true

                // 添加滚动条样式
                ScrollBar.vertical: ScrollBar {
                    active: true
                    policy: ScrollBar.AsNeeded
                }

                // 添加背景
                Rectangle {
                    anchors.fill: parent
                    color: "#EFEFEF"  // 聊天背景色
                    z: -1
                }

                delegate: Column {
                    spacing: 8
                    width: listView.width
                    anchors.margins: 10

                    // 时间戳
                    Rectangle {
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: timeText.width + 20
                        height: timeText.height + 10
                        radius: 10
                        color: "#DDDDDD"
                        //visible: fileType === "text" ? true : false
                        visible: false

                        Text {
                            id: timeText
                            text: time
                            color: "#666666"
                            font.pixelSize: 12
                            anchors.centerIn: parent
                        }
                    }

                    Row {
                        spacing: 10
                        layoutDirection: sender === "me" ? Qt.RightToLeft : Qt.LeftToRight
                        x: sender === "me" ? parent.width - width - 20 : 20  // 添加边距

                        // 头像
                        Rectangle {
                            width: 40
                            height: 40
                            radius: 20
                            color: sender === "me" ? "#07C160" : "#FFB6C1"

                            Text {
                                text: sender === "me" ? username[0] : sender[0]
                                anchors.centerIn: parent
                                color: "white"
                                font.bold: true
                                font.pixelSize: 16
                            }
                        }

                        // 添加文件预览组件
                        FilePreview {
                            id: filePreview
                            visible: fileType === "file"
                            width: Math.min(300, listView.width * 0.7)
                            height: 80
                            fileName: content
                            fileSize: model.fileSize
                            progress: model.progress || 0
                            isReceiving: sender !== "me"
                            isCompleted: model.isCompleted
                            isTranslate: model.isTrans

                            onDownloadRequested: {
                                console.log("开始下载文件:", fileName)
                                server.downLoadFile( sender === "me" ?  currentChat :  username , sender === "me" ? username : sender, content  , "./downloadFile/");

                            }
                            onOpenDownloadMenu:{
                                contextMenu.popup()
                            }

                            MouseArea{
                                anchors.fill: parent
                                propagateComposedEvents: true  // 允许事件传播
                                onDoubleClicked: {
                                    if(model.isCompleted)
                                    {
                                        server.openFilePath(content , sender === "me" ? username :sender , sender === "me" ? currentChat :username)
                                    }
                                }

                            }

                            // 右键菜单
                                Menu {
                                    id: contextMenu
                                    MenuItem {
                                        text: "打开所在文件夹"
                                        enabled: model.isCompleted
                                        onTriggered: {
                                            server.openFilePath(content , sender === "me" ? username :sender , sender === "me" ? currentChat :username)
                                        }
                                    }
                                    MenuItem {
                                        text: "另存为"
                                        //enabled: !isReceiving && model.isCompleted
                                        onTriggered: {
                                            server.selectDownLoadFilePath( sender === "me" ?  currentChat :  username , sender === "me" ? username : sender, content)
                                        }
                                    }
                                }
                        }

                        Rectangle {
                            id: textChat
                            visible: fileType === "text" ? true : false
                            width: Math.min(contentText.implicitWidth + 20, listView.width * 0.7)
                            height: contentText.implicitHeight + 20
                            color: sender === "me" ? "#95EC69" : "white"
                            radius: 8
                            clip:true
                            Text {
                                id: contentText
                                text: content
                                width: Math.min(implicitWidth, listView.width * 0.7 - 20)
                                wrapMode: Text.Wrap
                                textFormat: Text.RichText
                                padding: 10


                                // 添加鼠标事件处理
                                MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton
                                onDoubleClicked: {
                                    // 检查是否点击在图片上
                                    if (contentText.text.indexOf("<img") !== -1) {
                                        // 提取图片源
                                        var imgSrc = extractImageSource(contentText.text)
                                        if (imgSrc) {
                                        // 打开图片预览窗口
                                            console.log("这里有图片")
                                            imagePreview.imageSource = imgSrc
                                            imagePreview.visible = true
                                        }
                                    }
                                }

                                // 提取图片源的函数
                                function extractImageSource(htmlText) {
                                    var srcRegex = /<img[^>]*src=["']([^"']*)["'][^>]*>/i
                                    var match = htmlText.match(srcRegex)
                                    return match ? match[1] : null
                                }

                                }
                            }

                        }
                    }
                }
            }


            // 输入区域美化
            Rectangle {
                visible: currentChat ? true : false
                Layout.fillWidth: true
                height: 170
                color: "#F5F5F5"

                // 添加顶部边框
                Rectangle {
                    width: parent.width
                    height: 1
                    color: "#DDDDDD"
                    anchors.top: parent.top
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10

                    // 工具栏区域
                    ColumnLayout {
                        Layout.preferredWidth: 40
                        Layout.fillHeight: true
                        spacing: 15

                        // 表情按钮
                        Rectangle {
                            id: emojiButton
                            width: 24
                            height: 24
                            color: "transparent"
                            Layout.alignment: Qt.AlignTop

                            Text {
                                text: "😊"
                                font.pixelSize: 20
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                hoverEnabled: true  // 启用悬浮检测
                                onClicked: {
                                    // 表情选择功能
                                    console.log("表情按钮点击")
                                    // 显示表情选择面板，使用简单的相对位置
                                    var globalPos = emojiButton.mapToItem(null, 0, 0)
                                    emojiPopup.x = globalPos.x - 150  // 水平居中对齐
                                    emojiPopup.y = globalPos.y - 220  // 显示在按钮上方
                                    emojiPopup.open()
                                }
                            }

                        }

                        // 图片按钮
                        Rectangle {
                            width: 24
                            height: 24
                            color: "transparent"
                            Layout.alignment: Qt.AlignTop

                            Text {
                                text: "🖼️"
                                font.pixelSize: 20
                                anchors.centerIn: parent
                            }
                            ToolTip.text: "发送图片"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    // 图片选择功能
                                    server.openImageFileDialog()
                                    console.log("图片按钮点击")
                                }
                            }
                        }

                        // 文件按钮
                        Rectangle {
                            width: 24
                            height: 24
                            color: "transparent"
                            Layout.alignment: Qt.AlignTop

                            Text {
                                text: "📄"
                                font.pixelSize: 20
                                anchors.centerIn: parent
                            }
                            ToolTip.text: "发送文件"

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    // 文件选择功能
                                    console.log("文件按钮点击")
                                    server.openFileDialog( currentChat , username)

                                }
                            }
                        }
                    }

                    // 输入框和发送按钮区域
                    ColumnLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 5

                        // 输入框区域
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            color: "white"
                            radius: 6
                            border.color: inputField.focus ? "#07C160" : "#DDDDDD"
                            border.width: 1

                            ScrollView {
                                id: scrollView
                                anchors.fill: parent
                                anchors.margins: 2
                                clip: true

                                // // 美化滚动条

                                TextArea {
                                    id: inputField
                                    placeholderText: "输入消息..."
                                    wrapMode: Text.Wrap
                                    textFormat: TextEdit.RichText
                                    selectByMouse: true
                                    background: null // 移除背景，由外部Rectangle提供

                                    // 添加快捷键支持
                                    Keys.onPressed: event => {
                                        // Ctrl+V 粘贴处理
                                        if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_V) {
                                            handlePaste()
                                            event.accepted = true
                                        }
                                        // Ctrl+Enter 发送消息
                                        else if ((event.modifiers & Qt.ControlModifier) && event.key === Qt.Key_Return) {
                                            sendButton.clicked()
                                            event.accepted = true
                                        }
                                    }

                                    function handlePaste() {
                                        // 优先处理图片
                                        if(server.hasImage()) {
                                            const base64 = server.getImageBase64()
                                            if(base64) {
                                                // 插入图片时设置样式以限制宽度
                                                insert(cursorPosition, `<img src="${base64}" />`)
                                                return
                                            }
                                        }

                                        // 处理文本
                                        if(server.hasText()) {
                                            insert(cursorPosition, server.getText())
                                        }
                                    }

                                    function hasImage(text) {
                                        return text.indexOf("<img") !== -1;
                                    }
                                }
                            }
                        }

                        // 底部工具栏和发送按钮
                        RowLayout {
                            Layout.fillWidth: true
                            height: 30
                            spacing: 10

                            // 提示文本
                            Text {
                                text: "按Ctrl+Enter发送"
                                color: "#888888"
                                font.pixelSize: 12
                            }

                            Item { Layout.fillWidth: true } // 弹性空间

                            // 发送按钮
                            Button {
                                id: sendButton
                                text: "发送"

                                // 美化按钮样式
                                background: Rectangle {
                                    implicitWidth: 70
                                    implicitHeight: 32
                                    radius: 16
                                    color: sendButton.pressed ? "#059E4F" : "#07C160"
                                }

                                contentItem: Text {
                                    text: sendButton.text
                                    color: "white"
                                    font.pixelSize: 14
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }

                                onClicked: {
                                    // 发送按钮逻辑
                                    if(currentChat) { // 确保有选中的聊天
                                        if(inputField.hasImage(inputField.text)) {
                                            console.log("输入框有图片")
                                            //发送给服务器的逻辑待写
                                            server.sendLargeMessage_pictureOrText(currentChat , inputField.text)
                                            // 处理图片消息，
                                            // 处理图片消息，限制图片大小
                                            chatMessages[currentChat].push({
                                                sender: "me",
                                                content: inputField.text,
                                                time: new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm"),
                                                fileType: "text"
                                            })
                                            // 刷新显示
                                            loadMessages(currentChat)
                                            inputField.text = ""
                                            // 这里添加图片消息处理逻辑
                                            refrashUserNewMessage(currentChat)
                                        } else {
                                            console.log(inputField.text)
                                            var plainText = inputField.getText(0, inputField.length, TextEdit.PlainText);
                                            if(plainText.trim()) {
                                                // 添加到消息存储
                                                server.sendTextMessage(currentChat, inputField.text)
                                                chatMessages[currentChat].push({
                                                    sender: "me",
                                                    content: inputField.text,
                                                    time: new Date().toLocaleString(Qt.locale(), "yyyy-MM-dd hh:mm"),
                                                    fileType: "text"

                                                })
                                                // 刷新显示
                                                loadMessages(currentChat)
                                                inputField.text = ""
                                                refrashUserNewMessage(currentChat)
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }





    // 表情选择弹出框
    Popup {
        id: emojiPopup
        width: 300
        height: 200
        modal: false
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "white"
            radius: 8
            border.color: "#DDDDDD"
            border.width: 1

        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 5

            // 标题栏
            Rectangle {
                Layout.fillWidth: true
                height: 30
                color: "#F5F5F5"

                Text {
                    text: "选择表情"
                    font.pixelSize: 14
                    anchors.centerIn: parent
                    color: "#333333"
                }
            }

            // 表情网格
            GridView {
                id: emojiGrid
                Layout.fillWidth: true
                Layout.fillHeight: true
                cellWidth: 40
                cellHeight: 40
                clip: true

                model: ["😀", "😁", "😂", "🤣", "😃", "😄", "😅", "😆",
                        "😉", "😊", "😋", "😎", "😍", "😘", "😗", "😙",
                        "😚", "🙂", "🤗", "🤔", "😐", "😑", "😶", "🙄",
                        "😏", "😣", "😥", "😮", "🤐", "😯", "😪", "😫",
                        "❤️", "💕", "💞", "💓", "💗", "💖", "💘", "💝",
                        "👍", "👎", "👌", "✌️", "🤞", "🙏", "👏", "🙌"]

                delegate: Rectangle {
                    width: 38
                    height: 38
                    color: "transparent"

                    Text {
                        text: modelData
                        font.pixelSize: 24
                        anchors.centerIn: parent
                    }

                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onEntered: parent.color = "#F0F0F0"
                        onExited: parent.color = "transparent"
                        onClicked: {
                            // 将选中的表情插入到输入框
                            inputField.insert(inputField.cursorPosition, modelData)
                            emojiPopup.close()
                        }
                    }
                }

                ScrollBar.vertical: ScrollBar {
                    active: true
                    policy: ScrollBar.AsNeeded
                }
            }
        }
    }

    // // 引用图片预览组件
    ImagePreview {
        id: imagePreview
    }

    VoiceChatView {
        id: chatVoice
        onBreakChat:console.log("拒绝")
        onResponeVoiceChat:(statue)=>{
                                console.log(statue)
                                server.responeVoicdChat(chatVoice.remoteName , username , statue === true ? 1 : 2 )
                            }
    }

    Connections{
        target: server
        onIsAcceptVoiceChat:(is)=>{
                                if(is === true)
                                {
                                    chatVoice.callState = "connected";
                                    chatVoice.callDuration = 0;
                                    chatVoice.durationTimer.start();
                                    chatVoice.callSound.stop();
                                    chatVoice.connectSound.play();
                                }
                                else if(is === false)
                                {
                                    chatVoice.callState = "ended";
                                    chatVoice.durationTimer.stop();
                                    chatVoice.callSound.stop();
                                    chatVoice.endSound.play();
                                    chatVoice.endCallAnimation.start();
                                }
                            }
    }

}
