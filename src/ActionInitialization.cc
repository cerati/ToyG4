// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "ActionInitialization.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"

ActionInitialization::ActionInitialization(const DetectorConstruction* detector)
  : G4VUserActionInitialization(),
    fDetector(detector) {
}

ActionInitialization::~ActionInitialization() {
}

void ActionInitialization::Build() const {
  RunAction* runAction = new RunAction();
  SetUserAction(runAction);

  EventAction* eventAction = new EventAction(runAction);
  SetUserAction(eventAction);
  SetUserAction(new PrimaryGeneratorAction(eventAction));
  SetUserAction(new SteppingAction(fDetector, eventAction));
}
