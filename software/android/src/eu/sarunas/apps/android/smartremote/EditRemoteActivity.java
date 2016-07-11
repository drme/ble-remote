package eu.sarunas.apps.android.smartremote;

import android.app.Activity;
import android.os.Bundle;
import android.support.v4.app.NavUtils;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.EditText;
import android.widget.Spinner;

public class EditRemoteActivity extends Activity
{
	private EditText addressField = null;
	private Spinner protocolField = null;
	private Remote remote = null;
	private EditText titleField = null;

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
		BaseActivity.updateTitle(this, true, false, "Edit remote");

		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_edit_remote);

		this.remote = RemotesManager.getInstance().getActiveRemote();

		this.titleField = (EditText) findViewById(R.id.editTextName);
		this.addressField = (EditText) findViewById(R.id.editTextAddress);
		this.protocolField = (Spinner) findViewById(R.id.spinnerProtocol);

		this.titleField.setText(this.remote.getName());
		this.addressField.setText("" + this.remote.getAddress());
		this.protocolField.setSelection(this.remote.getType().getId());

		this.protocolField.setOnItemSelectedListener(new OnItemSelectedListener()
		{
			@Override
			public void onItemSelected(AdapterView<?> adapterview, View view, int i, long l)
			{
				BaseActivity.showPage(EditRemoteActivity.this, R.id.webViewProtocols, EditRemoteActivity.this.protocolField.getSelectedItemPosition());
			};

			@Override
			public void onNothingSelected(AdapterView<?> adapterview)
			{
			};
		});

		BaseActivity.showPage(this, R.id.webViewProtocols, this.protocolField.getSelectedItemPosition());
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
		this.remote.setName(this.titleField.getText().toString());
		this.remote.setAddress(BaseActivity.getNumber(this.addressField));
		this.remote.setType(IrProtocol.getById(this.protocolField.getSelectedItemPosition()));

		this.remote.save(this);
	};
};
