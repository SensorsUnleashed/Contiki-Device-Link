import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.0


ApplicationWindow {
    visible: true
    width: 1024
    height: 800
    color: "grey";
    title: qsTr("Sensors Unleased")

//    menuBar: MenuBar {
//        Menu {
//            title: "File"
//            MenuItem { text: "Open..." }
//            MenuItem { text: "Close" }
//        }

//        Menu {
//            title: "Edit"
//            MenuItem { text: "Cut" }
//            MenuItem { text: "Copy" }
//            MenuItem { text: "Paste" }
//        }
//    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

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
            text: qsTr("Main")
        }
        TabButton {
            text: qsTr("Pairing")
        }
    }
}
