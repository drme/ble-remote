import UIKit

class FanViewController : UIViewController
{
	@IBOutlet weak var connectingIndicator : UIActivityIndicatorView?
	@IBOutlet weak var fanSpeedSlider : UISlider?
	@IBOutlet weak var speedLabel : UILabel?

	override func viewDidLoad()
	{
		super.viewDidLoad()

		_ = NSTimer.scheduledTimerWithTimeInterval(0.4, target: self, selector: "update", userInfo: nil, repeats: true)

		self.connectingIndicator?.startAnimating()
	}

	func update()
	{
		if (DevicesManager.sharedInstance.connectedFan != nil)
		{
			self.connectingIndicator?.stopAnimating()
			self.fanSpeedSlider?.enabled = true
		}
		else
		{
			self.connectingIndicator?.startAnimating()
			self.fanSpeedSlider?.enabled = false
		}
	}

	@IBAction func speedChanged(_: AnyObject)
	{
		if (nil != DevicesManager.sharedInstance.connectedFan)
		{
			let fan : FanDevice = DevicesManager.sharedInstance.connectedFan as FanDevice

			fan.speed = (self.fanSpeedSlider?.value)!;

			self.speedLabel?.text = String(format: "%d %%", Int(fan.speed * 100.0))
		}
	}
}
