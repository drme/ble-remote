import Foundation;
import CoreBluetooth;

let TellyServiceUUID = CBUUID(string: "a739eeee-f6cd-1692-994a-d66d9e0ce048");

class TellyDevice: BleDevice, CBPeripheralDelegate
{
	private let TellyCodeUUID    = CBUUID(string: "a739eee1-f6cd-1692-994a-d66d9e0ce048");
	private let TellyTypeUUID    = CBUUID(string: "a739eee0-f6cd-1692-994a-d66d9e0ce048");
	private let TellyAddressUUID = CBUUID(string: "a739eee2-f6cd-1692-994a-d66d9e0ce048");
	private let TellyPinUUID     = CBUUID(string: "a739f102-f6cd-1692-994a-d66d9e0ce048");
	private var batteryLevelCharacteristic: CBCharacteristic!;
	private var encodingTypeCharacteristic: CBCharacteristic!;
	private var addressTypeCharacteristic: CBCharacteristic!;
	private var commandCharacteristic: CBCharacteristic!;
	private var pinCharacteristic: CBCharacteristic!;
	var rssiLevel: Int = 0;
	var batteryLevel: Int = 0;
	var deviceName = "";

	override init(peripherial: CBPeripheral)
	{
		super.init(peripherial: peripherial);

		self.device.delegate = self;

		startServicesSeach();
	}

	var pin: UInt32!
	{
		didSet
		{
			if ((oldValue != nil) && (oldValue != self.pin))
			{
				let commandBytes = [UInt32(self.pin)];

				let data: NSMutableData = NSMutableData(bytes: commandBytes, length: 1);

				self.device.writeValue(data, forCharacteristic: self.pinCharacteristic, type: CBCharacteristicWriteType.WithResponse);
			}
		}
	}

	func startServicesSeach()
	{
		self.device.discoverServices([TellyServiceUUID, BatteryServiceUUID]);
	}

	func sendCommand(command: Int)
	{
		if ((command >= 0) && (command <= 0xff))
		{
			print("Sending command: " + String(command));

			let commandBytes : [UInt8] = [UInt8(command)];

			let data: NSMutableData = NSMutableData(bytes: commandBytes, length: 1);

			device.writeValue(data, forCharacteristic: self.commandCharacteristic, type: CBCharacteristicWriteType.WithResponse);
		}
	}

	func sendType(type: Int)
	{
		let commandBytes : [UInt8] = [UInt8(type)];

		let data: NSMutableData = NSMutableData(bytes: commandBytes, length: 1);

		self.device.writeValue(data, forCharacteristic: self.encodingTypeCharacteristic, type: CBCharacteristicWriteType.WithResponse);
	}

	func sendAddress(command: Int)
	{
		if ((command >= 0) && (command <= 0xff))
		{
			let commandBytes : [UInt8] = [UInt8(command)];

			let data: NSMutableData = NSMutableData(bytes: commandBytes, length: 1);

			self.device.writeValue(data, forCharacteristic: self.addressTypeCharacteristic, type: CBCharacteristicWriteType.WithResponse);
		}
		else if ((command > 0xff) && (command <= 0xffff))
		{
			let loByte = command & 0xff;
			let hiByte = command >> 8;

			let commandBytes : [UInt8] = [UInt8(loByte), UInt8(hiByte)];

			let data: NSMutableData = NSMutableData(bytes: commandBytes, length: 2);

			self.device.writeValue(data, forCharacteristic: self.addressTypeCharacteristic, type: CBCharacteristicWriteType.WithResponse);
		}
	}

	func updateStatus()
	{
		if (nil != self.batteryLevelCharacteristic)
		{
			self.device.readValueForCharacteristic(self.batteryLevelCharacteristic);
		}

		self.device.readRSSI();

		self.deviceName = self.device.description;
	}

	func peripheral(peripheral: CBPeripheral, didDiscoverServices error: NSError?)
	{
		for service in peripheral.services!
		{
			let thisService = service as CBService;

			if (thisService.UUID == BatteryServiceUUID)
			{
				peripheral.discoverCharacteristics([BatteryLevelUUID], forService: thisService);
			}
			else if (thisService.UUID == TellyServiceUUID)
			{
				peripheral.discoverCharacteristics([TellyAddressUUID, TellyTypeUUID, TellyCodeUUID, TellyPinUUID], forService: thisService);
			}
		}
	}

	func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
	{
		for charateristic in service.characteristics!
		{
			let thisCharacteristic = charateristic as CBCharacteristic;

			if (thisCharacteristic.UUID == BatteryLevelUUID)
			{
				self.batteryLevelCharacteristic = thisCharacteristic;
				self.device.readValueForCharacteristic(self.batteryLevelCharacteristic);
			}
			else if (thisCharacteristic.UUID == TellyCodeUUID)
			{
				self.commandCharacteristic = thisCharacteristic;
			}
			else if (thisCharacteristic.UUID == TellyTypeUUID)
			{
				self.encodingTypeCharacteristic = thisCharacteristic;
			}
			else if (thisCharacteristic.UUID == TellyAddressUUID)
			{
				self.addressTypeCharacteristic = thisCharacteristic;
			}
			else if (thisCharacteristic.UUID == TellyPinUUID)
			{
				self.pinCharacteristic = thisCharacteristic;
				self.device.readValueForCharacteristic(self.pinCharacteristic);
			}
		}
	}

	func peripheral(peripheral: CBPeripheral, didUpdateValueForCharacteristic characteristic: CBCharacteristic, error: NSError?)
	{
		if (characteristic === self.batteryLevelCharacteristic)
		{
			let count = characteristic.value!.length;
			var array = [Int16](count: count, repeatedValue: 0);
			characteristic.value!.getBytes(&array, length:count * sizeof(UInt16));
			self.batteryLevel = Int(array[0]);
		}
		else if (characteristic === self.pinCharacteristic)
		{
			let count = characteristic.value!.length;
			var array = [UInt32](count: count, repeatedValue: 0);
			characteristic.value!.getBytes(&array, length:count * sizeof(UInt32));
			self.pin = array[0];
		}
	}
}
