package eu.sarunas.apps.android.smartremote;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;

public class EditButtonActivity extends Activity
{
	public static final String buttonId = "fa12";
	public static final String remoteId = "fa11";
	private RemoteButton button = null;
	private EditText commandField = null;
	private EditText heightField = null;
	private Remote remote = null;
	private EditText titleField = null;
	private EditText widthField = null;
	private EditText xField = null;
	private EditText yField = null;
	private EditText addressField = null;
	private Spinner protocolField = null;

	@Override
	public void onBackPressed()
	{
		save();

		super.onBackPressed();

		overridePendingTransition(R.anim.activity_back_in, R.anim.activity_back_out);
	};

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		BaseActivity.updateTitle(this, true, false, "Edit button");

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_edit_button);

		Intent intent = getIntent();

		this.remote = RemotesManager.getInstance().getActiveRemote();
		this.button = this.remote.getButtons().get(intent.getIntExtra(EditButtonActivity.buttonId, -1));

		this.titleField = (EditText) findViewById(R.id.editTextTitle);
		this.commandField = (EditText) findViewById(R.id.editTextCommand);
		this.xField = (EditText) findViewById(R.id.editTextX);
		this.yField = (EditText) findViewById(R.id.editTextY);
		this.widthField = (EditText) findViewById(R.id.editTextWidth);
		this.heightField = (EditText) findViewById(R.id.editTextHeight);
		this.addressField = (EditText) findViewById(R.id.editTextAddress2);
		this.protocolField = (Spinner) findViewById(R.id.spinnerProtocol2);

		this.titleField.setText(this.button.getTitle());
		this.commandField.setText("" + this.button.getCommand());
		this.xField.setText("" + this.button.getX());
		this.yField.setText("" + this.button.getY());
		this.widthField.setText("" + this.button.getWidth());
		this.heightField.setText("" + this.button.getHeight());
		this.addressField.setText("" + ((this.button.getAddress() != null) ? this.button.getAddress() : this.remote.getAddress()));
		this.protocolField.setSelection(null != this.button.getType() ? this.button.getType().getId() : this.remote.getType().getId());

		((Button) findViewById(R.id.buttonTestCommand)).setOnClickListener(new OnClickListener()
		{
			@Override
			public void onClick(View view)
			{
				save();

				EditButtonActivity.this.button.sendCommand(EditButtonActivity.this.remote);
			};
		});

		this.protocolField.setOnItemSelectedListener(new OnItemSelectedListener()
		{
			@Override
			public void onItemSelected(AdapterView<?> adapterview, View view, int i, long l)
			{
				BaseActivity.showPage(EditButtonActivity.this, R.id.webView, EditButtonActivity.this.protocolField.getSelectedItemPosition());
			};

			@Override
			public void onNothingSelected(AdapterView<?> adapterview)
			{
			};
		});

		BaseActivity.showPage(this, R.id.webView, this.protocolField.getSelectedItemPosition());
	};

	@Override
	public boolean onOptionsItemSelected(MenuItem item)
	{
		switch (item.getItemId())
		{
			case android.R.id.home:
				save();
				NavUtils.navigateUpFromSameTask(this);
				overridePendingTransition(R.anim.activity_back_in, R.anim.activity_back_out);
				break;
			default:
				return super.onOptionsItemSelected(item);
		}

		return true;
	};

	private void save()
	{
		this.button.setTitle(this.titleField.getText().toString());
		this.button.setCommand(BaseActivity.getNumber(this.commandField));
		this.button.setX(BaseActivity.getNumber(this.xField));
		this.button.setY(BaseActivity.getNumber(this.yField));
		this.button.setWidth(BaseActivity.getNumber(this.widthField));
		this.button.setHeight(BaseActivity.getNumber(this.heightField));

		int address = BaseActivity.getNumber(this.addressField);

		this.button.setAddress(address == this.remote.getAddress() ? null : address);

		this.button.setType(this.protocolField.getSelectedItemPosition() == this.remote.getType().getId() ? null : IrProtocol.getById(this.protocolField.getSelectedItemPosition()));

		this.remote.save(this);
	};
};
