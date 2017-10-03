import QtQuick 2.0
import QtQuick.Layouts 1.3

RowLayout{
    spacing: 10;
    SUButton{
        text: qsTr("Toggle RED Led");
        width: 175;
        onClicked: {
            activeSensor.toggleRedLED();
        }
    }
    SUButton{
        text: qsTr("Toggle Green Led");
        width: 175;
        onClicked: {
            activeSensor.toggleGreenLED();
        }
    }
    SUButton{
        text: qsTr("Toggle Orange Led");
        width: 175;
        onClicked: {
            activeSensor.toggleOrangeLED();
        }
    }
    SUButton{
        text: qsTr("Toggle Yellow Led");
        width: 175;
        onClicked: {
            activeSensor.toggleYellowLED();
        }
    }
}
