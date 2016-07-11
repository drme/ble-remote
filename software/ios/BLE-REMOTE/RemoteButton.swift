import Foundation;
import EVReflection;

class RemoteButton: EVObject
{
	var command = 0;
	var x = 0;
	var y = 0;
	var width = 100;
	var height = 100;
	var title = "";
	var round = false;
	var type: Int = -1;
	var address: Int = -1;

	convenience init(x: Int, y: Int, w: Int, h: Int, title: String, round: Bool, command: Int)
	{
		self.init(x: x, y: y, w: w, h: h, title: title, round: round, command: command, address: -1);
	}

	init(x: Int, y: Int, w: Int, h: Int, title: String, round: Bool, command: Int, address: Int)
	{
		self.x = x;
		self.y = y;
		self.width = w;
		self.height = h;
		self.command = command;
		self.title = title;
		self.round = round;
		self.address = address;
	}
	
	required init()
	{
	}

	var bounds: CGRect
	{
		return CGRect(x: CGFloat(self.x), y: CGFloat(self.y), width: CGFloat(self.width), height: CGFloat(self.height));
	}
}
