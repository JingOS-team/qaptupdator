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
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.12 as Kirigami

Item {
    id: jbtn

    property string btn_name: ""
    signal roBtnClicked();

    Rectangle {
        id: btn_layout
        width: 170 
        height: 42 
        color: "#FF3C4BE8"
        radius: 7
        

        Text {
            anchors.centerIn:parent
            text: btn_name 
            // font.pointSize: appFontSize + 2
            font.pixelSize: 14
            color:"white"
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
            onClicked : {
                roBtnClicked()
            }
           
        }
    }
}
