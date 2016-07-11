import Foundation;

class EditButtonViewController: UIViewController
{
	@IBOutlet weak var nameField: UITextField!;
	@IBOutlet weak var commandField: UITextField!;
	@IBOutlet weak var xField: UITextField!;
	@IBOutlet weak var yField: UITextField!;
	@IBOutlet weak var wField: UITextField!;
	@IBOutlet weak var hField: UITextField!;
	@IBOutlet weak var squareField: UISwitch!;
	@IBOutlet weak var webView: UIWebView!;
	@IBOutlet weak var addressField: UITextField!;
	@IBOutlet weak var protocolField: UISegmentedControl!;
	var remote: Remote!;
	var button: RemoteButton!;

	override func viewDidLoad()
	{
		super.viewDidLoad();

		addKeyboardDismisser();
	}

	override func viewWillAppear(animated: Bool)
	{
		super.viewWillAppear(animated);

		self.nameField.text = self.button.title;
		self.commandField.text = String(self.button.command);
		self.xField.text = String(self.button.x);
		self.yField.text = String(self.button.y);
		self.wField.text = String(self.button.width);
		self.hField.text = String(self.button.height);
		self.squareField.on = !self.button.round;

		if (-1 != self.button.address)
		{
			self.addressField.text = String(self.button.address);
		}
		else
		{
			self.addressField.text = String(self.remote.address);
		}

		if (-1 != self.button.type)
		{
			self.protocolField.selectedSegmentIndex = self.button.type;
		}
		else
		{
			self.protocolField.selectedSegmentIndex = self.remote.type;
		}

		typeChanged(self.protocolField);
	}

	override func viewWillDisappear(animated: Bool)
	{
		super.viewWillDisappear(animated);

		saveState();
	}

	func saveState()
	{
		self.button.title = self.nameField.text!;
		self.button.command = getNumber(self.commandField);
		self.button.x = getNumber(self.xField);
		self.button.y = getNumber(self.yField);
		self.button.width = getNumber(self.wField);
		self.button.height = getNumber(self.hField);
		self.button.round = !self.squareField.on;

		self.button.type = (self.remote.type == self.protocolField.selectedSegmentIndex) ? -1 : self.protocolField.selectedSegmentIndex;

		let address = getNumber(self.addressField);

		self.button.address = (address == self.remote.address) ? -1 : address;

		self.remote.save();
	}

	@IBAction func testCommand(sender: UIButton)
	{
		saveState();

		sendCommand(self.button, remote: self.remote);
	}

	@IBAction func typeChanged(sender: UISegmentedControl)
	{
		switch sender.selectedSegmentIndex
		{
		case 0:
			self.webView.loadRequest(NSURLRequest(URL: NSURL(string: "https://www.google.com/search?query=sirc+remote+addresses")!));
			break;
		case 1:
			self.webView.loadRequest(NSURLRequest(URL: NSURL(string: "https://www.google.com/search?query=nec+remote+addresses")!));
			break;
		case 2:
			self.webView.loadRequest(NSURLRequest(URL: NSURL(string: "https://en.wikipedia.org/wiki/RC-5#System_Number_Allocations")!));
			break;
		default:
			break;
		}
	}
}
