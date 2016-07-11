package eu.sarunas.apps.android.smartremote;

import java.util.ArrayList;
import java.util.List;
import android.annotation.SuppressLint;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;

public class RemoteFragment extends Fragment
{
	private static final int editButtonId = 1;
	private static final int moveButtonId = 2;
	private static final int removeButtonId = 0;
	private RemoteButton editableButton = null;
	private View editableView = null;
	private LinearLayout moveLayout = null;
	private Remote remote = null;
	private FrameLayout rootView = null;

	public RemoteFragment(Remote remote)
	{
		this.remote = remote;
	};

	@SuppressLint("InflateParams")
	public void buildUI()
	{
		List<View> toRemove = new ArrayList<View>();

		for (int i = 0; i < this.rootView.getChildCount(); i++)
		{
			View view = this.rootView.getChildAt(i);

			if (!(view instanceof LinearLayout))
			{
				toRemove.add(view);
			}
		}

		for (View view : toRemove)
		{
			this.rootView.removeView(view);
		}

		for (final RemoteButton button : this.remote.getButtons())
		{
			Button controllButton = (Button) getLayoutInflater(null).inflate(R.layout.remote_button, null);

			controllButton.setText(button.getTitle());
			setLayout(controllButton, button);

			controllButton.setOnClickListener(new OnClickListener()
			{
				@Override
				public void onClick(View view)
				{
					button.sendCommand(remote);
				};
			});

			controllButton.setTag(button);

			((ViewGroup) this.rootView).addView(controllButton);

			registerForContextMenu(controllButton);
		}
	};

	public Remote getRemote()
	{
		return this.remote;
	};

	@Override
	public void onActivityResult(int requestCode, int resultCode, Intent data)
	{
		super.onActivityResult(requestCode, resultCode, data);

		buildUI();
	};

	@Override
	public boolean onContextItemSelected(MenuItem item)
	{
		int id = this.remote.hashCode();

		if (item.getGroupId() == id)
		{
			switch (item.getItemId())
			{
				case editButtonId:
					Intent intent = new Intent(getActivity(), EditButtonActivity.class);
					intent.putExtra(EditButtonActivity.remoteId, this.remote.getId());
					intent.putExtra(EditButtonActivity.buttonId, this.remote.getButtons().indexOf(item.getActionView().getTag()));
					startActivityForResult(intent, 0xf00d);
					getActivity().overridePendingTransition(R.anim.activity_in, R.anim.activity_out);
					break;
				case removeButtonId:
					this.remote.getButtons().remove(item.getActionView().getTag());
					this.remote.save(getActivity());
					buildUI();
					break;
				case moveButtonId:
					this.moveLayout.setVisibility(View.VISIBLE);
					this.editableView = item.getActionView();
					this.editableButton = (RemoteButton) this.editableView.getTag();
					break;
				default:
					break;
			}
		}

		return super.onContextItemSelected(item);
	};

	@Override
	public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo)
	{
		int id = this.remote.hashCode();

		menu.add(id, RemoteFragment.editButtonId, 0, "Edit button").setActionView(v);
		menu.add(id, RemoteFragment.removeButtonId, 0, "Remove button").setActionView(v);
		menu.add(id, RemoteFragment.moveButtonId, 0, "Move button").setActionView(v);
	};

	@Override
	public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState)
	{
		this.rootView = (FrameLayout) inflater.inflate(R.layout.fragment_main, container, false);
		this.moveLayout = (LinearLayout) this.rootView.findViewById(R.id.movePanel);

		this.moveLayout.setVisibility(View.INVISIBLE);

		((Button) this.moveLayout.findViewById(R.id.buttonUp)).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				if (null != RemoteFragment.this.editableButton)
				{
					RemoteFragment.this.editableButton.setY(RemoteFragment.this.editableButton.getY() - 1);
					setLayout((Button) RemoteFragment.this.editableView, RemoteFragment.this.editableButton);
				}
			};
		});

		((Button) this.moveLayout.findViewById(R.id.buttonDown)).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				if (null != RemoteFragment.this.editableButton)
				{
					RemoteFragment.this.editableButton.setY(RemoteFragment.this.editableButton.getY() + 1);
					setLayout((Button) RemoteFragment.this.editableView, RemoteFragment.this.editableButton);
				}
			};
		});

		((Button) this.moveLayout.findViewById(R.id.buttonLeft)).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				if (null != RemoteFragment.this.editableButton)
				{
					RemoteFragment.this.editableButton.setX(RemoteFragment.this.editableButton.getX() - 1);
					setLayout((Button) RemoteFragment.this.editableView, RemoteFragment.this.editableButton);
				}
			};
		});

		((Button) this.moveLayout.findViewById(R.id.buttonRight)).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				if (null != RemoteFragment.this.editableButton)
				{
					RemoteFragment.this.editableButton.setX(RemoteFragment.this.editableButton.getX() + 1);
					setLayout((Button) RemoteFragment.this.editableView, RemoteFragment.this.editableButton);
				}
			};
		});

		((Button) this.moveLayout.findViewById(R.id.buttonDone)).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				RemoteFragment.this.moveLayout.setVisibility(View.INVISIBLE);
				RemoteFragment.this.remote.save(getActivity());
			};
		});

		buildUI();

		return this.rootView;
	};

	private void setLayout(Button button, RemoteButton remoteButton)
	{
		FrameLayout.LayoutParams layout = new FrameLayout.LayoutParams(remoteButton.getWidth(), remoteButton.getHeight());

		layout.leftMargin = remoteButton.getX();
		layout.topMargin = remoteButton.getY();

		button.setLayoutParams(layout);
	};
};
