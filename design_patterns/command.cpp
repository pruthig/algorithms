// A simple C++ program to demonstrate implementation of Command Pattern using a remote control example.
#include<iostream>

using namespace std;

// An interface for command
class Command
{
public:
  virtual void execute() {}
};

// Light class and its corresponding command classes
class Light
{
public:
  void on()
  {
    cout<<"Light is on";
  }
  void off()
  {
    cout<<"Light is off";
  }
};

class LightOnCommand : public Command
{
  Light light;
public:

  // The constructor is passed the light it
  // is going to control.
  LightOnCommand() {}
  LightOnCommand(Light light)
  {
    light = light;
  }
  void execute() override
  {
    light.on();
  }
};


class LightOffCommand : public Command
{
  Light light;
public:
  LightOffCommand() {}
  LightOffCommand(Light light)
  {
    light = light;
  }
  void execute() override
  {
    light.off();
  }
};

// Stereo and its command classes
class Stereo
{
public:
  Stereo() {}
  void on()
  {
    cout<<"Stereo is on";
  }
  void off()
  {
    cout<<"Stereo is off";
  }
  void setCD()
  {
    cout<<"Stereo is set for CD input";
  }
  void setDVD()
  {
    cout<<"Stereo is set for DVD input";
  }
  void setRadio()
  {
    cout<<"Stereo is set for Radio";
  }
  void setVolume(int volume)
  {
    // code to set the volume
    cout << "Stereo volume set to " << volume;
  }
};

class StereoOffCommand : public Command
{
  Stereo stereo;
public:
  StereoOffCommand() {}

  StereoOffCommand(Stereo stereo)
  {
    stereo = stereo;
  }
  void execute() override
  {
    stereo.off();
  }
};

class StereoOnWithCDCommand : public Command
{
  Stereo stereo;
public:
  StereoOnWithCDCommand() {}
  StereoOnWithCDCommand(Stereo stereo)
  {
    stereo = stereo;
  }
  void execute() override
  {
    stereo.on();
    stereo.setCD();
    stereo.setVolume(11);
  }
};

// A Simple remote control with one button
class SimpleRemoteControl
{
  Command slot; // only one button
public:
  SimpleRemoteControl()
  {
  }

  void setCommand(Command command)
  {
    // set the command the remote will
    // execute
    slot = command;
  }

  void buttonWasPressed()
  {
    slot.execute();
  }
};

// Driver class
int main()
{
  auto *remote = new SimpleRemoteControl();
  //auto *light = new Light();
  LightOnCommand light;
  StereoOnWithCDCommand stereoCD;
  StereoOffCommand stereoOff;
  //auto *stereo = new Stereo();

  // we can change command dynamically
  remote->setCommand(light);
  remote->buttonWasPressed();
  remote->setCommand(stereoCD);
  remote->buttonWasPressed();
  remote->setCommand(stereoOff);
  remote->buttonWasPressed();
  return 0;
}
