package eu.sarunas.apps.android.smartremote;

import java.util.Arrays;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.widget.Toast;

public class DevicesManager
{
	private static int GAP_ADTYPE_128BIT_MORE = 0x06;;
	private static DevicesManager instance = new DevicesManager();;
	private static final long scanPeriod = 10000;;
	private static byte[] service128Id = new byte[] { (byte) 0x48, (byte) 0xe0, (byte) 0x0c, (byte) 0x9e, (byte) 0x6d, (byte) 0xd6, (byte) 0x4a, (byte) 0x99, (byte) 0x92, (byte) 0x16, (byte) 0xcd, (byte) 0xf6, (byte) 0xee, (byte) 0xee, (byte) 0x39, (byte) 0xa7 };;
	private BluetoothAdapter bluetoothAdapter = null;;
	private IConnectionHandler connectionHandler = null;
	private Context context = null;
	private Handler handler = new Handler();
	private RemoteService remoteService = null;
	private boolean scanning = false;

	public static DevicesManager getInstance()
	{
		return DevicesManager.instance;
	};

	private BluetoothAdapter.LeScanCallback scanCallback = new BluetoothAdapter.LeScanCallback()
	{
		@Override
		public void onLeScan(BluetoothDevice device, int rssi, byte[] scanRecord)
		{
			parseRecord(device, rssi, scanRecord);
		};
	};

	public RemoteService getRemoteService()
	{
		return this.remoteService;
	};

	public void setRemoteService(RemoteService value)
	{
		this.remoteService = value; 
	};
	
	protected void parseRecord(final BluetoothDevice device, int rssi, byte[] scanRecord)
	{
		boolean valid = false;
		int index = 0;

		while (index < scanRecord.length)
		{
			int length = scanRecord[index++];

			if (length == 0)
			{
				break;
			}

			int type = scanRecord[index];

			if (type == 0)
			{
				break;
			}

			byte[] data = Arrays.copyOfRange(scanRecord, index + 1, index + length);

			if (type == DevicesManager.GAP_ADTYPE_128BIT_MORE)
			{
				if (data.length == DevicesManager.service128Id.length)
				{
					int matched = 0;

					for (int i = 0; i < data.length; i++)
					{
						if (data[i] == DevicesManager.service128Id[i])
						{
							matched++;
						}
					}

					valid = matched == data.length;
				}
			}

			index += length;
		}

		if ((true == valid) && (null == this.remoteService))
		{
			this.remoteService = new RemoteService(device, this.context, this.connectionHandler);
		}
	};

	@SuppressWarnings("deprecation")
	public void startScan(final Context context, IConnectionHandler connectionHandler)
	{
		this.connectionHandler = connectionHandler;
		this.context = context;

		if (true == this.scanning)
		{
			return;
		}

		if (this.remoteService != null)
		{
			if (null != this.connectionHandler)
			{
				this.connectionHandler.onConnected(null);
			}

			return;
		}

		if (false == context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE))
		{
			Toast.makeText(context, R.string.ble_not_supported, Toast.LENGTH_SHORT).show();

			return;
		}

		BluetoothManager bluetoothManager = (BluetoothManager) context.getSystemService(Context.BLUETOOTH_SERVICE);

		this.bluetoothAdapter = bluetoothManager.getAdapter();

		if (null == this.bluetoothAdapter)
		{
			Toast.makeText(context, R.string.error_bluetooth_not_supported, Toast.LENGTH_SHORT).show();
			return;
		}

		this.handler.postDelayed(new Runnable()
		{
			@Override
			public void run()
			{
				stopScan(context);
			}
		}, DevicesManager.scanPeriod);

		this.scanning = this.bluetoothAdapter.startLeScan(this.scanCallback);
	};

	@SuppressWarnings("deprecation")
	public void stopScan(final Context context)
	{
		this.scanning = false;

		if (null != this.bluetoothAdapter)
		{
			this.bluetoothAdapter.stopLeScan(this.scanCallback);
		}

		if (null == this.remoteService)
		{
			if (null != this.connectionHandler)
			{
				this.connectionHandler.onDisconnected();
			}

			this.handler.postDelayed(new Runnable()
			{
				@Override
				public void run()
				{
					startScan(context, DevicesManager.this.connectionHandler);
				}
			}, DevicesManager.scanPeriod);
		}
	};
};
