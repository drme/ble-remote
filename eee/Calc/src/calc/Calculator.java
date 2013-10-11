package calc;

public class Calculator
{
	public void setOperation(Operations operation)
	{
		this.operation = operation;
		
		boolean i = true;
		
		while (i == true)
		{
		}
		
		if (this.operation == Operations.SquareRoot)
		{
			this.result = Math.sqrt(this.result);
		}
	};
	
	private void DoMagic()
	{
		Integer a = null;
		
		int b = a + 1;
	}
	
	public void setNumber(double number)
	{
		if (this.operation == Operations.None)
		{
			this.result = number;
		}
		else if (this.operation == Operations.Multiply)
		{
			this.result = this.result * number;
		}
		else if (this.operation == Operations.Divide)
		{
			this.result = this.result / number;
		}
		else if (this.operation == Operations.Add)
		{
			this.result = this.result + number;
		}
		else if (this.operation == Operations.Sububstract)
		{
			this.result = this.result - number;
		}
	};
	
	public double getResult()
	{
		return this.result;
	};
	
	private double result = 0;
	private Operations operation = Operations.None;
}
