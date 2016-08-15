using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using Windows.Storage;
using Windows.UI.Xaml;

namespace BlueIR
{
	public class RemotesManager
	{
		private static RemotesManager instance = new RemotesManager();
		private List<Remote> remotes = null;
		private const String activeRemoteKey = "ActiveRemote";
		private Remote activeRemote = null;

		public async Task<Remote> GetActiveRemote()
		{
			if (null == this.activeRemote)
			{
				String activeRemoteId = null;

				Object id;

				if (ApplicationData.Current.LocalSettings.Values.TryGetValue(activeRemoteKey, out id))
				{
					activeRemoteId = (String)id;
				}

				foreach (var remote in await this.GetRemotes())
				{
					if (remote.Id == activeRemoteId)
					{
						this.activeRemote = remote;

						break;
					}
				}
			}

			return this.activeRemote;
		}

		public void SetActiveRemote(Remote value)
		{
			this.activeRemote = value;

			if (null != this.activeRemote)
			{
				ApplicationData.Current.LocalSettings.Values[activeRemoteKey] = this.activeRemote.Id;
			}
			else
			{
				ApplicationData.Current.LocalSettings.Values[activeRemoteKey] = "";
			}
		}

		public static RemotesManager Instance
		{
			get
			{
				return RemotesManager.instance;
			}
		}

		private async Task<Remote> AddDefaultRemoteNec(String name)
		{
			Remote remote = new Remote();

			remote.Address = 62596;
			remote.Name = name;
			remote.Type = IrProtocol.NEC;

			int c = 0;

			float m = GetScale();

			remote.Buttons.Add(new RemoteButton(m * 8, (m * 14) + c, m * 95, m * 46, "Input", false, 10));
			remote.Buttons.Add(new RemoteButton(m * 218, (m * 14) + c, m * 95, m * 46, "Power", false, 11));
			remote.Buttons.Add(new RemoteButton(m * 123, (m * 154) + c, m * 74, m * 74, "Enter", true, 76));

			remote.Buttons.Add(new RemoteButton(m * 123, (m * 72) + c, m * 74, m * 74, "\u21e7", true, 74));
			remote.Buttons.Add(new RemoteButton(m * 123, (m * 236) + c, m * 74, m * 74, "\u21e9", true, 75));
			remote.Buttons.Add(new RemoteButton(m * 41, (m * 154) + c, m * 74, m * 74, "\u21e6", true, 77));
			remote.Buttons.Add(new RemoteButton(m * 205, (m * 154) + c, m * 74, m * 74, "\u21e8", true, 78));

			remote.Buttons.Add(new RemoteButton(m * 8, (m * 328) + c, m * 46, m * 46, "\u21e9", false, 47));
			remote.Buttons.Add(new RemoteButton(m * 70, (m * 328) + c, m * 46, m * 46, "\u21e7", false, 44));

			remote.Buttons.Add(new RemoteButton(m * 129, (m * 328) + c, m * 74, m * 46, "ESC", false, 14));
			remote.Buttons.Add(new RemoteButton(m * 218, (m * 328) + c, m * 94, m * 46, "Menu", false, 12));

			await remote.Save();

			return remote;
		}

		private async Task<Remote> AddDefaultRemoteRC5(String name)
		{
			Remote remote = new Remote();

			remote.Address = 1;
			remote.Name = name;
			remote.Type = IrProtocol.RC5;

			int c = 0;

			float m = GetScale();

			remote.Buttons.Add(new RemoteButton(m * 8, (m * 14) + c, m * 95, m * 46, "CD", false, 63, 20));
			remote.Buttons.Add(new RemoteButton(m * 218, (m * 14) + c, m * 95, m * 46, "Power", false, 12, 20));
			remote.Buttons.Add(new RemoteButton(m * 123, (m * 154) + c, m * 74, m * 74, "Play", true, 53, 20));

			remote.Buttons.Add(new RemoteButton(m * 41, (m * 154) + c, m * 74, m * 74, "\u21e6", true, 33, 20));
			remote.Buttons.Add(new RemoteButton(m * 205, (m * 154) + c, m * 74, m * 74, "\u21e8", true, 32, 20));

			remote.Buttons.Add(new RemoteButton(m * 8, (m * 328) + c, m * 46, m * 46, "\u21e9", false, 17, 16));
			remote.Buttons.Add(new RemoteButton(m * 70, (m * 328) + c, m * 46, m * 46, "\u21e7", false, 16, 16));

			remote.Buttons.Add(new RemoteButton(m * 129, (m * 328) + c, m * 74, m * 46, "Tape", false, 63, 18));
			remote.Buttons.Add(new RemoteButton(m * 218, (m * 328) + c, m * 94, m * 46, "Aux", false, 63, 21));

			await remote.Save();

			return remote;
		}

		private async Task<Remote> AddDefaultRemoteSirc(String name)
		{
			Remote remote = new Remote();

			remote.Address = 1;
			remote.Name = name;
			remote.Type = IrProtocol.SIRC;

			int c = 0;

			float m = GetScale();

			remote.Buttons.Add(new RemoteButton(m * 8, (m * 14) + c, m * 95, m * 46, "Source", false, 37));
			remote.Buttons.Add(new RemoteButton(m * 218, (m * 14) + c, m * 95, m * 46, "Power", false, 21));
			remote.Buttons.Add(new RemoteButton(m * 123, (m * 154) + c, m * 74, m * 74, "OK", true, 101));

			remote.Buttons.Add(new RemoteButton(m * 123, (m * 72) + c, m * 74, m * 74, "\u21e7", true, 116));
			remote.Buttons.Add(new RemoteButton(m * 123, (m * 236) + c, m * 74, m * 74, "\u21e9", true, 117));
			remote.Buttons.Add(new RemoteButton(m * 41, (m * 154) + c, m * 74, m * 74, "\u21e6", true, 52));
			remote.Buttons.Add(new RemoteButton(m * 205, (m * 154) + c, m * 74, m * 74, "\u21e8", true, 51));

			remote.Buttons.Add(new RemoteButton(m * 8, (m * 328) + c, m * 46, m * 46, "\u21e9", false, 19));
			remote.Buttons.Add(new RemoteButton(m * 70, (m * 328) + c, m * 46, m * 46, "\u21e7", false, 18));

			remote.Buttons.Add(new RemoteButton(m * 129, (m * 328) + c, m * 74, m * 46, "Back", false, 99));
			remote.Buttons.Add(new RemoteButton(m * 218, (m * 328) + c, m * 94, m * 46, "Home", false, 96));

			await remote.Save();

			return remote;
		}
		
		public async Task<Remote> AddNewRemote(String name)
		{
			var remote = await AddDefaultRemoteSirc(name);

			(await GetRemotes()).Add(remote);

			SetActiveRemote(remote);

			return remote;
		}

		public float GetScale()
		{
			var width = Window.Current.Bounds.Width; //size.x;

			return (float)width / 350.0f;
		}

		/// <summary>
		/// Loads saved remotes from isolated storage.
		/// </summary>
		/// <returns>A list of loaded remotes or default ones if no remotes were stored before.</returns>
		public async Task<List<Remote>> GetRemotes()
		{
			if (null != this.remotes)
			{
				return this.remotes;
			}

			DataContractSerializer serializer = new DataContractSerializer(typeof(Remote));

			List<Remote> result = new List<Remote>();

			foreach (var file in await ApplicationData.Current.LocalFolder.GetFilesAsync())
			{
				if (file.Name.EndsWith(".blah"))
				{
					using (var inStream = await file.OpenSequentialReadAsync())
					{
						var remote = (Remote)serializer.ReadObject(inStream.AsStreamForRead());

						result.Add(remote);
					}
				}
			}

			if (result.Count <= 0)
			{
				result.Add(await AddDefaultRemoteSirc("demo sony telly"));
				result.Add(await AddDefaultRemoteNec("demo casio projector"));
				result.Add(await AddDefaultRemoteRC5("demo philips hifi"));
			}

			this.remotes = result;

			return result;
		}
	}
}
