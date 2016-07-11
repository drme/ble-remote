package eu.sarunas.apps.android.smartremote;

import java.io.FileInputStream;
import java.io.ObjectInputStream;
import java.util.ArrayList;
import java.util.List;
import android.content.Context;

public class RemotesManager
{
	private static RemotesManager instance = new RemotesManager();
	private Remote activeRemote = null;
	private List<Remote> remotes = null;

	public static RemotesManager getInstance()
	{
		return RemotesManager.instance;
	};

	public Remote addDefaultRemoteNec(String name, Context context)
	{
		Remote remote = new Remote();

		remote.setAddress(62596);
		remote.setName(name);
		remote.setType(IrProtocol.NEC);

		int c = 0;

		float m = 3.3f;

		remote.getButtons().add(new RemoteButton(m * 8, (m * 14) + c, m * 95, m * 46, "Input", false, 10));
		remote.getButtons().add(new RemoteButton(m * 218, (m * 14) + c, m * 95, m * 46, "Power", false, 11));
		remote.getButtons().add(new RemoteButton(m * 123, (m * 154) + c, m * 74, m * 74, "Enter", true, 76));

		remote.getButtons().add(new RemoteButton(m * 123, (m * 72) + c, m * 74, m * 74, "\u21e7", true, 74));
		remote.getButtons().add(new RemoteButton(m * 123, (m * 236) + c, m * 74, m * 74, "\u21e9", true, 75));
		remote.getButtons().add(new RemoteButton(m * 41, (m * 154) + c, m * 74, m * 74, "\u21e6", true, 77));
		remote.getButtons().add(new RemoteButton(m * 205, (m * 154) + c, m * 74, m * 74, "\u21e8", true, 78));

		remote.getButtons().add(new RemoteButton(m * 8, (m * 328) + c, m * 46, m * 46, "\u21e9", false, 47));
		remote.getButtons().add(new RemoteButton(m * 70, (m * 328) + c, m * 46, m * 46, "\u21e7", false, 44));

		remote.getButtons().add(new RemoteButton(m * 129, (m * 328) + c, m * 74, m * 46, "ESC", false, 14));
		remote.getButtons().add(new RemoteButton(m * 218, (m * 328) + c, m * 94, m * 46, "Menu", false, 12));

		remote.save(context);

		return remote;
	};

	public Remote addDefaultRemoteRC5(String name, Context context)
	{
		Remote remote = new Remote();

		remote.setAddress(1);
		remote.setName(name);
		remote.setType(IrProtocol.RC5);

		int c = 0;

		float m = 3.3f;

		remote.getButtons().add(new RemoteButton(m * 8, (m * 14) + c, m * 95, m * 46, "CD", false, 63, 20));
		remote.getButtons().add(new RemoteButton(m * 218, (m * 14) + c, m * 95, m * 46, "Power", false, 12, 20));
		remote.getButtons().add(new RemoteButton(m * 123, (m * 154) + c, m * 74, m * 74, "Play", true, 53, 20));

		remote.getButtons().add(new RemoteButton(m * 41, (m * 154) + c, m * 74, m * 74, "\u21e6", true, 33, 20));
		remote.getButtons().add(new RemoteButton(m * 205, (m * 154) + c, m * 74, m * 74, "\u21e8", true, 32, 20));

		remote.getButtons().add(new RemoteButton(m * 8, (m * 328) + c, m * 46, m * 46, "\u21e9", false, 17, 16));
		remote.getButtons().add(new RemoteButton(m * 70, (m * 328) + c, m * 46, m * 46, "\u21e7", false, 16, 16));

		remote.getButtons().add(new RemoteButton(m * 129, (m * 328) + c, m * 74, m * 46, "Tape", false, 63, 18));
		remote.getButtons().add(new RemoteButton(m * 218, (m * 328) + c, m * 94, m * 46, "Aux", false, 63, 21));

		remote.save(context);

		return remote;
	};

	public Remote addDefaultRemoteSirc(String name, Context context)
	{
		Remote remote = new Remote();

		remote.setAddress(1);
		remote.setName(name);
		remote.setType(IrProtocol.SIRC);

		int c = 0;

		float m = 3.3f;

		remote.getButtons().add(new RemoteButton(m * 8, (m * 14) + c, m * 95, m * 46, "Source", false, 37));
		remote.getButtons().add(new RemoteButton(m * 218, (m * 14) + c, m * 95, m * 46, "Power", false, 21));
		remote.getButtons().add(new RemoteButton(m * 123, (m * 154) + c, m * 74, m * 74, "OK", true, 101));

		remote.getButtons().add(new RemoteButton(m * 123, (m * 72) + c, m * 74, m * 74, "\u21e7", true, 116));
		remote.getButtons().add(new RemoteButton(m * 123, (m * 236) + c, m * 74, m * 74, "\u21e9", true, 117));
		remote.getButtons().add(new RemoteButton(m * 41, (m * 154) + c, m * 74, m * 74, "\u21e6", true, 52));
		remote.getButtons().add(new RemoteButton(m * 205, (m * 154) + c, m * 74, m * 74, "\u21e8", true, 51));

		remote.getButtons().add(new RemoteButton(m * 8, (m * 328) + c, m * 46, m * 46, "\u21e9", false, 19));
		remote.getButtons().add(new RemoteButton(m * 70, (m * 328) + c, m * 46, m * 46, "\u21e7", false, 18));

		remote.getButtons().add(new RemoteButton(m * 129, (m * 328) + c, m * 74, m * 46, "Back", false, 99));
		remote.getButtons().add(new RemoteButton(m * 218, (m * 328) + c, m * 94, m * 46, "Home", false, 96));

		remote.save(context);

		return remote;
	};

	public Remote getActiveRemote()
	{
		return this.activeRemote;
	};

	public Remote getRemote(String id)
	{
		for (Remote remote : this.remotes)
		{
			if (remote.getId().equals(id))
			{
				return remote;
			}
		}

		return null;
	};

	public List<Remote> getRemotes(Context context)
	{
		if (null == context)
		{
			return this.remotes;
		}

		List<Remote> result = new ArrayList<Remote>();

		for (String file : context.fileList())
		{
			if (file.endsWith(".blah"))
			{
				try
				{
					FileInputStream fis = context.openFileInput(file);
					ObjectInputStream is = new ObjectInputStream(fis);
					Remote remote = (Remote) is.readObject();
					is.close();
					fis.close();

					result.add(remote);
				}
				catch (Exception ex)
				{
					ex.printStackTrace();
				}
			}
		}

		if (result.size() <= 0)
		{
			result.add(addDefaultRemoteSirc("demo sony telly", context));
			result.add(addDefaultRemoteNec("demo casio projector", context));
			result.add(addDefaultRemoteRC5("demo philips hifi", context));

			result.get(0).save(context);
			result.get(1).save(context);
			result.get(2).save(context);
		}

		this.remotes = result;

		return result;
	};

	public void setActiveRemote(Remote activeRemote)
	{
		this.activeRemote = activeRemote;
	};
};
