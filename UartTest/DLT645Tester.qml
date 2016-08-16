import QtQuick 2.0
import QtQuick.Controls 1.2

Item{
    Column{
        width: parent.width;
        height: parent.width;
        Row{
            width: parent.width;
            height: 30;
            ComboBox{
                id: commandselbox;
                width: parent.width / 2;
                height: 30;
                model: comm.getCommandsList();
            }
            Button{
                width: parent.width / 3;
                height: 30;
                text: "Submit";
                onClicked:comm.execCommand(commandselbox.currentText);
            }


        }
    }
}

