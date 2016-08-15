using System;
using System.Threading.Tasks;

namespace BlueIR
{
	public class RemoteButton
	{
		public int? Address
		{
			get;
			set;
		}

		public int Command
		{
			get;
			set;
		}

		public int Height
		{
			get;
			set;
		}

		public bool Round
		{
			get;
			set;
		}

		public String Title
		{
			get;
			set;
		}

		public IrProtocol? Type
		{
			get;
			set;
		}

		public int Width
		{
			get;
			set;
		}

		public int X
		{
			get;
			set;
		}

		public int Y
		{
			get;
			set;
		}

		public RemoteButton()
		{
			this.Address = null;
			this.Command = 0;
			this.Height = 100;
			this.Round = false;
			this.Type = null;
			this.Width = 100;
			this.X = 0;
			this.Y = 0;
			this.Title = "";
		}

		public RemoteButton(float x, float y, float w, float h, String title, bool round, int command) : this(x, y, w, h, title, round, command, null)
		{
		}

		public RemoteButton(float x, float y, float w, float h, String title, bool round, int command, int? address) : this((int)x, (int)y, (int)w, (int)h, title, round, command)
		{
		}

		public RemoteButton(int x, int y, int w, int h, String title, bool round, int command)
		{
			this.X = x;
			this.Y = y;
			this.Width = w;
			this.Height = h;
			this.Command = command;
			this.Title = title;
			this.Round = round;
		}

		public async Task SendCommand(Remote remote)
		{
			var service = DevicesManager.Instance.RemoteService;

			if (null != service)
			{
				//await Task.Delay(10);

				if (null != this.Type)
				{
					await service.SendProtocol(this.Type);
				}
				else
				{
					await service.SendProtocol(remote.Type);
				}

				//await Task.Delay(10);

				if (null != this.Address)
				{
					await service.SendAddress(this.Address);
				}
				else
				{
					await service.SendAddress(remote.Address);
				}

				//await Task.Delay(10);

				await service.SendCommand(this.Command);
			}
		}
	}
}
