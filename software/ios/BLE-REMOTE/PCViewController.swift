import UIKit

class PCViewController: UIViewController
{
	@IBOutlet weak var connectingIndicator : UIActivityIndicatorView?

	override func viewDidLoad()
	{
		super.viewDidLoad()

		_ = NSTimer.scheduledTimerWithTimeInterval(0.4, target: self, selector: "update", userInfo: nil, repeats: true)

		self.connectingIndicator?.startAnimating()
	}

	func update()
	{
		if (DevicesManager.sharedInstance.connectedPC != nil)
		{
			self.connectingIndicator?.stopAnimating()
		}
		else
		{
			self.connectingIndicator?.startAnimating()
		}
	}

	@IBAction func powerPressed(_: AnyObject)
	{
		if (DevicesManager.sharedInstance.connectedPC != nil)
		{
			DevicesManager.sharedInstance.connectedPC.pressPowerButton()
		}
	}

	@IBAction func powerReleased(_: AnyObject)
	{
		if (DevicesManager.sharedInstance.connectedPC != nil)
		{
			DevicesManager.sharedInstance.connectedPC.releasePowerButton()
		}
	}

	@IBAction func resetPressed(_: AnyObject)
	{
		if (DevicesManager.sharedInstance.connectedPC != nil)
		{
			DevicesManager.sharedInstance.connectedPC.pressResetButton()
		}
	}

	@IBAction func resetReleased(_: AnyObject)
	{
		if (DevicesManager.sharedInstance.connectedPC != nil)
		{
			DevicesManager.sharedInstance.connectedPC.releaseResetButton()
		}
	}
}