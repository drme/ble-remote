import UIKit

class NeoPixelsViewController: UIViewController
{
	@IBOutlet weak var connectingIndicator : UIActivityIndicatorView?
	@IBOutlet weak var animationSwitch : UISwitch?

	override func viewDidLoad()
	{
		super.viewDidLoad()

		_ = NSTimer.scheduledTimerWithTimeInterval(0.4, target: self, selector: "update", userInfo: nil, repeats: true)

		self.connectingIndicator?.startAnimating()
	}

	func update()
	{
		if (DevicesManager.sharedInstance.connectedNeoPixels != nil)
		{
			self.connectingIndicator?.stopAnimating()
		}
		else
		{
			self.connectingIndicator?.startAnimating()
		}
	}

	@IBAction func turnOn(_: AnyObject)
	{
		if (DevicesManager.sharedInstance.connectedNeoPixels != nil)
		{
			let pixels = DevicesManager.sharedInstance.connectedNeoPixels

			pixels.setPixelColor(0xff, r: 0, g: 0, b: 33)
		}
	}

	@IBAction func turnOff(_: AnyObject)
	{
		if (DevicesManager.sharedInstance.connectedNeoPixels != nil)
		{
			let pixels = DevicesManager.sharedInstance.connectedNeoPixels

			pixels.setPixelColor(0xff, r: 0, g: 0, b: 0)
		}
	}

	@IBAction func setColor(button: UIButton)
	{
		if (DevicesManager.sharedInstance.connectedNeoPixels != nil)
		{
			let pixels = DevicesManager.sharedInstance.connectedNeoPixels

			let color = button.backgroundColor

			var red : CGFloat = 0
			var green : CGFloat = 0
			var blue : CGFloat = 0
			var alpha: CGFloat = 0

			color!.getRed(&red, green: &green, blue: &blue, alpha: &alpha)

			let r = UInt8(red * 255)
			let g = UInt8(green * 255)
			let b = UInt8(blue * 255)

			if (self.animationSwitch?.enabled == true)
			{
				let priority = DISPATCH_QUEUE_PRIORITY_DEFAULT

				dispatch_async(dispatch_get_global_queue(priority, 0))
				{
					for var i : UInt8 = 0; i < 240; i++
					{
						//dispatch_after(UInt64(i) * 100, dispatch_get_main_queue())
//						{
								pixels.setPixelColor(i, r: r, g: g, b: b)
//						}

						NSThread.sleepForTimeInterval(0.1)
					}
				}
			}
			else
			{
				pixels.setPixelColor(0xff, r: r, g: g, b: b)
			}
		}
	}
}
