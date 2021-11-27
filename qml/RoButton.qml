/*
 * Copyright (C) 2021 Beijing Jingling Information System Technology Co., Ltd. All rights reserved.
 *
 * Authors:
 * Bob <pengbo·wu@jingos.com>
 *
 */

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import jingos.display 1.0

Item {
    id: jbtn

    property string btn_name: ""
    signal roBtnClicked();

    Rectangle {
        id: btn_layout

        radius: JDisplay.dp(7)
        width: 170 * appScaleSize
        height: 42 * appScaleSize
        color: Kirigami.JTheme.highlightColor

        Text {
            anchors.centerIn:parent

            text: btn_name
            font.pixelSize: 14 * appFontSize
            color: "white"
        }

        MouseArea {
            anchors.fill:parent
            hoverEnabled: true
            onEntered: {
                btn_layout.color = "#aa3C4BE8"
            }
            onExited: {
                btn_layout.color = "#FF3C4BE8"
            }
            onClicked: {
                roBtnClicked()
            }
        }
    }
}
