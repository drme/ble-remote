Controlling IR devices using smart phones

Hardware – Bluetooth module design.
Software – Android, iOS Applications.
Firmware – Bluetooth module firmware.

![intro image](http://farm4.staticflickr.com/3796/11613272085_27cdb9315c.jpg)

Many home entertainment devices come with some kind of remote control. If we have several of such devices, we would end up with lots of big and sometime ugly remote controls in the end. Maybe it would be nice to control all devices only by smart phone with as a simple application.

### Goal

The aim of this project is to allow controlling TVs, DVD players, VCRs and similar devices using a smart phone application.

### Related projects

Logitech has Harmony Ultimate hub product that allows controlling various devices using smartphone as universal remote [1]. In this case, the special hub is used that emits infrared commands to devices, and accepts commands from the smart phone. The more different approach is implemented in the RedEye product [2]. The smart phone gets some additional plug that plugs into an audio jack and contains infrared emitter for sending commands from smart phone application.

### Solution

**Before**

Initially, each controllable device has its own remote and if we want to control it we have find the correct remote and use it. It gets complicated when there is a need to access several functions form couple devices at once (for example turning all stuff off, in some cases it’s easier to just plug out main power cord).

![Figure 1. Lots of remotes and devices.](http://farm6.staticflickr.com/5526/11323578163_096acac910.jpg)

**After**

The idea is to replace all remotes with one Bluetooth device (like universal remote), but instead of pressing the device buttons, the special application could be installed on the iPhone, iPad or Android smart phone. Then the application would send commands to the Bluetooth device, and that device would re-transmit commands to TVs, VCRs and other devices.

![Figure 2. Lots of devices and just one remote application.](http://farm4.staticflickr.com/3785/11323496636_1970d1166a.jpg)

For this project the Bluetooth device, iOS and Android applications are constructed. For the phone communication with the device the Bluetooth Smart (low energy) protocol is used. The main choice of this type Bluetooth is that its works in the low power mode. The Bluetooth device can work for a long time on a single coin (CR2032) battery. Also iOS application does not need any special certification on the Apple side and hardware does not need any specific (and expensive) Apple authorization chip (thus no need for MFi Program, hardware and licensing).

**Ingredients**:

* CC2540 Bluetooth Smart module (HM-10) from Fast Tech (the cheap module can be bought there at around 7$). Those modules have no CE or FCC certification. If certification is needed the module can be bought from Bluegiga (but some research is needed if the whole assembly needs re-certification even with the certified module).
* 1 resistor of 470? - for driving the infrared led transistor.
* CR2032 battery – for supplying power to the Bluetooth device.
* Battery holder – for holding the battery.
* Infrared led – for sending commands to TVs, VCRs, etc.
* NPN transistor – for switching power for the infrared led.
* PCB – for assembling all components on it.
* iOS application – for user control.
* Optionally - some TV for testing.
* Optionally - Arduino with infrared detector for extracting infrared codes from existing remotes.

### GATT design

For building application the Bluetooth low energy profile has to be specified. The Bluetooth device defines one service for controlling remote devices. The service has unique id and consist of 3 attributes:
* Device type – it specifies the device type, this type defines the modulation type that is used for sending commands to TVs, VCRs, etc. The possible values are: 0 – SIRC (mainly used for Sony devices), 1 – RC-5 (mainly used by Philips devices), and 2 – NEC (mainly used for NEC devices).
* Device address – device address is specified, based on device type. For example if device is a Sony TV (commands modulation is of SIRC type), device address is 1 (addresses can be found for SIRC [4])
* Command – the command is selected based on the device type, and address. For example: for powering a Sony TV, the command is 12. When this attribute is written, the command is sent to the TV using infrared protocol.For example, if the “power off” command is needed to be sent to a Sony TV, the device type should be written (1, SIRC) then the device address should be written with the value 1 (for TV), and 12 has to be written to the command attribute. If device remains the same, there is no need to change address and type attributes values, just writing to the command attribute is sufficient.

### Sending commands
TV or other equipment accepts commands modulated as the IR signal at some frequency. In order to send commands to a device, it’s needed to know the commands, and also those commands has to be modulated on some signal. Luckily, Texas Instruments provides RemoTI [5] library, which has some handy HAL drivers for modulating and sending commands over various infrared protocols. It contains 3 implementations for different infrared signal modulation types, such as:
* SIRC [4] – for working with Sony devices. Commands are modulated using 40 kHz signal. Provided as a hal_irgen_sirc driver.
* NEC [6] – for working with NEC, Sanyo device. Commands are modulated using 38 kHz signal. Provided as a hal_irgen_nec driver.
* RC-5 [7] – for working with Philips devices. Commands are modulated using 36 kHz signal. Provided as a hal_irgen_rc5 driver.

For sending a remote command to TVs or other devices only the command code is needed when the RemoTI is used. The command consists of 2 parts: a device type and an actual command. For example sending a command for a Sonny TV, the application has to send a command “1 + 12”. Where 1 is device type (1 specifies that it’s a TV) and 12 is on/off command.

### Obtaining commands

Command codes can be obtained by studying device, its documentation, its remote control type or searching and downloading command codes from the Internet. It’s also possible to build some small device what would capture and output commands from an existing device remote.
Here is a simple the Arduino sketch for obtaining commands and corresponding circuit.

![Figure 3. Commands capturing with arduino.](http://farm4.staticflickr.com/3803/11307613285_1f9513f58b.jpg)

The circuit just contains of an infrared detector (depending on a remote type, the detector of required de-modulation frequency has to be selected. For working with Sony remotes it has to be of 40 kHz, for Nec – 38 kHz , and for Philips – 36 kHz), Arduino and some power source. The circuit is easy to assemble: the first pin goes to a power (+5V), the third pin is connected to the ground and the middle one outputs the signal to the Arduino. Now only the Arduino code is needed to capture the commands and print them. The Arduino code uses IR library. The code is the following (It’s the default IR library sample with additional command code and address extraction, code is available here):

```c++
void loop()
{
  decode_results results;

  if (irrecv.decode(&results))
  {
    if (results.decode_type == NEC)
    {
      printNEC(results.value);
    }
    else if (results.decode_type == SONY)
    {
      printSIRC(results.value);
    }
    else if (results.decode_type == RC5)
    {
      Serial.print("RC5: ");
    }
    else if (results.decode_type == RC6)
    {
      Serial.print("RC6: ");
    }
    else if (results.decode_type == UNKNOWN)
    {
      Serial.print("UNKNOWN: ");
    }

    Serial.println(results.value, HEX);
    irrecv.resume(); // Receive the next value
  }
}
```

The captured command format using IR library is a little bit different than the infrared libraries for CC2540 accepts, so it’s needed to extract command and address from the captured command and to reverse bits order as well. The extraction works like this:

```c++
void printSIRC(unsigned short value)
{
  Serial.print("SIRC: address: ");
  unsigned short address = reverse((value & 0x1f) << 11);
  Serial.print(address, DEC);

  unsigned short command = reverse((value >> 5) << 9);
  Serial.print(" command: ");
  Serial.println(command, DEC);
}
```

After the desired codes are captured the iOS controlling application can be built that is capable of sending commands to the Bluetooth device.

### Hardware
**Hardware prototype**
Before building actual device, it’s a good idea to build its prototype on the breadboard, in order to check solutions feasibility. Some interesting aspects to check are: what is the transmission rage, where the Bluetooth device could be placed, and what usable distance can be from the device to a TV, what usable distance could be from a smart phone. The solution prototype was built using Arduino and a Bluetooth classic module. The prototype was assembled on a bread-board. The simple Arduino sketch was used for controlling infrared led along with a simple android application. 

![Figure 4. Hardware prototype using arduino.](http://farm3.staticflickr.com/2886/11307681514_bb8038a350.jpg)

### PCB and final harware

Then the feasibility of implementation was verified using Arduino prototype, the real device and its PCB was designed using Fritzing [8] and manufactured at the OSH Park [9]. 

![Figure 5. Hardware schematics.](http://farm8.staticflickr.com/7297/11307730043_86ff5e0e3d.jpg)

But before manufacturing actual PCB, it’s a good idea to build sample on the breadboard and check if the solution actually works:

![Figure 6. Hardware prototype on breadboard.](---)

The actual module was used, some connections were made on the bread-board, but the actual module was wired directly by thin wires as it had no breadboard friendly connectors.As breadboard prototype was ok, the actual PCB was produced. The frizzing tool was very helpful; its core is build using the MVC design pattern, so modules view has PCB, schematics and breadboard views. Those views allowed checking if prototype matches PCB design, and schematics are correct.

![Figure 7. PCB design with some optional components.](http://farm4.staticflickr.com/3665/11307729563_b8fa2a6983.jpg)

The produced PCB is just of 2x3cm size and it’s only powered by a cell battery. As there were some free space left on the PCB, temperature and light sensors were added.

![Figure 8. Final hardware (top).](http://farm3.staticflickr.com/2855/11307612205_1f70470803.jpg)

![Figure 9. Final hardware bottom.](http://farm3.staticflickr.com/2851/11307728533_8c94ee4e46.jpg)

We have persuaded our college at the office to do soldering job, as we have no skills in soldering area at all. And the final assembly looks nice.

### Firmware
The code was developed using IAR tool, the trial version with code size limit is sufficient. The PCB has a row of vias near each Bluetooth module pin. It was decided not to solder programming header, but just to plug jumper wires into corresponding vias and plug them into the programmer. 

![Figure 10. Flashing initial firmware.](http://farm6.staticflickr.com/5490/11307610895_ff6426a5b7.jpg)

The firmware was developed in the way that after the its initial upload, the new versions could be uploaded over the air using Texas Instruments OAD profile [10]. The firmware is organized into the main application, Bluetooth Smart profile, infrared drivers (copied from RemoTI stack) and module for sending infrared commands.
The firmware project source code is available here.

### iOS application

The iOS controlling application is just a single application that connects to the Bluetooth device and writes command, device type, and device number into devices gatt table. Then device detects the command attribute write, device emits infra-red signal at the pointed device or devices. The main part of the iOS application is the remote service definition in the Telly class. This class connects to the Bluetooth device, enumerates its services and characteristics, and if required services and characteristics are available on the Bluetooth device, it is able to send commands to the Bluetooth device. The main interface and command sending is presenter in the following picture.

The iOS application uses CoreBluetooh framework, that is the part of iOS SDK and supports Bluetooth Smart since iOS 5.0 version. The application communicates only with Bluetooth Smart devices, and in order to use this framework, the iPhone 4S, iPad 3 or newer ones are needed. 
The application source code can be downloaded here.

![Figure 11. iPhone application.](http://farm8.staticflickr.com/7342/11307609465_67f1d5ab30.jpg)

### Android application

Similarly the android application has been created. Since the Android 4.3 version, the Android libraries contain API for using Bluetooth Smart services. Using devices such as Nexus 4 it’s possible to interact with the previously constructed Bluetooth hardware. The android applications source code can be obtained from here.

![Figure 12. Android application.](http://farm4.staticflickr.com/3721/11307725713_49e9a7d57d.jpg)

### Enclosure

The device can be used without any enclosure. But it could be more appealing to have some enclosure. So for this device the case was designed and 3d printed. The most important part is to leave a hole for the infrared led. TV remotes have some black, semi-transparent plastic in the front side, that allows signal to pass through. For this device the small plastic enclosure was built with the opening in the front for inserting Bluetooth device and leaving an opening for the infrared led.

![Figure 13. The whole solution.](http://farm4.staticflickr.com/3741/11345130105_370e7f0c2e.jpg)

The Case was printed using the Shapeways [11] service. The enclosure design is provided here.

### Testing
It’s hard to assume that software and hardware just works as expected by looking at its source code. So testing comes to help. We have defined the criteria for solutions suitability for use, and some procedure for evaluation it. The main testing is the integration testing when we test if hardware and software components work together. Here are some of tests:
* Testing environment: Sony Bravia KDL46X520 TV, that accepts SIRC type commands, iPhone 4S with the iOS application and the assembled Bluetooth device.

Planned tests are:
* Checking if communication and control works: the goal of test is to check if command can be send from the iPhone and received and understood by the TV.
* * Power on TV: 
*** Pre-condition – TV is off, application is installed, the Bluetooth device is up and running (the battery is installed).
** Test steps: Select the device type TV, select manufacturer Sony, press power button on iOS application.
** Expected result: TV is powered on.
** Similarly – changing volume, charging input source.
* Checking if the complex interaction scenario can be performed
** Changing input source to PS3:
*** Pre-condition: TV is off, PS3 is connected to the hdmi 4th input, PS3 is off, PS3 is configured to be powered on when hdmi input gets active. 
** Test steps: launch iOS application, select device type TV, select manufacturer Sony, press power button, press home button, navigate to the input sources (press left twice, select hdmi4 press up twice) press ok.
** Expected result: PS3 is turned on, the PS3 dashboard is visible on the TV.

If those tests are passed, it is possible to assume that the constructed solution works and can be used.

### Concussions/summary

It’s possible to create simple device that would allow controlling various devices using infrared protocol from cell phone or tablet. The device only consists of several components: Bluetooth module, battery, battery holder, infrared led, resistor, transistor and PCB. The strategic placement of constructed Bluetooth device could allow controlling all your infrared devices from iPhone or Android device.

### References
[1] http://www.logitech.com/en-us/product/harmony-ultimate-hub?crid=60

[2] http://thinkflood.com/products/redeye-mini/

[3] http://www.sbprojects.com/knowledge/ir/sirc.php

[4] http://www.sbprojects.com/knowledge/ir/sirc.php

[5] http://www.ti.com/tool/remoti&DCMP=hpa_rfif_rf4ce&HQS=Other+OT+remoti

[6] http://www.sbprojects.com/knowledge/ir/nec.php

[7] http://www.sbprojects.com/knowledge/ir/rc5.php

[8] http://fritzing.org/

[9] http://oshpark.com/

[10] http://processors.wiki.ti.com/index.php/OAD

[11] http://www.shapeways.com/

[12] https://github.com/drme/ble-remote

