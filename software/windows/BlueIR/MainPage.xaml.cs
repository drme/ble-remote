using System;
using System.Threading.Tasks;
using Windows.ApplicationModel.Core;
using Windows.Foundation.Metadata;
using Windows.System;
using Windows.System.Profile;
using Windows.UI;
using Windows.UI.Core;
using Windows.UI.ViewManagement;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Media;
using Windows.UI.Xaml.Navigation;

namespace BlueIR
{
	public sealed partial class MainPage : Page
	{
		private MenuFlyout buttonContextMenu;
		private Button activeButton;
		private MenuFlyoutItem itemEditButton = new MenuFlyoutItem() { Text = "Edit" };
		private MenuFlyoutItem itemRemoveButton = new MenuFlyoutItem() { Text = "Remove" };
		private MenuFlyoutItem itemMoveButton = new MenuFlyoutItem() { Text = "Move" };

		public MainPage()
		{
			var menu = new MenuFlyout();

			menu.Items.Add(itemEditButton);
			menu.Items.Add(itemRemoveButton);
			menu.Items.Add(itemMoveButton);

			itemEditButton.Click += EditButtonClick;
			itemRemoveButton.Click += RemoveButtonClick;
			itemMoveButton.Click += MoveButtonClick;

			this.buttonContextMenu = menu;

			this.InitializeComponent();

			Window.Current.SizeChanged += UpdateStatusBar;

			UpdateStatusBar(null, null);

			this.rootPivot.SelectionChanged += SaveRemoteSelection;

			DevicesManager.Instance.Connected += async (x, y) =>
			{
				await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
				{
					this.ConnectingIndicator.Visibility = Visibility.Collapsed;
				});
			};

			DevicesManager.Instance.Disconnected += async (x, y) =>
			{
				await CoreApplication.MainView.CoreWindow.Dispatcher.RunAsync(CoreDispatcherPriority.Normal, () =>
				{
					this.ConnectingIndicator.Visibility = Visibility.Visible;
				});
			};
		}

		private void SaveRemoteSelection(object sender, SelectionChangedEventArgs e)
		{
			RemotesManager.Instance.SetActiveRemote((Remote)((PivotItem)this.rootPivot.SelectedItem).Tag);
		}

		protected override async void OnNavigatedTo(NavigationEventArgs e)
		{
			base.OnNavigatedTo(e);

			await LoadRemotes();
		}

		private async void MoveButtonClick(object sender, RoutedEventArgs e)
		{
			if (this.MoveLeft.Visibility == Visibility.Collapsed)
			{
				this.MoveLeft.Visibility = this.MoveRight.Visibility = this.MoveUp.Visibility = this.MoveDown.Visibility = Visibility.Visible;
				this.itemMoveButton.Text = "Finish move";
			}
			else
			{
				this.MoveLeft.Visibility = this.MoveRight.Visibility = this.MoveUp.Visibility = this.MoveDown.Visibility = Visibility.Collapsed;
				this.itemMoveButton.Text = "Move";
				(await RemotesManager.Instance.GetActiveRemote())?.Save();
			}
		}

		private async void RemoveButtonClick(object sender, RoutedEventArgs e)
		{
			((Canvas)this.activeButton.Parent).Children.Remove(this.activeButton);
			this.activeButton = null;
			(await RemotesManager.Instance.GetActiveRemote())?.Save();
		}

		private void EditButtonClick(object sender, RoutedEventArgs e)
		{
			this.Frame.Navigate(typeof(EditButtonPage), this.activeButton.Tag);
		}

		private void UpdateStatusBar(object sender, Windows.UI.Core.WindowSizeChangedEventArgs e)
		{
			var color = Color.FromArgb(127, 0x65, 0x8c, 0xdd);
			var phone = ("Windows.Mobile" == AnalyticsInfo.VersionInfo.DeviceFamily);

			if (phone)
			{
				if (Window.Current.Bounds.Width > Window.Current.Bounds.Height)
				{
					SetStatusBarColor(Colors.Black);
				}
				else
				{
					SetStatusBarColor(color);
				}
			}
			else
			{
				SetStatusBarColor(color);
			}
		}

		private void SetStatusBarColor(Color color)
		{
			var titleBar = ApplicationView.GetForCurrentView().TitleBar;

			titleBar.BackgroundColor = color;
			titleBar.ButtonBackgroundColor = titleBar.BackgroundColor;
			//titleBar.ButtonBackgroundColor = Colors.LightGray;
			//titleBar.ButtonForegroundColor = Colors.White;

			if (ApiInformation.IsTypePresent("Windows.UI.ViewManagement.StatusBar"))
			{
				StatusBar.GetForCurrentView().BackgroundColor = titleBar.BackgroundColor;
				StatusBar.GetForCurrentView().BackgroundOpacity = 1;
				StatusBar.GetForCurrentView().ForegroundColor = Colors.LightGray;
			}
		}

		private void OpenMenuClick(object sender, RoutedEventArgs e)
		{
			this.menuPanel.IsPaneOpen = !this.menuPanel.IsPaneOpen;
		}

		private async void GetBlueIRClick(object sender, RoutedEventArgs e)
		{
			this.menuPanel.IsPaneOpen = false;
			await Launcher.LaunchUriAsync(new Uri("https://www.tindie.com/products/some1/blue-ir/"));
		}

		private async void AddRemoteClick(object sender, RoutedEventArgs e)
		{
			this.menuPanel.IsPaneOpen = false;
			this.rootPivot.SelectedItem = await AddRemotePage(await RemotesManager.Instance.AddNewRemote("new remote"), null);



			/*			System.Diagnostics.Debug.WriteLine("Adding remove");

						var wlan = EspTouch.WiFiUtils.GetConnectedSsid();
						var psk = "flying horse";
						String bssid = await EspTouch.WiFiUtils.GetBssid(wlan);

						using (EspTouch.Task task = new EspTouch.Task(wlan, bssid, psk, false))
						{
							task.DeviceFound += (result) => { System.Diagnostics.Debug.WriteLine(result); };

							var results = await task.ExecuteForResult();

							System.Diagnostics.Debug.WriteLine(results);
						} */
		}

		private async void RemoveRemoteClick(object sender, RoutedEventArgs e)
		{
			this.menuPanel.IsPaneOpen = false;

			if (await Extensions.ShowRemove(this, "Remove", "Remove {0} remote ?", (await RemotesManager.Instance.GetActiveRemote())?.Name))
			{
				(await RemotesManager.Instance.GetActiveRemote()).Delete();
				RemotesManager.Instance.SetActiveRemote(null);
				this.rootPivot.Items.Remove(this.rootPivot.SelectedItem);
			}
		}

		private async Task LoadRemotes()
		{
			this.rootPivot.Items.Clear();

			PivotItem activePage = null;

			foreach (var remote in await RemotesManager.Instance.GetRemotes())
			{
				activePage = await AddRemotePage(remote, activePage);
			}

			if (null != activePage)
			{
				this.rootPivot.SelectedItem = activePage;
			}
		}

		private async Task<PivotItem> AddRemotePage(Remote remote, PivotItem activePage)
		{
			var pivotItem = new PivotItem();
			var result = activePage;

			BuildUI(pivotItem, remote);

			if (remote == await RemotesManager.Instance.GetActiveRemote())
			{
				result = pivotItem;
			}

			this.rootPivot.Items.Add(pivotItem);

			return result;
		}

		private void BuildUI(PivotItem pivotItem, Remote remote)
		{
			pivotItem.Header = remote.Name;

			var grid = new Canvas();

			grid.Background = new SolidColorBrush(Colors.Transparent);

			pivotItem.Content = grid;
			pivotItem.Tag = remote;

			foreach (RemoteButton button in remote.Buttons)
			{
				Button controllButton = new Button();

				controllButton.Content = button.Title;

				SetLayout(controllButton, button);

				controllButton.Tag = button;

				controllButton.Click += this.RemoteButtonClick;

				grid.Children.Add(controllButton);
			}
		}

		private async void RemoteButtonClick(object sender, RoutedEventArgs e)
		{
			var button = (Button)sender;

			if ((this.EditButton.IsChecked != null) && (this.EditButton.IsChecked == true))
			{
				this.activeButton = button;
				this.buttonContextMenu.ShowAt(button);
			}
			else
			{
				await ((RemoteButton)button.Tag).SendCommand(await RemotesManager.Instance.GetActiveRemote());
			}
		}

		private void SetLayout(Button button, RemoteButton remoteButton)
		{
			button.Width = remoteButton.Width;
			button.Height = remoteButton.Height;

			Canvas.SetLeft(button, remoteButton.X);
			Canvas.SetTop(button, remoteButton.Y);
		}

		private void StartEditClick(object sender, RoutedEventArgs e)
		{
			var editing = (this.EditButton.IsChecked != null) && ((bool)this.EditButton.IsChecked);

			this.AddButtonButton.Visibility = editing ? Visibility.Visible : Visibility.Collapsed;
		}

		private void MoveButtonLeftClick(object sender, RoutedEventArgs e)
		{
			if (null != this.activeButton)
			{
				RemoteButton button = (RemoteButton)this.activeButton.Tag;

				button.X--;

				SetLayout(this.activeButton, button);
			}
		}

		private void MoveButtonDownClick(object sender, RoutedEventArgs e)
		{
			if (null != this.activeButton)
			{
				RemoteButton button = (RemoteButton)this.activeButton.Tag;

				button.Y++;

				SetLayout(this.activeButton, button);
			}
		}

		private void MoveButtonUpClick(object sender, RoutedEventArgs e)
		{
			if (null != this.activeButton)
			{
				RemoteButton button = (RemoteButton)this.activeButton.Tag;

				button.Y--;

				SetLayout(this.activeButton, button);
			}
		}

		private void MoveButtonRightClick(object sender, RoutedEventArgs e)
		{
			if (null != this.activeButton)
			{
				RemoteButton button = (RemoteButton)this.activeButton.Tag;

				button.X++;

				SetLayout(this.activeButton, button);
			}
		}

		private void EditRemoteClick(object sender, RoutedEventArgs e)
		{
			this.Frame.Navigate(typeof(EditRemotePage), ((PivotItem)this.rootPivot.SelectedItem).Tag);
		}

		private async void AddButtonClick(object sender, RoutedEventArgs e)
		{
			var pos = 100.0f * RemotesManager.Instance.GetScale();

			var button = new RemoteButton(pos, pos, pos, pos, "new b", false, 0);

			var grid = (Canvas)((PivotItem)this.rootPivot.SelectedItem).Content;

			Button controllButton = new Button();

			controllButton.Content = button.Title;

			SetLayout(controllButton, button);

			controllButton.Tag = button;

			controllButton.Click += this.RemoteButtonClick;

			grid.Children.Add(controllButton);

			var remote = await RemotesManager.Instance.GetActiveRemote();
			remote.Buttons.Add(button);
			await remote.Save();
		}
	}
}
