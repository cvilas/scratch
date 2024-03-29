import QtQuick 2.12

Image {
    id: root
    source: "images/background.png"

    MouseArea {
        anchors.fill: parent
        onClicked: wheel.rotation += 90
    }
    
    Image {
        id: pole
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        source: "images/pole.png"
    }

    Image {
        id: wheel
        anchors.centerIn: parent
        source: "images/pinwheel.png"
        
        Behavior on rotation {
            NumberAnimation {
                duration: 250
            }
        }
    }
}
