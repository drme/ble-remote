using System;
using System.Threading.Tasks;
using Windows.UI.Popups;
using Windows.UI.Xaml.Controls;

namespace BlueIR
{
	static class Extensions
	{
		public static int GetNumber(this TextBox field)
		{
			String value = field.Text;

			if (value.Length > 0)
			{
				try
				{
					return int.Parse(value);
				}
				catch (Exception)
				{
					return 0;
				}
			}

			return 0;
		}

		public static async Task<bool> ShowRemove(this Page page, String title, String message, Object item)
		{
			if (null != item)
			{
				var dialog = new MessageDialog(String.Format(message, item));

				dialog.Commands.Add(new UICommand("Yes") { Id = item });
				dialog.Commands.Add(new UICommand("No") { Id = null });

				dialog.DefaultCommandIndex = 1;
				dialog.CancelCommandIndex = 1;

				var result = await dialog.ShowAsync();

				if (result.Id != null)
				{
					return true;
				}
			}

			return false;
		}

		public static void ShowPage(this WebView web, IrProtocol protocol)
		{
			switch (protocol)
			{
				case IrProtocol.SIRC:
					web.Source = new Uri("https://www.google.com/search?q=sirc+remote+addresses");
					break;
				case IrProtocol.NEC:
					web.Source = new Uri("https://www.google.com/search?q=nec+remote+addresses");
					break;
				default:
					web.Source = new Uri("https://en.wikipedia.org/wiki/RC-5#System_Number_Allocations");
					break;
			}
		}
	}
}
