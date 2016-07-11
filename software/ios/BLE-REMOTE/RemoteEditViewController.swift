import Foundation;

class RemoteEditViewController: UIViewController
{
	@IBOutlet weak var nameField: UITextField!;
	@IBOutlet weak var addressField: UITextField!;
	@IBOutlet weak var protocolField: UISegmentedControl!;
	@IBOutlet weak var webView: UIWebView!;
	var remote: Remote!;

	override func viewDidLoad()
	{
		super.viewDidLoad();

		addKeyboardDismisser();
	}

	override func viewWillAppear(animated: Bool)
	{
		super.viewWillAppear(animated);

		self.addressField.text = String(self.remote.address);
		self.nameField.text = self.remote.name;
		self.protocolField.selectedSegmentIndex = self.remote.type;

		typeChanged(self.protocolField);
	}

	override func viewWillDisappear(animated: Bool)
	{
		super.viewWillDisappear(animated);

		self.remote.name = self.nameField.text!;
		self.remote.address = getNumber(self.addressField);
		self.remote.type = self.protocolField.selectedSegmentIndex;

		self.remote.save();
	}

	@IBAction func typeChanged(sender: UISegmentedControl)
	{
		switch sender.selectedSegmentIndex
		{
			case 0:
				self.webView.loadRequest(NSURLRequest(URL: NSURL(string: "https://www.google.com/search?q=sirc+remote+addresses")!));
				break;
			case 1:
				self.webView.loadRequest(NSURLRequest(URL: NSURL(string: "https://www.google.com/search?q=nec+remote+addresses")!));
				break;
			case 2:
				self.webView.loadRequest(NSURLRequest(URL: NSURL(string: "https://en.wikipedia.org/wiki/RC-5#System_Number_Allocations")!));
				break;
			default:
				break;
		}
	}
}
