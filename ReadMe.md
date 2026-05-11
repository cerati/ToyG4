# Geant4 Liquid Argon Muon Voxel Study

This project builds a minimal Geant4 application for single-muon energy deposition in liquid argon and writes sparse voxelized output to a ROOT file.

## Voxelization

- Default voxel size: `5 mm`
- Configurable via macro command (after `/run/initialize`):
  - `/toyG4/detector/setVoxelSize <size> <unit>`
- Example: `/toyG4/detector/setVoxelSize 1 cm`

## Geometry

- Active volume: liquid argon sphere with radius `20 m`
- World volume: vacuum box large enough to contain the active volume
- The whole liquid-argon sphere is treated as active

## Primary Particle

- One particle per event
- Initial position: origin
- Initial direction: isotropic

Default configuration (same as original behavior):

- PDG `13` (`mu-`)
- Kinetic energy sampled uniformly in `[0.1, 10] GeV`

Generator is configurable by PDG code with a separate kinetic-energy range for each PDG.
At each event, one configured PDG entry is selected uniformly, then kinetic energy is sampled
uniformly in that PDG-specific range.

Macro commands:

- `/toyG4/generator/addPdgRange <pdg> <emin> <emax> <unit>`
- `/toyG4/generator/clearPdgs`
- `/toyG4/generator/listPdgs`

Use these commands after `/run/initialize` in batch macros.

Examples:

- `/toyG4/generator/addPdgRange 13 0.1 10 GeV`
- `/toyG4/generator/addPdgRange -13 0.1 10 GeV`
- `/toyG4/generator/addPdgRange 2212 0.2 5 GeV`

## Physics

- Hadronic physics list: `QGSP_BERT`
- Electromagnetic physics: `G4EmStandardPhysics`

## Energy Deposition

Energy deposition is taken from Geant4 step energy loss:

- `G4Step::GetTotalEnergyDeposit()`
- Only steps inside the active liquid-argon volume are counted
- Deposited energy is accumulated into `5 mm` cubes
- Cubes are stored sparsely, so only cubes with nonzero deposited energy are written

The voxel coordinate written to output is the cube center in `mm`.

## Derived LAr Quanta Model

In addition to `edep`, the code derives scintillation photons and surviving ionization electrons from deposited energy using a simple parameterized liquid-argon model.

Constants used:

- `W = 23.6 eV`
- Exciton-to-ion ratio `alpha = 0.21`
- Recombination fraction `r = 0.5`

Per voxel:

- `Nq = Edep / W`
- `Ni = Nq / (1 + alpha)`
- `Nex = Nq - Ni`
- `Nphot = Nex + r * Ni`
- `Nelec = (1 - r) * Ni`

This is a simple yield model only. It is not a field-dependent recombination model and does not simulate optical photon transport or electron drift.

## Output

Default output file:

- `lar_muon_voxels.root`

ROOT tree:

- `events`

Branches:

- `event`
- `pdgCode`
- `energy_MeV`
- `px_MeV`
- `py_MeV`
- `pz_MeV`
- `pabs_MeV`
- `cube_x_mm`
- `cube_y_mm`
- `cube_z_mm`
- `edep_MeV`
- `voxel_dominant_pdg`
- `voxel_dominant_trackID`
- `voxel_dominant_fraction`
- `n_scint_photons`
- `n_ionization_electrons`

Each tree entry corresponds to one event. The vector branches are aligned element-by-element per nonzero voxel.

## Build

Requirements:

- Geant4
- ROOT
- CMake
- C++17 compiler

Build commands:

```bash
cmake -S . -B build
cmake --build build -j 4
```

## Run

Run the default macro:

```bash
./build/lar_muon_voxels run.mac
```

If no macro is provided, the executable runs `run.mac` by default.

## Files

- [CMakeLists.txt](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/CMakeLists.txt)
- [run.mac](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/run.mac)
- [src/main.cc](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/src/main.cc)
- [src/DetectorConstruction.cc](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/src/DetectorConstruction.cc)
- [src/PrimaryGeneratorAction.cc](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/src/PrimaryGeneratorAction.cc)
- [src/EventAction.cc](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/src/EventAction.cc)
- [src/SteppingAction.cc](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/src/SteppingAction.cc)
- [src/RunAction.cc](/home/linyan/Dropbox/Documents/Playground/Geant4/G4test/src/RunAction.cc)
