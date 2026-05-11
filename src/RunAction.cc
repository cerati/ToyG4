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

#include <iomanip>
#include <iostream>

#ifdef TOYG4_USE_HDF5
#include "hdf5.h"

// Append `count` elements of `type` to an extendible 1-D dataset.
static bool H5Append(hid_t dset, hid_t type, hsize_t count, const void* data) {
  hid_t filespace = H5Dget_space(dset);
  hsize_t currentSize = 0;
  H5Sget_simple_extent_dims(filespace, &currentSize, 0);
  H5Sclose(filespace);

  const hsize_t newSize = currentSize + count;
  if (H5Dset_extent(dset, &newSize) < 0) return false;

  filespace = H5Dget_space(dset);
  if (H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &currentSize, 0, &count, 0) < 0) {
    H5Sclose(filespace);
    return false;
  }

  const hid_t memspace = H5Screate_simple(1, &count, 0);
  const bool ok = (H5Dwrite(dset, type, memspace, filespace, H5P_DEFAULT, data) >= 0);
  H5Sclose(memspace);
  H5Sclose(filespace);
  return ok;
}

// Create an empty unlimited/chunked 1-D dataset.
static hid_t H5CreateExtendible(hid_t file, const char* name, hid_t type, hsize_t chunkSize) {
  const hsize_t initDims = 0;
  const hsize_t maxDims  = H5S_UNLIMITED;
  const hid_t space = H5Screate_simple(1, &initDims, &maxDims);
  if (space < 0) return -1;

  const hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(dcpl, 1, &chunkSize);
  const hid_t dset = H5Dcreate2(file, name, type, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
  H5Pclose(dcpl);
  H5Sclose(space);
  return dset;
}
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
  fTotalEvents       = 0;
  fProcessedEvents   = 0;
  fLastPrintedPercent = -1;
#ifdef TOYG4_USE_HDF5
  fH5File           = -1;
  fH5DsetPdgCode    = -1;
  fH5DsetEnergy     = -1;
  fH5DsetEdepFlat   = -1;
  fH5DsetCubeXFlat  = -1;
  fH5DsetCubeYFlat  = -1;
  fH5DsetCubeZFlat  = -1;
  fH5DsetOffsets    = -1;
  fH5CurrentOffset  = 0;
#endif
}

RunAction::~RunAction() {
  delete fMessenger;
}

void RunAction::BeginOfRunAction(const G4Run* run) {
  fTotalEvents        = run ? run->GetNumberOfEventToBeProcessed() : 0;
  fProcessedEvents    = 0;
  fLastPrintedPercent = -1;

  if (fOutputFormat == "hdf5") {
#ifdef TOYG4_USE_HDF5
    fH5CurrentOffset = 0;
    fH5File = H5Fcreate(fOutputFileName, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    if (fH5File < 0) {
      G4ExceptionDescription desc;
      desc << "Failed to create HDF5 output file: " << fOutputFileName;
      G4Exception("RunAction::BeginOfRunAction", "ToyG4Run003", FatalException, desc);
      return;
    }

    const hsize_t chunkEvt  = 1024;
    const hsize_t chunkFlat = 65536;
    fH5DsetPdgCode   = H5CreateExtendible(fH5File, "pdgCode",        H5T_NATIVE_INT,    chunkEvt);
    fH5DsetEnergy    = H5CreateExtendible(fH5File, "energy_MeV",     H5T_NATIVE_DOUBLE, chunkEvt);
    fH5DsetEdepFlat  = H5CreateExtendible(fH5File, "edep_MeV_flat",  H5T_NATIVE_DOUBLE, chunkFlat);
    fH5DsetCubeXFlat = H5CreateExtendible(fH5File, "cube_x_mm_flat", H5T_NATIVE_DOUBLE, chunkFlat);
    fH5DsetCubeYFlat = H5CreateExtendible(fH5File, "cube_y_mm_flat", H5T_NATIVE_DOUBLE, chunkFlat);
    fH5DsetCubeZFlat = H5CreateExtendible(fH5File, "cube_z_mm_flat", H5T_NATIVE_DOUBLE, chunkFlat);
    fH5DsetOffsets   = H5CreateExtendible(fH5File, "edep_offsets",   H5T_NATIVE_UINT64, chunkEvt);

    if (fH5DsetPdgCode < 0 || fH5DsetEnergy < 0 || fH5DsetEdepFlat < 0 ||
        fH5DsetCubeXFlat < 0 || fH5DsetCubeYFlat < 0 || fH5DsetCubeZFlat < 0 ||
        fH5DsetOffsets < 0) {
      G4Exception("RunAction::BeginOfRunAction", "ToyG4Run010", FatalException,
                  "Failed to create one or more HDF5 datasets.");
      return;
    }

    // Write the initial offset entry (0) so edep_offsets has length nEvents+1.
    const std::uint64_t zero = 0;
    H5Append(fH5DsetOffsets, H5T_NATIVE_UINT64, 1, &zero);

    G4cout << "[ToyG4] Writing HDF5 output to: " << fOutputFileName << G4endl;
#endif
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
#ifdef TOYG4_USE_HDF5
    if (fH5File >= 0) {
      H5Dclose(fH5DsetPdgCode);
      H5Dclose(fH5DsetEnergy);
      H5Dclose(fH5DsetEdepFlat);
      H5Dclose(fH5DsetCubeXFlat);
      H5Dclose(fH5DsetCubeYFlat);
      H5Dclose(fH5DsetCubeZFlat);
      H5Dclose(fH5DsetOffsets);
      H5Fclose(fH5File);
      fH5File = -1;
      G4cout << "[ToyG4] HDF5 file closed: " << fOutputFileName << G4endl;
    }
#endif
    // End the progress bar line
    std::cout << "\n" << std::flush;
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
  // End the progress bar line
  std::cout << "\n" << std::flush;
}

void RunAction::FillEvent(const EventRecord& record) {
  ++fProcessedEvents;

  if (fTotalEvents > 0) {
    const int pct = (fProcessedEvents * 100) / fTotalEvents;
    const int updateEvery = std::max(1, fTotalEvents / 1000);
    const bool shouldRefreshPct =
        (pct != fLastPrintedPercent) ||
        (fProcessedEvents == 1) ||
        (fProcessedEvents == fTotalEvents) ||
        ((fProcessedEvents % updateEvery) == 0);

    if (shouldRefreshPct) {
      fLastPrintedPercent = pct;
    }

    const int shownPct = std::max(0, fLastPrintedPercent);
    const int filled = shownPct / 2;           // bar width = 50 chars
    const int empty  = 50 - filled;
    std::cout << "\r[ToyG4] ["
              << std::string(filled, '#')
              << std::string(empty,  ' ')
              << "] "
              << std::setw(3) << shownPct << "% ("
              << fProcessedEvents << "/" << fTotalEvents << ")"
              << std::flush;
  }

  if (fOutputFormat == "hdf5") {
#ifdef TOYG4_USE_HDF5
    const int    pdg    = record.pdgCode;
    const double energy = record.energy / MeV;
    H5Append(fH5DsetPdgCode, H5T_NATIVE_INT,    1, &pdg);
    H5Append(fH5DsetEnergy,  H5T_NATIVE_DOUBLE, 1, &energy);

    const hsize_t nVoxels = static_cast<hsize_t>(record.edep.size());
    if (nVoxels > 0) {
      std::vector<double> edep(nVoxels), cx(nVoxels), cy(nVoxels), cz(nVoxels);
      for (hsize_t i = 0; i < nVoxels; ++i) {
        edep[i] = record.edep[i]  / MeV;
        cx[i]   = record.cubeX[i] / mm;
        cy[i]   = record.cubeY[i] / mm;
        cz[i]   = record.cubeZ[i] / mm;
      }
      H5Append(fH5DsetEdepFlat,  H5T_NATIVE_DOUBLE, nVoxels, edep.data());
      H5Append(fH5DsetCubeXFlat, H5T_NATIVE_DOUBLE, nVoxels, cx.data());
      H5Append(fH5DsetCubeYFlat, H5T_NATIVE_DOUBLE, nVoxels, cy.data());
      H5Append(fH5DsetCubeZFlat, H5T_NATIVE_DOUBLE, nVoxels, cz.data());
    }

    fH5CurrentOffset += nVoxels;
    H5Append(fH5DsetOffsets, H5T_NATIVE_UINT64, 1, &fH5CurrentOffset);
#endif
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
