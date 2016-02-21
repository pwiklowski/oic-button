import QtQuick 2.4
import QtQuick.Window 2.2
import QtQuick.Controls 1.2


Window {
    visible: true
    width: 300
    height:  500



    Rectangle{
        width: parent.width
        height: parent.height
        Button{
            id: onButton
            width: parent.width
            Text {
                text: "ON"
                anchors.centerIn: parent
                font.pixelSize: 40
            }
            height: parent.height / 2
            onClicked: app.notifyObservers("/switch", 1)
        }
        Button{
            Text {
                text: "OFF"
                anchors.centerIn: parent
                font.pixelSize: 40
            }
            anchors.top: onButton.bottom
            width: parent.width
            height: parent.height / 2
            onClicked: app.notifyObservers("/switch", 0)
        }
    }


}
