package eu.sarunas.apps.android.smartremote;

import java.io.Serializable;

public class RemoteButton implements Serializable
{
	private static final long serialVersionUID = 3722197592066143882L;
	private Integer address = null;
	private int command = 0;
	private int height = 100;
	private boolean round = false;
	private String title = "";
	private IrProtocol type = null;
	private int width = 100;
	private int x = 0;
	private int y = 0;

	public RemoteButton()
	{
	};

	public RemoteButton(float x, float y, float w, float h, String title, boolean round, int command)
	{
		this(x, y, w, h, title, round, command, null);
	};
	
	public RemoteButton(float x, float y, float w, float h, String title, boolean round, int command, Integer address)
	{
		this((int)x, (int)y, (int)w, (int)h, title, round, command);
	};

	public RemoteButton(int x, int y, int w, int h, String title, boolean round, int command)
	{
		this.x = x;
		this.y = y;
		this.width = w;
		this.height = h;
		this.command = command;
		this.title = title;
		this.round = round;
	};

	public Integer getAddress()
	{
		return this.address;
	};

	public int getCommand()
	{
		return this.command;
	};

	public int getHeight()
	{
		return this.height;
	};

	public String getTitle()
	{
		return this.title;
	};

	public IrProtocol getType()
	{
		return this.type;
	};

	public int getWidth()
	{
		return this.width;
	};

	public int getX()
	{
		return this.x;
	};

	public int getY()
	{
		return this.y;
	};

	public boolean isRound()
	{
		return this.round;
	};

	public void sendCommand(final Remote remote)
	{
		final RemoteService service = DevicesManager.getInstance().getRemoteService();

		if (null != service)
		{
			new Thread(new Runnable()
			{
				@Override
				public void run()
				{
					sleep();

					if (null != RemoteButton.this.type)
					{
						service.sendProtocol(RemoteButton.this.type);
					}
					else
					{
						service.sendProtocol(remote.getType());
					}

					sleep();

					if (null != RemoteButton.this.address)
					{
						service.sendAddress(RemoteButton.this.address);
					}
					else
					{
						service.sendAddress(remote.getAddress());
					}

					sleep();

					service.sendCommand(RemoteButton.this.command);
				};

				private void sleep()
				{
					try
					{
						Thread.sleep(10);
					}
					catch (InterruptedException ex)
					{
						ex.printStackTrace();
					}
				};
			}).start();
		}
	};

	public void setAddress(Integer address)
	{
		this.address = address;
	};

	public void setCommand(int command)
	{
		this.command = command;
	};

	public void setHeight(int height)
	{
		this.height = height;
	};

	public void setRound(boolean round)
	{
		this.round = round;
	};

	public void setTitle(String title)
	{
		this.title = title;
	};

	public void setType(IrProtocol type)
	{
		this.type = type;
	};

	public void setWidth(int width)
	{
		this.width = width;
	};

	public void setX(int x)
	{
		this.x = x;
	};

	public void setY(int y)
	{
		this.y = y;
	};
};
