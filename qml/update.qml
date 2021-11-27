/*
 * Copyright (C) 2021 Beijing Jingling Information System Technology Co., Ltd. All rights reserved.
 *
 * Authors:
 * Bob <pengbo·wu@jingos.com>
 *
 */

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.15
import QtQml.Models 2.15
import org.jingos.updator 1.0
import org.kde.kirigami 2.15 as Kirigami
import jingos.display 1.0
import org.kde.plasma.networkmanagement 0.2 as PlasmaNM

Kirigami.ApplicationWindow {
    id: update_root

    property int screenWidth: 888
    property int screenHeight: 648
    property int statusbar_height : 22 * appScaleSize
    property int statusbar_icon_size: 22 * appScaleSize
    property int default_setting_item_height: 45 * appScaleSize
    property int marginTitle2Top : 44 * appScaleSize
    property int marginItem2Title : 36 * appScaleSize
    property int marginLeftAndRight : 20 * appScaleSize
    property int marginItem2Top : 24 * appScaleSize
    property int radiusCommon: 10
    property int fontNormal: 14 * appFontSize
    property int upgradeStatus: 0
    property int upgradeResult: 0
    property bool upgradeFinish: false
    property string progressTxt : i18n("Prepare all packages...")
    property var appScaleSize: JDisplay.dp(1.0)
    property var appFontSize: JDisplay.sp(1.0)

    width: screenWidth
    height: screenHeight

    PlasmaNM.NetworkStatus {
        id: networkStatusNM
    }

    Connections {
        target: RecordingModel
        onActivateRequested: {
            update_root.hide()
            update_root.show()
        }
    }

    UpgradeModel {
        id: upgrader

        onUpgradeStatusChanged: {
            upgradeStatus = status
        }

        onUpgradeFinished: {
            upgradeFinish = true
            upgradeResult = result
            if (upgradeResult == 1) {
                progressTxt = i18n("Update completed \n The system has been updated to %1", currentVersion)
                upgrader.resetNewVersion()
            } else if (upgradeResult == 2) {
                progressTxt = i18n("Update failed \n The system update to %1 error" , currentVersion)
            }
        }

        onProgressUpdated: {
            progress_bar.value = value
            progressTxt = i18n("Updating %1 current progress is %2%" , currentVersion , value)
        }
    }

    function stopInit() {
        progressTxt = i18n("Update failed \n The system update to %1 error" , currentVersion)
        upgradeFinish = true
        upgradeStatus = -1
        upgradeResult = 2
    }

    function startInit() {
        upgrader.init();
        checkUpdateTimer.start();
    }

    Component.onCompleted: {
        if(networkStatusNM.networkStatus === "Connected") {
            upgrader.init();
            checkUpdateTimer.start();
        } else {
           stopInit()
        }
    }

    Timer {
        id: checkUpdateTimer

        repeat: false
        interval: 1000
        onTriggered: {
            upgrader.setCurrentVersion(currentVersion)
            upgrader.updateCache()
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Kirigami.JTheme.settingMinorBackground

        Text {
            id: update_title

            anchors {
                top: parent.top
                left: parent.left
                topMargin: 30 * appScaleSize
                leftMargin: marginLeftAndRight
            }

            text: i18n("Software Update")
            font.pixelSize: 20 * appFontSize
            color: Kirigami.JTheme.majorForeground
        }

        Kirigami.Icon {
            id: update_logo

            anchors {
                top: update_title.bottom
                topMargin: 68 * appScaleSize
                horizontalCenter: parent.horizontalCenter
            }
            width: 69 * appScaleSize
            height: 76 * appScaleSize

            source: "qrc:/image/jingos_logo_update.png"
            color: Kirigami.JTheme.majorForeground
        }

        Rectangle {
            id: progress_layout

            anchors {
                top: update_logo.bottom
                topMargin: JDisplay.dp(15)
                horizontalCenter: parent.horizontalCenter
            }
            width: 373 * appScaleSize
            height: (progress_bar.visible ? 26 : 15) * appScaleSize +  loadingItem.height + progress_bar.height + check_tip.contentHeight + progress_txt.contentHeight

            color: "transparent"

            Text {
                id: check_tip

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    top: progress_layout.top
                }
                width: parent.width
                wrapMode: Text.WordWrap
                maximumLineCount: 2

                text: i18n("Before updating, please make sure your device is charged. During the updating process, please connect to the Internet throughout.")
                font.pixelSize: 12 * appFontSize
                horizontalAlignment: lineCount > 1 ? Text.AlignLeft : Text.AlignHCenter
                color: Kirigami.JTheme.majorForeground//"black"
            }

            Text {
                id: progress_txt

                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: check_tip.bottom
                anchors.topMargin: 15 * appScaleSize
                horizontalAlignment: Text.AlignHCenter

                text: progressTxt
                font.pixelSize: 14 * appFontSize
                color: Kirigami.JTheme.majorForeground
            }

            Loading {
                id: loadingItem
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -8 * appScaleSize
                anchors.horizontalCenter: parent.horizontalCenter
                height: visible ? 16 * appScaleSize : 0

                visible: upgradeStatus == 0
            }

            ProgressBar {
                id:progress_bar

                anchors.bottom: parent.bottom
                anchors.bottomMargin: 0
                anchors.horizontalCenter: parent.horizontalCenter
                width: 350 * appScaleSize
                height: visible ? 4 * appScaleSize : 0

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
                topMargin: 23 * appScaleSize
            }
            width: 555 * appScaleSize
            height: 214 * appScaleSize + updataLog.height

            visible: upgradeStatus != 0
            color: "transparent"

            Label {
                id: updataLog

                anchors.left: listview.left
                anchors.leftMargin: 6 * appScaleSize
                text: "  " + i18n("Update log:")
                color: Kirigami.JTheme.minorForeground
                font.pixelSize: 14 * appFontSize
                visible: listview.count !== 0
            }

            ListView {
                id: listview

                width: parent.width
                height: parent.height - updataLog.height
                anchors.top: updataLog.bottom
                model: RecordingModel
                clip: true
                orientation: ListView.Vertical
                highlightMoveDuration: 1
                currentIndex: listview.count > 1 ? (listview.count - 1) : 0

                delegate: Rectangle {
                    id: delegate_list

                    width: scrollview_layout.width
                    height: 17 * appScaleSize
                    color: "transparent"

                    signal signalShowMenu(var id, int x, int y)

                    RowLayout {
                        anchors {
                            left: parent.left
                            leftMargin: 6 * appScaleSize
                            right: parent.right
                            rightMargin: 6 * appScaleSize
                        }
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: 7 * appScaleSize

                        Label{
                            id: left_list_user

                            Layout.alignment: Qt.AlignLeft
                            Layout.leftMargin: 6 * appScaleSize
                            text: i18n("· ") + recording.fileName
                            color: Kirigami.JTheme.minorForeground
                            font.pixelSize: 14 * appFontSize
                        }
                    }
                }
            }
        }

        Rectangle {
            id: result_layout

            anchors {
                bottom: parent.bottom
                bottomMargin: 20 * appScaleSize
                horizontalCenter: parent.horizontalCenter
            }
            width: (170 * 2 + 31) * appScaleSize
            height: 42 * appScaleSize

            color: "transparent"
            visible: upgradeFinish

            Rectangle {
                anchors.fill: parent
                color: "transparent"
                visible: upgradeResult == 1

                RoButton {
                    id: reboot_btn

                    anchors.top : parent.top
                    anchors.left: parent.left
                    width: 170 * appScaleSize
                    btn_name: i18n("Restart now")
                    onRoBtnClicked: {
                        upgrader.restartDevice();
                    }
                }

                RoButton {
                    anchors.top : parent.top
                    anchors.left: reboot_btn.right
                    anchors.leftMargin: 31 * appScaleSize

                    btn_name: i18n("Restart later")
                    onRoBtnClicked: {
                        Qt.quit();
                    }
                }
            }

            RoButton {
                anchors.centerIn: parent
                width: 170 * appScaleSize
                height: 42 * appScaleSize

                visible: upgradeResult == 2
                btn_name : i18n("I know")
                onRoBtnClicked: {
                    Qt.quit();
                }
            }
        }
    }
}
