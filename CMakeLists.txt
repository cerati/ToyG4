# Prompt:
# Build a Geant4 project with a simple 20 meter radius world filled with liquid argon.
# The whole world is active volume. Particle gun emits single energy muon of 0.1 - 10 GeV.
# Physics list is QGSP_BERT and everything necessary for EM. Then record the energy deposition
# in 5 mm cubes. Save output to a root file with format (evt no, muon momentum px, py, pz,
# then a long vector of cube x, cube y, cube z, energy deposition in the cube).

cmake_minimum_required(VERSION 3.10)
project(LArMuonVoxelScan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Geant4 REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core RIO Tree)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${Geant4_INCLUDE_DIRS})
include_directories(${ROOT_INCLUDE_DIRS})

add_definitions(${Geant4_DEFINITIONS})

set(GEANT4_BATCH_LIBRARIES ${Geant4_LIBRARIES})
list(FILTER GEANT4_BATCH_LIBRARIES EXCLUDE REGEX "Qt")
list(FILTER GEANT4_BATCH_LIBRARIES EXCLUDE REGEX "G4(OpenGL|gl2ps|FR|GMocren|visHepRep|RayTracer|VRML|vis_management|modeling|interfaces)")

set(SOURCES
  src/ActionInitialization.cc
  src/DetectorConstruction.cc
  src/EventAction.cc
  src/PrimaryGeneratorAction.cc
  src/RunAction.cc
  src/SteppingAction.cc
  src/main.cc
)

set(HEADERS
  include/ActionInitialization.hh
  include/DetectorConstruction.hh
  include/EventAction.hh
  include/EventData.hh
  include/PrimaryGeneratorAction.hh
  include/RunAction.hh
  include/SteppingAction.hh
)

add_executable(lar_muon_voxels ${SOURCES} ${HEADERS})
target_link_libraries(
  lar_muon_voxels
  ${GEANT4_BATCH_LIBRARIES}
  ROOT::Core
  ROOT::RIO
  ROOT::Tree
)
