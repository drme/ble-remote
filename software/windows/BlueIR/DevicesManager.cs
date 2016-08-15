using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.Advertisement;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Devices.Enumeration;

namespace BlueIR
{
	class DevicesManager
	{
		private static Guid serviceUuid = Guid.Parse("a739eeee-f6cd-1692-994a-d66d9e0ce048");
		private static DevicesManager instance = new DevicesManager();
		public event EventHandler<DevicesManager> Connected;
		public event EventHandler<DevicesManager> Disconnected;
		private bool scanning = true;
		private BluetoothLEAdvertisementWatcher watcher;
		private HashSet<ulong> advertisedDevices = new HashSet<ulong>();

		public static DevicesManager Instance
		{
			get
			{
				return instance;
			}
		}

		public RemoteService RemoteService
		{
			get;
			private set;
		}

		public void StartScan()
		{
			this.scanning = true;

			if (null == this.RemoteService)
			{
				StartScanAsync();
			}
		}

		private async void StartScanAsync()
		{
			if (false == this.scanning)
			{
				return;
			}

			var devices = await DeviceInformation.FindAllAsync(GattDeviceService.GetDeviceSelectorFromUuid(serviceUuid), new string[] { "System.Devices.ContainerId" });

			if (devices.Count > 0)
			{
				var remoteService = new RemoteService(devices[0]);

				remoteService.Disconnected += RemoteServiceDisconnected;
				remoteService.Connected += RemoteServiceConnected;

				remoteService.Connect();
			}
			else
			{
				await Task.Delay(100);

				StartWatcher();
				StartScanAsync();
			}
		}

		public void StopScan()
		{
			if (null != this.watcher)
			{
				this.watcher.Stop();
			}

			this.scanning = false;
		}

		private void RemoteServiceConnected(object sender, RemoteService service)
		{
			this.RemoteService = service;

			if (null != this.watcher)
			{
				this.watcher.Stop();
			}

			if (null != this.Connected)
			{
				this.Connected(this, this);
			}
		}

		private void RemoteServiceDisconnected(object sender, RemoteService service)
		{
			if (service == this.RemoteService)
			{
				this.RemoteService = null;

				StartScanAsync();

				if (null != this.Disconnected)
				{
					this.Disconnected(this, this);
				}
			}
		}

		private void StartWatcher()
		{
			this.watcher = new BluetoothLEAdvertisementWatcher();
			this.watcher.AdvertisementFilter.Advertisement.ServiceUuids.Add(serviceUuid);
			this.watcher.ScanningMode = BluetoothLEScanningMode.Active;
			this.watcher.Received += DeviceDiscovered;
			this.watcher.Start();
		}

		private async void DeviceDiscovered(BluetoothLEAdvertisementWatcher sender, BluetoothLEAdvertisementReceivedEventArgs advertisement)
		{
			var isNew = false;

			lock (this)
			{
				if (false == advertisedDevices.Contains(advertisement.BluetoothAddress))
				{
					this.advertisedDevices.Add(advertisement.BluetoothAddress);
					isNew = true;
				}
			}

			if (true == isNew)
			{
				var device = await BluetoothLEDevice.FromBluetoothAddressAsync(advertisement.BluetoothAddress);

				DevicePairingKinds ceremoniesSelected = DevicePairingKinds.ConfirmOnly;
				DevicePairingProtectionLevel protectionLevel = DevicePairingProtectionLevel.None;
				DeviceInformationCustomPairing customPairing = device.DeviceInformation.Pairing.Custom;

				customPairing.PairingRequested += PairingRequestedHandler;
				DevicePairingResult result = await customPairing.PairAsync(ceremoniesSelected, protectionLevel);
				customPairing.PairingRequested -= PairingRequestedHandler;

				if ((result.Status == DevicePairingResultStatus.AlreadyPaired) || (result.Status == DevicePairingResultStatus.Paired))
				{
					sender.Stop();
				}
			}
		}

		private void PairingRequestedHandler(DeviceInformationCustomPairing sender, DevicePairingRequestedEventArgs arguments)
		{
			switch (arguments.PairingKind)
			{
				case DevicePairingKinds.ConfirmOnly:
					arguments.Accept();
					break;
				case DevicePairingKinds.ProvidePin:
					var collectPinDeferral = arguments.GetDeferral();
					string pin = "000000";
					arguments.Accept(pin);
					collectPinDeferral.Complete();
					break;
			}
		}
	}
}
