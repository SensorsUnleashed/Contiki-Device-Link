import QtQuick 2.0
import QtQuick.Controls 2.0

Row{
    width: 300;
    height: 50;

    property alias name: name.text;
    property alias to: msgsize.to;
    property alias from: msgsize.from;
    property alias value: msgsize.value;
    property alias stepSize: msgsize.stepSize;
    property alias resultval: valbox.text;

    Label{
        id: name;
        width: parent.width * 0.4;
        height: parent.height;
        verticalAlignment: Text.AlignVCenter;
        text: "";
    }

    Slider {
        id: msgsize;
        width: parent.width * 0.5;
        height: parent.height;
        onValueChanged: {
            value = Math.round(value);
        }
    }
    Label{
        id: valbox;
        width: parent.width * 0.10;
        height: parent.height;
        verticalAlignment: Text.AlignVCenter;
        text: msgsize.value;
    }
}
