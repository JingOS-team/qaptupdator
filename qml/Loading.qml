/*
 * Copyright (C) 2021 Beijing Jingling Information System Technology Co., Ltd. All rights reserved.
 *
 * Authors:
 * Bob <pengbo·wu@jingos.com>
 *
 */

import QtQuick 2.9
import QtGraphicalEffects 1.0

Rectangle {
    id:rect

    width: 16 * appScaleSize
    height: 16 * appScaleSize
    color:"transparent"

    Image {
        anchors.centerIn: parent
        width: parent.width
        height:parent.height

        source: "qrc:/image/loading.png"
        RotationAnimation on rotation {
            from: 0
            to: 360
            duration: 1500
            loops: Animation.Infinite
        }
    }
}