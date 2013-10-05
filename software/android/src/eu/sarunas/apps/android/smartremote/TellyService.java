package eu.sarunas.apps.android.smartremote;

import java.util.UUID;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothManager;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.util.Log;

public class TellyService
{
	public TellyService(String address, Activity activity, IConnectionHandler connectionHandler) throws Exception
	{
		this.connectionHandler = connectionHandler;

		if (null == this.blueToothManager)
		{
			this.blueToothManager = (BluetoothManager) activity.getSystemService(Context.BLUETOOTH_SERVICE);

			if (null == this.blueToothManager)
			{
				throw new Exception("Unable to initialize BluetoothManager.");
			}
		}

		this.blueToothAdapter = this.blueToothManager.getAdapter();

		if (null == this.blueToothAdapter)
		{
			throw new Exception("Unable to obtain a BluetoothAdapter.");
		}

		if ((null == this.blueToothAdapter) || (null == address))
		{
			throw new Exception("BluetoothAdapter not initialized or unspecified address.");
		}

		if ((null != this.blueToothDeviceAddress) && (address.equals(blueToothDeviceAddress)) && (null != blueToothGatt))
		{
			Log.d(TAG, "Trying to use an existing mBluetoothGatt for connection.");

			if (false == this.blueToothGatt.connect())
			{
				throw new Exception("Failed to connect");
			}
		}

		BluetoothDevice device = this.blueToothAdapter.getRemoteDevice(address);

		if (null == device)
		{
			throw new Exception("Device not found.  Unable to connect.");
		}

		this.blueToothGatt = device.connectGatt(activity, false, this.gattCallBack);
		Log.d(TAG, "Trying to create a new connection.");
		this.blueToothDeviceAddress = address;
	};

	private final BluetoothGattCallback gattCallBack = new BluetoothGattCallback()
	{
		@Override
		public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState)
		{
			if (newState == BluetoothProfile.STATE_CONNECTED)
			{
				Log.i(TAG, "Connected to GATT server.");
				Log.i(TAG, "Attempting to start service discovery:" + blueToothGatt.discoverServices());
			}
			else if (newState == BluetoothProfile.STATE_DISCONNECTED)
			{
				Log.i(TAG, "Disconnected from GATT server.");

				connectionHandler.onDisconnected(TellyService.this, "");
			}
		};

		@Override
		public void onServicesDiscovered(BluetoothGatt gatt, int status)
		{
			if (status != BluetoothGatt.GATT_SUCCESS)
			{
				Log.w(TAG, "onServicesDiscovered received: " + status);
			}

			connectionHandler.onConnected(TellyService.this);
		};

		@Override
		public void onCharacteristicRead(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status)
		{
		};

		@Override
		public void onCharacteristicChanged(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic)
		{
		};
	};

	public void disconnect()
	{
		if ((null == this.blueToothAdapter) || (null == this.blueToothGatt))
		{
			Log.w(TAG, "BluetoothAdapter not initialized");
			return;
		}

		this.blueToothGatt.disconnect();
	};

	public void close()
	{
		if (null != this.blueToothGatt)
		{
			return;
		}

		this.blueToothGatt.close();
		this.blueToothGatt = null;
	};

	private void sendCommand(int command)
	{
		if (null == this.blueToothGatt)
		{
			return;
		}

		BluetoothGattService service = this.blueToothGatt.getService(UUID.fromString("0000eeee-0000-1000-8000-00805f9b34fb"));

		if (null == service)
		{
			return;
		}

		BluetoothGattCharacteristic characteristic = service.getCharacteristic(UUID.fromString("0000eee1-0000-1000-8000-00805f9b34fb"));

		if (null == characteristic)
		{
			return;
		}

		characteristic.setValue(new byte[] { (byte) command });

		this.blueToothGatt.writeCharacteristic(characteristic);
	};

	public void sendPower()
	{
		sendCommand(21);
	};

	public void sendHome()
	{
		sendCommand(96);
	};

	public void sendSource()
	{
		sendCommand(37);
	};

	public void sendBack()
	{
		sendCommand(96);
	};

	public void sendVolumeUp()
	{
		sendCommand(18);
	};

	public void sendVolumeDown()
	{
		sendCommand(19);
	};

	public void sendOk()
	{
		sendCommand(101);
	};

	public void sendLeft()
	{
		sendCommand(52);
	};

	public void sendRight()
	{
		sendCommand(51);
	};

	public void sendUp()
	{
		sendCommand(116);
	};

	public void sendDown()
	{
		sendCommand(117);
	};

	private final static String TAG = "BLE";
	private IConnectionHandler connectionHandler = null;
	private BluetoothManager blueToothManager = null;
	private BluetoothAdapter blueToothAdapter = null;
	private String blueToothDeviceAddress = null;
	private BluetoothGatt blueToothGatt = null;
};
