import Foundation;

class StyledButton: UIButton
{
	var remoteButton: RemoteButton!;

	init(frame: CGRect, round: Bool)
	{
		super.init(frame: frame);

		self.backgroundColor = UIColor(white: 1.0, alpha: 0.3);

		if (true == round)
		{
			self.layer.cornerRadius = 0.5 * self.bounds.size.width
		}
		else
		{
			self.layer.cornerRadius = 4;
		}

		self.layer.borderColor = UIColor(white: 1.0, alpha: 0.7).CGColor;
		self.layer.borderWidth = 2.0;
		self.clipsToBounds = true;
	}
	
	required init?(coder aDecoder: NSCoder)
	{
		super.init(coder: aDecoder);
	}
}
