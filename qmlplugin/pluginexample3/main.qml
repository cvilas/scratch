import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import dkqmlcomponents 1.0


ApplicationWindow
{
    visible: true
    width: 640
    height: 480

    ColumnLayout
    {
        anchors.fill: parent

        ListView
        {
            id: itemsView
            anchors.fill: parent
            orientation: ListView.Horizontal
            cacheBuffer: 2000
            snapMode: ListView.SnapOneItem
            highlightRangeMode: ListView.ApplyRange

            model: MyModel
            delegate: CustomObjectView
            {
                width:100
                height: 100
                name: model.item.name
                value: model.item.value
            }
        }

        Item
        {
            anchors.fill: parent
        }

        Button
        {
            id: button
            text: "update"
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            onClicked:
            {
            }
        }
    }
}
