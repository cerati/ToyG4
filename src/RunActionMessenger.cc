#include "RunActionMessenger.hh"

#include "RunAction.hh"

#include "G4ApplicationState.hh"
#include "G4Exception.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIdirectory.hh"

RunActionMessenger::RunActionMessenger(RunAction* runAction)
  : G4UImessenger(),
    fRunAction(runAction),
    fRunDirectory(0),
    fSetOutputFileCmd(0),
    fSetOutputFormatCmd(0) {
  fRunDirectory = new G4UIdirectory("/toyG4/run/");
  fRunDirectory->SetGuidance("Run-level configuration.");

  fSetOutputFileCmd = new G4UIcmdWithAString("/toyG4/run/setOutputFile", this);
  fSetOutputFileCmd->SetGuidance("Set output file path/name.");
  fSetOutputFileCmd->SetParameterName("filePath", false);
  fSetOutputFileCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

  fSetOutputFormatCmd = new G4UIcmdWithAString("/toyG4/run/setOutputFormat", this);
  fSetOutputFormatCmd->SetGuidance("Set output format: root or hdf5.");
  fSetOutputFormatCmd->SetParameterName("format", false);
  fSetOutputFormatCmd->SetDefaultValue("root");
  fSetOutputFormatCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

RunActionMessenger::~RunActionMessenger() {
  delete fSetOutputFormatCmd;
  delete fSetOutputFileCmd;
  delete fRunDirectory;
}

void RunActionMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
  if (command == fSetOutputFileCmd) {
    if (newValue.empty()) {
      G4Exception("RunActionMessenger::SetNewValue",
                  "ToyG4Run001",
                  JustWarning,
                  "Output file path is empty. Configuration unchanged.");
      return;
    }

    fRunAction->SetOutputFileName(newValue);
    return;
  }

  if (command == fSetOutputFormatCmd) {
    fRunAction->SetOutputFormat(newValue);
    return;
  }

  G4Exception("RunActionMessenger::SetNewValue",
              "ToyG4Run002",
              JustWarning,
              "Unknown run command.");
}
