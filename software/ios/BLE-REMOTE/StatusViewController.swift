import Foundation;

class StatusViewController: UIViewController
{
	@IBOutlet weak var connectedLabel: UILabel!;
	@IBOutlet weak var batteryLabel: UILabel!;
	@IBOutlet weak var pinField: UITextField!;

	override func viewDidLoad()
	{
		super.viewDidLoad();

		NSTimer.scheduledTimerWithTimeInterval(0.4, target: self, selector: #selector(self.update), userInfo: nil, repeats: true)

		update();
	}

	override func viewWillAppear(animated: Bool)
	{
		super.viewWillAppear(animated);

		if (DevicesManager.sharedInstance.connectedRemote != nil)
		{
			self.pinField.text = String(DevicesManager.sharedInstance.connectedRemote.pin);
		}
	}

	override func viewWillDisappear(animated: Bool)
	{
		super.viewWillDisappear(animated);

		if (DevicesManager.sharedInstance.connectedRemote != nil)
		{
			DevicesManager.sharedInstance.connectedRemote.pin = UInt32(getNumber(self.pinField));
		}
	}

	func update()
	{
		if (DevicesManager.sharedInstance.connectedRemote != nil)
		{
			self.connectedLabel.text = "connected";
			self.pinField.enabled = true;
			self.batteryLabel.text = String(DevicesManager.sharedInstance.connectedRemote.batteryLevel) + " %";
		}
		else
		{
			self.connectedLabel.text = "disconnected";
			self.pinField.enabled = false;
			self.batteryLabel.text = "";
		}
	}
}
