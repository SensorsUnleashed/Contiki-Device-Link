import QtQuick 2.0
import QtQuick.Controls 1.4

Item {
    property alias text: textinput.text;

    Column{
        anchors.fill: parent;
        spacing: 10;
        Button{
            id: clearbutton;
            height: 50;
            width: parent.width;
            text: "Clear";
            onClicked: {
                textinput.text = "";
            }
        }
        Rectangle{
            width: parent.width;
            height: parent.height - clearbutton.height - parent.spacing;
            TextArea {
                id: textinput;
                width: parent.width;
                height: parent.height;
            }
        }
    }
}
