import Foundation
import CoreBluetooth

let BatteryServiceUUID = CBUUID(string: "180f")
let FanServiceUUID     = CBUUID(string: "a739BCC0-f6cd-1692-994a-d66d9e0ce048")
let BatteryLevelUUID   = CBUUID(string: "2a19")
let FanSpeedUUID       = CBUUID(string: "a739BCC1-f6cd-1692-994a-d66d9e0ce048")

class FanDevice: BleDevice, CBPeripheralDelegate
{
	private var fanSpeed : Float = 0.0
	private var batteryLevelCharacteristic : CBCharacteristic! = nil
	private var fanSpeedCharacteristic : CBCharacteristic! = nil
	var batteryLevel : Int = 0

	var speed : Float
	{
		get
		{
			return self.fanSpeed
		}
		set(value)
		{
			self.fanSpeed = value

		//	let command : [UInt8] = (value > 0) ? [0xfe, 0xfe] : [0x00, 0x00]
			//[0x00, UInt8(self.fanSpeed * 0xff)]

			let command : [UInt8] = (value > 0) ? [UInt8(self.fanSpeed * 0xfe), UInt8(self.fanSpeed * 0xfe)] : [0x00, 0x00]

			let data: NSMutableData = NSMutableData(bytes: command, length: sizeof(UInt16))

			device.writeValue(data, forCharacteristic: self.fanSpeedCharacteristic, type: CBCharacteristicWriteType.WithoutResponse)
		}
	}

	override init(peripherial: CBPeripheral)
	{
		super.init(peripherial: peripherial);

		self.device.delegate = self;

		startServicesSeach();
	}

	func startServicesSeach()
	{
		self.device.discoverServices([FanServiceUUID, BatteryServiceUUID])
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
			else if (thisService.UUID == FanServiceUUID)
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
			else if (thisCharacteristic.UUID == FanSpeedUUID)
			{
				self.fanSpeedCharacteristic = thisCharacteristic
			}
		}
	}

	func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
	{
		if (characteristic.UUID == BatteryLevelUUID)
		{
			let count = characteristic.value!.length
			var array = [Int16](count: count, repeatedValue: 0)
			characteristic.value!.getBytes(&array, length:count * sizeof(UInt16))
			self.batteryLevel = Int(array[0])
		}
	}
}
