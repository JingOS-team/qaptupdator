/***************************************************************************
 *   Copyright © 2021 Wang Rui <wangrui@jingos.com>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15
import org.jingos.updator 1.0
import org.kde.kirigami 2.15 as Kirigami

Kirigami.ApplicationWindow {
    id: update_root

    property int screenWidth: 888
    property int screenHeight: 648
    property int appFontSize: theme.defaultFont.pointSize

    property int statusbar_height : 22
    property int statusbar_icon_size: 22
    property int default_setting_item_height: 45

    property int marginTitle2Top : 44 
    property int marginItem2Title : 36
    property int marginLeftAndRight : 20 
    property int marginItem2Top : 24
    property int radiusCommon: 10 
    property int fontNormal: 14 

    // property int screenWidth: Screen.width
    // property int screenHeight: Screen.height
    property int upgradeStatus: 0 
    property bool upgradeFinish: false 
    property int upgradeResult: 0 //0 : nothing upgrade // 1: success //2: error
    property string progressTxt : i18n("Prepare all packages...")


    width: screenWidth
    height: screenHeight

    UpgradeModel{
        id: upgrader

        onUpgradeStatusChanged:{
            upgradeStatus = status
        }

        onUpgradeFinished: {
            console.log("update.qml******onUpgradeFinished****** 结果:",result)
            upgradeFinish = true 
            upgradeResult = result 
            if(upgradeResult == 1){
                progressTxt = i18n("Update completed \n The system has been updated to %1", currentVersion)
                upgrader.resetNewVersion()
            }   
            else if(upgradeResult == 2){
                progressTxt = i18n("Update failed \n The system update to %1 error" , currentVersion)
            }
        }

        onProgressUpdated:{
            progress_bar.value = value 
            progressTxt = i18n("Updating %1 current progress is %2%" , currentVersion , value)
            // progressTxt = i18n("Updating %1 current progress is %2" , "111" , "222")
        }
    }

    Component.onCompleted:{
       checkUpdateTimer.start();
    }

    Timer {
        id: checkUpdateTimer
        repeat:false 
        interval: 1000
        onTriggered: {
            upgrader.updateCache()
        }
    }


    Rectangle {
        anchors.fill: parent
        color:"#FFF6F9FF"
        
        Text {
            id: update_title

            anchors {
                top: parent.top
                left: parent.left
                topMargin: 20
                leftMargin: marginLeftAndRight
            }

            text: i18n("Update")
            // font.pointSize: appFontSize + 12
            font.pixelSize: 20
        }

        Image {
            id: update_logo

            anchors {
                top: update_title.bottom
                topMargin: 68 
                horizontalCenter: parent.horizontalCenter
            }
            width: 69 
            height: 76 
            sourceSize.width: 69
            sourceSize.height: 76
            source: "qrc:/image/jingos_logo_update.png"
        }

        Rectangle {
            id: progress_layout

            anchors {
                top: update_logo.bottom
                topMargin: 36
                horizontalCenter: parent.horizontalCenter
            }
            width: 373
            height: 4+12+17
            color:"transparent"
            // color:"red"
            
            Text {
                id: progress_txt

                text: progressTxt
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                horizontalAlignment: Text.AlignHCenter
                // font.pointSize: appFontSize + 2
                font.pixelSize: 14
            }

            Loading {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -8
                anchors.horizontalCenter: parent.horizontalCenter
                visible: upgradeStatus == 0
            }

            ProgressBar {
                id:progress_bar

                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.horizontalCenter: parent.horizontalCenter
                width: 350 
                height: 4
                visible: !upgradeFinish && upgradeStatus == 1
                value: 0 
                from: 0
                to: 100
            }

        }

        Rectangle {
            id: scrollview_layout

            anchors {
                top: progress_layout.bottom
                horizontalCenter: parent.horizontalCenter
                topMargin: 32
            }

            width: 555 
            height: 214 
            visible: upgradeStatus != 0

            ListView {
                id:listview

                anchors.fill: parent
                model:RecordingModel
                clip:true
                orientation:  ListView.Vertical
                cacheBuffer:  20 * height
                currentIndex: listview.count > 1 ? (listview.count - 1) : 0

                delegate: Rectangle{
                    id:delegate_list

                    width: scrollview_layout.width
                    height: 17

                    signal signalShowMenu(var id,int x,int y)
                    
                    MouseArea{
                        id:mouse_delegate

                        anchors.fill: parent
                        enabled:true
                        hoverEnabled: true
                        propagateComposedEvents: true
                        acceptedButtons: Qt.RightButton|Qt.LeftButton

                        onClicked: {
                            mouse.accepted = false;
                        }
                        onDoubleClicked: {
                            mouse.accepted = false;
                        }
                    }

                    Component.onCompleted:{
                        console.log("item name start");
                        console.log("item name:"+recording.fileName);
                    }

                    RowLayout{
                        
                        anchors{
                            left: parent.left
                            leftMargin: 6
                            right: parent.right
                            rightMargin: 6
                        }
                        anchors.verticalCenter: parent.verticalCenter

                        spacing: 7

                        Label{
                            id:left_list_user

                            Layout.alignment: Qt.AlignLeft
                            Layout.leftMargin: 6 
                            text:recording.fileName
                            color:"#333333"
                            font.pixelSize: 14 
                        }
                    }
                }
            }
        }

        Rectangle {
            id: result_layout

            anchors {
                top: scrollview_layout.bottom
                topMargin: 30 
                horizontalCenter:parent.horizontalCenter
            }
            width: (170 * 2 + 31) 
            height: 42 
            color:"transparent"
            visible: upgradeFinish

            Rectangle {
                anchors.fill:parent
                color:"transparent"
                visible: upgradeResult == 1

                RoButton {
                    id:reboot_btn

                    // anchors.verticalCenter: parent.verticalCenter
                    anchors.top :parent.top 
                    anchors.left:parent.left
                    width: 170 
                    btn_name: i18n("Restart now")
                    onRoBtnClicked: {
                        upgrader.restartDevice();
                    }
                }

                RoButton {
                    // anchors.verticalCenter: parent.verticalCenter
                    anchors.top :parent.top 
                    anchors.left: reboot_btn.right

                    anchors.leftMargin: 31
                    btn_name: i18n("Restart later")
                    onRoBtnClicked: {
                        Qt.quit();
                    }
                }
            }

            RoButton {
                anchors.centerIn: parent
                visible: upgradeResult == 2
                btn_name : i18n("I know")
                onRoBtnClicked: {
                    Qt.quit();
                }
            }
        }
    }
}
