/*
 * Copyright (C) 2021 Beijing Jingling Information System Technology Co., Ltd. All rights reserved.
 *
 * Authors:
 * Bob <pengbo·wu@jingos.com>
 *
 */

import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import org.kde.kirigami 2.15 as Kirigami

import org.jingos.updator 1.0 as JUpdator

Kirigami.ApplicationWindow {
    id: root

    width: Screen.width
    height: Screen.height

    visible: true
    title: i18n("Updator")
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    Button {
        id: pushBtn

        width: 320
        height: 80
        anchors.centerIn: parent

        text: "Enter Update"

        onClicked: {
            var component = Qt.createComponent("update.qml")
            var window    = component.createObject(root)
            window.show()
        }
    }
}
