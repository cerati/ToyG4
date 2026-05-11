// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "EventAction.hh"

#include "RunAction.hh"
#include "DetectorMessenger.hh"

#include "G4Event.hh"
#include "G4Exception.hh"
#include "G4ios.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"

#include <cmath>

namespace {
  const G4double kLArWValue = 23.6 * eV;
  const G4double kExcitonToIonRatio = 0.21;
  const G4double kRecombinationFraction = 0.5;
}

EventAction::EventAction(RunAction* runAction)
  : G4UserEventAction(),
    fRunAction(runAction),
    fRecord(),
    fVoxelMap(),
    fContributorMap(),
    fVoxelSize(5.0 * mm),
    fMessenger(0) {
  fMessenger = new DetectorMessenger(this);
}

EventAction::~EventAction() {
  delete fMessenger;
}

void EventAction::BeginOfEventAction(const G4Event* event) {
  fRecord = EventRecord();
  fRecord.eventNumber = event->GetEventID();

  G4PrimaryVertex* primaryVertex = event->GetPrimaryVertex();
  if (primaryVertex) {
    G4PrimaryParticle* primary = primaryVertex->GetPrimary();
    if (primary) {
      fRecord.pdgCode = primary->GetPDGcode();
      fRecord.px = primary->GetPx();
      fRecord.py = primary->GetPy();
      fRecord.pz = primary->GetPz();

      const G4double totalEnergy = primary->GetTotalEnergy();
      const G4double mass = primary->GetMass();
      fRecord.energy = totalEnergy - mass;
      fRecord.momentumAbs = std::sqrt(fRecord.px * fRecord.px +
                                      fRecord.py * fRecord.py +
                                      fRecord.pz * fRecord.pz);
    }
  }

  fVoxelMap.clear();
  fContributorMap.clear();
}

void EventAction::EndOfEventAction(const G4Event*) {

  for (VoxelMap::const_iterator it = fVoxelMap.begin(); it != fVoxelMap.end(); ++it) {
    const VoxelIndex& voxelIdx = it->first;
    const G4double totalEdep = it->second;

    const G4int ix = std::get<0>(voxelIdx);
    const G4int iy = std::get<1>(voxelIdx);
    const G4int iz = std::get<2>(voxelIdx);

    fRecord.cubeX.push_back((ix + 0.5) * fVoxelSize);
    fRecord.cubeY.push_back((iy + 0.5) * fVoxelSize);
    fRecord.cubeZ.push_back((iz + 0.5) * fVoxelSize);
    fRecord.edep.push_back(totalEdep);

    G4double maxContributorEdep = 0.;
    G4int dominantPdg = 0;
    G4int dominantTrackID = -1;

    for (ContributorMap::const_iterator contrib = fContributorMap.begin();
         contrib != fContributorMap.end(); ++contrib) {
      if (contrib->first.first == voxelIdx) {
        if (contrib->second > maxContributorEdep) {
          maxContributorEdep = contrib->second;
          dominantPdg = contrib->first.second.first;
          dominantTrackID = contrib->first.second.second;
        }
      }
    }

    fRecord.voxelDominantPdg.push_back(dominantPdg);
    fRecord.voxelDominantTrackID.push_back(dominantTrackID);
    
    G4double fraction = (totalEdep > 0.) ? (maxContributorEdep / totalEdep) : 0.;
    fRecord.voxelDominantFraction.push_back(fraction);

    const G4double nQuanta = totalEdep / kLArWValue;
    const G4double nIons = nQuanta / (1.0 + kExcitonToIonRatio);
    const G4double nExcitons = nQuanta - nIons;
    const G4double nRecombinedIons = kRecombinationFraction * nIons;

    fRecord.scintPhotons.push_back(nExcitons + nRecombinedIons);
    fRecord.ionizationElectrons.push_back((1.0 - kRecombinationFraction) * nIons);
  }

  fRunAction->FillEvent(fRecord);
}

void EventAction::AddEnergyDeposit(const G4ThreeVector& position, G4double edep,
                                   G4int pdgCode, G4int trackID) {
  const G4int ix = static_cast<G4int>(std::floor(position.x() / fVoxelSize));
  const G4int iy = static_cast<G4int>(std::floor(position.y() / fVoxelSize));
  const G4int iz = static_cast<G4int>(std::floor(position.z() / fVoxelSize));
  const VoxelIndex voxelIdx(ix, iy, iz);
  
  fVoxelMap[voxelIdx] += edep;
  
  const std::pair<G4int, G4int> contributor(pdgCode, trackID);
  fContributorMap[std::make_pair(voxelIdx, contributor)] += edep;
}

void EventAction::SetVoxelSize(G4double size) {
  if (size <= 0.) {
    G4Exception("EventAction::SetVoxelSize",
                "ToyG4Det001",
                JustWarning,
                "Voxel size must be positive. Ignoring invalid value.");
    return;
  }
  fVoxelSize = size;
  G4cout << "[ToyG4] Voxel size set to " << fVoxelSize / mm << " mm" << G4endl;
}

void EventAction::SetPrimaryParticleInfo(const G4ThreeVector& momentum,
                                         G4double energy,
                                         G4double momentumAbs,
                                         G4int pdgCode) {
  fRecord.px = momentum.x();
  fRecord.py = momentum.y();
  fRecord.pz = momentum.z();
  fRecord.energy = energy;
  fRecord.momentumAbs = momentumAbs;
  fRecord.pdgCode = pdgCode;
}
