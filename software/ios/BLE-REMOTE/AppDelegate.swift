import UIKit;

@UIApplicationMain class AppDelegate: UIResponder, UIApplicationDelegate
{
	var window: UIWindow?;

	func application(application: UIApplication, didFinishLaunchingWithOptions launchOptions: [NSObject: AnyObject]?) -> Bool
	{
		return true;
	}

	func applicationWillResignActive(application: UIApplication)
	{
		DevicesManager.sharedInstance.stopDevicesSearch();
	}

	func applicationDidEnterBackground(application: UIApplication)
	{
		DevicesManager.sharedInstance.startDeviceSearch();
	}

	func applicationWillEnterForeground(application: UIApplication)
	{
		DevicesManager.sharedInstance.startDeviceSearch();
	}

	func applicationDidBecomeActive(application: UIApplication)
	{
		DevicesManager.sharedInstance.startDeviceSearch();
	}

	func applicationWillTerminate(application: UIApplication)
	{
		DevicesManager.sharedInstance.stopDevicesSearch();
	}
}
