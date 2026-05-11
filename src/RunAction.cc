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

#ifdef TOYG4_USE_HDF5
#include "hdf5.h"
#endif

#include "TFile.h"
#include "TTree.h"

RunAction::RunAction()
  : G4UserRunAction(),
    fOutputFileName("lar_muon_voxels.root"),
    fOutputFormat("root"),
    fMessenger(0),
    fOutputFile(),
    fTree(0),
    fH5PdgCode(),
    fH5Energy(),
    fH5EdepFlat(),
    fH5CubeXFlat(),
    fH5CubeYFlat(),
    fH5CubeZFlat(),
    fH5EdepOffsets(),
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
  if (fOutputFormat == "hdf5") {
    fH5PdgCode.clear();
    fH5Energy.clear();
    fH5EdepFlat.clear();
    fH5CubeXFlat.clear();
    fH5CubeYFlat.clear();
    fH5CubeZFlat.clear();
    fH5EdepOffsets.clear();
    fH5EdepOffsets.push_back(0);
    G4cout << "[ToyG4] Writing HDF5 output to: " << fOutputFileName << G4endl;
    return;
  }

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
  if (fOutputFormat == "hdf5") {
    WriteHdf5Output();
    return;
  }

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
  if (fOutputFormat == "hdf5") {
    fH5PdgCode.push_back(record.pdgCode);
    fH5Energy.push_back(record.energy / MeV);
    for (std::size_t i = 0; i < record.edep.size(); ++i) {
      fH5EdepFlat.push_back(record.edep[i] / MeV);
      fH5CubeXFlat.push_back(record.cubeX[i] / mm);
      fH5CubeYFlat.push_back(record.cubeY[i] / mm);
      fH5CubeZFlat.push_back(record.cubeZ[i] / mm);
    }
    fH5EdepOffsets.push_back(static_cast<std::uint64_t>(fH5EdepFlat.size()));
    return;
  }

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

void RunAction::SetOutputFormat(const G4String& format) {
  const G4String normalized = G4StrUtil::to_lower_copy(format);

  if (normalized != "root" && normalized != "hdf5") {
    G4Exception("RunAction::SetOutputFormat",
                "ToyG4Run004",
                JustWarning,
                "Unknown output format. Use 'root' or 'hdf5'. Configuration unchanged.");
    return;
  }

#ifndef TOYG4_USE_HDF5
  if (normalized == "hdf5") {
    G4Exception("RunAction::SetOutputFormat",
                "ToyG4Run005",
                FatalException,
                "HDF5 support is not enabled in this build. Reconfigure with -DTOYG4_ENABLE_HDF5=ON.");
  }
#endif

  fOutputFormat = normalized;
  G4cout << "[ToyG4] Output format set to: " << fOutputFormat << G4endl;
}

const G4String& RunAction::GetOutputFormat() const {
  return fOutputFormat;
}

void RunAction::WriteHdf5Output() {
#ifndef TOYG4_USE_HDF5
  G4Exception("RunAction::WriteHdf5Output",
              "ToyG4Run006",
              FatalException,
              "HDF5 output requested but HDF5 support is not enabled in this build.");
#else
  const hid_t file = H5Fcreate(fOutputFileName, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (file < 0) {
    G4ExceptionDescription description;
    description << "Failed to create HDF5 output file: " << fOutputFileName;
    G4Exception("RunAction::WriteHdf5Output",
                "ToyG4Run007",
                FatalException,
                description);
    return;
  }

  auto write1D = [file](const char* name,
                        hid_t h5Type,
                        hsize_t size,
                        const void* data) -> bool {
    const hid_t space = H5Screate_simple(1, &size, 0);
    if (space < 0) {
      return false;
    }

    const hid_t dset = H5Dcreate2(file, name, h5Type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dset < 0) {
      H5Sclose(space);
      return false;
    }

    if (size > 0 && data) {
      if (H5Dwrite(dset, h5Type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data) < 0) {
        H5Dclose(dset);
        H5Sclose(space);
        return false;
      }
    }

    H5Dclose(dset);
    H5Sclose(space);
    return true;
  };

  const hsize_t nEvents = static_cast<hsize_t>(fH5PdgCode.size());
  const hsize_t nFlat = static_cast<hsize_t>(fH5EdepFlat.size());
  const hsize_t nOffsets = static_cast<hsize_t>(fH5EdepOffsets.size());

  // Sanity check: coordinate arrays must be the same length as edep_flat
  if (fH5CubeXFlat.size() != fH5EdepFlat.size() ||
      fH5CubeYFlat.size() != fH5EdepFlat.size() ||
      fH5CubeZFlat.size() != fH5EdepFlat.size()) {
    G4Exception("RunAction::WriteHdf5Output",
                "ToyG4Run009",
                FatalException,
                "HDF5 coordinate flat arrays are not aligned with edep_flat.");
    return;
  }

  const bool ok =
      write1D("pdgCode", H5T_NATIVE_INT, nEvents, fH5PdgCode.empty() ? 0 : fH5PdgCode.data()) &&
      write1D("energy_MeV", H5T_NATIVE_DOUBLE, nEvents, fH5Energy.empty() ? 0 : fH5Energy.data()) &&
      write1D("edep_MeV_flat", H5T_NATIVE_DOUBLE, nFlat, fH5EdepFlat.empty() ? 0 : fH5EdepFlat.data()) &&
      write1D("cube_x_mm_flat", H5T_NATIVE_DOUBLE, nFlat, fH5CubeXFlat.empty() ? 0 : fH5CubeXFlat.data()) &&
      write1D("cube_y_mm_flat", H5T_NATIVE_DOUBLE, nFlat, fH5CubeYFlat.empty() ? 0 : fH5CubeYFlat.data()) &&
      write1D("cube_z_mm_flat", H5T_NATIVE_DOUBLE, nFlat, fH5CubeZFlat.empty() ? 0 : fH5CubeZFlat.data()) &&
      write1D("edep_offsets", H5T_NATIVE_UINT64, nOffsets, fH5EdepOffsets.empty() ? 0 : fH5EdepOffsets.data());

  H5Fclose(file);

  if (!ok) {
    G4ExceptionDescription description;
    description << "Failed while writing datasets to HDF5 file: " << fOutputFileName;
    G4Exception("RunAction::WriteHdf5Output",
                "ToyG4Run008",
                FatalException,
                description);
    return;
  }

  G4cout << "[ToyG4] HDF5 events written: " << fH5PdgCode.size() << G4endl;
#endif
}
