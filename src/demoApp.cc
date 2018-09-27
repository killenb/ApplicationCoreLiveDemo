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

struct Oven : public ctk::ModuleGroup {
    using ctk::ModuleGroup::ModuleGroup;
    
    Controller controller{this, "controller", "Proportional controller for temperature"};
    ctk::ArrayPipe<int> supplyVoltages{this, "supplyVoltages", "mV", 4, "Supply voltages of our heating device", {"HEATER"}, {"CS"}};
        
};

struct ExampleApp : public ctk::Application {
    ExampleApp() : Application("exampleApp") {}
    ~ExampleApp() { shutdown(); }

    std::vector<Oven> ovens;
    std::vector<ctk::DeviceModule> heaters;

    ctk::DeviceModule timer{"timer"};
    ctk::ControlSystemModule cs{"Bakery"};
    
    void defineConnections();
};
ExampleApp theExampleApp;

void ExampleApp::defineConnections() {
    ctk::setDMapFilePath("devices.dmap");
    
    auto triggerNr = timer("triggerNr", typeid(int), 1, ctk::UpdateMode::push);
    triggerNr >> cs("triggerNr");
    
    for(size_t i=0; i<2; ++i) {
      ovens.emplace_back(this, "oven"+std::to_string(i), "Oven "+std::to_string(i));
      heaters.emplace_back("oven"+std::to_string(i),"heater");

      ovens[i].findTag("HEATER").flatten().connectTo(heaters[i], triggerNr);
    }

    findTag("CS").connectTo(cs);
    
}

