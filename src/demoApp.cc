#include <iostream>

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ApplicationCore/EnableXMLGenerator.h>
#include <mtca4u/Utilities.h>

namespace ctk = ChimeraTK;

struct Controller : public ctk::ApplicationModule {
    using ctk::ApplicationModule::ApplicationModule;

    ctk::ScalarPollInput<double> temperatureSetpoint{this, "temperatureSetpoint", "degC", "The setpoint for the temperature control"};
    ctk::ScalarPushInput<double> temperatureReadback{this, "temperatureReadback", "degC", "The measured temperature"};
    ctk::ScalarOutput<double> heatingCurrent{this, "heatingCurrent", "mA", "Supply current for the heating device"};
    
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

    ctk::DeviceModule heater{"oven","heater"};
    ctk::DeviceModule timer{"timer"};
    ctk::ControlSystemModule cs{"Bakery"};
    
    void defineConnections();
};
ExampleApp theExampleApp;

void ExampleApp::defineConnections() {
    mtca4u::setDMapFilePath("devices.dmap");

    auto triggerNr = timer("triggerNr", typeid(int), 1, ctk::UpdateMode::push);

    cs("temperatureSetpoint") >> controller.temperatureSetpoint;
    controller.heatingCurrent >> heater("heatingCurrent");
    heater("temperatureReadback") [ triggerNr ] >> controller.temperatureReadback
        >> cs("temperatureReadback");

    heater("supplyVoltages", typeid(int), 4) [ triggerNr ] >> cs("supplyVoltages");
    triggerNr >> cs("triggerNr");
}

