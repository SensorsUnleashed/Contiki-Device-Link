import QtQuick 2.7
import QtQuick.Controls 2.0

GroupBox{
    function updateScreen(list){
        //The range need to be changed first
        var i;
        for(i=0; i<list.length; i++){
            if(list[i]["id"] === "RangeMin"){
                changeventslider.from = list[i]["value"];
                beloweventslider.from = list[i]["value"];
                aboveeventslider.from = list[i]["value"];
            }
            else if(list[i]["id"] === "RangeMax"){
                changeventslider.to = list[i]["value"];
                beloweventslider.to = list[i]["value"];
                aboveeventslider.to = list[i]["value"];
            }
        }

        for(i=0; i<list.length; i++){
            if(list[i]["id"] === "AboveEventAt"){
                aboveeventslider.value = list[i]["value"];
                above = list[i]["value"];
            }
            else if(list[i]["id"] === "BelowEventAt"){
                beloweventslider.value = list[i]["value"];
                below = list[i]["value"];
            }
            else if(list[i]["id"] === "ChangeEvent"){
                changeventslider.value = list[i]["value"];
                change = list[i]["value"];
            }
            else if(list[i]["id"] === "eventsActive"){
                /*
                enum eventstate{
                    NoEventActive =     (1 << 0),
                    AboveEventActive =  (1 << 1),
                    BelowEventActive =  (1 << 2),
                    ChangeEventActive = (1 << 3),
                };
                */

                eventsActive = list[i]["value"];
                aboveeventchkbox.checked =  ((1 << 1) & eventsActive) > 0;
                beloweventchkbox.checked =  ((1 << 2) & eventsActive) > 0;
                changeeventchkbox.checked = ((1 << 3) & eventsActive) > 0;
            }
        }
    }

    property real above;
    property real below;
    property real change;
    property int eventsActive;

    title: qsTr("Event setup:");
    font.pointSize: 10;

    Column{
        Row{
            spacing: 20;
            SettingSlider{
                id: changeventslider;
                name: qsTr("Change event at:");
                stepSize: 1;
                onToChanged: value = change;
                onFromChanged: value = change;
                onValueChanged: submitbutton.enabled = true;
            }
            CheckBox {
                id: changeeventchkbox;
                text: qsTr("Enable")

                onCheckedChanged: {
                    if(checked){
                        eventsActive = eventsActive | (1 << 3);
                    }
                    else{
                        eventsActive = eventsActive & ~(1 << 3);
                    }
                    submitbutton.enabled = true;
                }
            }
        }
        Row{
            spacing: 20;
            SettingSlider{
                id: beloweventslider;
                name: qsTr("Below event at:");
                stepSize: 1;
                onToChanged: value = below;
                onFromChanged: value = below;
                onValueChanged: submitbutton.enabled = true;
            }
            CheckBox {
                id: beloweventchkbox;
                text: qsTr("Enable")

                onCheckedChanged: {
                    if(checked){
                        eventsActive = eventsActive | (1 << 2);
                    }
                    else{
                        eventsActive = eventsActive & ~(1 << 2);
                    }
                    submitbutton.enabled = true;
                }
            }
        }
        Row{
            spacing: 20;
            SettingSlider{
                id: aboveeventslider;
                name: qsTr("Above event at:");
                stepSize: 1;

                onToChanged: value = above;
                onFromChanged: value = above;
                onValueChanged: submitbutton.enabled = true;
            }
            CheckBox {
                id: aboveeventchkbox;
                text: qsTr("Enable")

                onCheckedChanged: {
                    if(checked){
                        eventsActive = eventsActive | (1 << 1);
                    }
                    else{
                        eventsActive = eventsActive & ~(1 << 1);
                    }
                    submitbutton.enabled = true;
                }
            }
        }
        Row{
            spacing: 20;
            SUButton{
                text: qsTr("Read config");
                width: 150;
                onClicked: {
                    activeSensor.req_eventSetup();
                }
            }
            SUButton{
                id: submitbutton;
                text: qsTr("Submit");
                width: 100;
                onClicked: {
                    var updatevalues = {
                        AboveEventAt: aboveeventslider.value,
                        BelowEventAt: beloweventslider.value,
                        ChangeEvent: changeventslider.value,
                        eventsActive: eventsActive,
                    }

                    activeSensor.updateConfig(updatevalues);
                }
            }
        }
    }
    Connections{
        target: activeSensor;
        onRangeMaxValueReceived:{
            //maxrange = result["value"];
            changeventslider.to = result["value"];
            beloweventslider.to = result["value"];
            aboveeventslider.to = result["value"];
        }
    }
    Connections{
        target: activeSensor;
        onRangeMinValueReceived:{
            //minrange = result["value"];
            changeventslider.from = result["value"];
            beloweventslider.from = result["value"];
            aboveeventslider.from = result["value"];
        }
    }
    Connections{
        target: activeSensor;
        onAboveEventValueChanged:{
            above = result["value"];
            aboveeventslider.value = result["value"];
        }
    }
    Connections{
        target: activeSensor;
        onBelowEventValueChanged:{
            below = result["value"];
            beloweventslider.value = result["value"];
        }
    }
    Connections{
        target: activeSensor;
        onChangeEventValueChanged:{
            change = result["value"];
            changeventslider.value = result["value"];
        }
    }

    Component.onCompleted: {
        updateScreen(activeSensor.getConfigValues());
        submitbutton.enabled = false;
    }

    Connections{
        target: activeSensor;
        onEventSetupRdy: {
            updateScreen(activeSensor.getConfigValues());
            submitbutton.enabled = false;
        }
    }
}
