package calc;

import junit.framework.Assert;

import org.junit.Test;

public class CalculatorTests
{
	@Test
	public void testMultiply2by5()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Multiply);
		calc.setNumber(5);
		
		double result = calc.getResult();
		
		Assert.assertEquals(10.0, result);
	};

	@Test
	public void testDivide2by5()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Divide);
		calc.setNumber(5);
		
		double result = calc.getResult();
		
		Assert.assertEquals(0.4, result);
	};

	@Test
	public void testDivide2by0()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Divide);
		calc.setNumber(0);
		
		double result = calc.getResult();
		
		Assert.assertEquals(Double.POSITIVE_INFINITY, result);
	};

	@Test
	public void testDivide2by1()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Divide);
		calc.setNumber(1);
		
		double result = calc.getResult();
		
		Assert.assertEquals(2.0, result);
	};


	@Test
	public void testAdd2plus5()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Add);
		calc.setNumber(5);
		
		double result = calc.getResult();
		
		Assert.assertEquals(7.0, result);
	};

	@Test
	public void tes2minus5()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Sububstract);
		calc.setNumber(5);
		
		double result = calc.getResult();
		
		Assert.assertEquals(-3.0, result);
	};

	@Test
	public void tes2minusNeg5()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(2);
		calc.setOperation(Operations.Sububstract);
		calc.setNumber(-5);
		
		double result = calc.getResult();
		
		Assert.assertEquals(7.0, result);
	};

	@Test
	public void testQS2()
	{
		Calculator calc = new Calculator();
		
		calc.setNumber(4);
		calc.setOperation(Operations.SquareRoot);
		
		double result = calc.getResult();
		
		Assert.assertEquals(2.0, result);
	};
};
