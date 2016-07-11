import Foundation;

class RemoteViewController: UIViewController
{
	@IBOutlet weak var page: UIImageView!;
	@IBOutlet weak var moveView: UIView!;
	@IBOutlet weak var connectingIndicator: UIActivityIndicatorView!;
	@IBOutlet weak var buttonsView: UIView!;
	var remote: Remote!;
	private var selectedButton: RemoteButton!;
	private var oldButtons = [UIView]();

	var editMode = false
	{
		didSet
		{
			if (self.editMode == false)
			{
				if (nil != self.moveView)
				{
					self.moveView.hidden = true;
				}

				if (nil != self.remote)
				{
					self.remote.save();
				}
			}
		}
	}

	override func viewDidLoad()
	{
		super.viewDidLoad();

		buildUI();

		NSTimer.scheduledTimerWithTimeInterval(0.4, target: self, selector: #selector(self.update), userInfo: nil, repeats: true)

		self.connectingIndicator?.startAnimating();
	}

	override func viewWillAppear(animated: Bool)
	{
		super.viewWillAppear(animated);

		buildUI();
	}

	func buildUI()
	{
		for view in self.oldButtons
		{
			view.removeFromSuperview();
		}

		self.oldButtons.removeAll();

		for remoteButton in self.remote.buttons
		{
			let button = StyledButton(frame: remoteButton.bounds, round: remoteButton.round);

			button.setTitle(remoteButton.title, forState: .Normal);

			if (remoteButton.title.hasPrefix("/./"))
			{
				button.setImage(UIImage(named: remoteButton.title.substringWithRange(remoteButton.title.startIndex.advancedBy(3) ..< remoteButton.title.endIndex)), forState: .Normal);
			}

			button.addTarget(self, action: #selector(self.buttonAction), forControlEvents: .TouchUpInside);
			button.remoteButton = remoteButton;

			self.buttonsView.addSubview(button);
			self.oldButtons.append(button);
		}
	}

	@objc private func buttonAction(sender: StyledButton)
	{
		if (true == self.editMode)
		{
			let optionMenu = UIAlertController(title: nil, message: "Edit", preferredStyle: .ActionSheet);

			let changecCommandAction = UIAlertAction(title: "Edit button", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				self.selectedButton = sender.remoteButton;

				self.performSegueWithIdentifier("EditButton", sender: nil);
			}

			let moveCommandAction = UIAlertAction(title: "Move button", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				self.selectedButton = sender.remoteButton;

				self.moveView.hidden = false;
			}

			let stopMoveCommandAction = UIAlertAction(title: "Stop move button", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				self.selectedButton = sender.remoteButton;

				self.moveView.hidden = true;
			}

			let removeCommandAction = UIAlertAction(title: "Remove button", style: .Destructive)
			{
				(alert: UIAlertAction!) -> Void in

				let button = sender.remoteButton;

				self.remote.buttons.removeAtIndex(self.remote.buttons.indexOf(button)!);

				self.remote.save();

				self.buildUI();
			}

			let cancelAction = UIAlertAction(title: "Cancel", style: .Cancel)
			{
				(alert: UIAlertAction!) -> Void in
			}

			optionMenu.addAction(changecCommandAction);

			if (self.moveView.hidden)
			{
				optionMenu.addAction(moveCommandAction);
			}
			else
			{
				optionMenu.addAction(stopMoveCommandAction);
			}

			optionMenu.addAction(removeCommandAction);
			optionMenu.addAction(cancelAction)

			self.presentViewController(optionMenu, animated: true, completion: nil);
		}
		else
		{
			sendCommand(sender.remoteButton, remote: self.remote);
		}
	}

	@objc private func update()
	{
		if (DevicesManager.sharedInstance.connectedRemote != nil)
		{
			self.connectingIndicator.stopAnimating();
		}
		else
		{
			self.connectingIndicator.startAnimating();
		}
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject!)
	{
		if (segue.identifier == "EditButton")
		{
			let target = segue.destinationViewController as! EditButtonViewController;

			target.remote = self.remote;
			target.button = self.selectedButton;
		}
	}

	@IBAction func moveUp(sender: UIButton)
	{
		self.selectedButton.y = self.selectedButton.y - 1;
		buildUI();
	}

	@IBAction func moveDown(sender: UIButton)
	{
		self.selectedButton.y = self.selectedButton.y + 1;
		buildUI();
	}

	@IBAction func moveLeft(sender: UIButton)
	{
		self.selectedButton.x = self.selectedButton.x - 1;
		buildUI();
	}

	@IBAction func moveRight(sender: UIButton)
	{
		self.selectedButton.x = self.selectedButton.x + 1;
		buildUI();
	}

	func addButton()
	{
		let button = RemoteButton(x: 150, y: 150, w: 75, h: 46, title: "New button", round: false, command: 1);

		self.remote.buttons.append(button);

		buildUI();
	}
}
