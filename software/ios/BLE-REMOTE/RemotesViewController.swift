import Foundation;

class RemotesViewController: BasePageViewController, UIPageViewControllerDataSource, UIPageViewControllerDelegate
{
	@IBOutlet weak var nameBar: UINavigationItem!;
	@IBOutlet weak var editButton: UIBarButtonItem!;
	private var pages = [UIViewController]();
	private var remotesEditing = false;

	override func viewDidLoad()
	{
		self.navigationController!.navigationBar.barTintColor = UIColor(red: 0.0 / 255.0, green: 54.0 / 255.0, blue: 104.0 / 255.0, alpha: 0.7);
		self.navigationController!.navigationBar.titleTextAttributes = [NSForegroundColorAttributeName: UIColor.whiteColor()];

		super.viewDidLoad();

		loadPages();

		self.dataSource = self;
		self.delegate = self;
	}

	override func viewWillAppear(animated: Bool)
	{
		super.viewWillAppear(animated);

		updateLabel();
	}

	func loadPages()
	{
		self.pages.removeAll();

		for remote in RemotesManager.sharedInstance.remotes
		{
			let page = self.storyboard!.instantiateViewControllerWithIdentifier("RemoteView") as! RemoteViewController;

			page.remote = remote;

			self.pages.append(page);
		}

		dispatch_async(dispatch_get_main_queue())
		{
			self.setViewControllers([self.pages[0]], direction: .Reverse, animated: true, completion: nil);

			self.nameBar.title = (self.pages[0] as? RemoteViewController)!.remote.name;
		}
	}

	func pageViewController(pageViewController: UIPageViewController, viewControllerBeforeViewController viewController: UIViewController) -> UIViewController?
	{
		if (self.pages.count > 1)
		{
			for i in 1...(self.pages.count - 1)
			{
				if (viewController == self.pages[i])
				{
					return self.pages[i - 1];
				}
			}
		}

		return nil;
	}

	func pageViewController(pageViewController: UIPageViewController, viewControllerAfterViewController viewController: UIViewController) -> UIViewController?
	{
		if (self.pages.count > 1)
		{
			for i in 0...(self.pages.count - 2)
			{
				if (viewController == self.pages[i])
				{
					return self.pages[i + 1];
				}
			}
		}

		return nil;
	}

	func presentationCountForPageViewController(pageViewController: UIPageViewController) -> Int
	{
		return self.pages.count;
	}

	func presentationIndexForPageViewController(pageViewController: UIPageViewController) -> Int
	{
		return 0;
	}

	func pageViewController(pageViewController: UIPageViewController, didFinishAnimating finished: Bool, previousViewControllers: [UIViewController], transitionCompleted completed: Bool)
	{
		updateLabel();
	}

	private func updateLabel()
	{
		if (self.viewControllers?.count > 0)
		{
			let view: UIViewController! = self.viewControllers![0];

			if ((nil != view) && (view is RemoteViewController))
			{
				self.nameBar.title = (view as! RemoteViewController).remote.name;
			}
			else
			{
				self.nameBar.title = "";
			}
		}
		else
		{
			self.nameBar.title = "";
		}
	}

	@IBAction func editChanged(sender: UIBarButtonItem)
	{
		if (true == self.remotesEditing)
		{
			sender.title = "Edit";
			self.remotesEditing = false;

			for page in self.pages
			{
				if (page is RemoteViewController)
				{
					let controller = page as! RemoteViewController;

					controller.editMode = false;
				}
			}
		}
		else
		{
			let optionMenu = UIAlertController(title: nil, message: "Edit", preferredStyle: .ActionSheet);

			let editAction = UIAlertAction(title: "Edit buttons", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				sender.title = "Done";
				self.remotesEditing = true;

				for page in self.pages
				{
					if (page is RemoteViewController)
					{
						let controller = page as! RemoteViewController;

						controller.editMode = true;
					}
				}
			}

			let cancelAction = UIAlertAction(title: "Cancel", style: .Cancel)
			{
				(alert: UIAlertAction!) -> Void in
			}

			let deleteRemoteAction = UIAlertAction(title: "Delete remote", style: .Destructive)
			{
				(alert: UIAlertAction!) -> Void in

				let view: UIViewController! = self.viewControllers![0];

				if ((nil != view) && (view is RemoteViewController))
				{
					(view as! RemoteViewController).remote.delete();
				}

				self.loadPages();
			}

			let addRemoteAction = UIAlertAction(title: "Add remote", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				RemotesManager.sharedInstance.addDefaultRemote1("new remote");

				self.loadPages();
			}

			let addRemoteButtonAction = UIAlertAction(title: "Add remote button", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				let view: UIViewController! = self.viewControllers![0];

				if ((nil != view) && (view is RemoteViewController))
				{
					(view as! RemoteViewController).addButton();
				}
			}

			let editRemoteAction = UIAlertAction(title: "Edit remote", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				self.performSegueWithIdentifier("EditRemote", sender: nil);
			}

			let statusAction = UIAlertAction(title: "Status", style: .Default)
			{
				(alert: UIAlertAction!) -> Void in

				self.performSegueWithIdentifier("Status", sender: nil);
			}

			optionMenu.addAction(addRemoteButtonAction);
			optionMenu.addAction(editAction);
			optionMenu.addAction(editRemoteAction);
			optionMenu.addAction(addRemoteAction);
			optionMenu.addAction(deleteRemoteAction);
			optionMenu.addAction(statusAction);
			optionMenu.addAction(cancelAction);

			self.presentViewController(optionMenu, animated: true, completion: nil);
		}
	}

	override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject!)
	{
		if (segue.identifier == "EditRemote")
		{
			let target = segue.destinationViewController as! RemoteEditViewController;

			let view: UIViewController! = self.viewControllers![0];

			if ((nil != view) && (view is RemoteViewController))
			{
				target.remote = (view as! RemoteViewController).remote;
			}
		}
	}
}
