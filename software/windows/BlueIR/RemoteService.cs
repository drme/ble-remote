using System;
using System.Diagnostics;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;
using Windows.Devices.Enumeration.Pnp;
using Windows.Storage.Streams;

namespace BlueIR
{
	class RemoteService
	{
		public event EventHandler<RemoteService> Connected;
		public event EventHandler<RemoteService> Disconnected;
		private DeviceInformation device = null;
		private GattCharacteristic command = null;
		private GattCharacteristic address = null;
		private GattCharacteristic type = null;
		private GattDeviceService service = null;
		private PnpObjectWatcher watcher = null;
		private String deviceContainerId = null;

		public RemoteService(DeviceInformation device)
		{
			this.device = device;
		}

		public async void Connect()
		{
			try
			{
				StartDeviceConnectionWatcher();

				this.deviceContainerId = "{" + device.Properties["System.Devices.ContainerId"] + "}";

				this.service = await GattDeviceService.FromIdAsync(device.Id);

				if (this.service != null)
				{
					this.address = GetCharacteristicObject("a739" + "eee2" + "-f6cd-1692-994a-d66d9e0ce048");
					this.type = GetCharacteristicObject("a739" + "eee0" + "-f6cd-1692-994a-d66d9e0ce048");
					this.command = GetCharacteristicObject("a739" + "eee1" + "-f6cd-1692-994a-d66d9e0ce048");

					if (null != this.Connected)
					{
						this.Connected(this, this);
					}
				}
				else
				{
					Debug.WriteLine("Access to the device is denied, because the application was not granted access or the device is currently in use by another application.");

					if (null != this.Disconnected)
					{
						this.Disconnected(this, this);
					}
				}
			}
			catch (Exception ex)
			{
				Debug.WriteLine(ex.StackTrace.ToString());

				Debug.WriteLine("ERROR: Accessing your device failed." + Environment.NewLine + ex.Message);

				if (null != this.Disconnected)
				{
					this.Disconnected(this, this);
				}
			}
		}

		private GattCharacteristic GetCharacteristicObject(String id)
		{
			var characteristics = this.service.GetCharacteristics(Guid.Parse(id));

			if ((null != characteristics) && (characteristics.Count > 0))
			{
				return characteristics[0];
			}

			return null;
		}

		public void Disconnect()
		{
			if (null != this.service)
			{
				this.service = null;
			}

			if (null != this.Disconnected)
			{
				this.Disconnected(this, this);
			}
		}

		private void StartDeviceConnectionWatcher()
		{
			this.watcher = PnpObject.CreateWatcher(PnpObjectType.DeviceContainer, new string[] { "System.Devices.Connected" }, String.Empty);
			this.watcher.Updated += (PnpObjectWatcher sender, PnpObjectUpdate args) =>
			{
				var connectedProperty = args.Properties["System.Devices.Connected"];

				bool isConnected = false;

				if ((deviceContainerId == args.Id) && Boolean.TryParse(connectedProperty.ToString(), out isConnected) && isConnected)
				{
					//		this.watcher.Stop();
					//		this.watcher = null;

					if (true == isConnected)
					{
						if (null != this.Connected)
						{
							this.Connected(this, this);
						}
					}
					else
					{
						if (null != this.Disconnected)
						{
							this.Disconnected(this, this);
						}
					}
				}
			};

			this.watcher.Start();
		}

		private async Task SendData(int value, GattCharacteristic characteristic)
		{
			if (null != characteristic)
			{
				var writer = new DataWriter();

				if (value <= 0xff)
				{
					writer.WriteByte((byte)value);
				}
				else if (value <= 0xffff)
				{
					byte loByte = (byte)(value & 0xff);
					byte hiByte = (byte)(value >> 8);

					writer.WriteBytes(new byte[] { loByte, hiByte });
				}

				var status = await characteristic.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithResponse);

				System.Diagnostics.Debug.WriteLine(status);
			}
		}

		public async Task SendProtocol(IrProtocol? protocol)
		{
			await SendData((int)(IrProtocol)protocol, this.type);
		}

		public async Task SendAddress(int? address)
		{
			await SendData((int)address, this.address);
		}

		public async Task SendCommand(int command)
		{
			await SendData(command, this.command);
		}
	}
}
