import Foundation;
import EVReflection;

class Remote: EVObject
{
	var name = "";
	var type = IrProtocol.SIRC;
	var address = 0;
	var buttons = [RemoteButton]();
	var id = NSUUID().UUIDString;

	required init()
	{
	}
	
	func save()
	{
		self.saveToDocuments(self.id + ".json");
	}

	func delete()
	{
		let filePath = (NSSearchPathForDirectoriesInDomains(.DocumentDirectory, .UserDomainMask, true)[0] as NSString).stringByAppendingPathComponent(self.id + ".json");

		let fileManager = NSFileManager.defaultManager();

		do
		{
			try fileManager.removeItemAtPath(filePath)
		}
		catch let error as NSError
		{
			print("Ooops! Something went wrong: \(error)")
		}
	}
}
