import Foundation;
import CoreBluetooth;

@objc public class DevicesManager: NSObject, CBCentralManagerDelegate
{
	var connectedRemote: TellyDevice! = nil;
	var connectedFan: FanDevice! = nil;
	var connectedNeoPixels: NeoPixelsDevice! = nil;
	var connectedPC: ComputerDevice! = nil;
	var connectedXBox: XBoxDevice! = nil;
	public static let sharedInstance = DevicesManager();
	var centralManager: CBCentralManager! = nil;
	var devices = [CBPeripheral: CBUUID]();
	var scanning = false;

	public override init()
	{
		super.init();

		self.centralManager = CBCentralManager(delegate: self, queue: nil);
	}

	func startDeviceSearch()
	{
		if (self.centralManager.state != .PoweredOn)
		{
			return;
		}

		if (false == self.scanning)
		{
			self.scanning = true;

			let services = [TellyServiceUUID, NeoPixelsServiceUUID, FanServiceUUID, ComputerServiceUUID, XBoxServiceUUID];

			self.centralManager.scanForPeripheralsWithServices(services, options: nil);
		}
	}

	func stopDevicesSearch()
	{
		self.scanning = false;

		self.centralManager.stopScan();
	}

	public func centralManagerDidUpdateState(central: CBCentralManager)
	{
		if (central.state == .PoweredOn)
		{
			startDeviceSearch();
		}
		else
		{
			self.scanning = false;
		}
	}

	public func centralManager(central: CBCentralManager, didDiscoverPeripheral peripheral: CBPeripheral, advertisementData: [String : AnyObject], RSSI: NSNumber)
	{
		NSLog("Discovered peripheral: %@ advertisement %@ RSSI: %@", peripheral.description, advertisementData.description, RSSI.description)

		if (peripheral.state != .Connected)
		{
			self.devices[peripheral] = (advertisementData["kCBAdvDataServiceUUIDs"] as! [CBUUID])[0]

			central.connectPeripheral(peripheral, options: nil)
		}
	}

	public func centralManager(central: CBCentralManager, didConnectPeripheral peripheral: CBPeripheral)
	{
		let serviceId = self.devices[peripheral]

		if (serviceId == FanServiceUUID)
		{
			self.connectedFan = FanDevice(peripherial: peripheral)
		}
		else if (serviceId == NeoPixelsServiceUUID)
		{
			self.connectedNeoPixels = NeoPixelsDevice(peripherial: peripheral)
		}
		else if (serviceId == ComputerServiceUUID)
		{
			self.connectedPC = ComputerDevice(peripherial: peripheral)
		}
		else if (serviceId == TellyServiceUUID)
		{
			self.connectedRemote = TellyDevice(peripherial: peripheral)
		}
		else if (serviceId == XBoxServiceUUID)
		{
			self.connectedXBox = XBoxDevice(peripherial: peripheral)
		}
	}

	public func centralManager(central: CBCentralManager, didFailToConnectPeripheral peripheral: CBPeripheral, error: NSError?)
	{
	}

	private func compare(peripheral: CBPeripheral!, device: BleDevice!) -> Bool
	{
		if ((nil == peripheral) || (nil == device))
		{
			return false;
		}

		return device.device == peripheral;
	}

	public func centralManager(central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: NSError?)
	{
		if (compare(peripheral, device: self.connectedRemote))
		{
			self.connectedRemote = nil;
		}
		else if (compare(peripheral, device: self.connectedFan))
		{
			self.connectedFan = nil;
		}
		else if (compare(peripheral, device: self.connectedNeoPixels))
		{
			self.connectedNeoPixels = nil;
		}
		else if (compare(peripheral, device: self.connectedPC))
		{
			self.connectedPC = nil;
		}
		else if (compare(peripheral, device: self.connectedXBox))
		{
			self.connectedXBox = nil;
		}
	}
}
