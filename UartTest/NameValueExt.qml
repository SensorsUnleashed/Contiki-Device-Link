import QtQuick 2.0
import QtQuick.Controls 1.2

Row{
    width: 200;
    height: 30;
    property string name: "NA";
    property real value: 0.0;
    property string extension: "NA";
    property string valuekey;

    function updateReading(){
        valuelabel.text = comm.get_readings(valuekey);
    }

    Item{
        //id: name;
        width: parent.width * 0.5;
        height: parent.height;
        Label{
            anchors.fill: parent;
            text: name;
            color: "#AFDB71";
        }
    }
    Item{
        //id: value;
        width: parent.width * 0.4;
        height: parent.height;
        Label{
            id: valuelabel;
            anchors.fill: parent;
            //text: value;
            text: comm.get_readings(valuekey);
            color: "#AFDB71";
        }
    }
    Item{
        //id: extension
        width: parent.width * 0.1;
        height: parent.height;
        Label{
            anchors.fill: parent;
            text: extension;
            color: "#AFDB71";
        }
    }


}

