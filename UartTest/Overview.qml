import QtQuick 2.0

Item{

    function addItem(rowid, name, value, extension, valuekey){
        var widget;
        widget = Qt.createComponent("NameValueExt.qml");
        widget.createObject(rowid, {
                                "name": name,
                                "value": value,
                                "extension": extension,
                                "valuekey": valuekey,
                            });
    }

    function updateChildren(){
        var id = leftcolumn;

        for (var i = 0; i < id.children.length; ++i)
            id.children[i].updateReading();
    }

    Column {
        id: leftcolumn;
        width: parent.width / 2;
    }
    Column {
        id: rightcolumn;
        width: parent.width / 2;
    }

    Rectangle{
        id: totalsreadings;
        height: 50;
        width: parent.width;
        color: "transparent";
        border.color: "#64892E";
        border.width: 2;
        anchors.bottom: parent.bottom;

        Column {
            id: buttomcolumn;
            width: parent.width-10;
            anchors.centerIn: parent;
        }
    }

    Connections{
        target: comm;
        onGet_readings_phase_1_rdy:{
            updateChildren();
        }
    }

    Timer{
        interval: 5000;
        running: true;
        repeat: true;
        onTriggered: {
            comm.execCommand("HOST_CMD_GET_READINGS_PHASE_1");
        }
    }

    Component.onCompleted: {
        addItem(leftcolumn, "Voltage RMS", 0.0, "Volt", "PHASE_1_Voltage");
        addItem(leftcolumn, "Current", 0.0, "Amp", "PHASE_1_Current");
        addItem(leftcolumn, "Frequency", 0.0, "Hz", "PHASE_1_Frequency");

        addItem(leftcolumn, "Active Power", 0.0, "Watt", "PHASE_1_PowerActive");
        addItem(leftcolumn, "ReActive Power", 0.0, "Watt", "PHASE_1_PowerReActive");
        addItem(leftcolumn, "Apparent Power", 0.0, "Watt", "PHASE_1_PowerApparent");
        addItem(leftcolumn, "Power Factor", 0.0, "", "PHASE_1_PowerFactor");
        addItem(leftcolumn, "DC V-Offset", 0.0, "Volt", "PHASE_1_CHVoltDCOffset");
        addItem(leftcolumn, "DCI-Offset", 0.0, "Amp", "PHASE_1_CHCurrentDCOffset");

        //addItem(buttomcolumn, "Total Consumption", 300.0, "Watt");


    }
}



