package eu.sarunas.apps.android.smartremote;

import java.lang.reflect.Field;
import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.view.ViewConfiguration;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebView;
import android.widget.EditText;

public class BaseActivity
{
	public static int getNumber(EditText field)
	{
		String value = field.getText().toString();

		if (value.length() > 0)
		{
			try
			{
				return Integer.parseInt(value);
			}
			catch (NumberFormatException ex)
			{
				return 0;
			}
		}

		return 0;
	};

	private static void getOverflowMenu(Activity activity)
	{
		try
		{
			ViewConfiguration config = ViewConfiguration.get(activity);
			Field menuKeyField = ViewConfiguration.class.getDeclaredField("sHasPermanentMenuKey");
			if (menuKeyField != null)
			{
				menuKeyField.setAccessible(true);
				menuKeyField.setBoolean(config, false);
			}
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	};

	@SuppressLint("NewApi")
	@SuppressWarnings("deprecation")
	public static void updateTitle(Activity activity, boolean hasBack, boolean hasOverflow, String title)
	{
		ActionBar bar = activity.getActionBar();

		if (null != bar)
		{
			if (true == hasBack)
			{
				bar.setHomeButtonEnabled(true);
				bar.setIcon(R.drawable.abc_ic_ab_back_mtrl_am_alpha);
			}
			else
			{
				bar.setIcon(new ColorDrawable(activity.getResources().getColor(android.R.color.transparent)));
			}

			bar.setBackgroundDrawable(new ColorDrawable(0xff658cdd));

			bar.setTitle(title);
		}

		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
		{
			Window window = activity.getWindow();
			window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
			window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);

			int color = 0xff658cdd;

			float hsv[] = { 0.0f, 0.0f, 0.0f };

			Color.colorToHSV(color, hsv);
			hsv[2] = 0.9f;

			window.setStatusBarColor(Color.HSVToColor(Color.alpha(color), hsv));
		}

		if (hasOverflow)
		{
			BaseActivity.getOverflowMenu(activity);
		}
	};
	
	public static void showPage(Activity activity, int id, int protocol)
	{
		WebView web = (WebView)activity.findViewById(id);

		switch (protocol)
		{
			case 0:
				web.loadUrl("https://www.google.com/search?q=sirc+remote+addresses");
				break;
			case 1:
				web.loadUrl("https://www.google.com/search?q=nec+remote+addresses");
				break;
			case 2:
				web.loadUrl("https://en.wikipedia.org/wiki/RC-5#System_Number_Allocations");
				break;
			default:
				break;
		}
	};	
};
