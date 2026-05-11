// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "PrimaryGeneratorAction.hh"

#include "EventAction.hh"
#include "PrimaryGeneratorMessenger.hh"

#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4ios.hh"
#include "G4MuonMinus.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4RandomDirection.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"

#include <cmath>
#include <iterator>

PrimaryGeneratorAction::PrimaryGeneratorAction(EventAction* eventAction)
  : G4VUserPrimaryGeneratorAction(),
    fEventAction(eventAction),
    fParticleGun(new G4ParticleGun(1)),
    fPdgEntries(),
    fMessenger(0) {
  G4ParticleDefinition* particle = G4MuonMinus::MuonMinusDefinition();
  fParticleGun->SetParticleDefinition(particle);
  fParticleGun->SetParticlePosition(G4ThreeVector(0., 0., 0.));
  SetEnergyRangeForPdg(13, 0.1 * GeV, 10.0 * GeV);
  fMessenger = new PrimaryGeneratorMessenger(this);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction() {
  delete fMessenger;
  delete fParticleGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event) {
  EnsureDefaultConfiguration();

  const std::size_t entryCount = fPdgEntries.size();
  std::size_t selectedIndex = static_cast<std::size_t>(G4UniformRand() * entryCount);
  if (selectedIndex >= entryCount) {
    selectedIndex = entryCount - 1;
  }

  const EnergyConfig& selected = fPdgEntries[selectedIndex];

  G4ParticleDefinition* particle =
      G4ParticleTable::GetParticleTable()->FindParticle(selected.pdgCode);
  if (!particle) {
    G4Exception("PrimaryGeneratorAction::GeneratePrimaries",
                "ToyG4Gen010",
                FatalException,
                "Configured PDG code has no Geant4 particle definition.");
    return;
  }

  fParticleGun->SetParticleDefinition(particle);

  const G4double minEnergy = selected.minEnergy;
  const G4double maxEnergy = selected.maxEnergy;
  G4double kineticEnergy;
  if (selected.mode == kLog) {
    kineticEnergy = minEnergy * std::pow(maxEnergy / minEnergy, G4UniformRand());
  } else {
    kineticEnergy = minEnergy + G4UniformRand() * (maxEnergy - minEnergy);
  }
  const G4ThreeVector direction = G4RandomDirection();

  fParticleGun->SetParticleEnergy(kineticEnergy);
  fParticleGun->SetParticleMomentumDirection(direction);

  const G4double mass = particle->GetPDGMass();
  const G4double totalEnergy = kineticEnergy + mass;
  const G4double momentumMagnitude = std::sqrt(totalEnergy * totalEnergy - mass * mass);
  fEventAction->SetPrimaryParticleInfo(momentumMagnitude * direction,
                                       kineticEnergy,
                                       momentumMagnitude,
                                       particle->GetPDGEncoding());

  fParticleGun->GeneratePrimaryVertex(event);
}

void PrimaryGeneratorAction::SetEnergyRangeForPdg(G4int pdgCode,
                                                  G4double minEnergy,
                                                  G4double maxEnergy,
                                                  SamplingMode mode) {
  G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(pdgCode);
  if (!particle) {
    G4Exception("PrimaryGeneratorAction::SetEnergyRangeForPdg",
                "ToyG4Gen011",
                JustWarning,
                "Unknown PDG code. Configuration unchanged.");
    return;
  }

  if (minEnergy <= 0. || maxEnergy < minEnergy) {
    G4Exception("PrimaryGeneratorAction::SetEnergyRangeForPdg",
                "ToyG4Gen012",
                JustWarning,
                "Invalid energy range. Require min > 0 and max >= min.");
    return;
  }

  fPdgEntries.push_back(EnergyConfig{pdgCode, minEnergy, maxEnergy, mode});

  G4cout << "[ToyG4] Added PDG " << pdgCode
         << " (" << particle->GetParticleName() << ")"
         << " with kinetic energy range [" << minEnergy / GeV << ", "
         << maxEnergy / GeV << "] GeV"
         << " (sampling: " << (mode == kLog ? "log" : "linear") << ")" << G4endl;
}

void PrimaryGeneratorAction::ClearConfiguredPdgs() {
  fPdgEntries.clear();
  G4cout << "[ToyG4] Cleared all configured PDG ranges." << G4endl;
}

void PrimaryGeneratorAction::PrintConfiguration() const {
  if (fPdgEntries.empty()) {
    G4cout << "[ToyG4] No PDG ranges configured. Default will be restored at generation time." << G4endl;
    return;
  }

  G4cout << "[ToyG4] Configured PDG entries (" << fPdgEntries.size() << "):" << G4endl;
  for (std::vector<EnergyConfig>::const_iterator it = fPdgEntries.begin();
       it != fPdgEntries.end();
       ++it) {
    G4ParticleDefinition* particle = G4ParticleTable::GetParticleTable()->FindParticle(it->pdgCode);
    const G4String particleName = particle ? particle->GetParticleName() : "<unknown>";
    G4cout << "  PDG " << it->pdgCode << " (" << particleName << "): ["
           << it->minEnergy / GeV << ", "
           << it->maxEnergy / GeV << "] GeV"
           << " [" << (it->mode == kLog ? "log" : "linear") << "]" << G4endl;
  }
}

void PrimaryGeneratorAction::EnsureDefaultConfiguration() {
  if (!fPdgEntries.empty()) {
    return;
  }

  G4Exception("PrimaryGeneratorAction::EnsureDefaultConfiguration",
              "ToyG4Gen013",
              JustWarning,
              "No PDG ranges configured. Restoring default PDG 13 in [0.1, 10] GeV.");
  SetEnergyRangeForPdg(13, 0.1 * GeV, 10.0 * GeV);
}
