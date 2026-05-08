// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef STEPPINGACTION_HH
#define STEPPINGACTION_HH

#include "G4UserSteppingAction.hh"

class DetectorConstruction;
class EventAction;
class G4LogicalVolume;
class G4Step;

class SteppingAction : public G4UserSteppingAction {
public:
  SteppingAction(const DetectorConstruction* detector, EventAction* eventAction);
  virtual ~SteppingAction();

  virtual void UserSteppingAction(const G4Step* step);

private:
  const DetectorConstruction* fDetector;
  EventAction* fEventAction;
  G4LogicalVolume* fActiveLogicalVolume;
};

#endif
