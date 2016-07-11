package eu.sarunas.apps.android.smartremote;

import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;
import android.content.Context;

public class Remote implements Serializable
{
	private static final long serialVersionUID = 4940292328022839482L;
	private int address = 0;
	private List<RemoteButton> buttons = new ArrayList<RemoteButton>();
	private String id = UUID.randomUUID().toString();
	private String name = "";
	private IrProtocol type = IrProtocol.SIRC;

	public void delete(Context context)
	{
		context.deleteFile(this.id.toString() + ".blah");
	};

	public int getAddress()
	{
		return this.address;
	};

	public List<RemoteButton> getButtons()
	{
		return this.buttons;
	};

	public String getId()
	{
		return this.id;
	};

	public String getName()
	{
		return this.name;
	};

	public IrProtocol getType()
	{
		return this.type;
	};

	public void save(Context context)
	{
		try
		{
			FileOutputStream fos = context.openFileOutput(this.id + ".blah", Context.MODE_PRIVATE);
			ObjectOutputStream os = new ObjectOutputStream(fos);
			os.writeObject(this);
			os.close();
			fos.close();
		}
		catch (IOException ex)
		{
			ex.printStackTrace();
		}
	};

	public void setAddress(int address)
	{
		this.address = address;
	};

	public void setId(String id)
	{
		this.id = id;
	};

	public void setName(String name)
	{
		this.name = name;
	};

	public void setType(IrProtocol type)
	{
		this.type = type;
	};
};
