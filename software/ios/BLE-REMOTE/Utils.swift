import Foundation;

extension UIViewController
{
	func addKeyboardDismisser()
	{
		self.view.addGestureRecognizer(UITapGestureRecognizer(target: self, action: #selector(self.dismissKeyboard)));
	}

	func dismissKeyboard()
	{
		self.view.endEditing(true);
	}

	func delay(delay: Double, closure:()->())
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(delay * Double(NSEC_PER_SEC))), dispatch_get_main_queue(), closure);
	}

	func getNumber(field: UITextField!) -> Int
	{
		if ((nil != field) && (nil != field.text) && (field.text?.characters.count > 0))
		{
			if let number = Int(field.text!)
			{
				return number;
			}
		}

		return 0;
	}

	func sendCommand(button: RemoteButton, remote: Remote)
	{
		let device = DevicesManager.sharedInstance.connectedRemote;

		if (device != nil)
		{
			if (-1 != button.type)
			{
				device.sendType(Int(button.type));
			}
			else
			{
				device.sendType(Int(remote.type));
			}

			if (-1 != button.address)
			{
				device.sendAddress(button.address)
			}
			else
			{
				device.sendAddress(remote.address);
			}

			device.sendCommand(button.command);
		}
	}
}
