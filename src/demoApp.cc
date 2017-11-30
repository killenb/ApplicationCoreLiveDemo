/*
 * demoApp.cc
 *
 *  Created on: Nov 30, 2017
 *      Author: Martin Hierholzer
 */

#include <iostream>

#include <ChimeraTK/ApplicationCore/ApplicationCore.h>
#include <ChimeraTK/ApplicationCore/EnableXMLGenerator.h>

namespace ctk = ChimeraTK;

struct Automation : public ctk::ApplicationModule {
    using ctk::ApplicationModule::ApplicationModule;

    ctk::ScalarPollInput<double> operatorSetpoint{this, "operatorSetpoint", "degC", "Temperature setpoint requested by the operator"};
    ctk::ScalarOutput<double> actualSetpoint{this, "actualSetpoint", "degC", "Temperature setpoint passed on to the controller"};
    ctk::ScalarPushInput<int> trigger{this, "trigger", "", "Trigger from the timing system"};
    
    void mainLoop() {
      while(true) {
        trigger.read();
        operatorSetpoint.readLatest();
        if(std::abs(operatorSetpoint - actualSetpoint) > 0.01) {
          actualSetpoint += std::max( std::min(operatorSetpoint - actualSetpoint, 2.0), -2.0);
          actualSetpoint.write();
        }
      }
    }
};

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

    Automation automation{this, "automation", "Automated setpoint ramping"};
    Controller controller{this, "controller", "Proportional controller for temperature"};

    ctk::DeviceModule dev{"Heater","heater"};
    ctk::DeviceModule timer{"Timer"};
    ctk::ControlSystemModule cs{"TemperatureController"};
    
    void defineConnections();
    
};
ExampleApp theExampleApp;


void ExampleApp::defineConnections() {
    mtca4u::setDMapFilePath("devices.dmap");

    cs("temperatureSetpoint") >> automation.operatorSetpoint;
    automation.actualSetpoint >> controller.temperatureSetpoint >> cs("actualSetpoint");
    
    auto triggerNr = timer("triggerNr", typeid(int), 1, ctk::UpdateMode::push);
    triggerNr >> automation.trigger;
    
    controller.heatingCurrent >> dev("heatingCurrent");
    dev("temperatureReadback") [ triggerNr ] >> controller.temperatureReadback >> cs("temperatureReadback");

}

