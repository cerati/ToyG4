// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "globals.hh"
#include "G4UserRunAction.hh"

#include <cstdint>
#include <memory>
#include <vector>

#ifdef TOYG4_USE_HDF5
#include "hdf5.h"
#endif

class G4Run;
class TFile;
class TTree;
class RunActionMessenger;

struct EventRecord;

class RunAction : public G4UserRunAction {
public:
  RunAction();
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run* run);
  virtual void EndOfRunAction(const G4Run* run);

  void FillEvent(const EventRecord& record);
  void SetOutputFileName(const G4String& fileName);
  const G4String& GetOutputFileName() const;
  void SetOutputFormat(const G4String& format);
  const G4String& GetOutputFormat() const;

private:
  G4String fOutputFileName;
  G4String fOutputFormat;
  RunActionMessenger* fMessenger;
  std::unique_ptr<TFile> fOutputFile;
  TTree* fTree;

#ifdef TOYG4_USE_HDF5
  hid_t fH5File;
  hid_t fH5DsetPdgCode;
  hid_t fH5DsetEnergy;
  hid_t fH5DsetEdepFlat;
  hid_t fH5DsetCubeXFlat;
  hid_t fH5DsetCubeYFlat;
  hid_t fH5DsetCubeZFlat;
  hid_t fH5DsetOffsets;
  std::uint64_t fH5CurrentOffset;
#endif

  int fEventNumber;
  int fPdgCode;
  double fPx;
  double fPy;
  double fPz;
  double fEnergy;
  double fMomentumAbs;
  std::vector<double> fCubeX;
  std::vector<double> fCubeY;
  std::vector<double> fCubeZ;
  std::vector<double> fEdep;
  std::vector<int> fVoxelDominantPdg;
  std::vector<int> fVoxelDominantTrackID;
  std::vector<double> fVoxelDominantFraction;
  std::vector<double> fScintPhotons;
  std::vector<double> fIonizationElectrons;

  // Progress bar state
  int fTotalEvents;
  int fProcessedEvents;
  int fLastPrintedPercent;
};

#endif
