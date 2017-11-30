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

struct ExampleApp : public ctk::Application {
    ExampleApp() : Application("exampleApp") {}
    ~ExampleApp() { shutdown(); }

    ctk::DeviceModule dev{"Heater","heater"};
    ctk::DeviceModule timer{"Timer"};
    ctk::ControlSystemModule cs{"TemperatureController"};
    
    void defineConnections();
};
ExampleApp theExampleApp;

void ExampleApp::defineConnections() {
    mtca4u::setDMapFilePath("devices.dmap");

    auto triggerNr = timer("triggerNr", typeid(int), 1, ctk::UpdateMode::push);

    cs("heatingCurrent", typeid(int), 1) >> dev("heatingCurrent");
    dev("temperatureReadback", typeid(double), 1) [ triggerNr ]
        >> cs("temperatureReadback");
}

