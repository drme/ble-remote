import Foundation;

class BasePageViewController: UIPageViewController
{
	//private var overlay: UIView!;
	//private var showLoader = false;
	//private var statusUpdateTimer: NSTimer!;

	override func viewDidLoad()
	{
		super.viewDidLoad();

	//	self.overlay = UINib(nibName: "LoadingView", bundle: nil).instantiateWithOwner(nil, options: nil)[0] as! UIView;
	//	self.overlay!.backgroundColor = UIColor.blackColor();
	//	self.overlay!.alpha = 0.8;
	//	self.overlay!.bounds = view.bounds;
	}

	func startLoading()
	{
//		self.showLoader = true;

//		NSTimer.scheduledTimerWithTimeInterval(0.001, target: self, selector: #selector(self.addLoader), userInfo: nil, repeats: false);
	}

	func addLoader()
	{
	//	if ((true == self.showLoader) && (nil != self.overlay))
	//	{
	//		dispatch_async(dispatch_get_main_queue())
	//		{
	//			self.view.addSubview(self.overlay!);
	//		}
	//	}
	}

	func stopLoading()
	{
	//	self.showLoader = false;

	//	if (nil != self.overlay)
	//	{
		//	dispatch_async(dispatch_get_main_queue())
			//{
		//		self.overlay!.removeFromSuperview();
		//	}
	//	}
	}

	func delay(delay: Double, handler: ()->())
	{
		dispatch_after(dispatch_time(DISPATCH_TIME_NOW, Int64(delay * Double(NSEC_PER_SEC))), dispatch_get_main_queue(), handler);
	}

	func startUpdater(interval: Double)
	{
		stopUpdater();

	//	self.statusUpdateTimer = NSTimer.scheduledTimerWithTimeInterval(interval, target: self, selector: #selector(self.updateStatus), userInfo: nil, repeats: true);
	}

	func stopUpdater()
	{
	//	if (nil != statusUpdateTimer)
	//	{
	//		self.statusUpdateTimer!.invalidate();
	//		self.statusUpdateTimer = nil;
	//	}
	}

	override func viewWillDisappear(animated: Bool)
	{
		super.viewWillDisappear(animated);

		stopUpdater();
	}

	func updateStatus()
	{
	}

//	func handleError(error: ConnectionError)
///	{
//	}
}
