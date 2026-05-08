// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef ACTIONINITIALIZATION_HH
#define ACTIONINITIALIZATION_HH

#include "G4VUserActionInitialization.hh"

class DetectorConstruction;

class ActionInitialization : public G4VUserActionInitialization {
public:
  explicit ActionInitialization(const DetectorConstruction* detector);
  virtual ~ActionInitialization();

  virtual void Build() const;

private:
  const DetectorConstruction* fDetector;
};

#endif
