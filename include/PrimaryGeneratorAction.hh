// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef PRIMARYGENERATORACTION_HH
#define PRIMARYGENERATORACTION_HH

#include "globals.hh"
#include "G4VUserPrimaryGeneratorAction.hh"

#include <vector>

class EventAction;
class G4Event;
class G4ParticleGun;
class PrimaryGeneratorMessenger;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction {
public:
  enum SamplingMode { kLinear, kLog };

  struct EnergyConfig {
    G4int pdgCode;
    G4double minEnergy;
    G4double maxEnergy;
    SamplingMode mode;
  };

  explicit PrimaryGeneratorAction(EventAction* eventAction);
  virtual ~PrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event* event);

  void SetEnergyRangeForPdg(G4int pdgCode, G4double minEnergy, G4double maxEnergy,
                             SamplingMode mode = kLinear);
  void ClearConfiguredPdgs();
  void PrintConfiguration() const;

private:
  void EnsureDefaultConfiguration();

  EventAction* fEventAction;
  G4ParticleGun* fParticleGun;
  std::vector<EnergyConfig> fPdgEntries;
  PrimaryGeneratorMessenger* fMessenger;
};

#endif
