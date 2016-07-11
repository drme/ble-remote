package eu.sarunas.apps.android.smartremote;

public enum IrProtocol
{
	SIRC(0), NEC(1), RC5(2);

	IrProtocol(int id)
	{
		this.id = id;
	};

	public int getId()
	{
		return this.id;
	};

	private int id;

	public static IrProtocol getById(int selectedItemPosition)
	{
		switch (selectedItemPosition)
		{
			case 0:
				return SIRC;
			case 1:
				return NEC;
			default:
				return RC5;
		}
	};
};
