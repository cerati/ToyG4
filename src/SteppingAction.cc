// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4LogicalVolume.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4TouchableHandle.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"

SteppingAction::SteppingAction(const DetectorConstruction* detector, EventAction* eventAction)
  : G4UserSteppingAction(),
    fDetector(detector),
    fEventAction(eventAction),
    fActiveLogicalVolume(0) {
}

SteppingAction::~SteppingAction() {
}

void SteppingAction::UserSteppingAction(const G4Step* step) {
  if (fActiveLogicalVolume == 0) {
    fActiveLogicalVolume = fDetector->GetActiveLogicalVolume();
  }

  G4double edep = step->GetTotalEnergyDeposit();
  if (edep <= 0.) {
    return;
  }

  G4VPhysicalVolume* volume = step->GetPreStepPoint()->GetTouchableHandle()->GetVolume();
  if (!volume || volume->GetLogicalVolume() != fActiveLogicalVolume) {
    return;
  }

  const G4ThreeVector midpoint =
    0.5 * (step->GetPreStepPoint()->GetPosition() + step->GetPostStepPoint()->GetPosition());
  
  const G4Track* track = step->GetTrack();
  const G4int pdgCode = track->GetDefinition()->GetPDGEncoding();
  const G4int trackID = track->GetTrackID();
  
  fEventAction->AddEnergyDeposit(midpoint, edep, pdgCode, trackID);
}
