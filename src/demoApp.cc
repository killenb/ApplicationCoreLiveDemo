#include <iostream>

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ApplicationCore/EnableXMLGenerator.h>
#include <ChimeraTK/ApplicationCore/Pipe.h>

namespace ctk = ChimeraTK;

struct Controller : public ctk::ApplicationModule {
    using ctk::ApplicationModule::ApplicationModule;

    ctk::ScalarPollInput<double> temperatureSetpoint{this, "temperatureSetpoint", "degC", "The setpoint for the temperature control", {"CS"}};
    ctk::ScalarPushInput<double> temperatureReadback{this, "temperatureReadback", "degC", "The measured temperature", {"CS", "HEATER"}};
    ctk::ScalarOutput<double> heatingCurrent{this, "heatingCurrent", "mA", "Supply current for the heating device", {"HEATER"}};
    
    void mainLoop() {
      const double gain = 100.0;

      while(true) {
        temperatureReadback.read();
        temperatureSetpoint.readNonBlocking();
        heatingCurrent = gain * (temperatureSetpoint - temperatureReadback);
        heatingCurrent.write();
      }
    }
};

struct ExampleApp : public ctk::Application {
    ExampleApp() : Application("exampleApp") {}
    ~ExampleApp() { shutdown(); }

    Controller controller{this, "controller", "Proportional controller for temperature"};
    ctk::ArrayPipe<int> supplyVoltages{this, "supplyVoltages", "mV", 4, "Supply voltages of our heating device", {"HEATER"}, {"CS"}};

    ctk::DeviceModule heater{"oven","heater"};
    ctk::DeviceModule timer{"timer"};
    ctk::ControlSystemModule cs{"Bakery"};
    
    void defineConnections();
};
ExampleApp theExampleApp;

void ExampleApp::defineConnections() {
    mtca4u::setDMapFilePath("devices.dmap");
    
    auto triggerNr = timer("triggerNr", typeid(int), 1, ctk::UpdateMode::push);
    triggerNr >> cs("triggerNr");

    findTag("HEATER").connectTo(heater, triggerNr);
    findTag("CS").connectTo(cs);
    
}

