package eu.sarunas.apps.android.smartremote;

public interface IConnectionHandler
{
	public void onConnected(TellyService service);
	public void onDisconnected(TellyService service, String message);
};
