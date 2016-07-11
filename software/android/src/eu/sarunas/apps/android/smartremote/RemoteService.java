package eu.sarunas.apps.android.smartremote;

import java.util.UUID;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.content.Context;

public class RemoteService
{
	private BluetoothGatt blueToothGatt = null;;
	private IConnectionHandler connectionHandler = null;;
	private boolean writing = false;
	private BluetoothGattCharacteristic address = null;
	private BluetoothGattCharacteristic command = null;
	private BluetoothGattCharacteristic type = null;

	private final BluetoothGattCallback gattCallBack = new BluetoothGattCallback()
	{
		@Override
		public void onCharacteristicWrite(BluetoothGatt gatt, BluetoothGattCharacteristic characteristic, int status)
		{
			System.out.println("Written " + characteristic.getUuid().toString());

			synchronized (RemoteService.this)
			{
				RemoteService.this.writing = false;
			}
		};

		@Override
		public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState)
		{
			if (newState == BluetoothProfile.STATE_CONNECTED)
			{
				System.out.println("Discover start: " + RemoteService.this.blueToothGatt.discoverServices());
			}
			else if (newState == BluetoothProfile.STATE_DISCONNECTED)
			{
				if (null != RemoteService.this.connectionHandler)
				{
					RemoteService.this.connectionHandler.onDisconnected();
				}
			}
		};

		@Override
		public void onServicesDiscovered(BluetoothGatt gatt, int status)
		{
			BluetoothGattService service = RemoteService.this.blueToothGatt.getService(UUID.fromString("a739eeee-f6cd-1692-994a-d66d9e0ce048"));

			if (null == service)
			{
				System.out.println("No service");
				
				return;
			}

			address = service.getCharacteristic(UUID.fromString("a739" + "eee2" + "-f6cd-1692-994a-d66d9e0ce048"));
			type = service.getCharacteristic(UUID.fromString("a739" + "eee0" + "-f6cd-1692-994a-d66d9e0ce048"));
			command = service.getCharacteristic(UUID.fromString("a739" + "eee1" + "-f6cd-1692-994a-d66d9e0ce048"));

			if ((null == address) || (null == command) || (null == type))
			{
				System.out.println("No char");
				
				return;
			}
			
			RemoteService.this.connectionHandler.onConnected(RemoteService.this);
		};
	};;

	public RemoteService(BluetoothDevice device, Context context, IConnectionHandler connectionHandler)
	{
		this.connectionHandler = connectionHandler;
		this.blueToothGatt = device.connectGatt(context, false, this.gattCallBack);
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

	public void disconnect()
	{
		if (null == this.blueToothGatt)
		{
			return;
		}

		this.blueToothGatt.disconnect();
	};

	public void sendAddress(int address)
	{
		sendData(address, this.address);
	}

	public void sendCommand(int command)
	{
		sendData(command, this.command);
	};

	private void sendData(final int command, final BluetoothGattCharacteristic characteristic)
	{
		if (null == characteristic)
		{
			return;
		}
		
		long waited = 0;
		
		while ((true == RemoteService.this.writing) && (waited < 10000))
		{
			try
			{
				Thread.sleep(10);
				waited += 10;
			}
			catch (InterruptedException e)
			{
				e.printStackTrace();
			}
		}

		synchronized (this)
		{
			System.out.println("Writing: " + characteristic.toString());
			
			RemoteService.this.writing = true;

			if (command <= 0xff)
			{
				characteristic.setValue(new byte[] { (byte) command });
			}
			else if (command <= 0xffff)
			{
				byte loByte = (byte)(command & 0xff);
				byte hiByte = (byte)(command >> 8);

				characteristic.setValue(new byte[] { loByte, hiByte });
			}
			
			characteristic.setWriteType(BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT);

			RemoteService.this.blueToothGatt.writeCharacteristic(characteristic);
		}
	};

	public void sendProtocol(IrProtocol protocol)
	{
		sendData(protocol.getId(), this.type);
	};
};
