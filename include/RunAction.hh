// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#ifndef RUNACTION_HH
#define RUNACTION_HH

#include "G4UserRunAction.hh"

#include <memory>
#include <vector>

class G4Run;
class TFile;
class TTree;

struct EventRecord;

class RunAction : public G4UserRunAction {
public:
  RunAction();
  virtual ~RunAction();

  virtual void BeginOfRunAction(const G4Run* run);
  virtual void EndOfRunAction(const G4Run* run);

  void FillEvent(const EventRecord& record);

private:
  std::unique_ptr<TFile> fOutputFile;
  TTree* fTree;

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
};

#endif
