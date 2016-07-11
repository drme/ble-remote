package eu.sarunas.apps.android.smartremote;

import java.util.ArrayList;
import java.util.List;
import android.content.Context;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.FragmentStatePagerAdapter;

public class RemotesPagerAdapter extends FragmentStatePagerAdapter
{
	private List<RemoteFragment> remotesPages = new ArrayList<RemoteFragment>();

	public RemotesPagerAdapter(FragmentManager fragmentManager, Context context)
	{
		super(fragmentManager);
	};

	@Override
	public synchronized int getCount()
	{
		return this.remotesPages.size();
	};

	@Override
	public synchronized Fragment getItem(int position)
	{
		return this.remotesPages.get(position);
	};

	@Override
	public synchronized CharSequence getPageTitle(int position)
	{
		return this.remotesPages.get(position).getRemote().getName();
	};

	public synchronized void reload(Context context)
	{
		this.remotesPages.clear();

		List<RemoteFragment> pages = new ArrayList<RemoteFragment>();
		
		for (Remote remote : RemotesManager.getInstance().getRemotes(context))
		{
			pages.add(new RemoteFragment(remote));
		}

		this.remotesPages = pages;
		
		notifyDataSetChanged();
	};
	
	@Override
	public int getItemPosition(Object object) {
	    return POSITION_NONE;
	}	
};
