import QtQuick 2.0
import QtQuick.Layouts 1.3

RowLayout{
    spacing: 10;
    SUButton{
        text: qsTr("Toggle");
        width: 150;
        onClicked: {
            activeSensor.toggleRelay();
        }
    }
    SUButton{
        text: qsTr("Set ON");
        width: 150;
        onClicked: {
            activeSensor.setOn();
        }
    }
    SUButton{
        text: qsTr("Set OFF");
        width: 150;
        onClicked: {
            activeSensor.setOff();
        }
    }
}
