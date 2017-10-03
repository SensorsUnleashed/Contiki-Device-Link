import QtQuick 2.7

ListModel{
    ListElement {
        actions: [
            ListElement {
                name: "Turn ON"
                actionenum: 0;
            },
            ListElement {
                name: "Turn OFF"
                actionenum: 1;
            },
            ListElement {
                name: "TOGGLE"
                actionenum: 2;
            }
        ]
    }
}


