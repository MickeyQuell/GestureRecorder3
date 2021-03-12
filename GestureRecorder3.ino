
#include <Arduino_LSM9DS1.h>
const int Rx = 4;
const int Tx = 5;

UART hardwareUart (digitalPinToPinName(Tx), digitalPinToPinName(Rx), NC, NC);

void setup() 
{
  Serial.begin(250000);
  hardwareUart.begin(115200);
  if (IMU.begin(LSM9DS1Class::AccelerometerSpeeds::_952_16G, 
                LSM9DS1Class::GyroSpeeds::_952_16Hz))
  {
      Serial.println("IMU success!");
      IMU.setContinuousMode();
  }
  else
  {
      Serial.println("Failed to initialize IMU!");
      while (1);
  }
}

void DoublePrint(const char* msg)
{
  Serial.println(msg);
  hardwareUart.println(msg);
}
void FormatBuffer(char* buffer, int deviceType, long timeStamp, bool isButtonPressed, 
    float aX, float aY, float aZ, 
    float gX, float gY, float gZ, 
    float mX, float mY, float mZ);

int timeoutBeforeSend = 0;
void InterpretCommands(int numBytes);
char commandBuffer[2000];
int commandBufferIndex = 0;

void loop() 
{
  

    int numBytes = hardwareUart.available();
    if(numBytes)
    {
      InterpretCommands(numBytes);
    }
    if(millis() > timeoutBeforeSend)
    {
      float aX, aY, aZ, gX, gY, gZ, mX, mY, mZ;
      int deviceType = 1;
  
      if (IMU.accelerationAvailable() && IMU.gyroscopeAvailable())
      {
        char buffer[200];
        IMU.readAcceleration(aX, aY, aZ); // read the acceleration and gyroscope data
        IMU.readGyroscope(gX, gY, gZ);
        IMU.readMagneticField(mX, mY, mZ);
        long timeStamp = millis();
        //int isButtonPressed = (int)SampleButtonState();
        int isButtonPressed = 0;
  
        FormatBuffer(buffer, deviceType, timeStamp, isButtonPressed, aX, aY, aZ, gX, gY, gZ, mX, mY, mZ);
        DoublePrint(buffer);
      }
    }

  delay(2);
}

///////////////////////////////////////////////////////////

bool IsNumber(char value)
{
    if (value >= '0' && value <= '9')
        return true;
    return false;
}

bool IsAlphaNumeric(char value)
{
  if(IsNumber(value) == true)
    return true;
  if (value >= 'a' && value <= 'z')
        return true;
  if (value >= 'A' && value <= 'Z')
        return true;

  return false;
}
bool IsSymbolic(char value)
{
  if (value >= '!' && value <= '/')
        return true;
  if (value >= ':' && value <= '@')
        return true;
  if (value >= '[' && value <= '\'')
        return true;
  if (value >= '{' && value <= '~')
    return true;

  return false;
}

bool IsPrintableCharacter(int value)
{
  if (value >= '!' && value <= '~')
        return true;

  return false;
}

///////////////////////////////////////////////////////////

void InterpretCommands(int numBytes)
{
  //return;
  //hardwareUart.write(status);
  
  if(numBytes>2000)
    numBytes = 2000;
  char* ptr = commandBuffer + commandBufferIndex;
  int realLen = 0;

  for(int i=0; i<numBytes; i++)
  {
    int val = hardwareUart.read();
    if(val == -1)
    {
      *ptr = 0;
      break;
    }
    else
    { 
      if(IsPrintableCharacter(val))
      {
        *ptr++ = val; 
        realLen ++;
      }
    }
  }

  const char* status = "InterpretCommands: ";
  Serial.print(status);
  
  commandBufferIndex += realLen;
  commandBuffer[commandBufferIndex] = 0;
  Serial.println(commandBuffer);

  Serial.println(commandBufferIndex, DEC);
  const char* reset = "<reset>";
  if(strncmp(commandBuffer, reset, strlen(reset)) == 0)
  {
    Serial.println("resetting");
    timeoutBeforeSend = millis() + 1000;
    commandBufferIndex = 0;
  }
  const char* battery = "<battery>";
  if(strncmp(commandBuffer, battery, strlen(battery)) == 0)
  {
    Serial.println("sending battery status");
    const char * msg = "<battery>60</battery>\a";
    hardwareUart.write(msg);
    commandBufferIndex = 0;
  }

  if(commandBufferIndex > 64)// clearing the buffer;
    commandBufferIndex = 0;
}

////////////////////////////////////////////////////////////////

void FormatBuffer(char* buffer, int deviceType, long timeStamp, bool isButtonPressed, 
    float aX, float aY, float aZ, 
    float gX, float gY, float gZ, 
    float mX, float mY, float mZ)
{
  // Martin... here
    float accelMultiplier = 100.0f;
    float gyroMult = 100.0f;
    
    *buffer++ = '{';
    *buffer++ = deviceType + '0';// convert to printable number
    *buffer++ = ',';
    *buffer++ = (int)(isButtonPressed)+'0';// convert to printable number
    *buffer++ = '}';
    *buffer++ = '{';
    itoa((int)(aX * accelMultiplier), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = ',';
    itoa((int)(aY * accelMultiplier), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = ',';
    itoa((int)(aZ * accelMultiplier), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = '}';
    *buffer++ = ' ';
    *buffer++ = '{';
    itoa((int)(gX * gyroMult), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = ',';
    itoa((int)(gY * gyroMult), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = ',';
    itoa((int)(gZ * gyroMult), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = '}';
    *buffer++ = ' ';
    *buffer++ = '{';
    itoa((int)(mX * 100), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = ',';
    itoa((int)(mY * 100), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = ',';
    itoa((int)(mZ * 100), buffer, 10);
    buffer += strlen(buffer);
    *buffer++ = '}';
    *buffer++ = ' ';
    itoa(timeStamp, buffer, 10);
}
