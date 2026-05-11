# Geant4 Liquid Argon Voxel Scan

This project simulates single-particle events in liquid argon and writes event-level sparse voxel energy deposition to a ROOT file.

## What It Simulates

- Geometry: liquid argon sphere of radius 20 m in a vacuum world.
- Active volume: the full liquid argon sphere.
- Primary generator: one particle per event, shot from the origin with isotropic direction.
- Physics: QGSP_BERT with G4EmStandardPhysics.

## Quick Start

1. Configure and build.

```bash
cmake -S . -B build
cmake --build build -j 4
```

Enable HDF5 output support (optional):

```bash
cmake -S . -B build -DTOYG4_ENABLE_HDF5=ON
cmake --build build -j 4
```

Note: this project uses the HDF5 C library target from hdf5-config.cmake.

On SL7/CVMFS, this explicit command is known to work:

```bash
rm -rf build
cmake -S . -B build \
  -DTOYG4_ENABLE_HDF5=ON \
  -DHDF5_DIR=/cvmfs/larsoft.opensciencegrid.org/products/hdf5/v1_12_2a/Linux64bit+3.10-2.17-e26-prof/cmake
cmake --build build -j 4
```

2. Run with a macro.

```bash
./build/lar_muon_voxels run.mac
```

If no macro is passed, the executable runs run.mac.

## Macro Configuration Reference

### Run-level commands

- /random/setSeeds <seed1> <seed2>
  - Sets Geant4 random seeds for reproducibility.
  - Example: /random/setSeeds 12345 67890

- /toyG4/run/setOutputFormat <root|hdf5>
  - Selects the output format.
  - Default is root.
  - Example: /toyG4/run/setOutputFormat hdf5

- /toyG4/run/setOutputFile <path/to/output.root>
  - Sets output file path/name.
  - Accepts relative or absolute paths.
  - Example: /toyG4/run/setOutputFile ./outputs/scan.root

### Detector command

- /toyG4/detector/setVoxelSize <size> <unit>
  - Sets voxel size for deposition binning.
  - Default is 5 mm.
  - Example: /toyG4/detector/setVoxelSize 1 cm

### Generator commands

- /toyG4/generator/addPdgRange <pdg> <emin> <emax> <unit> [linear|log]
- /toyG4/generator/clearPdgs
- /toyG4/generator/listPdgs

Notes:

- Multiple entries per PDG are allowed.
- Sampling mode defaults to linear if omitted.
- If no PDG ranges are configured, the code restores a default mu- setup.

## Recommended Command Order In Macros

Use this order for clarity and reproducibility:

1. /random/setSeeds ...
2. /run/initialize
3. /toyG4/run/setOutputFormat ...
4. /toyG4/run/setOutputFile ...
5. /toyG4/detector/setVoxelSize ...
6. /toyG4/generator/... configuration
7. /run/beamOn ...

The provided run macros follow this pattern.

## Default Generator Behavior

When no custom generator ranges are configured:

- PDG: 13 (mu-)
- Kinetic energy: uniform in [0.1, 10] GeV

## Energy Deposition and Voxelization

- Uses G4Step::GetTotalEnergyDeposit().
- Counts only steps in active liquid argon.
- Accumulates deposition into sparse 3D voxel bins.
- Writes only non-zero voxels.
- Voxel coordinates are saved as voxel-center positions in mm.

## Derived LAr Quanta Model

The code also derives approximate scintillation and ionization yields per voxel from deposited energy.

Constants:

- W = 23.6 eV
- alpha = 0.21
- r = 0.5

Per voxel:

- Nq = Edep / W
- Ni = Nq / (1 + alpha)
- Nex = Nq - Ni
- Nphot = Nex + r * Ni
- Nelec = (1 - r) * Ni

This is a simplified yield model. It is not field-dependent recombination and does not model optical transport or electron drift.

## Output File and Tree Schema

Default output file:

- lar_muon_voxels.root

Tree:

- events

Branches:

- event
- pdgCode
- energy_MeV
- px_MeV
- py_MeV
- pz_MeV
- pabs_MeV
- cube_x_mm
- cube_y_mm
- cube_z_mm
- edep_MeV
- voxel_dominant_pdg
- voxel_dominant_trackID
- voxel_dominant_fraction
- n_scint_photons
- n_ionization_electrons

Each entry is one event. Vector branches are index-aligned voxel-by-voxel.

HDF5 mode (set /toyG4/run/setOutputFormat hdf5) writes a reduced schema:

- pdgCode (1D int, one value per event)
- energy_MeV (1D double, one value per event)
- edep_MeV_flat (1D double, flattened voxel edep values for all events)
- edep_offsets (1D uint64, length = nEvents + 1)

For event i, voxel edep values are:

- edep_MeV_flat[edep_offsets[i] : edep_offsets[i+1]]

## Common Pitfalls

- If /toyG4/run/setOutputFile points to a non-existent directory, ROOT file creation will fail.
- If you need strict reproducibility, keep /random/setSeeds fixed and avoid changing event count or physics configuration.
- HDF5 output requires configuring with -DTOYG4_ENABLE_HDF5=ON and having an HDF5 CMake config package that exports target hdf5-shared.
- If you switch between HDF5-enabled and non-HDF5 builds, reconfigure from a clean build directory.
- Requesting /toyG4/run/setOutputFormat hdf5 without HDF5 build support now stops with a fatal error.

## Main Project Files

- [run.mac](run.mac)
- [run-multi-particle.mac](run-multi-particle.mac)
- [include/RunAction.hh](include/RunAction.hh)
- [include/RunActionMessenger.hh](include/RunActionMessenger.hh)
- [src/RunAction.cc](src/RunAction.cc)
- [src/RunActionMessenger.cc](src/RunActionMessenger.cc)
- [src/PrimaryGeneratorAction.cc](src/PrimaryGeneratorAction.cc)
- [src/PrimaryGeneratorMessenger.cc](src/PrimaryGeneratorMessenger.cc)
- [src/DetectorMessenger.cc](src/DetectorMessenger.cc)
- [CMakeLists.txt](CMakeLists.txt)
