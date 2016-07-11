package eu.sarunas.apps.android.smartremote;

public interface IConnectionHandler
{
	public void onConnected(RemoteService service);
	public void onDisconnected();
};
