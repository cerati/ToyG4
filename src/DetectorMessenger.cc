// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "DetectorMessenger.hh"

#include "EventAction.hh"

#include "G4ApplicationState.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "G4UIdirectory.hh"

DetectorMessenger::DetectorMessenger(EventAction* eventAction)
  : G4UImessenger(),
    fEventAction(eventAction),
    fDetectorDirectory(0),
    fSetVoxelSizeCmd(0) {
  fDetectorDirectory = new G4UIdirectory("/toyG4/detector/");
  fDetectorDirectory->SetGuidance("Detector geometry configuration.");

  fSetVoxelSizeCmd = new G4UIcmdWithADoubleAndUnit("/toyG4/detector/setVoxelSize", this);
  fSetVoxelSizeCmd->SetGuidance("Set the voxel size for energy deposition binning.");
  fSetVoxelSizeCmd->SetParameterName("size", false);
  fSetVoxelSizeCmd->SetDefaultValue(5.0);
  fSetVoxelSizeCmd->SetDefaultUnit("mm");
  fSetVoxelSizeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

DetectorMessenger::~DetectorMessenger() {
  delete fSetVoxelSizeCmd;
  delete fDetectorDirectory;
}

void DetectorMessenger::SetNewValue(G4UIcommand* command, G4String newValue) {
  if (command == fSetVoxelSizeCmd) {
    G4double size = fSetVoxelSizeCmd->GetNewDoubleValue(newValue);
    fEventAction->SetVoxelSize(size);
    return;
  }

  G4Exception("DetectorMessenger::SetNewValue",
              "ToyG4Det002",
              JustWarning,
              "Unknown detector command.");
}
