import Foundation
import CoreBluetooth

let XBoxServiceUUID = CBUUID(string: "5ff0");
let XBoxServiceLongUUID = CBUUID(string: "a7395ff0-f6cd-1692-994a-d66d9e0ce048");
let XBoxKeysUUID = CBUUID(string: "a7395ff1-f6cd-1692-994a-d66d9e0ce048");

class XBoxDevice: BleDevice, CBPeripheralDelegate
{
	var batteryLevelCharacteristic : CBCharacteristic! = nil
	var backCharacteristic : CBCharacteristic! = nil
//	var guideCharacteristic : CBCharacteristic! = nil
	var startCharacteristic : CBCharacteristic! = nil
	var lxCharacteristic : CBCharacteristic! = nil
	var lyCharacteristic : CBCharacteristic! = nil
	var ltCharacteristic : CBCharacteristic! = nil
	var rxCharacteristic : CBCharacteristic! = nil
	var ryCharacteristic : CBCharacteristic! = nil
	var rtCharacteristic : CBCharacteristic! = nil
	var aCharacteristic : CBCharacteristic! = nil
	var bCharacteristic : CBCharacteristic! = nil
	var xCharacteristic : CBCharacteristic! = nil
	var yCharacteristic : CBCharacteristic! = nil
	var upCharacteristic : CBCharacteristic! = nil
	var dnCharacteristic : CBCharacteristic! = nil
	var leftCharacteristic : CBCharacteristic! = nil
	var rightCharacteristic : CBCharacteristic! = nil
	var lbCharacteristic : CBCharacteristic! = nil
	var rbCharacteristic : CBCharacteristic! = nil
	var lsCharacteristic : CBCharacteristic! = nil
	var rsCharacteristic : CBCharacteristic! = nil

	var keysCharacteristic: CBCharacteristic! = nil;

	var batteryLevel : Int = 0

	override init(peripherial : CBPeripheral)
	{
		super.init(peripherial: peripherial);

		self.device.delegate = self

		startServicesSeach()
	}

	func startServicesSeach()
	{
		self.device.discoverServices([XBoxServiceLongUUID, BatteryServiceUUID])
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
			else if (thisService.UUID == XBoxServiceLongUUID)
			{
				peripheral.discoverCharacteristics(nil, forService: thisService)
			}
		}
	}

	func peripheral(peripheral: CBPeripheral, didDiscoverCharacteristicsForService service: CBService, error: NSError?)
	{
		for c in service.characteristics!
		{
			let characteristic = c as CBCharacteristic

			if (c.UUID == XBoxKeysUUID)
			{
				self.keysCharacteristic = c;

				continue;
			}


			var b1 : [UInt8] = [0,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5];

			c.UUID.data.getBytes(&b1, length: 16)

			let uuid : UInt16 = ((UInt16(b1[0]) << 8) | UInt16(b1[1]))

			switch (uuid)
			{
				case 0x2a19:
					self.batteryLevelCharacteristic = characteristic;
					break;
				case 0xaffc:
					self.backCharacteristic = characteristic;
					break;
//				case 0xaffd:
//					self.guideCharacteristic = characteristic;
//					break;
				case 0xaffb:
					self.startCharacteristic = characteristic;
					break;
				case 0xaff1:
					self.aCharacteristic = characteristic;
					break;
				case 0xaff2:
					self.bCharacteristic = characteristic;
					break;
				case 0xaff3:
					self.xCharacteristic = characteristic;
					break;
				case 0xaff4:
					self.yCharacteristic = characteristic;
					break;
				case 0xaff5:
					self.upCharacteristic = characteristic;
					break;
				case 0xaff6:
					self.dnCharacteristic = characteristic;
					break;
				case 0xaff7:
					self.leftCharacteristic = characteristic;
					break;
				case 0xaff8:
					self.rightCharacteristic = characteristic;
					break;
				case 0xaff9:
					self.lbCharacteristic = characteristic;
					break;
				case 0xaffa:
					self.rbCharacteristic = characteristic;
					break;
				case 0xaffe:
					self.lsCharacteristic = characteristic;
					break;
				case 0xafff:
					self.rsCharacteristic = characteristic;
					break;
				case 0xbff1:
					self.lxCharacteristic = characteristic;
					break;
				case 0xbff2:
					self.lyCharacteristic = characteristic;
					break;
				case 0xdff1:
					self.ltCharacteristic = characteristic;
					break;
				case 0xcff1:
					self.rxCharacteristic = characteristic;
					break;
				case 0xcff2:
					self.ryCharacteristic = characteristic;
					break;
				case 0xdff2:
					self.rtCharacteristic = characteristic;
					break;
				default:
					NSLog("Unhandled characteristic: %d", uuid);
					break;
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


	func sendCommand(command1 : UInt8, characteristic : CBCharacteristic)
	{
		let command : [UInt8] = [command1]

		let data: NSMutableData = NSMutableData(bytes: command, length: 1)

		self.device.writeValue(data, forCharacteristic: characteristic, type: CBCharacteristicWriteType.WithoutResponse)
	}


	func sendState(guide: Bool)
	{
		var command = [UInt8](count: 8, repeatedValue: 127);

		command[0] = 0x00;
		command[1] = 0x00;

		if (guide)
		{
			command[1] |= 1 << (14 - 8);
		}


		let data: NSMutableData = NSMutableData(bytes: command, length: 8)

		self.device.writeValue(data, forCharacteristic: self.keysCharacteristic, type: CBCharacteristicWriteType.WithoutResponse)
	}



}
