using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Text;
using System.Threading.Tasks;
using Windows.Devices.WiFi;
using Windows.Networking;
using Windows.Networking.Connectivity;
using Windows.Networking.Sockets;
using Windows.Security.Cryptography;
using Windows.Storage.Streams;

namespace BlueIR.EspTouch
{
	class Task : IDisposable
	{
		/// <summary>
		/// When new esptouch result is added, the listener will call onEsptouchResultAdded callback.
		/// </summary>
		/// <param name="result">the Esptouch result.</param>
		public delegate void DeviceFoundHandler(Result result);

		/// <summary>
		/// EsptouchDelegate when one device is connected to the Ap, it will be called back.
		/// </summary>
		public event DeviceFoundHandler DeviceFound;

		public bool IsCancelled
		{
			get;
			set;
		}

		private String ssid;
		private String bssid;
		private String password;
		private bool success = false;
		private bool stop = false;
		private UdpSocketClient udpSocketClient = new UdpSocketClient();
		private UdpSocketServer udpSocketserver = new UdpSocketServer();
		private List<Result> results = new List<Result>();
		private bool isExecutedAlready = false;
		private bool hidden;
		private TaskParameters parameters = new TaskParameters();
		private Dictionary<String, int> discoveredBssids = new Dictionary<string, int>();
		private byte[] localInetAddressData;
		private const int oneDataLength = 3;
		private String ssidAndPassword;
		private byte expectOneByte;

		/// <summary>
		/// Constructor of EsptouchTask. 
		/// </summary>
		/// <param name="ssid">the Ap's ssid.</param>
		/// <param name="bssid">the Ap's bssid.</param>
		/// <param name="password">the Ap's password.</param>
		/// <param name="isHidden">whether the Ap's ssid is hidden.</param>
		/// <param name="timeout">(it should be >= 15000+6000) millisecond of total timeout.</param>
		public Task(String ssid, String bssid, String password, bool isHidden, TimeSpan? timeout = null)
		{
			if ((ssid == null) || (ssid == ""))
			{
				throw new Exception("ESPTouchTask initWithApSsid() apSsid shouldn't be null or empty");
			}

			if (password == null)
			{
				password = "";
			}

			Debug.WriteLine("ESPTouchTask init");

			this.ssid = ssid;
			this.password = password;
			this.bssid = bssid;

			// check whether IPv4 and IPv6 is supported
			var localInetAddress4 = NetUtil.LocalIPv4;

			if (!NetUtil.IsIPv4PrivateAddress(localInetAddress4))
			{
				localInetAddress4 = null;
			}

			var localInetAddr6 = NetUtil.LocalIPv6;

			this.parameters.IsIPv4Supported = localInetAddress4 != null;
			this.parameters.IsIPv6Supported = localInetAddr6 != null;

			// update listening port for IPv6
			//this.parameters.ListeningPort6 = this._server.port;

			//Debug.WriteLine("ESPTouchTask app server port is {0}", this._server.port);

			if (localInetAddress4 != null)
			{
				this.localInetAddressData = localInetAddress4.GetLocalIpV4AddressBytes();
			}
			else
			{
				int localPort = this.parameters.ListeningPort;
				this.localInetAddressData = NetUtil.GetLocalInetAddress6ByPort(localPort);
			}

			// for ESPTouchGenerator only receive 4 bytes for local address no matter IPv4 or IPv6
			Debug.WriteLine("ESPTouchTask executeForResult() localInetAddr: {0}", NetUtil.FormatIpv4Address(this.localInetAddressData));

			this.hidden = isHidden;

			if (null != timeout)
			{
				this.parameters.TotalUdpWait = (TimeSpan)timeout;
			}
		}

		public void Dispose()
		{
			Stop();
			this.udpSocketserver.Dispose();
		}

		/// <summary>
		/// Interrupt the Esptouch Task when User tap back or close the Application.
		/// </summary>
		public void Interrupt()
		{
			Debug.WriteLine("ESPTouchTask interrupt()");

			this.IsCancelled = true;

			Stop();
		}

		/// <summary>
		/// Note: !!!Don't call the task at UI Main Thread 
		/// Smart Config v2.4 support the API
		/// </summary>
		/// <returns>the ESPTouchResult</returns>
		public async Task<Result> ExecuteForResult()
		{
			return (await ExecuteForResults(1))[0];
		}

		/// <summary>
		/// Note: !!!Don't call the task at UI Main Thread
		///
		/// Smart Config v2.4 support the API
		///
		/// It will be blocked until the client receive result count >= expectTaskResultCount.
		/// If it fail, it will return one fail result will be returned in the list.
		/// If it is cancelled while executing,
		/// if it has received some results, all of them will be returned in the list.
		/// if it hasn't received any results, one cancel result will be returned in the list.
		/// </summary>
		/// <param name="expectTaskResultCount">the expect result count(if expectTaskResultCount <= 0, expectTaskResultCount = INT32_MAX</param>
		/// <returns></returns>
		public async Task<List<Result>> ExecuteForResults(int expectTaskResultCount = 0)
		{
			// set task result count
			if (expectTaskResultCount <= 0)
			{
				expectTaskResultCount = int.MaxValue;
			}

			this.parameters.ExpectedTaskResultsCount = expectTaskResultCount;

			CheckIfTaskIsValid();

			// generator the esptouch byte[][] to be transformed, which will cost
			// some time(maybe a bit much)
			Generator generator = new Generator(this.ssid, this.bssid, this.password, this.localInetAddressData, this.hidden);

			await this.udpSocketserver.StartListening(this.parameters.ListeningPort);

			// listen the esptouch result asyn
			ListenForResults(this.parameters.ResultTotalLength);

			for (int i = 0; i < this.parameters.TotalRepeatCount; i++)
			{
				await SendData(generator);

				if (this.success)
				{
					Stop();

					return this.Results;
				}
			}

			if (!this.stop)
			{
				WaitForTimeOut(this.parameters.UdpReceivingWaitTime);
				Stop();
			}

			return this.Results;
		}

		private void CheckIfTaskIsValid()
		{
			if (this.isExecutedAlready)
			{
				throw new Exception("ESPTouchTask __checkTaskValid() fail, the task could be executed only once");
			}

			// !!!NOTE: the esptouch task could be executed only once
			Debug.Assert(!this.isExecutedAlready);

			this.isExecutedAlready = true;
		}

		private void ListenForResults(int expectedDataLength)
		{
			Debug.WriteLine("ESPTouchTask __listenAsyn() start an asyn listen task, current thread is: {0}", -1);

			this.ssidAndPassword = String.Format("{0}{1}", this.ssid, this.password);
			this.expectOneByte = (byte)(ByteUtil.GetBytes(this.ssidAndPassword).Length + 9);

			Debug.WriteLine("ESPTouchTask __listenAsyn() expectOneByte: {0}", expectOneByte);

			this.udpSocketserver.ExpectedMessageLength = expectedDataLength;
			this.udpSocketserver.MessageReceived += this.UdpMessageReceived;
		}

		private void UdpMessageReceived(byte[] receiveData)
		{
			if (this.results.Count < this.parameters.ExpectedTaskResultsCount && !this.stop)
			{
				byte receiveOneByte = ((receiveData != null) && (receiveData.Length > 0)) ? receiveData[0] : (byte)0xff; //(-1);

				if (receiveOneByte == expectOneByte)
				{
					Debug.WriteLine("ESPTouchTask __listenAsyn() receive correct broadcast");

					if (receiveData != null)
					{
						var bssid = ByteUtil.ParseBssid(receiveData, this.parameters.ResultOneLength, this.parameters.ResultMacLength);
						var inetAddrData = NetUtil.ParseAddress(receiveData, this.parameters.ResultOneLength + this.parameters.ResultMacLength, this.parameters.ResultIpLength);

						AddResult(true, bssid, inetAddrData);
					}
				}
			}

			this.success = this.results.Count >= this.parameters.ExpectedTaskResultsCount;

			if (this.success)
			{
				Stop();

				Debug.WriteLine("ESPTouchTask __listenAsyn() finish");
			}
		}

		private async Task<bool> SendData(Generator generator)
		{
			var startTime = DateTime.Now;
			var currentTime = startTime;
			var lastTime = currentTime - this.parameters.TotalCodeTimeout;

			var guideCode = generator.GuideCode;
			var datumCode = generator.DatumCode;

			int index = 0;

			while (!this.stop)
			{
				if (currentTime - lastTime >= this.parameters.TotalCodeTimeout)
				{
					Debug.WriteLine("ESPTouchTask __execute() send gc code ");

					// send guide code
					while (!this.stop && (DateTime.Now - currentTime) < this.parameters.GuideCodeTimeout)
					{
						await this.udpSocketClient.SendData(guideCode, this.parameters.GetTargetHostname(), this.parameters.TargetPort, this.parameters.GuideCodeInterval);

						// check whether the udp is send enough time
						if (DateTime.Now - startTime > this.parameters.UdpSendingWaitTime)
						{
							break;
						}
					}

					lastTime = currentTime;
				}
				else
				{
					Debug.WriteLine("ESPTouchTask __execute() send dc code ");

					await this.udpSocketClient.SendData(datumCode, index, oneDataLength, this.parameters.GetTargetHostname(), this.parameters.TargetPort, this.parameters.DataCodeInterval);
					index = (index + oneDataLength) % datumCode.Count;
				}

				currentTime = DateTime.Now;

				// check whether the udp is send enough time
				if (DateTime.Now - startTime > this.parameters.UdpSendingWaitTime)
				{
					break;
				}
			}

			return true;
		}

		private List<Result> Results
		{
			get
			{
				lock (this)
				{
					if (this.results.Count == 0)
					{
						this.results.Add(new Result(false, null, null, this.IsCancelled));
					}

					return this.results;
				}
			}
		}

		private void Stop()
		{
			this.stop = true;

			this.udpSocketClient.Interrupt();
			this.udpSocketserver.Interrupt();
		}

		/// <summary>
		/// Sleep some milliseconds.
		/// </summary>
		/// <param name="waitTime"></param>
		/// <returns></returns>
		private async void WaitForTimeOut(TimeSpan waitTime)
		{
			Debug.WriteLine("ESPTouchTask __sleep() start");

			var sleepTime = TimeSpan.FromMilliseconds(100);
			var waited = TimeSpan.Zero;

			while ((waited < waitTime) && (false == this.stop))
			{
				await System.Threading.Tasks.Task.Delay(sleepTime);
				waited += sleepTime;
			}

			Debug.WriteLine("ESPTouchTask __sleep() end, receive signal is {0}", !this.stop ? "YES" : "NO");
		}

		private void AddResult(bool success, String bssid, HostName address)
		{
			lock (this)
			{
				// check whether the result receive enough UDP response
				int count = 0;

				if (false == this.discoveredBssids.TryGetValue(bssid, out count))
				{
					count = 0;
				}

				count++;

				Debug.WriteLine("ESPTouchTask __putEsptouchResult(): count = %d", count);

				this.discoveredBssids[bssid] = count;

				if (count < this.parameters.SuccessfullBroadcastsCountThreshold)
				{
					Debug.WriteLine("ESPTouchTask __putEsptouchResult(): count = %d, isn't enough", count);

					return;
				}

				// check whether the result is in the mEsptouchResultList already
				bool alreadyAdded = false;

				foreach (var result in this.results)
				{
					if (result.BSSID == bssid)
					{
						alreadyAdded = true;
						break;
					}
				}

				// only add the result who isn't in the mEsptouchResultList
				if (!alreadyAdded)
				{
					Debug.WriteLine("ESPTouchTask __putEsptouchResult(): put one more result");

					var result = new Result(success, bssid, address);
					this.results.Add(result);

					if (null != this.DeviceFound)
					{
						this.DeviceFound(result);
					}
				}
			}
		}
	}

	class Result
	{
		/// <summary>
		/// It is used to check whether the esptouch task is executed successfully.
		/// </summary>
		public bool IsSuccessfull
		{
			get;
			private set;
		}

		/// <summary>
		/// It is used to store the device's bssid.
		/// </summary>
		public String BSSID
		{
			get;
			private set;
		}

		/// <summary>
		/// It is used to check whether the esptouch task is cancelled by user.
		/// </summary>
		public bool IsCancelled
		{
			get;
			private set;
		}

		/// <summary>
		/// It is used to store the devices' ip address.
		/// </summary>
		public HostName HostName
		{
			get;
			private set;
		}

		/// <summary>
		/// Constructor of EsptouchResult.
		/// </summary>
		/// <param name="isSu">whether the esptouch task is executed suc.</param>
		/// <param name="bssid">the device's bssid.</param>
		/// <param name="hostName">the device's ip address.</param>
		public Result(bool isSuccessfull, String bssid, HostName hostName, bool canceled = false)
		{
			this.IsSuccessfull = isSuccessfull;
			this.BSSID = bssid;
			this.IsCancelled = canceled;
			this.HostName = hostName;
		}

		public override string ToString()
		{
			return String.Format("Sucessfull: {0}, Cancelled: {1}, bssid: {2}, hostname: {3}]", this.IsSuccessfull, this.IsCancelled, this.BSSID, this.HostName.DisplayName);
		}
	}

	/// <summary>
	/// one data format:(data code should have 2 to 65 data)
	///
	/// control byte high 4 bits low 4 bits
	/// 1st 9bits:       0x0             crc(high)      data(high)
	/// 2nd 9bits:       0x1                sequence header
	/// 3rd 9bits:       0x0             crc(low)       data(low)
	///
	/// sequence header: 0,1,2,...
	/// </summary>
	class DataCode
	{
		private const int maxIndex = 127;
		public const int Length = 6;
		private byte sequenceHeader;
		private byte dataHigh;
		private byte dataLow;
		private byte crcHigh;
		private byte crcLow;

		public DataCode(byte value, int index)
		{
			if (index > maxIndex)
			{
				throw new Exception("index > INDEX_MAX");
			}

			var data = ByteUtil.SplitTo2Bytes(value);
			this.dataHigh = data[0];
			this.dataLow = data[1];

			var crc = new Crc8();
			crc.UpdateWithValue(value);
			crc.UpdateWithValue(index);

			var crcData = ByteUtil.SplitTo2Bytes(crc.Value);

			this.crcHigh = crcData[0];
			this.crcLow = crcData[1];

			this.sequenceHeader = (byte)index;
		}

		public byte[] Values
		{
			get
			{
				byte[] bytes = new byte[Length];

				bytes[0] = 0x00;
				bytes[1] = ByteUtil.CombineToOne(crcHigh, dataHigh);
				bytes[2] = 0x01;
				bytes[3] = sequenceHeader;
				bytes[4] = 0x00;
				bytes[5] = ByteUtil.CombineToOne(crcLow, dataLow);

				return bytes;
			}
		}

		public override string ToString()
		{
			return CryptographicBuffer.EncodeToHexString(this.Values.AsBuffer());
		}
	}

	class GuideCode
	{
		private const int length = 4;

		public UInt16[] Values
		{
			get
			{
				UInt16[] values = new UInt16[length];

				values[0] = 515;
				values[1] = 514;
				values[2] = 513;
				values[3] = 512;

				return values;
			}
		}

		public override string ToString()
		{
			return CryptographicBuffer.EncodeToHexString(this.Values.AsBuffer());
		}
	}

	class Crc8
	{
		private const ushort CRC_POLYNOM = 0x8c;
		private const ushort CRC_INITIAL = 0x00;
		private static ushort[] crcTable = null;
		private int value;

		public Crc8()
		{
			Initialize();
			Reset();
		}

		public byte Value
		{
			get
			{
				return (byte)(this.value & 0xff);
			}
		}

		public void Reset()
		{
			this.value = CRC_INITIAL;
		}

		private void UpdateWithBuf(Byte[] buffer, int offset, int count)
		{
			for (int i = 0; i < count; i++)
			{
				int data = buffer[offset + i] ^ this.value;

				this.value = crcTable[data & 0xff] ^ (this.value << 8);
			}
		}

		public void UpdateWithBuf(Byte[] buffer, int count)
		{
			UpdateWithBuf(buffer, 0, count);
		}

		public void UpdateWithValue(int value)
		{
			UpdateWithBuf(new byte[] { (Byte)value }, 1);
		}

		private static void Initialize()
		{
			if (null == crcTable)
			{
				ushort[] table = new ushort[256];

				for (ushort dividend = 0; dividend < 256; dividend++)
				{
					ushort remainder = dividend;

					for (ushort bit = 0; bit < 8; ++bit)
					{
						if ((remainder & 0x01) != 0)
						{
							remainder = (ushort)((remainder >> 1) ^ CRC_POLYNOM);
						}
						else
						{
							remainder >>= 1;
						}

						table[dividend] = (ushort)remainder;
					}
				}

				crcTable = table;
			}
		}
	}

	class DatumCode
	{
		/// <summary>
		/// Define by the Esptouch protocol, all of the datum code should add EXTRA_LEN to prevent 0.
		/// </summary>
		private const int extraLength = 40;
		private const int extraHeaderLength = 5;
		private List<DataCode> dataCodes;

		/// <summary>
		/// Constructor of DatumCode. 
		/// </summary>
		/// <param name="ssid">the Ap's ssid.</param>
		/// <param name="bssid">the Ap's bssid.</param>
		/// <param name="password">the Ap's password ssid.</param>
		/// <param name="ipAddress">the ip address of the phone or pad.</param>
		/// <param name="hidden">whether the Ap's ssid is hidden.</param>
		public DatumCode(String ssid, String bssid, String password, byte[] ipAddress, bool hidden)
		{
			// Data = total len(1 byte) + apPwd len(1 byte) + SSID CRC(1 byte) +
			// BSSID CRC(1 byte) + TOTAL XOR(1 byte) + ipAddress(4 byte) + apPwd + apSsid apPwdLen <=
			// 105 at the moment

			int totalXor = 0;

			var passwordBytes = ByteUtil.GetBytes(password);
			var ssidBytes = ByteUtil.GetBytes(ssid);
			var passwordLength = (byte)passwordBytes.Length;
			var ssidCrc = ssidBytes.GetCrc8();
			var bssidBytes = NetUtil.ParseBssid(bssid);
			var bssidCrc = bssidBytes.GetCrc8();
			var ssidLength = (byte)ssidBytes.Length;

			// only support ipv4 at the moment
			var ipAddressLength = (byte)ipAddress.Length;

			var _totalLen = (byte)(extraHeaderLength + ipAddressLength + passwordLength + ssidLength);
			var totalLen = (byte)(hidden ? (extraHeaderLength + ipAddressLength + passwordLength + ssidLength) : (extraHeaderLength + ipAddressLength + passwordLength));

			// build data codes
			this.dataCodes = new List<DataCode>(totalLen);

			this.dataCodes.Add(new DataCode(_totalLen, 0));
			totalXor ^= _totalLen;

			this.dataCodes.Add(new DataCode(passwordLength, 1));
			totalXor ^= passwordLength;

			this.dataCodes.Add(new DataCode(ssidCrc, 2));
			totalXor ^= ssidCrc;

			this.dataCodes.Add(new DataCode(bssidCrc, 3));
			totalXor ^= bssidCrc;

			// ESPDataCode 4 is nil
			for (int i = 0; i < ipAddressLength; i++)
			{
				this.dataCodes.Add(new DataCode(ipAddress[i], i + extraHeaderLength));
				totalXor ^= ipAddress[i];
			}

			for (int i = 0; i < passwordLength; i++)
			{
				this.dataCodes.Add(new DataCode(passwordBytes[i], i + extraHeaderLength + ipAddressLength));
				totalXor ^= passwordBytes[i];
			}

			// totalXor will xor apSsidChars no matter whether the ssid is hidden
			for (int i = 0; i < ssidLength; i++)
			{
				totalXor ^= ssidBytes[i];
			}

			if (hidden)
			{
				for (int i = 0; i < ssidLength; i++)
				{
					this.dataCodes.Add(new DataCode(ssidBytes[i], i + extraHeaderLength + ipAddressLength + passwordLength));
				}
			}

			// add total xor last
			this.dataCodes.Insert(4, new DataCode((byte)totalXor, 4));
		}

		private byte[] GetBytes()
		{
			var datumCode = new byte[dataCodes.Count * DataCode.Length];

			for (int i = 0; i < dataCodes.Count; i++)
			{
				DataCode dataCode = dataCodes[i];

				byte[] bytes = dataCode.Values;

				int dest = i * DataCode.Length * sizeof(byte);
				int src = 0;

				Array.Copy(bytes, src, datumCode, dest, DataCode.Length);
			}

			return datumCode;
		}

		public UInt16[] Values
		{
			get
			{
				var dataBytes = GetBytes();
				var resultLength = dataBytes.Length / 2;
				var result = new UInt16[resultLength];

				for (int i = 0; i < resultLength; i++)
				{
					byte high = dataBytes[i * 2];
					byte low = dataBytes[i * 2 + 1];

					result[i] = (UInt16)(ByteUtil.Combine2BytesToUInt16(high, low) + extraLength);
				}

				return result;
			}
		}

		public override string ToString()
		{
			return CryptographicBuffer.EncodeToHexString(GetBytes().AsBuffer());
		}
	}

	class Generator
	{
		private List<byte[]> guideCodeBytes = new List<byte[]>();
		private List<byte[]> datumCodeBytes = new List<byte[]>();

		/// <summary>
		/// Constructor of EsptouchGenerator, it will cost some time(maybe a bit much).
		/// </summary>
		/// <param name="ssid">the Ap's ssid.</param>
		/// <param name="bssid">the Ap's bssid.</param>
		/// <param name="password">the Ap's password.</param>
		/// <param name="ipAddress">the ip address of the phone or pad.</param>
		/// <param name="hidden">whether the Ap's ssid is hidden.</param>
		public Generator(String ssid, String bssid, String password, byte[] ipAddress, bool hidden)
		{
			foreach (var guide in new GuideCode().Values)
			{
				this.guideCodeBytes.Add(ByteUtil.GenerateSpecificBytes(guide));
			}

			foreach (var value in new DatumCode(ssid, bssid, password, ipAddress, hidden).Values)
			{
				this.datumCodeBytes.Add(ByteUtil.GenerateSpecificBytes(value));
			}
		}

		/// <summary>
		/// Guide code by the format of byte[][].
		/// </summary>
		public List<byte[]> GuideCode
		{
			get
			{
				return this.guideCodeBytes;
			}
		}

		/// <summary>
		/// Data code by the format of byte[][].
		/// </summary>
		public List<byte[]> DatumCode
		{
			get
			{
				return this.datumCodeBytes;
			}
		}
	}

	class UdpSocketClient
	{
		private bool stop = false;

		public void Interrupt()
		{
			this.stop = true;
		}

		/// <summary>
		/// Send the data by UDP
		/// </summary>
		/// <param name="data">the array of datas to be sent</param>
		/// <param name="target">the host name of target, e.g. 192.168.1.101.</param>
		/// <param name="port">the port of target.</param>
		/// <param name="interval">the milliseconds to between each UDP sent.</param>
		public async Task<bool> SendData(List<byte[]> data, HostName target, int port, TimeSpan interval)
		{
			return await SendData(data, 0, data.Count, target, port, interval);
		}

		/// <summary>
		/// Send the data by UDP.
		/// </summary>
		/// <param name="data">the data to be sent.</param>
		/// <param name="offset">the offset which data to be sent.</param>
		/// <param name="count">the count of the data.</param>
		/// <param name="target">the host name of target, e.g. 192.168.1.101.</param>
		/// <param name="port">the port of target.</param>
		/// <param name="interval">the milliseconds to between each UDP sent.</param>
		public async Task<bool> SendData(List<byte[]> data, int offset, int count, HostName target, int port, TimeSpan interval)
		{
			// check data is valid
			if ((null == data || 0 == data.Count))
			{
				Debug.WriteLine("client: data is null or data's length equals 0, so sendData fail\n");
				return false;
			}

			// send data gotten from the array
			for (int i = offset; !this.stop && i < offset + count; i++)
			{
				var bytes = data[i];

				if (0 == bytes.Length)
				{
					continue;
				}

				try
				{
					using (var socket = new DatagramSocket())
					{
						using (var stream = await socket.GetOutputStreamAsync(target, port.ToString()))
						{
							using (var writer = new DataWriter(stream))
							{
								writer.WriteBytes(bytes);
								await writer.StoreAsync();
							}
						}
					}
				}
				catch (Exception ex)
				{
					Debug.WriteLine("client: sendto fail, but just ignore it\n" + ex.Message);
					// for the Ap will make some troubles when the phone send too many UDP packets,
					// but we don't expect the UDP packet received by others, so just ignore it
				}

				await System.Threading.Tasks.Task.Delay(interval);
			}

			return true;
		}
	}

	class UdpSocketServer : IDisposable
	{
		public delegate void MessageReceivedHandler(byte[] message);
		private DatagramSocket socket = new DatagramSocket();
		public event MessageReceivedHandler MessageReceived;

		public int ExpectedMessageLength
		{
			get;
			set;
		}

		public void Dispose()
		{
			lock (this)
			{
				if (null != this.socket)
				{
					this.socket.MessageReceived -= this.SocketMessageReceived;
					this.socket.Dispose();
					this.socket = null;
				}
			}
		}

		public void Interrupt()
		{
			this.MessageReceived = null;
			Dispose();
		}

		public async Task<bool> StartListening(int port)
		{
			Debug.WriteLine("##########################server init(): _sck_fd4={0}", this.socket);

			this.socket.MessageReceived += this.SocketMessageReceived;

			await this.socket.BindServiceNameAsync(port.ToString());

			return true;
		}

		private void SocketMessageReceived(DatagramSocket sender, DatagramSocketMessageReceivedEventArgs args)
		{
			try
			{
				var result = args.GetDataStream();
				var resultStream = result.AsStreamForRead(1024);

				using (var memoryStream = new MemoryStream())
				{
					resultStream.CopyTo(memoryStream);

					var data = memoryStream.ToArray();

					if (null != this.MessageReceived)
					{
						this.MessageReceived(ReceiveSpecialLength(data, this.ExpectedMessageLength));
					}
				}
			}
			catch (Exception ex)
			{
				Debug.WriteLine(ex.Message);
			}
		}

		private byte[] ReceiveSpecialLength(byte[] buffer, int length)
		{
			int recNumber = buffer.Length;

			if (recNumber == length)
			{
				return buffer;
			}
			else if (recNumber == 0)
			{
				Debug.WriteLine("server: receiveOneByte4 socket is closed by the other\n");
			}
			else if (recNumber < 0)
			{
				Debug.WriteLine("server: receiveOneByte4 fail\n");
			}
			else
			{
				Debug.WriteLine(" receive rubbish message, just ignore it");
			}

			return null;
		}
	}

	class WiFiUtils
	{
		/// <summary>
		/// Local ip address by IPv4(or nil when en0/ipv4 unaccessible)
		/// </summary>
		public static HostName IPv4Address
		{
			get
			{
				foreach (var hostName in GetWiFiHostNames())
				{
					if (hostName.Type == HostNameType.Ipv4)
					{
						return hostName;
					}
				}

				return null;
			}
		}

		private static IEnumerable<HostName> GetWiFiHostNames()
		{
			var hostNames = NetworkInformation.GetHostNames();
			var profile = NetworkInformation.GetInternetConnectionProfile();

			if (profile != null)
			{
				if (null != profile.WlanConnectionProfileDetails)
				{
					return hostNames.Where(host => (host.IPInformation != null) && (host.IPInformation.NetworkAdapter != null) && (host.IPInformation.NetworkAdapter.NetworkAdapterId == profile.NetworkAdapter.NetworkAdapterId));
				}
			}

			return null;
		}

		public static String GetConnectedSsid()
		{
			var profile = NetworkInformation.GetInternetConnectionProfile();

			if (profile != null)
			{
				if (profile.WlanConnectionProfileDetails != null)
				{
					return profile.WlanConnectionProfileDetails.GetConnectedSsid();
				}
			}

			return null;
		}

		/// <summary>
		/// Local ip address by IPv6(or nil when en0/ipv6 unaccessible).
		/// </summary>
		public static HostName IPV6Address
		{
			get
			{
				foreach (var hostName in GetWiFiHostNames())
				{
					if (hostName.Type == HostNameType.Ipv6)
					{
						return hostName;
					}
				}

				return null;
			}
		}

		public static async Task<String> GetBssid(String ssid)
		{
			foreach (var adapter in await WiFiAdapter.FindAllAdaptersAsync())
			{
				foreach (var network in adapter.NetworkReport.AvailableNetworks)
				{
					if (network.Ssid == ssid)
					{
						return network.Bssid;
					}
				}
			}

			return "";
		}
	}

	static class Extensions
	{
		public static IBuffer AsBuffer(this UInt16[] values)
		{
			var data = new byte[values.Length * 2];

			for (int i = 0; i < values.Length; i++)
			{
				var bytes = BitConverter.GetBytes(values[i]);

				if (BitConverter.IsLittleEndian)
				{
					data[i * 2 + 0] = bytes[0];
					data[i * 2 + 1] = bytes[1];
				}
				else
				{
					data[i * 2 + 0] = bytes[1];
					data[i * 2 + 1] = bytes[0];
				}
			}

			return data.AsBuffer();
		}

		public static byte GetCrc8(this byte[] values)
		{
			Crc8 crc = new Crc8();

			crc.UpdateWithBuf(values, values.Length);

			return crc.Value;
		}
	}

	class TaskParameters
	{
		/// <summary>
		/// Interval millisecond for guide code (the time between each guide code sending).
		/// </summary>
		public TimeSpan GuideCodeInterval
		{
			get;
			private set;
		}

		/// <summary>
		/// Interval millisecond for data code(the time between each data code sending).
		/// </summary>
		public TimeSpan DataCodeInterval
		{
			get;
			private set;
		}

		/// <summary
		/// Timeout millisecond for guide code(the time how much the guide code sending).
		/// </summary>
		public TimeSpan GuideCodeTimeout
		{
			get;
			private set;
		}

		/// <summary>
		/// Timeout millisecond for data code(the time how much the data code sending).
		/// </summary>
		private TimeSpan DataCodeTimeout
		{
			get;
			set;
		}

		/// <summary>
		/// Timeout millisecond for total code(guide code and data code altogether).
		/// </summary>
		public TimeSpan TotalCodeTimeout
		{
			get;
			private set;
		}

		/// <summary>
		/// Total repeat time for executing esptouch task
		/// </summary>
		public int TotalRepeatCount
		{
			get;
			private set;
		}

		/// <summary>
		/// EsptouchResult length of one.
		/// The length of the Esptouch result 1st byte is the total length of ssid and password, the other 6 bytes are the device's bssid.
		/// </summary>
		public int ResultOneLength
		{
			get;
			private set;
		}

		/// <summary>
		/// EsptouchResult length of mac.
		/// </summary>
		public int ResultMacLength
		{
			get;
			private set;
		}

		/// <summary>
		/// EsptouchResult length of ip.
		/// </summary>
		public int ResultIpLength
		{
			get
			{
				return this.IsIPv4Supported ? this.resultIpv4Length : this.resultIpv6Length;
			}
		}

		/// <summary>
		/// EsptouchResult total length.
		/// </summary>
		public int ResultTotalLength
		{
			get
			{
				return (this.IsIPv4Supported) ? this.resultTotalIpv4Length : this.resultTotalIpv6Length;
			}
		}

		/// <summary>
		/// Port for listening(used by server).
		/// </summary>
		public int ListeningPort
		{
			get
			{
				if (this.IsIPv4Supported)
				{
					return this.ipv4ListeningPort;
				}
				else
				{
					return this.ipv6ListeningPort;
				}
			}
		}

		/// <summary>
		/// Target hostame(used by client).
		/// target hostname is : 234.1.1.1, 234.2.2.2, 234.3.3.3 to 234.100.100.100 for IPv4
		/// target hostname is : ff02::1 for IPv6
		/// </summary>
		public HostName GetTargetHostname()
		{
			if (this.IsIPv4Supported)
			{
				int count = GetNextDatagramCount();

				return new HostName(String.Format("234.{0}.{1}.{2}", count, count, count));

				/*
				// init socket parameters
				bool isBroadcast = target.CanonicalName == "255.255.255.255";
				//		socklen_t addr_len;
				//	struct sockaddr_in target_addr;

				//memset(&target_addr, 0, sizeof(target_addr));
				//target_addr.sin_family = AF_INET;
				//target_addr.sin_addr.s_addr = inet_addr([targetHostName cStringUsingEncoding:NSASCIIStringEncoding]);
				//target_addr.sin_port = htons(port);
				//addr_len = sizeof(target_addr);

				if (isBroadcast)
				{
					const int opt = 1;
					// set whether the socket is broadcast or not
					//	if (setsockopt(self._sck_fd4, SOL_SOCKET, SO_BROADCAST,(char*)&opt, sizeof(opt)) < 0)
					{
						//		if (DEBUG_ON)
						{

							//			perror("client: setsockopt SO_BROADCAST fail, but just ignore it\n");
						}
						// for the Ap will make some troubles when the phone send too many UDP packets,
						// but we don't expect the UDP packet received by others, so just ignore it
					}
				}
				*/
			}
			else
			{
				return new HostName("ff02::1%en0");
			}
		}

		/// <summary>
		/// Target port(used by client).
		/// </summary>
		public int TargetPort
		{
			get
			{
				return (this.IsIPv4Supported) ? this.ipv4TargetPort : this.ipv6TargetPort;
			}
		}

		/// <summary>
		/// Millisecond for waiting udp receiving(receiving without sending).
		/// </summary>
		public TimeSpan UdpReceivingWaitTime
		{
			get;
			private set;
		}

		/// <summary>
		/// Millisecond for waiting udep sending(sending including receiving).
		/// </summary>
		public TimeSpan UdpSendingWaitTime
		{
			get;
			private set;
		}

		/// <summary>
		/// Millisecond for waiting udp sending and receiving.
		/// </summary>
		public TimeSpan TotalUdpWait
		{
			get
			{
				return this.UdpReceivingWaitTime + this.UdpSendingWaitTime;
			}
			set
			{
				if (value < this.UdpReceivingWaitTime + this.TotalCodeTimeout)
				{
					// if it happen, even one turn about sending udp broadcast can't be completed
					throw new Exception("ESPTouchTaskParameter waitUdpTotalMillisecod is invalid, it is less than mWaitUdpReceivingMilliseond + [self getTimeoutTotalCodeMillisecond]");
				}

				this.UdpSendingWaitTime = value - this.UdpReceivingWaitTime;
			}
		}

		/// <summary>
		/// The threshold for how many correct broadcast should be received.
		/// </summary>
		public int SuccessfullBroadcastsCountThreshold
		{
			get;
			private set;
		}

		/// <summary>
		/// The count of expect task results.
		/// </summary>
		public int ExpectedTaskResultsCount
		{
			get;
			set;
		}

		/// <summary>
		/// Whether the router support IPv4.
		/// </summary>
		public bool IsIPv4Supported
		{
			get;
			set;
		}

		/// <summary>
		/// Whether the router support IPv6.
		/// </summary>
		public bool IsIPv6Supported
		{
			get;
			set;
		}

		/// <summary>
		/// Listening port for IPv6.
		/// </summary>
		public int ListeningPort6
		{
			set
			{
				this.ipv6ListeningPort = value;
			}
		}

		private int resultIpv4Length = 4;
		private int resultIpv6Length = 16;
		private int resultTotalIpv4Length = 1 + 6 + 4;
		private int resultTotalIpv6Length = 1 + 6 + 16;
		private int ipv4ListeningPort = 18266;
		private int ipv6ListeningPort = 0;
		private int ipv4TargetPort = 7001;
		private int ipv6TargetPort = 7001;
		private static int datagramCount = 0;

		public TaskParameters()
		{
			this.GuideCodeInterval = TimeSpan.FromMilliseconds(10);
			this.DataCodeInterval = TimeSpan.FromMilliseconds(10);
			this.GuideCodeTimeout = TimeSpan.FromMilliseconds(2000);
			this.DataCodeTimeout = TimeSpan.FromMilliseconds(4000);
			this.TotalCodeTimeout = this.GuideCodeTimeout + this.DataCodeTimeout;
			this.TotalRepeatCount = 1;
			this.ResultOneLength = 1;
			this.ResultMacLength = 6;
			this.UdpReceivingWaitTime = TimeSpan.FromMilliseconds(15000);
			this.UdpSendingWaitTime = TimeSpan.FromMilliseconds(45000);
			this.SuccessfullBroadcastsCountThreshold = 1;
			this.ExpectedTaskResultsCount = 1;
			this.IsIPv4Supported = true;
			this.IsIPv6Supported = true;
		}

		private static int GetNextDatagramCount()
		{
			return 1 + (datagramCount++) % 100;
		}
	}

	static class NetUtil
	{
		private const int Ipv4AddressLength = 4;
		private const int IP6_LEN = 16;

		/// <summary>
		/// Local ip v4 or null.
		/// </summary>
		public static HostName LocalIPv4
		{
			get
			{
				return WiFiUtils.IPv4Address;
			}
		}

		/// <summary>
		/// Local ip v6 or null.
		/// </summary>
		public static HostName LocalIPv6
		{
			get
			{
				return WiFiUtils.IPV6Address;
			}
		}

		/// <summary>
		/// Whether the ipAddr v4 is private.
		/// </summary>
		/// <param name="ipAddr"></param>
		/// <returns></returns>
		public static bool IsIPv4PrivateAddress(HostName address)
		{
			if (null != address)
			{
				var bits = address.CanonicalName.Split('.');
				var byte0 = byte.Parse(bits[0]);
				var byte1 = byte.Parse(bits[1]);

				if (byte0 == 10)
				{
					//    10.0.0.0~10.255.255.255
					return true;
				}
				else if (byte0 == 172 && byte1 <= 31)
				{
					//    172.16.0.0~172.31.255.255
					return true;
				}
				else if (byte0 == 192 && byte1 == 168)
				{
					//    192.168.0.0~192.168.255.255
					return true;
				}
			}

			return false;
		}

		/// <summary>
		/// Get the local ip address by local inetAddress ip4.
		/// </summary>
		/// <param name="hostName">local inetAddress ip4.</param>
		/// <returns></returns>
		public static byte[] GetLocalIpV4AddressBytes(this HostName hostName)
		{
			if (null != hostName)
			{
				var ip4array = hostName.CanonicalName.Split('.');

				var byte0 = byte.Parse(ip4array[0]);
				var byte1 = byte.Parse(ip4array[1]);
				var byte2 = byte.Parse(ip4array[2]);
				var byte3 = byte.Parse(ip4array[3]);

				return new byte[] { byte0, byte1, byte2, byte3 };
			}

			return null;
		}

		/// <summary>
		/// Get the invented local ip address by local port.
		/// </summary>
		/// <param name="localPort"></param>
		/// <returns></returns>
		public static byte[] GetLocalInetAddress6ByPort(int localPort)
		{
			byte lowPort = (byte)(localPort & 0xff);
			byte highPort = (byte)((localPort >> 8) & 0xff);

			return new byte[] { highPort, lowPort, 0xff, 0xff };
		}

		/// <summary>
		/// Parse InetAddress.
		/// </summary>
		/// <param name="address"></param>
		/// <param name="offset"></param>
		/// <param name="count"></param>
		/// <returns></returns>
		public static HostName ParseAddress(byte[] address, int offset, int count)
		{
			byte[] result = new byte[count];

			for (int i = 0; i < count; i++)
			{
				result[i] = address[offset + i];
			}

			if (count == 4)
			{
				return new HostName(address[offset + 0] + "." + address[offset + 1] + "." + address[offset + 2] + "." + address[offset + 3]);
			}
			else
			{
				throw new NotImplementedException("ipv6?");
			}
		}

		/// <summary>
		/// Descrpion inetAddrData for print pretty IPv4.
		/// </summary>
		/// <param name="address"></param>
		/// <returns></returns>
		public static String FormatIpv4Address(byte[] address)
		{
			// check whether inetAddrData is belong to IPv4
			if (address.Length != Ipv4AddressLength)
			{
				return null;
			}

			return String.Format("{0}.{1}.{2}.{3}", address[0], address[1], address[2], address[3]);
		}

		/// <summary>
		/// Parse bssid.
		/// </summary>
		/// <param name="bssid">the bssid.</param>
		/// <returns>byte converted from bssid.</returns>
		public static byte[] ParseBssid(string bssid)
		{
			if (null != bssid)
			{
				bssid = bssid.Trim();

				if (bssid.Length > 0)
				{
					var bits = bssid.Split(':');
					var size = bits.Length;

					var result = new byte[size];

					for (int i = 0; i < size; i++)
					{
						result[i] = Convert.ToByte(bits[i], 16);
					}

					return result;
				}
			}

			return new byte[0];
		}
	}

	class ByteUtil
	{
		/// <summary>
		/// Convert char into uint8( we treat char as uint8 ).
		/// </summary>
		/// <param name="b">the byte to be converted</param>
		/// <returns>the UInt8(uint8)</returns>
		private static Byte ConvertToUInt8(Byte b)
		{
			return (Byte)(b & 0xff);
		}

		/// <summary>
		/// Split uint8 to 2 bytes of high byte and low byte. e.g. 20 = 0x14 should be split to [0x01,0x04] 0x01 is high byte and 0x04 is low byte.
		/// </summary>
		/// <param name="value">the char(uint8).</param>
		/// <returns>the high and low bytes be split, byte[0] is high and byte[1] is low.</returns>
		public static byte[] SplitTo2Bytes(byte value)
		{
			byte high = (byte)((value & 0xf0) >> 4);
			byte low = (byte)(value & 0x0f);

			return new byte[] { high, low };
		}

		/// <summary>
		/// Combine 2 bytes (high byte and low byte) to one whole byte.
		/// </summary>
		/// <param name="high">the high byte.</param>
		/// <param name="low">the low byte.</param>
		/// <returns>the whole byte.</returns>
		public static byte CombineToOne(byte high, byte low)
		{
			return (byte)(high << 4 | low);
		}

		/// <summary>
		/// Combine 2 bytes (high byte and low byte) to one UInt16.
		/// </summary>
		/// <param name="high">the high byte.</param>
		/// <param name="low">the low byte.</param>
		/// <returns>the UInt8.</returns>
		public static UInt16 Combine2BytesToUInt16(byte high, byte low)
		{
			Byte highU8 = ConvertToUInt8(high);
			Byte lowU8 = ConvertToUInt8(low);
			return (UInt16)(highU8 << 8 | lowU8);
		}

		public static byte[] GenerateSpecificBytes(UInt16 length)
		{
			var bytes = new byte[length];

			for (int i = 0; i < length; i++)
			{
				bytes[i] = (byte)'1';
			}

			return bytes;
		}

		public static String ParseBssid(Byte[] bssidBytes, int offset, int count)
		{
			Byte[] bytes = new Byte[count];

			for (int i = 0; i < count; i++)
			{
				bytes[i] = bssidBytes[i + offset];
			}

			return ParseBssid(bytes, count);
		}

		/// <summary>
		/// Parse "24,-2,52,-102,-93,-60" to "18,fe,34,9a,a3,c4" parse the bssid from hex to String.
		/// </summary>
		/// <param name="bssidBytes">the hex bytes bssid, e.g. {24,-2,52,-102,-93,-60}</param>
		/// <param name="len">the len of bssidBytes</param>
		/// <returns>the String of bssid, e.g. 18fe349aa3c4</returns>
		private static String ParseBssid(Byte[] bssidBytes, int len)
		{
			return CryptographicBuffer.EncodeToHexString(bssidBytes.AsBuffer());
		}

		/// <summary>
		/// string according to ESPTOUCH_NSStringEncoding. //ESPTOUCH_NSStringEncoding NSUTF8StringEncoding
		/// </summary>
		/// <param name="string1">the string to be used.</param>
		/// <returns>the Byte[] of string according to ESPTOUCH_NSStringEncoding.</returns>
		public static byte[] GetBytes(String string1)
		{
			return Encoding.UTF8.GetBytes(string1);
		}
	}
}
