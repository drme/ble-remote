package eu.sarunas.apps.android.smartremote;

import java.io.IOException;
import java.util.Locale;
import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.AlertDialog;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.Intent;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.support.v4.app.NavUtils;
import android.support.v4.view.ViewPager;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends FragmentActivity implements ActionBar.TabListener, IConnectionHandler
{


	/**
	 * The {@link android.support.v4.view.PagerAdapter} that will provide fragments for each of the sections. We use a {@link android.support.v4.app.FragmentPagerAdapter} derivative, which will keep every loaded fragment in memory. If this becomes too memory intensive, it may be best to switch to a {@link android.support.v4.app.FragmentStatePagerAdapter}.
	 */
	SectionsPagerAdapter mSectionsPagerAdapter;

	/**
	 * The {@link ViewPager} that will host the section contents.
	 */
	ViewPager mViewPager;

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		this.handler = new Handler();
		
		
		// Set up the action bar.
		final ActionBar actionBar = getActionBar();
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);

		// Create the adapter that will return a fragment for each of the three
		// primary sections of the app.
		mSectionsPagerAdapter = new SectionsPagerAdapter(getSupportFragmentManager());

		// Set up the ViewPager with the sections adapter.
		mViewPager = (ViewPager) findViewById(R.id.pager);
		mViewPager.setAdapter(mSectionsPagerAdapter);

		// When swiping between different sections, select the corresponding
		// tab. We can also use ActionBar.Tab#select() to do this if we have
		// a reference to the Tab.
		mViewPager.setOnPageChangeListener(new ViewPager.SimpleOnPageChangeListener()
		{
			@Override
			public void onPageSelected(int position)
			{
				actionBar.setSelectedNavigationItem(position);
			}
		});

		// For each of the sections in the app, add a tab to the action bar.
		for (int i = 0; i < mSectionsPagerAdapter.getCount(); i++)
		{
			// Create a tab with text corresponding to the page title defined by
			// the adapter. Also specify this Activity object, which implements
			// the TabListener interface, as the callback (listener) for when
			// this tab is selected.
			actionBar.addTab(actionBar.newTab().setText(mSectionsPagerAdapter.getPageTitle(i)).setTabListener(this));
		}
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu)
	{
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main_menu, menu);
		return true;
	}

	@Override
	public void onTabSelected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction)
	{
		// When the given tab is selected, switch to the corresponding page in
		// the ViewPager.
		mViewPager.setCurrentItem(tab.getPosition());
	}

	@Override
	public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction)
	{
	}

	@Override
	public void onTabReselected(ActionBar.Tab tab, FragmentTransaction fragmentTransaction)
	{
	}

	/**
	 * A {@link FragmentPagerAdapter} that returns a fragment corresponding to one of the sections/tabs/pages.
	 */
	public class SectionsPagerAdapter extends FragmentPagerAdapter
	{

		public SectionsPagerAdapter(FragmentManager fm)
		{
			super(fm);
		}

		@Override
		public Fragment getItem(int position)
		{
			if (0 == position)
			{
					return new MainFragment();
			}
			
			
			// getItem is called to instantiate the fragment for the given page.
			// Return a DummySectionFragment (defined as a static inner class
			// below) with the page number as its lone argument.
			Fragment fragment = new DummySectionFragment();
			Bundle args = new Bundle();
			args.putInt(DummySectionFragment.ARG_SECTION_NUMBER, position + 1);
			fragment.setArguments(args);
			return fragment;
		}

		@Override
		public int getCount()
		{
			// Show 3 total pages.
			return 2;
		}

		@Override
		public CharSequence getPageTitle(int position)
		{
			Locale l = Locale.getDefault();
			switch (position)
			{
				case 0:
					return getString(R.string.title_section1).toUpperCase(l);
				case 1:
					return getString(R.string.title_section2).toUpperCase(l);
			}
			return null;
		}
	}

	/**
	 * A dummy fragment representing a section of the app, but that simply displays dummy text.
	 */
	public static class DummySectionFragment extends Fragment
	{
		/**
		 * The fragment argument representing the section number for this fragment.
		 */
		public static final String ARG_SECTION_NUMBER = "section_number";

		public DummySectionFragment()
		{
		}

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
		{
			View rootView = inflater.inflate(R.layout.fragment_main_dummy, container, false);
			TextView dummyTextView = (TextView) rootView.findViewById(R.id.section_label);
			dummyTextView.setText(Integer.toString(getArguments().getInt(ARG_SECTION_NUMBER)));
			return rootView;
		}
	}
	
	
	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case R.id.menu_connect:
                Intent newIntent = new Intent(this, DeviceScanActivity.class);
                startActivityForResult(newIntent, REQUEST_SELECT_DEVICE);
				return true;
			default:
				return super.onOptionsItemSelected(item);
		}
	};	
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		if (REQUEST_SELECT_DEVICE == requestCode)
		{
			if (resultCode == RESULT_OK)
			{
				String deviceAddress = data.getExtras().getString(BluetoothDevice.EXTRA_DEVICE);
			
				connectLE(deviceAddress);
			}
		}
	};
	
	private void connectLE(String deviceAddress)
	{
		this.progress = ProgressDialog.show(this, "", "Connecting to: " + deviceAddress, true);
		
		try
		{
			tellyService = null;
			
			new TellyService(deviceAddress, this, this);
		}
		catch (Throwable ex)
		{
			tellyService = null;

			AlertDialog alertDialog = new AlertDialog.Builder(this).create();
			alertDialog.setTitle("BT connect error: " + deviceAddress);
			alertDialog.setMessage(ex.getMessage());

			alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener()
			{
				public void onClick(DialogInterface dialog, int whichButton)
				{
				}
			});

			alertDialog.show();
		}
	};	

	public static class MainFragment extends Fragment
	{
		public MainFragment()
		{
		};

		@Override
		public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
		{
			View rootView = inflater.inflate(R.layout.fragment_main, container, false);

			((Button) rootView.findViewById(R.id.buttonPower)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendPower();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonSource)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendSource();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonHome)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendHome();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonBack)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendBack();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonVolumeUp)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendVolumeUp();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonVolumeDown)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendVolumeDown();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonOk)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendOk();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonUp)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendUp();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonDown)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendDown();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonLeft)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendLeft();

					return false;
				}
			});

			((Button) rootView.findViewById(R.id.buttonRight)).setOnTouchListener(new OnTouchListener()
			{
				@Override
				public boolean onTouch(View arg0, MotionEvent arg1)
				{
					if (null != tellyService)
						tellyService.sendRight();

					return false;
				}
			});

			return rootView;
		};
	}
	
	@Override
	public void onConnected(final TellyService service)
	{
		tellyService = service;
		
		this.handler.post(new Runnable()
		{
			public void run()
			{
				Toast.makeText(getApplicationContext(), "Connected to: " + "client", Toast.LENGTH_SHORT).show();
			}
		});

		if (null != this.progress)
		{
			this.progress.dismiss();
			this.progress = null;
		}
	};

	@Override
	public void onDisconnected(final TellyService client, final String message)
	{
		tellyService = null;
		
		if (null != this.progress)
		{
			this.progress.dismiss();
			this.progress = null;
		}

		if (null != message)
		{
			this.handler.post(new Runnable()
			{
				public void run()
				{
					Toast.makeText(getApplicationContext(), "Dissonnected", Toast.LENGTH_SHORT).show();
					
					AlertDialog alertDialog = new AlertDialog.Builder(MainActivity.this).create();
					alertDialog.setTitle("BT connect error");
					alertDialog.setMessage(message);

					alertDialog.setButton(AlertDialog.BUTTON_POSITIVE, "Ok", new DialogInterface.OnClickListener()
					{
						public void onClick(DialogInterface dialog, int whichButton)
						{
						}
					});

					alertDialog.show();
				}
			});
		}
	};

    private Handler handler = null;
    private ProgressDialog progress = null;
	private static TellyService tellyService = null;
	private static final int REQUEST_SELECT_DEVICE = 2;
};
