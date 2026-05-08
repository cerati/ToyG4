// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef DETECTORCONSTRUCTION_HH
#define DETECTORCONSTRUCTION_HH

#include "G4VUserDetectorConstruction.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction {
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();

  virtual G4VPhysicalVolume* Construct();

  G4LogicalVolume* GetActiveLogicalVolume() const;

private:
  G4LogicalVolume* fActiveLogicalVolume;
};

#endif
