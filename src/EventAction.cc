// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "EventAction.hh"

#include "RunAction.hh"

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"

#include <cmath>

namespace {
  const G4double kVoxelSize = 5.0 * mm;
  const G4double kLArWValue = 23.6 * eV;
  const G4double kExcitonToIonRatio = 0.21;
  const G4double kRecombinationFraction = 0.5;
}

EventAction::EventAction(RunAction* runAction)
  : G4UserEventAction(),
    fRunAction(runAction),
    fRecord(),
    fVoxelMap() {
}

EventAction::~EventAction() {
}

void EventAction::BeginOfEventAction(const G4Event* event) {
  fRecord = EventRecord();
  fRecord.eventNumber = event->GetEventID();
  fVoxelMap.clear();
}

void EventAction::EndOfEventAction(const G4Event* event) {
  G4PrimaryVertex* primaryVertex = event->GetPrimaryVertex();
  if (primaryVertex) {
    G4PrimaryParticle* primary = primaryVertex->GetPrimary();
    if (primary) {
      fRecord.px = primary->GetPx();
      fRecord.py = primary->GetPy();
      fRecord.pz = primary->GetPz();
    }
  }

  for (VoxelMap::const_iterator it = fVoxelMap.begin(); it != fVoxelMap.end(); ++it) {
    const G4int ix = std::get<0>(it->first);
    const G4int iy = std::get<1>(it->first);
    const G4int iz = std::get<2>(it->first);

    fRecord.cubeX.push_back((ix + 0.5) * kVoxelSize);
    fRecord.cubeY.push_back((iy + 0.5) * kVoxelSize);
    fRecord.cubeZ.push_back((iz + 0.5) * kVoxelSize);
    fRecord.edep.push_back(it->second);

    const G4double nQuanta = it->second / kLArWValue;
    const G4double nIons = nQuanta / (1.0 + kExcitonToIonRatio);
    const G4double nExcitons = nQuanta - nIons;
    const G4double nRecombinedIons = kRecombinationFraction * nIons;

    fRecord.scintPhotons.push_back(nExcitons + nRecombinedIons);
    fRecord.ionizationElectrons.push_back((1.0 - kRecombinationFraction) * nIons);
  }

  fRunAction->FillEvent(fRecord);
}

void EventAction::AddEnergyDeposit(const G4ThreeVector& position, G4double edep) {
  const G4int ix = static_cast<G4int>(std::floor(position.x() / kVoxelSize));
  const G4int iy = static_cast<G4int>(std::floor(position.y() / kVoxelSize));
  const G4int iz = static_cast<G4int>(std::floor(position.z() / kVoxelSize));
  fVoxelMap[VoxelIndex(ix, iy, iz)] += edep;
}

void EventAction::SetPrimaryMomentum(const G4ThreeVector& momentum) {
  fRecord.px = momentum.x();
  fRecord.py = momentum.y();
  fRecord.pz = momentum.z();
}
