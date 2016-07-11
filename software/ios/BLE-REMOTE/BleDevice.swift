import Foundation;
import CoreBluetooth;

class BleDevice: NSObject
{
	var device: CBPeripheral;

	init(peripherial: CBPeripheral)
	{
		self.device = peripherial;

		super.init();
	}
}
