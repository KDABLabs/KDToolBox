/*
  This file is part of KDToolBox.

  SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT
*/

import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
    width: 400
    height: 400

    TextField {
        id: searchField
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        placeholderText: "Find a country..."
        focus: true
        // one would normally use a Binding element here, but fixed string
        // isn't a property of QSortFilterProxymodel...
        onTextChanged: _filter.setFilterFixedString(text)
    }

    ListView {
        anchors {
            top: searchField.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        model: _model
        clip: true

        delegate: Rectangle {
            height: childrenRect.height
            width: parent.width

            Image {
                id: image
                source: model.flag
                width: 64
                height: 64
                fillMode: Image.PreserveAspectFit

                anchors.left: parent.left
            }

            Text {
                text: model.name + "\n" + "Population: " + model.population.toFixed(3) + " millions"
                anchors {
                    left: image.right
                    verticalCenter: image.verticalCenter
                    leftMargin: 5
                }
            }
        }
    }
}
