using System;
using System.Threading.Tasks;
using Windows.Phone.UI.Input;
using Windows.System.Profile;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace BlueIR
{
	public sealed partial class EditButtonPage : Page
	{
		private RemoteButton button = null;
		private Remote remote = null;

		public EditButtonPage()
		{
			this.InitializeComponent();

			try
			{
				var phone = ("Windows.Mobile" == AnalyticsInfo.VersionInfo.DeviceFamily);

				if (phone)
				{
					HardwareButtons.BackPressed += BackPressed;
				}
			}
			catch (Exception)
			{
			}
		}

		private void BackPressed(object sender, BackPressedEventArgs e)
		{
			e.Handled = true;

			this.Frame.GoBack();
		}

		protected async override void OnNavigatedTo(NavigationEventArgs e)
		{
			base.OnNavigatedTo(e);

			this.button = (RemoteButton)e.Parameter;
			this.remote = await RemotesManager.Instance.GetActiveRemote();
			this.DataContext = this.button;

			if (null == this.button.Address)
			{
				this.textBoxAddress.Text = this.remote.Address.ToString();
			}

			if (null == this.button.Type)
			{
				this.comboBoxProtocol.SelectedIndex = (int)this.remote.Type;
				this.webView.ShowPage(this.remote.Type);
			}
			else
			{
				this.comboBoxProtocol.SelectedIndex = (int)this.button.Type;
				this.webView.ShowPage((IrProtocol)this.button.Type);
			}
		}

		private void BackClick(object sender, RoutedEventArgs e)
		{
			this.Frame.GoBack();
		}

		protected async override void OnNavigatingFrom(NavigatingCancelEventArgs e)
		{
			base.OnNavigatingFrom(e);

			await Save();
		}

		private async Task Save()
		{
			this.button.Title = this.textBoxName.Text;
			this.button.Command = this.textBoxCommand.GetNumber();
			this.button.X = this.textBoxX.GetNumber();
			this.button.Y = this.textBoxY.GetNumber();
			this.button.Width = this.textBoxWidth.GetNumber();
			this.button.Height = this.textBoxHeight.GetNumber();

			int address = this.textBoxAddress.GetNumber();

			if (address == this.remote.Address)
			{
				this.button.Address = null;
			}
			else
			{
				this.button.Address = address;
			}

			IrProtocol protocol = (IrProtocol)this.comboBoxProtocol.SelectedIndex;

			if (protocol == this.remote.Type)
			{
				this.button.Type = null;
			}
			else
			{
				this.button.Type = protocol;
			}

			await this.remote.Save();
		}

		private void ProtocolSelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			this.webView.ShowPage((IrProtocol)this.comboBoxProtocol.SelectedIndex);
		}

		private async void TestClick(object sender, RoutedEventArgs e)
		{
			await Save();

			await this.button.SendCommand(this.remote);
		}
	}
}
