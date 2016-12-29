import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls.Material 2.0

ApplicationWindow {
    id: mainwindow;
    //Material.theme: Material.dark;
    SystemPalette {
        id: suPalette;
        colorGroup: SystemPalette.Active
    }

    visible: true
    width: 1024
    height: 800
    color: suPalette.dark;
    title: qsTr("Sensors Unleased")

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        NodesPage{

        }

        TestPage {

        }

        FrontPage {

        }
    }

    Loader{
        id: setupboxloader;
        width: parent.width;
        height: parent.height;

    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        TabButton {
            text: qsTr("Nodes")
        }
        TabButton {
            text: qsTr("Main")
        }
        TabButton {
            text: qsTr("Pairing")
        }

    }
}
