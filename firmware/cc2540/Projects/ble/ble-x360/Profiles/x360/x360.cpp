#pragma once

enum Pot
{
	LSX = 1,
	LSY = 2;
};

class X360
{
	public:
		X360();

		void setButtons(uint16_t buttonsFlag);
		void setPot(Pot pot, uint16_t, value);
		
		void onConnected();
		void onDisconnected();
};
