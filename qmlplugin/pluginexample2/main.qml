import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import dkqmlcomponents 1.0


ApplicationWindow
{
    visible: true
    width: 640
    height: 480

    CustomObjectView
    {
        objectName: "myCustomObject"
        name: "unnamed"
        value: -1
        anchors.fill: parent
    }
}
