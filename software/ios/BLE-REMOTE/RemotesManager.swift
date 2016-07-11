import Foundation;

class RemotesManager
{
	static let sharedInstance = RemotesManager();

	var remotes: [Remote]
	{
		var result = [Remote]();

		let documentsUrl =  NSFileManager.defaultManager().URLsForDirectory(.DocumentDirectory, inDomains: .UserDomainMask).first!;

		do
		{
			let directoryContents = try NSFileManager.defaultManager().contentsOfDirectoryAtURL( documentsUrl, includingPropertiesForKeys: nil, options: []);

			let files = directoryContents.filter{ $0.pathExtension == "json" };
			let fileNames = files.flatMap({$0.URLByDeletingPathExtension?.lastPathComponent});

			for file in fileNames
			{
				let remote = Remote(fileNameInDocuments: file + ".json");

				print("Loaded [" + remote.name + "] - " + file);

				result.append(remote);
			}

		}
		catch let error as NSError
		{
			print(error.localizedDescription);
		}

		if (result.count <= 0)
		{
			result.append(addDefaultRemote1("demo sony telly"));
			result.append(addDefaultRemote2("demo philips hifi"));
			result.append(addDefaultRemote3("demo casio projector"));
		}

		return result;
	}

	func addDefaultRemote2(name: String) -> Remote
	{
		let remote = Remote();

		remote.address = 1;
		remote.name = name;
		remote.type = IrProtocol.RC5;

		let c = 70;

		remote.buttons.append(RemoteButton(x: 8, y: 14 + c, w: 95, h: 46, title: "CD", round: false, command: 63, address: 20));
		remote.buttons.append(RemoteButton(x: 218, y: 14 + c, w: 95, h: 46, title: "Power", round: false, command: 12, address: 20));
		remote.buttons.append(RemoteButton(x: 123, y: 154 + c, w: 74, h: 74, title: "Play", round: true, command: 53, address: 20));

		//remote.buttons.append(RemoteButton(x: 123, y: 72 + c, w: 74, h: 74, title: "/./UpButton", round: true, command: 116));
		//remote.buttons.append(RemoteButton(x: 123, y: 236 + c, w: 74, h: 74, title: "/./DownButton", round: true, command: 117));
		remote.buttons.append(RemoteButton(x: 41, y: 154 + c, w: 74, h: 74, title: "/./LeftButton", round: true, command: 33, address: 20));
		remote.buttons.append(RemoteButton(x: 205, y: 154 + c, w: 74, h: 74, title: "/./RightButton", round: true, command: 32, address: 20));

		remote.buttons.append(RemoteButton(x: 8, y: 328 + c, w: 46, h: 46, title: "/./DownButton", round: false, command: 17, address: 16));
		remote.buttons.append(RemoteButton(x: 70, y: 328 + c, w: 46, h: 46, title: "/./UpButton", round: false, command: 16, address: 16));

		remote.buttons.append(RemoteButton(x: 129, y: 328 + c, w: 74, h: 46, title: "Tape", round: false, command: 63, address: 18));
		remote.buttons.append(RemoteButton(x: 218, y: 328 + c, w: 94, h: 46, title: "Aux", round: false, command: 63, address: 21));

		remote.save();

		return remote;
	}

	func addDefaultRemote1(name: String) -> Remote
	{
		let remote = Remote();

		remote.address = 1;
		remote.name = name;
		remote.type = IrProtocol.SIRC;

		let c = 70;

		remote.buttons.append(RemoteButton(x: 8, y: 14 + c, w: 95, h: 46, title: "Source", round: false, command: 37));
		remote.buttons.append(RemoteButton(x: 218, y: 14 + c, w: 95, h: 46, title: "Power", round: false, command: 21));
		remote.buttons.append(RemoteButton(x: 123, y: 154 + c, w: 74, h: 74, title: "OK", round: true, command: 101));

		remote.buttons.append(RemoteButton(x: 123, y: 72 + c, w: 74, h: 74, title: "/./UpButton", round: true, command: 116));
		remote.buttons.append(RemoteButton(x: 123, y: 236 + c, w: 74, h: 74, title: "/./DownButton", round: true, command: 117));
		remote.buttons.append(RemoteButton(x: 41, y: 154 + c, w: 74, h: 74, title: "/./LeftButton", round: true, command: 52));
		remote.buttons.append(RemoteButton(x: 205, y: 154 + c, w: 74, h: 74, title: "/./RightButton", round: true, command: 51));

		remote.buttons.append(RemoteButton(x: 8, y: 328 + c, w: 46, h: 46, title: "/./DownButton", round: false, command: 19));
		remote.buttons.append(RemoteButton(x: 70, y: 328 + c, w: 46, h: 46, title: "/./UpButton", round: false, command: 18));

		remote.buttons.append(RemoteButton(x: 129, y: 328 + c, w: 74, h: 46, title: "Back", round: false, command: 116)); // command ???
		remote.buttons.append(RemoteButton(x: 218, y: 328 + c, w: 94, h: 46, title: "Home", round: false, command: 96));

		remote.save();

		return remote;
	}

	func addDefaultRemote3(name: String) -> Remote
	{
		let remote = Remote();

		remote.address = 62596;
		remote.name = name;
		remote.type = IrProtocol.NEC;

		let c = 70;

		remote.buttons.append(RemoteButton(x: 8, y: 14 + c, w: 95, h: 46, title: "Input", round: false, command: 10));
		remote.buttons.append(RemoteButton(x: 218, y: 14 + c, w: 95, h: 46, title: "Power", round: false, command: 11));
		remote.buttons.append(RemoteButton(x: 123, y: 154 + c, w: 74, h: 74, title: "Enter", round: true, command: 76));

		remote.buttons.append(RemoteButton(x: 123, y: 72 + c, w: 74, h: 74, title: "/./UpButton", round: true, command: 74));
		remote.buttons.append(RemoteButton(x: 123, y: 236 + c, w: 74, h: 74, title: "/./DownButton", round: true, command: 75));
		remote.buttons.append(RemoteButton(x: 41, y: 154 + c, w: 74, h: 74, title: "/./LeftButton", round: true, command: 77));
		remote.buttons.append(RemoteButton(x: 205, y: 154 + c, w: 74, h: 74, title: "/./RightButton", round: true, command: 78));

		remote.buttons.append(RemoteButton(x: 8, y: 328 + c, w: 46, h: 46, title: "/./DownButton", round: false, command: 47));
		remote.buttons.append(RemoteButton(x: 70, y: 328 + c, w: 46, h: 46, title: "/./UpButton", round: false, command: 44));

		remote.buttons.append(RemoteButton(x: 129, y: 328 + c, w: 74, h: 46, title: "ESC", round: false, command: 14));
		remote.buttons.append(RemoteButton(x: 218, y: 328 + c, w: 94, h: 46, title: "Menu", round: false, command: 12));

		remote.save();

		return remote;
	}
}
