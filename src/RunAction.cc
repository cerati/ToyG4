// Prompt:
// Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
// The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
// Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
// in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
// then a long vector of cube x, cube y, cube z, energy deposition in the cube).

#include "RunAction.hh"

#include "EventData.hh"
#include "RunActionMessenger.hh"

#include "G4Exception.hh"
#include "G4Run.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include "TFile.h"
#include "TTree.h"

RunAction::RunAction()
  : G4UserRunAction(),
    fOutputFileName("lar_muon_voxels.root"),
    fMessenger(0),
    fOutputFile(),
    fTree(0),
    fEventNumber(-1),
    fPdgCode(0),
    fPx(0.),
    fPy(0.),
    fPz(0.),
    fEnergy(0.),
    fMomentumAbs(0.),
    fCubeX(),
    fCubeY(),
    fCubeZ(),
    fEdep(),
    fVoxelDominantPdg(),
    fVoxelDominantTrackID(),
    fVoxelDominantFraction(),
    fScintPhotons(),
    fIonizationElectrons() {
  fMessenger = new RunActionMessenger(this);
}

RunAction::~RunAction() {
  delete fMessenger;
}

void RunAction::BeginOfRunAction(const G4Run*) {
  fOutputFile.reset(TFile::Open(fOutputFileName, "RECREATE"));
  if (!fOutputFile || fOutputFile->IsZombie()) {
    G4ExceptionDescription description;
    description << "Failed to create output ROOT file: " << fOutputFileName;
    G4Exception("RunAction::BeginOfRunAction",
                "ToyG4Run003",
                FatalException,
                description);
    return;
  }

  G4cout << "[ToyG4] Writing output to: " << fOutputFileName << G4endl;
  fTree = new TTree("events", "Muon energy deposition in 5 mm cubes");

  fTree->Branch("event", &fEventNumber);
  fTree->Branch("pdgCode", &fPdgCode);
  fTree->Branch("energy_MeV", &fEnergy);
  fTree->Branch("px_MeV", &fPx);
  fTree->Branch("py_MeV", &fPy);
  fTree->Branch("pz_MeV", &fPz);
  fTree->Branch("pabs_MeV", &fMomentumAbs);
  fTree->Branch("cube_x_mm", &fCubeX);
  fTree->Branch("cube_y_mm", &fCubeY);
  fTree->Branch("cube_z_mm", &fCubeZ);
  fTree->Branch("edep_MeV", &fEdep);
  fTree->Branch("voxel_dominant_pdg", &fVoxelDominantPdg);
  fTree->Branch("voxel_dominant_trackID", &fVoxelDominantTrackID);
  fTree->Branch("voxel_dominant_fraction", &fVoxelDominantFraction);
  fTree->Branch("n_scint_photons", &fScintPhotons);
  fTree->Branch("n_ionization_electrons", &fIonizationElectrons);
}

void RunAction::EndOfRunAction(const G4Run*) {
  if (!fOutputFile) {
    return;
  }

  fOutputFile->cd();
  if (fTree) {
    fTree->Write();
  }
  fOutputFile->Close();
  fOutputFile.reset();
  fTree = 0;
}

void RunAction::FillEvent(const EventRecord& record) {
  fEventNumber = record.eventNumber;
  fPdgCode = record.pdgCode;
  fEnergy = record.energy / MeV;
  fPx = record.px / MeV;
  fPy = record.py / MeV;
  fPz = record.pz / MeV;
  fMomentumAbs = record.momentumAbs / MeV;

  fCubeX.assign(record.cubeX.begin(), record.cubeX.end());
  fCubeY.assign(record.cubeY.begin(), record.cubeY.end());
  fCubeZ.assign(record.cubeZ.begin(), record.cubeZ.end());
  fEdep.assign(record.edep.begin(), record.edep.end());
  fVoxelDominantPdg.assign(record.voxelDominantPdg.begin(), record.voxelDominantPdg.end());
  fVoxelDominantTrackID.assign(record.voxelDominantTrackID.begin(), record.voxelDominantTrackID.end());
  fVoxelDominantFraction.assign(record.voxelDominantFraction.begin(), record.voxelDominantFraction.end());
  fScintPhotons.assign(record.scintPhotons.begin(), record.scintPhotons.end());
  fIonizationElectrons.assign(record.ionizationElectrons.begin(),
                              record.ionizationElectrons.end());

  for (std::size_t i = 0; i < fCubeX.size(); ++i) {
    fCubeX[i] /= mm;
    fCubeY[i] /= mm;
    fCubeZ[i] /= mm;
    fEdep[i] /= MeV;
  }

  if (fTree) {
    fTree->Fill();
  }
}

void RunAction::SetOutputFileName(const G4String& fileName) {
  fOutputFileName = fileName;
  G4cout << "[ToyG4] Output file set to: " << fOutputFileName << G4endl;
}

const G4String& RunAction::GetOutputFileName() const {
  return fOutputFileName;
}
