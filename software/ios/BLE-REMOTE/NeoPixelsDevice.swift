import Foundation
import CoreBluetooth

let NeoPixelsServiceUUID = CBUUID(string: "aff0")
let NeoPixelUUID         = CBUUID(string: "aff1")

class NeoPixelsDevice: BleDevice, CBPeripheralDelegate
{
	private var batteryLevelCharacteristic : CBCharacteristic! = nil
	private var pixelCharacteristic : CBCharacteristic! = nil
	var batteryLevel : Int = 0

	override init(peripherial : CBPeripheral)
	{
		super.init(peripherial: peripherial);

		self.device.delegate = self;

		startServicesSeach();
	}

	func startServicesSeach()
	{
		self.device.discoverServices([NeoPixelsServiceUUID, BatteryServiceUUID])
	}

	func setPixelColor(pixel : UInt8, r : UInt8, g : UInt8, b : UInt8)
	{
		let command : [UInt8] = [pixel, r, g, b]

		let data: NSMutableData = NSMutableData(bytes: command, length: 4)

		device.writeValue(data, forCharacteristic: self.pixelCharacteristic, type: CBCharacteristicWriteType.WithoutResponse)
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
			else if (thisService.UUID == NeoPixelsServiceUUID)
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
			else if (thisCharacteristic.UUID == NeoPixelUUID)
			{
				self.pixelCharacteristic = thisCharacteristic
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
