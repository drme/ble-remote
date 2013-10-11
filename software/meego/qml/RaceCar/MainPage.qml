import QtQuick 1.1
import com.nokia.meego 1.0
import QtMobility.connectivity 1.2
import QtMobility.sensors 1.2

Page
{
    property string btAddress: "00:06:66:08:e5:1b"
    tools: commonTools

    Label {
        id: macAddress
        anchors.left: parent.left
        anchors.leftMargin: 32
        anchors.verticalCenter: parent.verticalCenter

        text: btAddress
        platformStyle: LabelStyle {
            fontFamily: "Nokia Pure"
            fontPixelSize: 32
        }
    }

    Switch {
        id: blueSwitch

    //    anchors.left: parent.left
      //  anchors.leftMargin: 380

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.horizontalCenter:  parent.horizontalCenter

        //anchors.verticalCenter: parent.verticalCenter

        checked: false
        onCheckedChanged: {
            console.log("switch: " + checked)
            if (blueSwitch.checked == false) {
                blueSocket.connected = false //connect to bluetooth
            } else {
                blueSocket.connected = true //connect to bluetooth
            }
        }
    }


    BluetoothSocket {
        id: blueSocket

        connected: false
        service: blueService    // This is defined in BluetoothService element

        onErrorChanged: {
            blueSwitch.checked = false
            console.log("Error: " + blueSocket.error) //Errors are printed in Application Output underneeth.
        }
        onDataAvailable: blueSocket.sendStringData(stringData)

        onConnectedChanged: {
            blueSwitch.checked = connected
            console.log("Connection: " + connected)
        }

     //   BluetoothSocket.
    }

    BluetoothService {
        id: blueService

        deviceAddress: btAddress //This is MAC address of your bluetooth module connected to Arduino
        serviceUuid: "00001101-0000-1000-8000-00805F9B34FB"  //famous UUID for serial port
        serviceProtocol: "rfcomm" //rfcomm emulates RS-232 serial ports. It uses 9600 baudrate, none parity, 8 bits, 1 stop bit
    }

    RotationSensor {
        id: rotationSensor
        active: true
    }


    Button
    {
        id: btnPower
        anchors.left: parent.left
        anchors.right: parent.right
        text: "ON/OFF"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(11))
    }



    Button
    {
        id: btnUp
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: btnPower.bottom
        text: "Up"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(16))
    }

    Button
    {
        id: btnLeft
        anchors.left: parent.left
       // anchors.right: parent.right
        anchors.top: btnUp.bottom
        width:100
        text: "Left"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(14))
    }

    Button
    {
        id: btnRight
       // anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: btnUp.bottom
        width: 100
        text: "Right"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(15))
    }


    Button
    {
        id: btnOk
        anchors.left: btnLeft.right
        anchors.right: btnRight.left
        anchors.top: btnUp.bottom
        text: "OK"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(18))
    }


    Button
    {
        id: btnDown
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: btnLeft.bottom
        text: "Down"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(17))
    }

    Button
    {
        id: btnMenu
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: btnDown.bottom
        text: "Menu"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(19))
    }

    Button
    {
        id: btnVlUp
        anchors.left: parent.left
       // anchors.right: btnVlDown.left
        anchors.top: btnMenu.bottom
        width: 50
        text: "VL UP"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(12))
    }

    Button
    {
        id: btnVlDown
        //anchors.left: btnVlUp.right
        anchors.right: parent.right
        width: 50
        anchors.top: btnMenu.bottom
        text: "VL DN"
        onClicked: blueSocket.sendStringData("" + String.fromCharCode(13))
    }




    /*

    Timer {
        //        property real previousAngle: -1
        interval: 500; running: true; repeat: true
        onTriggered: {
            if (blueSocket.connected == true) {
                var angleZ = rotationSensor.reading.z
                var angleY = rotationSensor.reading.y
                var angleX = rotationSensor.reading.x
                //                if (previousAngle >= 0) angle = angle - previousAngle
                //blueSocket.sendStringData(String.fromCharCode(angle))

                //blueSocket.sendStringData()

                //                previousAngle = angle

                var c = 0;

                if (angleX > 30)
                {
                    c |= 1;
                }

                if (angleX < -30)
                {
                    c |= 2;
                }

                if ((angleY > -40) && (angleY < 0))
                {
                    c |= 8;
                }

                if (angleY < -90)
                {
                    c |= 4;
                }

              //  c+= 128;

//                blueService.

        //        blueSocket.sendStringData("" + String.fromCharCode(c))

        //        console.log("z: " + angleZ + ": y: " + angleY + " x: " + angleX + " => " + String.fromCharCode(c))
            }
        }
    }
    */
}
