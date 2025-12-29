// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtGraphs

GraphsView {
    id: graphView
    anchors.fill: parent

    theme: GraphsTheme {
        id: graphsTheme
        theme: GraphsTheme.Theme.BlueSeries
        labelBorderVisible: true
        labelBackgroundVisible: true
        backgroundColor: "white"
    }
    
    // X Axis configuration - sliding window
    axisX: ValueAxis {
        id: axisX
        min: 0
        max: 6280  // Window size: 6280ms = ~6.3 seconds
        tickInterval: 1000.0
        labelFormat: "%.0f"
        titleText: "Time (ms)"
        titleVisible: true
    }

    // Y Axis configuration
    axisY: ValueAxis {
        id: axisY
        min: -10
        max: 10
        tickInterval: 5
        labelFormat: "%.1f"
        titleText: "sin(x)"
        titleVisible:  true
    }


    // Use the series from C++
    Component.onCompleted: {
        if (typeof lineSeries !== 'undefined') {
            lineSeries.axisX = axisX
            lineSeries.axisY = axisY
            graphView.addSeries(lineSeries)
        }
    }

    // Handle lineSeriesChanged signal from the LineGraph object
    Connections {
        target: typeof lineGraph !== 'undefined' ? lineGraph : null
        function onLineSeriesChanged() {
            // Update sliding window - show last 6280ms of data
            if (lineSeries && lineSeries.count > 0) {
                var lastPoint = lineSeries.at(lineSeries.count - 1)
                var currentTime = lastPoint.x
                
                // Shift window when we exceed the max
                if (currentTime > axisX.max) {
                    var windowSize = 6280
                    axisX.min = currentTime - windowSize
                    axisX.max = currentTime
                }
            }
        }
    }
}


