/****************************************************************************
**
** Copyright (C) 2014 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.io
**
** This file is part of the Qt Data Visualization module.
**
** Licensees holding valid commercial license for Qt may use this file in
** accordance with the Qt License Agreement provided with the Software
** or, alternatively, in accordance with the terms contained in a written
** agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.io
**
****************************************************************************/

import QtQuick 2.1
import QtQuick.Controls 1.0
import QtQuick.Layouts 1.0
import QtDataVisualization 1.0
import "."

Rectangle {
    id: mainView
    width: 800
    height: 600

    property int buttonLayoutHeight: 180;

    Data {
        id: graphData
    }

    Theme3D {
        id: firstTheme
        type: Theme3D.ThemeQt
    }

    Theme3D {
        id: secondTheme
        type: Theme3D.ThemeEbony
    }

    Item {
        id: dataView
        anchors.fill: parent

        Bars3D {
            id: barGraph
            anchors.fill: parent
            selectionMode: AbstractGraph3D.SelectionItemAndRow
            scene.activeCamera.cameraPreset: Camera3D.CameraPresetIsometricLeftHigh
            theme: firstTheme
            valueAxis.labelFormat: "%d\u00B0C"

            Bar3DSeries {
                id: station1
                name: "Station 1"
                itemLabelFormat: "Temperature at @seriesName for @colLabel, @rowLabel: @valueLabel"

                ItemModelBarDataProxy {
                    itemModel: graphData.model
                    rowRole: "year"
                    columnRole: "month"
                    valueRole: "s1"
                }
            }
            Bar3DSeries {
                id: station2
                name: "Station 2"
                itemLabelFormat: "Temperature at @seriesName for @colLabel, @rowLabel: @valueLabel"

                ItemModelBarDataProxy {
                    itemModel: graphData.model
                    rowRole: "year"
                    columnRole: "month"
                    valueRole: "s2"
                }
            }
            Bar3DSeries {
                id: station3
                name: "Station 3"
                itemLabelFormat: "Temperature at @seriesName for @colLabel, @rowLabel: @valueLabel"

                ItemModelBarDataProxy {
                    itemModel: graphData.model
                    rowRole: "year"
                    columnRole: "month"
                    valueRole: "s2"
                }
            }
        }
    }

    Rectangle {
        property int legendLocation: 3
        // Make the height and width fractional of main view height and width.
        // Reverse the relation if screen is in portrait - this makes legend look the same
        // if the orientation is rotated.
        property int fractionalHeight: mainView.width > mainView.height ? mainView.height / 5 : mainView.width / 5
        property int fractionalWidth: mainView.width > mainView.height ? mainView.width / 5 : mainView.height / 5

        id: legendPanel
        width: fractionalWidth > 150 ? fractionalWidth : 150
        // Adjust legendpanel height to avoid gaps between layouted items.
        height: fractionalHeight > 99 ? fractionalHeight - fractionalHeight % 3 : 99
        border.color: barGraph.theme.labelTextColor
        border.width: 3
        color: "#00000000" // Transparent

        //! [0]
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: parent.border.width
            spacing: 0
            clip: true
            LegendItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                series: station1
                theme: barGraph.theme
            }
            LegendItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                series: station2
                theme: barGraph.theme
            }
            LegendItem {
                Layout.fillWidth: true
                Layout.fillHeight: true
                series: station3
                theme: barGraph.theme
            }
        }
        //! [0]

        states: [
            State  {
                name: "topleft"
                when: legendPanel.legendLocation === 1
                AnchorChanges {
                    target: legendPanel
                    anchors.top: buttonLayout.bottom
                    anchors.bottom: undefined
                    anchors.left: dataView.left
                    anchors.right: undefined
                }
            },
            State  {
                name: "topright"
                when: legendPanel.legendLocation === 2
                AnchorChanges {
                    target: legendPanel
                    anchors.top: buttonLayout.bottom
                    anchors.bottom: undefined
                    anchors.left: undefined
                    anchors.right: dataView.right
                }
            },
            State  {
                name: "bottomleft"
                when: legendPanel.legendLocation === 3
                AnchorChanges {
                    target: legendPanel
                    anchors.top: undefined
                    anchors.bottom: dataView.bottom
                    anchors.left: dataView.left
                    anchors.right: undefined
                }
            },
            State  {
                name: "bottomright"
                when: legendPanel.legendLocation === 4
                AnchorChanges {
                    target: legendPanel
                    anchors.top: undefined
                    anchors.bottom: dataView.bottom
                    anchors.left: undefined
                    anchors.right: dataView.right
                }
            }
        ]
    }

    RowLayout {
        id: buttonLayout
        Layout.minimumHeight: themeToggle.height
        width: parent.width
        anchors.left: parent.left
        spacing: 0

        NewButton {
            id: themeToggle
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Change Theme"
            onClicked: {
                if (barGraph.theme === firstTheme) {
                    barGraph.theme = secondTheme
                } else {
                    barGraph.theme = firstTheme
                }
            }
        }
        NewButton {
            id: repositionLegend
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Reposition Legend"
            onClicked: {
                if (legendPanel.legendLocation === 4) {
                    legendPanel.legendLocation = 1
                } else {
                    legendPanel.legendLocation++
                }
            }
        }
        NewButton {
            id: exitButton
            Layout.fillHeight: true
            Layout.fillWidth: true
            text: "Quit"
            onClicked: Qt.quit(0);
        }
    }

}
