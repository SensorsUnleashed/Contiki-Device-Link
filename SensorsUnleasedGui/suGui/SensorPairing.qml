import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

ColumnLayout{
    Layout.fillWidth: true;
    Layout.fillHeight: true;

    SUButton{
        id: headerbutton;
        Layout.fillWidth: true;
        Layout.preferredHeight: 50;
        text: "PAIRINGS";

        onClicked: {
            pairingsstack.currentIndex = 0;
        }
    }

    /*
        The list of pairs is initially shown

        ADD new sensor pair:
        1. Show complete list of available sensors
        2. Select a new sensor to pair -> edit screen
        3. Attach events to actions
        3. Submit -> Pair and wait -> Show pairs; Cancel -> show pairs

        Edit sensor pair:
        1. Select one of the pairs on the pairs list -> Edit screen
        2. Attach events to actions
        3. Submit -> Pair and wait -> Show pairs; Cancel -> show pairs
    */

    StackLayout{
        id: pairingsstack;
        Layout.fillWidth: true;
        Layout.fillHeight: true;

        SensorListofPairs{  //Index 0
            anchors.fill: parent;
            onAddNew:{  //Show the edit event/actions page
                pairingsstack.currentIndex = 1;
                headerbutton.text = "Add a new sensor pair";
            }
            onSelect: {
                eventsensorSetup.setSource("ActionsSetup.qml",
                                           {
                                               actionmodel: activeSensor.getActionModel(),
                                               dstsensor: data
                                           });
                headerbutton.text = data['url'] + " events trigger actions...";
                pairingsstack.currentIndex = 2;
            }
            onVisibleChanged: if(visible) refreshbutton.visible = true;
        }

        SensorList{ //Index 1
            anchors.fill: parent;
            onCancel: {
                pairingsstack.currentIndex = 0;
                headerbutton.text = "PAIRINGS";
            }
            onSelect: {
                eventsensorSetup.setSource("ActionsSetup.qml", {actionmodel: activeSensor.getActionModel(), dstsensor: data});
                headerbutton.text = data['url'] + " events trigger actions...";
                pairingsstack.currentIndex = 2;
            }

            onVisibleChanged: if(visible) refreshbutton.visible = false;
        }

        Loader{ //Index 2
            id: eventsensorSetup;
            anchors.fill: parent;
        }

        Connections{
            target: eventsensorSetup.item;
            onCancel: {
                pairingsstack.currentIndex = 0;
                headerbutton.text = "PAIRINGS";
            }
            onSubmit: {
                pairingsstack.currentIndex = 0;
                headerbutton.text = "PAIRINGS";
            }
        }

        onVisibleChanged: {
            if(visible){
                currentIndex = 0;
            }
        }
    }
}
