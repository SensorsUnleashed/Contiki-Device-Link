import QtQuick 2.0
import QtQuick.Controls 1.4

Item{

    Rectangle{
        id: emu_actuatordevice;
        width: 100;
        height: 100;
        color: "black";
    }

    Rectangle{
        id: emu_sensordevice;
        width: 100;
        height: 100;
        anchors.left: emu_actuatordevice.right;
        color: "yellow";

        property string url: "Button/push";

        Label{
            anchors.centerIn: parent;
            text: parent.url;
            color: "black";
        }

        MouseArea{
            anchors.fill: parent;
            onClicked: {
                guiglue.updateValue(0);
            }
        }

    }
}
