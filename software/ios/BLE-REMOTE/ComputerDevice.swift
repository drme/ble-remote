import Foundation
import CoreBluetooth

let ComputerServiceUUID = CBUUID(string: "5ff9") // 5ff0
let PowerButtonUUID     = CBUUID(string: "fff1")
let ResetButtonUUID     = CBUUID(string: "fff2")
let PowerLedUUID        = CBUUID(string: "ffe1")
let HddLedUUID          = CBUUID(string: "ffe2")

class ComputerDevice: BleDevice, CBPeripheralDelegate
{
	private var batteryLevelCharacteristic : CBCharacteristic! = nil
	private var powerButtonCharacteristic : CBCharacteristic! = nil
	private var resetButtonCharacteristic : CBCharacteristic! = nil
	private var powerLedCharacteristic : CBCharacteristic! = nil
	private var hddLedCharacteristic : CBCharacteristic! = nil
	var batteryLevel : Int = 0

	override init(peripherial: CBPeripheral)
	{
		super.init(peripherial: peripherial);

		self.device.delegate = self;

		startServicesSeach();
	}

	func startServicesSeach()
	{
		self.device.discoverServices([ComputerServiceUUID, BatteryServiceUUID])
	}

	func sendCommand(command : UInt8, characteristic : CBCharacteristic)
	{
		let commandBytes : [UInt8] = [command]

		let data: NSMutableData = NSMutableData(bytes: commandBytes, length: 1)

		device.writeValue(data, forCharacteristic: characteristic, type: CBCharacteristicWriteType.WithResponse)
	}

	func pressPowerButton()
	{
		sendCommand(1, characteristic: self.powerButtonCharacteristic)
	}

	func pressResetButton()
	{
		sendCommand(1, characteristic: self.resetButtonCharacteristic)
	}

	func releasePowerButton()
	{
		sendCommand(0, characteristic: self.powerButtonCharacteristic)
	}

	func releaseResetButton()
	{
		sendCommand(0, characteristic: self.resetButtonCharacteristic)
	}

	func updateStatus()
	{
		if (nil != self.batteryLevelCharacteristic)
		{
			self.device.readValueForCharacteristic(self.batteryLevelCharacteristic)
			self.device.readValueForCharacteristic(self.powerLedCharacteristic)
			self.device.readValueForCharacteristic(self.hddLedCharacteristic)
		}
	}

	func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
	{
		for service in peripheral.services!
		{
			let thisService = service as CBService

			if (thisService.UUID == BatteryServiceUUID)
			{
				peripheral.discoverCharacteristics(nil, forService: thisService)
			}
			else if (thisService.UUID == ComputerServiceUUID)
			{
				peripheral.discoverCharacteristics(nil, forService: thisService)
			}
		}
	}

	func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
	{
		for charateristic in service.characteristics!
		{
			let thisCharacteristic = charateristic as CBCharacteristic

			if (thisCharacteristic.UUID == BatteryLevelUUID)
			{
				self.batteryLevelCharacteristic = thisCharacteristic
			}
			else if (thisCharacteristic.UUID == PowerButtonUUID)
			{
				self.powerButtonCharacteristic = thisCharacteristic
			}
			else if (thisCharacteristic.UUID == ResetButtonUUID)
			{
				self.resetButtonCharacteristic = thisCharacteristic
			}
			else if (thisCharacteristic.UUID == PowerLedUUID)
			{
				self.powerLedCharacteristic = thisCharacteristic
			}
			else if (thisCharacteristic.UUID == HddLedUUID)
			{
				self.hddLedCharacteristic = thisCharacteristic
			}
		}
	}

	func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
	{
		if (characteristic == self.batteryLevelCharacteristic)
		{
			let count = characteristic.value!.length
			var array = [Int16](count: count, repeatedValue: 0)
			characteristic.value!.getBytes(&array, length:count * sizeof(UInt16))
			self.batteryLevel = Int(array[0])
		}
	}
}
