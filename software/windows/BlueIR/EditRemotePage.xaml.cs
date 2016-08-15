using System;
using Windows.Phone.UI.Input;
using Windows.System.Profile;
using Windows.UI.Xaml;
using Windows.UI.Xaml.Controls;
using Windows.UI.Xaml.Navigation;

namespace BlueIR
{
	public sealed partial class EditRemotePage : Page
	{
		private Remote remote = null;

		public EditRemotePage()
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

		private void BackClick(object sender, RoutedEventArgs e)
		{
			this.Frame.GoBack();
		}

		protected override void OnNavigatedTo(NavigationEventArgs e)
		{
			base.OnNavigatedTo(e);

			this.remote = (Remote)e.Parameter;
			this.DataContext = this.remote;
			this.comboBoxProtocol.SelectedIndex = (int)this.remote.Type;
			this.webView.ShowPage(this.remote.Type);
		}

		protected override void OnNavigatingFrom(NavigatingCancelEventArgs e)
		{
			base.OnNavigatingFrom(e);

			Save();
		}

		private async void Save()
		{
			this.remote.Name = this.textBoxName.Text;
			this.remote.Address = this.textBoxAddress.GetNumber();
			this.remote.Type = (IrProtocol)this.comboBoxProtocol.SelectedIndex;

			await this.remote.Save();
		}

		private void ProtocolSelectionChanged(object sender, SelectionChangedEventArgs e)
		{
			this.remote.Type = (IrProtocol)this.comboBoxProtocol.SelectedIndex;
			this.webView.ShowPage(this.remote.Type);
		}
	}
}
