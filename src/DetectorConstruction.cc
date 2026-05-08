// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "DetectorConstruction.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4Orb.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Box.hh"

DetectorConstruction::DetectorConstruction()
  : G4VUserDetectorConstruction(),
    fActiveLogicalVolume(0) {
}

DetectorConstruction::~DetectorConstruction() {
}

G4VPhysicalVolume* DetectorConstruction::Construct() {
  G4NistManager* nist = G4NistManager::Instance();

  G4Material* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
  G4Material* liquidArgon = nist->FindOrBuildMaterial("G4_lAr");

  G4double activeRadius = 20.0 * m;
  G4double worldHalfLength = 22.0 * m;

  G4Box* solidWorld = new G4Box("World", worldHalfLength, worldHalfLength, worldHalfLength);
  G4LogicalVolume* logicWorld = new G4LogicalVolume(solidWorld, vacuum, "World");
  G4VPhysicalVolume* physWorld = new G4PVPlacement(
    0, G4ThreeVector(), logicWorld, "World", 0, false, 0, true);

  G4Orb* solidActive = new G4Orb("ActiveVolume", activeRadius);
  fActiveLogicalVolume = new G4LogicalVolume(solidActive, liquidArgon, "ActiveVolume");
  new G4PVPlacement(
    0, G4ThreeVector(), fActiveLogicalVolume, "ActiveVolume", logicWorld, false, 0, true);

  return physWorld;
}

G4LogicalVolume* DetectorConstruction::GetActiveLogicalVolume() const {
  return fActiveLogicalVolume;
}
