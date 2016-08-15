using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace BlueIR
{
	public enum IrProtocol: int
	{
		SIRC = 0,
		NEC = 1,
		RC5 = 2


		/*
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
			};*/
	}
}
