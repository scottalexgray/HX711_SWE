#include "HX711_SWE.h"



HX711_SWE::HX711_SWE(byte dout, byte pd_sck, byte gain) :
  PD_SCK(pd_sck), DOUT(dout) {
	switch (gain) {
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
    default:
      GAIN = 1;
      break;
	}
}

void HX711_SWE::begin() 
{
	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);
	digitalWrite(PD_SCK, LOW);



	long value = read();

	if(value == 999999)
	{
		Serial.println("Loadcell reading timed out");
	}
}

/*void HX711_SWE::begin(byte dout, byte pd_sck, byte gain) {
 	PD_SCK = pd_sck;
	DOUT = dout;
	pinMode(PD_SCK, OUTPUT);
	pinMode(DOUT, INPUT);
  set_gain(gain);
}*/

/*void HX711_SWE::set_gain(byte gain) {
	switch (gain) {
		case 128:		// channel A, gain factor 128
			GAIN = 1;
			break;
		case 64:		// channel A, gain factor 64
			GAIN = 3;
			break;
		case 32:		// channel B, gain factor 32
			GAIN = 2;
			break;
    default:
      GAIN = 1;
      break;
	}

	digitalWrite(PD_SCK, LOW);
	read();
}*/

long HX711_SWE::read(time_t timeout) 
{
	power_up();

	unsigned long value = 0;


	time32_t loadcellReadyTimeout = 5000;
	time32_t startMillis = millis();  
	while((millis() - startMillis) < loadcellReadyTimeout && !is_ready())
	{
		//do nothing
	}
	if(is_ready())
	{
		//continue
	}
	else
	{
		value = 999999;
		return value;
	}

	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	// pulse the clock pin 24 times to read the data
	data[2] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[1] = shiftIn(DOUT, PD_SCK, MSBFIRST);
	data[0] = shiftIn(DOUT, PD_SCK, MSBFIRST);

	// set the channel and the gain factor for the next reading using the clock pin
	for (unsigned int i = 0; i < GAIN; i++) 
	{
		digitalWrite(PD_SCK, HIGH);
		digitalWrite(PD_SCK, LOW);
	}

	// Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = static_cast<unsigned long>(filler)  << 24
		  | static_cast<unsigned long>(data[2]) << 16
		  | static_cast<unsigned long>(data[1]) << 8
		  | static_cast<unsigned long>(data[0]) ;

	//power off the loadcell amplifier
	power_down(); 
	delay(150);

	return static_cast<long>(value);
}

long HX711_SWE::read_average(byte times) {
  if (times <= 0) return NAN;
	long sum = 0;
	for (byte i = 0; i < times; i++) {
		sum += read();
		yield();
	}
	return sum / times;
}

double HX711_SWE::get_value(byte times) {
	return read_average(times) - OFFSET;
}

float HX711_SWE::get_units(byte times) {
	return get_value(times) / SCALE;
}

void HX711_SWE::tare(byte times) {
	double sum = read_average(times);
	set_offset(sum);
}

void HX711_SWE::set_scale(float scale) {
  if (scale) {
	  SCALE = scale;
  }
  else {
    SCALE = 1;
  }
}

float HX711_SWE::get_scale() {
	return SCALE;
}

void HX711_SWE::set_offset(long offset) {
	OFFSET = offset;
}

long HX711_SWE::get_offset() {
	return OFFSET;
}

void HX711_SWE::power_down() {
	digitalWrite(PD_SCK, LOW);
	digitalWrite(PD_SCK, HIGH);
	delay(150);
}

void HX711_SWE::power_up() {
	digitalWrite(PD_SCK, LOW);
}
