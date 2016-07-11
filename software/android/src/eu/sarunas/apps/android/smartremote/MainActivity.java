package eu.sarunas.apps.android.smartremote;

import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.DataSetObserver;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.view.ViewPager;
import android.view.Menu;
import android.view.MenuItem;

@SuppressWarnings("deprecation")
public class MainActivity extends FragmentActivity implements ActionBar.TabListener, IConnectionHandler
{
	private RemotesPagerAdapter remotesPagerAdapter = null;
	private ViewPager viewPager = null;

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		this.remotesPagerAdapter.reload(getApplicationContext());
	};

	@Override
	public void onConnected(final RemoteService service)
	{
		invalidateOptionsMenu();
	};

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		BaseActivity.updateTitle(this, false, true, "Smart Remote");
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

//		final ActionBar actionBar = getActionBar();
	//	actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

		this.remotesPagerAdapter = new RemotesPagerAdapter(getSupportFragmentManager(), this);

		this.viewPager = (ViewPager) findViewById(R.id.pager);
		this.viewPager.setAdapter(this.remotesPagerAdapter);

		this.viewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener()
		{
			@Override
			public void onPageSelected(int position)
			{
	//			actionBar.setSelectedNavigationItem(position);
				
				RemoteFragment page = (RemoteFragment)remotesPagerAdapter.getItem(position);
				
				RemotesManager.getInstance().setActiveRemote(page.getRemote());
				
			//	page.buildUI();
				
//				actionBar.setTitle(RemotesManager.getInstance().getActiveRemote().getName());
			};
		});

		this.remotesPagerAdapter.registerDataSetObserver(new DataSetObserver()
		{
			@Override
			public void onChanged()
			{
				if (null == RemotesManager.getInstance().getActiveRemote())
				{
					RemoteFragment page = (RemoteFragment)remotesPagerAdapter.getItem(viewPager.getCurrentItem());
					
					RemotesManager.getInstance().setActiveRemote(page.getRemote());
				}
				
				//updateActiveRemote();
				//actionBar.setTitle(RemotesManager.getInstance().getActiveRemote().getName());
				
				
				/*actionBar.removeAllTabs();

				for (int i = 0; i < MainActivity.this.remotesPagerAdapter.getCount(); i++)
				{
					actionBar.addTab(actionBar.newTab().setText(MainActivity.this.remotesPagerAdapter.getPageTitle(i)).setTabListener(MainActivity.this));
				} */
			};
		});

		this.remotesPagerAdapter.reload(getApplicationContext());
	};

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		getMenuInflater().inflate(R.menu.main_menu, menu);

		if (null != DevicesManager.getInstance().getRemoteService())
		{
			menu.findItem(R.id.menu_refresh).setActionView(null);
		}
		else
		{
			menu.findItem(R.id.menu_refresh).setActionView(R.layout.actionbar_indeterminate_progress);
		}

		return true;
	};

	@Override
	public void onDisconnected()
	{
		DevicesManager.getInstance().setRemoteService(null);
		DevicesManager.getInstance().startScan(getApplicationContext(), this);
		
		invalidateOptionsMenu();
	};
	
	private void updateActiveRemote()
	{
		if (RemotesManager.getInstance().getActiveRemote() == null)
		{
			RemotesManager.getInstance().setActiveRemote(((RemoteFragment)this.remotesPagerAdapter.getItem(this.viewPager.getCurrentItem())).getRemote());
		}
	};

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case R.id.edit_remote:
				updateActiveRemote();
				Intent newIntent = new Intent(this, EditRemoteActivity.class);
				startActivityForResult(newIntent, 0xf00d);
				overridePendingTransition(R.anim.activity_in, R.anim.activity_out);
				return true;
			case R.id.add_remote:
				updateActiveRemote();
				RemotesManager.getInstance().addDefaultRemoteSirc("new remote", getApplicationContext());
				this.remotesPagerAdapter.reload(getApplicationContext());
				return true;
			case R.id.add_remote_button:
				updateActiveRemote();
				Remote remote = RemotesManager.getInstance().getActiveRemote();
				remote.getButtons().add(new RemoteButton(300, 300, 300, 300, "new b", false, 0));
				remote.save(getApplicationContext());
				RemoteFragment fragment = (RemoteFragment) this.remotesPagerAdapter.getItem(this.viewPager.getCurrentItem());
				fragment.buildUI();
				return true;
			case R.id.delete_remote:
				updateActiveRemote();
				AlertDialog.Builder builder = new AlertDialog.Builder(this);
				builder.setMessage("Delete remote?");
				builder.setCancelable(true);

				builder.setPositiveButton("Delete", new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int id)
					{
						RemotesManager.getInstance().getActiveRemote().delete(getApplicationContext());
						RemotesManager.getInstance().setActiveRemote(null);
						MainActivity.this.remotesPagerAdapter.reload(getApplicationContext());
						dialog.dismiss();
					};
				});

				builder.setNegativeButton("Cancel", new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int id)
					{
						dialog.cancel();
					};
				});

				builder.show();
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	};

	@Override
	protected void onPause()
	{
		DevicesManager.getInstance().stopScan(getApplicationContext());

		super.onPause();
	};

	@Override
	protected void onResume()
	{
		super.onResume();

		DevicesManager.getInstance().startScan(getApplicationContext(), this);
	};

	@Override
	public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction)
	{
	};

	@Override
	public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction)
	{
		this.viewPager.setCurrentItem(tab.getPosition());
	};

	@Override
	public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction)
	{
	};
};
