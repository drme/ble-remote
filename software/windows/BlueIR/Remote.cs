using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.Serialization;
using System.Threading.Tasks;
using Windows.Storage;

namespace BlueIR
{
	/// <summary>
	/// Stores remote configuration.
	/// </summary>
	public class Remote
	{
		public int Address
		{
			get;
			set;
		}

		public List<RemoteButton> Buttons
		{
			get;
			set;
		}

		public String Id
		{
			get;
			set;
		}

		public String Name
		{
			get;
			set;
		}

		public IrProtocol Type
		{
			get;
			set;
		}

		public Remote()
		{
			this.Id = Guid.NewGuid().ToString();
			this.Address = 0;
			this.Buttons = new List<RemoteButton>();
			this.Name = "";
			this.Type = IrProtocol.SIRC;
		}

		public async void Delete()
		{
			try
			{
				StorageFile file = await ApplicationData.Current.LocalFolder.GetFileAsync(this.Id + ".blah");

				await file.DeleteAsync(StorageDeleteOption.PermanentDelete);

				System.Diagnostics.Debug.WriteLine("Remove: " + this.Id + " " + this.Name);
			}
			catch (Exception ex)
			{
				System.Diagnostics.Debug.WriteLine("Failed to remove: " + this.Id + " " + this.Name + " " + ex.Message);
			}
		}

		public async Task Save()
		{
			try
			{
				using (MemoryStream sessionData = new MemoryStream())
				{
					DataContractSerializer serializer = new DataContractSerializer(typeof(Remote));
					serializer.WriteObject(sessionData, this);

					StorageFile file = await ApplicationData.Current.LocalFolder.CreateFileAsync(this.Id + ".blah", CreationCollisionOption.ReplaceExisting);

					using (Stream fileStream = await file.OpenStreamForWriteAsync())
					{
						sessionData.Seek(0, SeekOrigin.Begin);
						await sessionData.CopyToAsync(fileStream);
						await fileStream.FlushAsync();
					}
				}
			}
			catch (Exception ex)
			{
				System.Diagnostics.Debug.WriteLine(ex.Message);
			}
		}
	}
}
