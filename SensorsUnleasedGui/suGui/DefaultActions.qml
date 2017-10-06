import QtQuick 2.7

ListModel{
    ListElement {
        actions: [
            ListElement {
                name: "Set ON"
                actionenum: 0;
            },
            ListElement {
                name: "Set OFF"
                actionenum: 1;
            },
            ListElement {
                name: "TOGGLE"
                actionenum: 2;
            }
        ]
    }
}


