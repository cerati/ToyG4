// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef EVENTACTION_HH
#define EVENTACTION_HH

#include "EventData.hh"
#include "G4UserEventAction.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"

class G4Event;
class RunAction;
class DetectorMessenger;

class EventAction : public G4UserEventAction {
public:
  explicit EventAction(RunAction* runAction);
  virtual ~EventAction();

  virtual void BeginOfEventAction(const G4Event* event);
  virtual void EndOfEventAction(const G4Event* event);

  void AddEnergyDeposit(const G4ThreeVector& position, G4double edep,
                        G4int pdgCode, G4int trackID);
  void SetPrimaryParticleInfo(const G4ThreeVector& momentum,
                              G4double energy,
                              G4double momentumAbs,
                              G4int pdgCode);

  void SetVoxelSize(G4double size);
  G4double GetVoxelSize() const { return fVoxelSize; }

private:
  RunAction* fRunAction;
  EventRecord fRecord;
  VoxelMap fVoxelMap;
  ContributorMap fContributorMap;
  G4double fVoxelSize;
  DetectorMessenger* fMessenger;
};

#endif
