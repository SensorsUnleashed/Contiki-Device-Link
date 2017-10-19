import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQml.Models 2.2
import QtQuick.Layouts 1.3

Item{
    signal cancel;
    signal submit;

    property string actionmodel;
    property var dstsensor;

    /* Any event fired from a paired device can trigger any Action
       E.g. an Above and below event can both trigger the same action if necessary
       An event can only be set ones. You cant have an above event trigger both an Off and On action

       TODO: Implement validation of the rules applied here
    */
    function addAction(){
        if(mod.count >= de.get(0).deviceevents.count) return;

        var a = {
            "actions" : [],
            "deviceevents" : [],
        };

        for(var i=0; i<am.item.get(0).actions.count; i++){
            a['actions'][i] = {
                "name" : am.item.get(0).actions.get(i).name,
                "actionenum" : am.item.get(0).actions.get(i).actionenum
            }
        }

        for(i=0; i<de.get(0).deviceevents.count; i++){
            a['deviceevents'][i] = {
                "name" : de.get(0).deviceevents.get(i).name
            }
        }

        mod.append(a);
    }


    ListModel{
        id: de;
        ListElement {
            deviceevents: [
                ListElement { name: "Above event" },
                ListElement { name: "Below event" },
                ListElement { name: "Change event" }
            ]
        }
    }

    ListModel{
        id: mod;
    }

    /* This loader is used to pass in a set of available
       actions from the calling qml file
    */
    Loader{
        id: am;
        source: actionmodel;
    }

    ListView{
        id: lw;
        anchors.fill: parent;
        anchors.margins: 5;
        height: parent.height;
        width: parent.width;

        model: mod;

        highlightFollowsCurrentItem: false;
        focus: true
        spacing: 25;
        delegate: GroupBox{
            width: 400 + 40;
            Row{
                spacing: 20;
                ComboBox{
                    id: comboevents;
                    width: 200;
                    model: deviceevents;
                    currentIndex: 0;
                    onCurrentTextChanged: {
                        dstsensor['triggers'][index] = {
                            'eventname': currentText,
                            'actionname': actiontrigger.currentText,
                            'actionenum': actiontrigger.model.get(actiontrigger.currentIndex).actionenum,
                        }
                    }
                }

                ComboBox{
                    id: actiontrigger;
                    width: 200;
                    model: actions;
                    textRole: "name"
                    currentIndex: 0;
                    onCurrentTextChanged: {
                        dstsensor['triggers'][index] = {
                            'eventname': comboevents.currentText,
                            'actionname': currentText,
                            'actionenum': model.get(currentIndex).actionenum,
                        }
                    }
                }

            }
        }
        footerPositioning: ListView.OverlayFooter;
        footer: Row{
            width: lw.width - 10;
            spacing: 10;
            SUButton{
                text: "+";
                width: (parent.width - parent.spacing) / 3 -1;
                onClicked: {
                    addAction();
                }
            }

            SUButton{
                text: "SUBMIT";
                width: (parent.width - parent.spacing) / 3 -1;
                onClicked: {
                    activeSensor.pair(dstsensor);
                    submit();
                }
            }
            SUButton{
                text: "CANCEL"
                width: (parent.width - parent.spacing) / 3 -1;
                onClicked: {
                    cancel();
                }
            }
        }
    }

    Component.onCompleted: {
        if(dstsensor['triggers'] === undefined) dstsensor['triggers'] = [];

        var len = dstsensor['triggers'].length;
        for(var i=0; i<len; i++){
            addAction();    //First add the actionmodel
            //TODO: Set the comboboxes to the right Event/Action binding
        }
    }
}


