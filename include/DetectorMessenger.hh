// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef DETECTORMESSENGER_HH
#define DETECTORMESSENGER_HH

#include "G4UImessenger.hh"

class G4UIdirectory;
class G4UIcmdWithADoubleAndUnit;
class G4UIcommand;
class EventAction;

class DetectorMessenger : public G4UImessenger {
public:
  explicit DetectorMessenger(EventAction* eventAction);
  virtual ~DetectorMessenger();

  virtual void SetNewValue(G4UIcommand* command, G4String newValue);

private:
  EventAction* fEventAction;
  G4UIdirectory* fDetectorDirectory;
  G4UIcmdWithADoubleAndUnit* fSetVoxelSizeCmd;
};

#endif
