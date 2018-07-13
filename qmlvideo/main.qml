import QtQuick 2.9
import QtQuick.Window 2.3
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtMultimedia 5.9

ApplicationWindow
{
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")

    Camera
    {
        id: camera
    }

    VideoOutput
    {
        source: camera
        anchors.fill: parent
        focus : visible // to receive focus and capture key events when visible
    }
}
