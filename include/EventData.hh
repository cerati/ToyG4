// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef EVENTDATA_HH
#define EVENTDATA_HH

#include "globals.hh"

#include <map>
#include <tuple>
#include <vector>

struct ContributorInfo {
  G4int pdgCode = 0;
  G4int trackID = -1;
  G4double energy = 0.;
};

struct EventRecord {
  G4int eventNumber = -1;
  G4int pdgCode = 0;
  G4double px = 0.;
  G4double py = 0.;
  G4double pz = 0.;
  G4double energy = 0.;
  G4double momentumAbs = 0.;
  std::vector<G4double> cubeX;
  std::vector<G4double> cubeY;
  std::vector<G4double> cubeZ;
  std::vector<G4double> edep;
  std::vector<G4int> voxelDominantPdg;
  std::vector<G4int> voxelDominantTrackID;
  std::vector<G4double> voxelDominantFraction;
  std::vector<G4double> scintPhotons;
  std::vector<G4double> ionizationElectrons;
};

typedef std::tuple<G4int, G4int, G4int> VoxelIndex;
typedef std::map<VoxelIndex, G4double> VoxelMap;
typedef std::map<VoxelIndex, ContributorInfo> DominantContributorMap;

#endif
