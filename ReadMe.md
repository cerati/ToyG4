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

- /toyG4/run/setOutputFile <path/to/output.root>
  - Sets ROOT output file path/name.
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
3. /toyG4/run/setOutputFile ...
4. /toyG4/detector/setVoxelSize ...
5. /toyG4/generator/... configuration
6. /run/beamOn ...

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

## Common Pitfalls

- If /toyG4/run/setOutputFile points to a non-existent directory, ROOT file creation will fail.
- If you need strict reproducibility, keep /random/setSeeds fixed and avoid changing event count or physics configuration.

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
