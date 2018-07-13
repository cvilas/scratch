import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import dkqmlcomponents 1.0

Item
{
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

            model: wrapper.objects
            delegate: CustomObjectView
            {
                width:100
                height: 100
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
