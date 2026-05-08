// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "QGSP_BERT.hh"
#include "G4EmStandardPhysics.hh"
#include "G4VModularPhysicsList.hh"

int main(int argc, char** argv) {
  G4RunManager* runManager = new G4RunManager;

  DetectorConstruction* detector = new DetectorConstruction();
  runManager->SetUserInitialization(detector);

  G4VModularPhysicsList* physicsList = new QGSP_BERT;
  physicsList->ReplacePhysics(new G4EmStandardPhysics());
  runManager->SetUserInitialization(physicsList);
  runManager->SetUserInitialization(new ActionInitialization(detector));

  G4UImanager* uiManager = G4UImanager::GetUIpointer();

  if (argc > 1) {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    uiManager->ApplyCommand(command + fileName);
  } else {
    uiManager->ApplyCommand("/control/execute run.mac");
  }

  delete runManager;
  return 0;
}
