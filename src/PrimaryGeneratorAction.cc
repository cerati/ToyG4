// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "PrimaryGeneratorAction.hh"

#include "EventAction.hh"

#include "G4Event.hh"
#include "G4MuonMinus.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction(EventAction* eventAction)
  : G4VUserPrimaryGeneratorAction(),
    fEventAction(eventAction),
    fParticleGun(new G4ParticleGun(1)) {
  G4ParticleDefinition* particle = G4MuonMinus::MuonMinusDefinition();
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
  const G4double minEnergy = 0.1 * GeV;
  const G4double maxEnergy = 10.0 * GeV;
  const G4double kineticEnergy = minEnergy + G4UniformRand() * (maxEnergy - minEnergy);
  const G4ThreeVector direction = G4RandomDirection();

  fParticleGun->SetParticleEnergy(kineticEnergy);
  fParticleGun->SetParticleMomentumDirection(direction);

  const G4double totalEnergy = kineticEnergy + G4MuonMinus::MuonMinusDefinition()->GetPDGMass();
  const G4double mass = G4MuonMinus::MuonMinusDefinition()->GetPDGMass();
  const G4double momentumMagnitude = std::sqrt(totalEnergy * totalEnergy - mass * mass);
  fEventAction->SetPrimaryMomentum(momentumMagnitude * direction);

  fParticleGun->GeneratePrimaryVertex(event);
}
