import UIKit
import CoreMotion
import CoreBluetooth

class XBoxViewController : UIViewController, UIAccelerometerDelegate
{
	@IBOutlet weak var connectingIndicator : UIActivityIndicatorView?
	@IBOutlet weak var accelerometerSwitch : UISwitch?
	@IBOutlet weak var leftStick : JoyStickView?
	@IBOutlet weak var rightStick : JoyStickView?
	var motionManager: CMMotionManager! = nil

	override func viewDidLoad()
	{
		super.viewDidLoad()

		_ = NSTimer.scheduledTimerWithTimeInterval(0.4, target: self, selector: "update", userInfo: nil, repeats: true)

		self.connectingIndicator?.startAnimating()

		self.motionManager = CMMotionManager()

		self.motionManager.accelerometerUpdateInterval = 10

		self.motionManager.startAccelerometerUpdatesToQueue(NSOperationQueue.currentQueue()!, withHandler:
		{
			data, error in

			if (true == self.accelerometerSwitch!.on)
			{
				var v = data?.acceleration.y

				if (v < -0.5)
				{
					v = -0.5
				}

				if (v > 0.5)
				{
					v = 0.5
				}

				let nds = 127.0 - 0.0;
				let pds = 255.0 - 127.0;

				let steering = 127.0 + ((v < 0.0) ? (nds * v! * 2.0) : (pds * v! * 2.0))

				if (nil != DevicesManager.sharedInstance.connectedXBox)
				{
//					self.move(self.getS().lxCharacteristic, val: steering)
				}
			}
		})

		NSNotificationCenter.defaultCenter().addObserver(self, selector: "sticksChanged:", name: "StickChanged", object: nil)
	}

	@objc func sticksChanged(notification: NSNotification)
	{
		let vdir : NSValue = (notification.userInfo!["dir"]) as! NSValue

		let dir : CGPoint = vdir.CGPointValue()

		if (self.leftStick === notification.object)
		{
			move(getS().lxCharacteristic, val: 2.0 * Double(dir.x))
			move(getS().lyCharacteristic, val: 2.0 * Double(dir.y))
		}
		else if (notification.object === self.rightStick)
		{
			move(getS().rxCharacteristic, val: 2.0 * Double(dir.x))
			move(getS().ryCharacteristic, val: 2.0 * Double(dir.y))
		}
	}

	func update()
	{
		if (DevicesManager.sharedInstance.connectedXBox != nil)
		{
			self.connectingIndicator?.stopAnimating()
		}
		else
		{
			self.connectingIndicator?.startAnimating()
		}
	}

	func getS() -> XBoxDevice
	{
		return DevicesManager.sharedInstance.connectedXBox
	}
	
	func press(charr : CBCharacteristic)
	{
		if (nil != DevicesManager.sharedInstance.connectedXBox)
		{
			DevicesManager.sharedInstance.connectedXBox.sendCommand(1, characteristic: charr)
		}
	}

	func release(charr : CBCharacteristic)
	{
		if (nil != DevicesManager.sharedInstance.connectedXBox)
		{
			DevicesManager.sharedInstance.connectedXBox.sendCommand(0, characteristic: charr)
		}
	}

	func move(cc : CBCharacteristic, val : Double)
	{
		let v = 1.0 * val

		let value = UInt8(v)

		if (nil != DevicesManager.sharedInstance.connectedXBox)
		{
			DevicesManager.sharedInstance.connectedXBox.sendCommand(value, characteristic: cc)
		}
	}

	@IBAction func guidePressed(_: AnyObject)
	{
		DevicesManager.sharedInstance.connectedXBox.sendState(true);
	}

	@IBAction func guideReleased(_: AnyObject)
	{
		DevicesManager.sharedInstance.connectedXBox.sendState(false);
	}

	@IBAction func bPressed(_: AnyObject)
	{
		press(getS().bCharacteristic)
	}

	@IBAction func bReleased(_: AnyObject)
	{
		release(getS().bCharacteristic)
	}

	@IBAction func aPressed(_: AnyObject)
	{
		press(getS().aCharacteristic)
	}

	@IBAction func aReleased(_: AnyObject)
	{
		release(getS().aCharacteristic)
	}

	@IBAction func xPressed(_: AnyObject)
	{
		press(getS().xCharacteristic)
	}

	@IBAction func xReleased(_: AnyObject)
	{
		release(getS().xCharacteristic)
	}

	@IBAction func yPressed(_: AnyObject)
	{
		press(getS().yCharacteristic)
	}

	@IBAction func yReleased(_: AnyObject)
	{
		release(getS().yCharacteristic)
	}

	@IBAction func backPressed(_: AnyObject)
	{
		press(getS().backCharacteristic)
	}

	@IBAction func backReleased(_: AnyObject)
	{
		release(getS().backCharacteristic)
	}

	@IBAction func startPressed(_: AnyObject)
	{
		press(getS().startCharacteristic)
	}

	@IBAction func startReleased(_: AnyObject)
	{
		release(getS().startCharacteristic)
	}

	@IBAction func upPressed(_: AnyObject)
	{
		release(getS().upCharacteristic)
	}

	@IBAction func upReleased(_: AnyObject)
	{
		release(getS().upCharacteristic)
	}

	@IBAction func downPressed(_: AnyObject)
	{
		press(getS().dnCharacteristic)
	}

	@IBAction func downReleased(_: AnyObject)
	{
		release(getS().dnCharacteristic)
	}

	@IBAction func leftPressed(_: AnyObject)
	{
		press(getS().leftCharacteristic)
	}

	@IBAction func leftReleased(_: AnyObject)
	{
		release(getS().leftCharacteristic)
	}

	@IBAction func rightPressed(_: AnyObject)
	{
		press(getS().rightCharacteristic)
	}

	@IBAction func rightReleased(_: AnyObject)
	{
		release(getS().rightCharacteristic)
	}

	@IBAction func lbPressed(_: AnyObject)
	{
		press(getS().lbCharacteristic)
	}

	@IBAction func lbReleased(_: AnyObject)
	{
		release(getS().lbCharacteristic)
	}

	@IBAction func rbPressed(_: AnyObject)
	{
		press(getS().rbCharacteristic)
	}

	@IBAction func rbReleased(_: AnyObject)
	{
		release(getS().rbCharacteristic)
	}

	@IBAction func rsPressed(_: AnyObject)
	{
		press(getS().rsCharacteristic)
	}

	@IBAction func rsReleased(_: AnyObject)
	{
		release(getS().rsCharacteristic)
	}

	@IBAction func lsPressed(_: AnyObject)
	{
		press(getS().lsCharacteristic)
	}

	@IBAction func lsReleased(_: AnyObject)
	{
		release(getS().lsCharacteristic)
	}

	@IBAction func ltPressed(_: AnyObject)
	{
		move(getS().ltCharacteristic, val: 255)
	}

	@IBAction func ltReleased(_: AnyObject)
	{
		move(getS().ltCharacteristic, val : Double(0.0))
	}

	@IBAction func rtPressed(_: AnyObject)
	{
		move(getS().rtCharacteristic, val : 255)
	}

	@IBAction func rtReleased(_: AnyObject)
	{
		move(getS().rtCharacteristic, val : 0.0)
	}
}
