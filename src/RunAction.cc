// RunAction.cc
// --------------------------------------------------------------
// History:
//
// Created by Emily Tsai (emily.tsai11@gmail.com) 2020-7-9
// --------------------------------------------------------------

#include "RunAction.hh"

#include "G4RunManager.hh"

#include "G4Run.hh"
#include "G4Timer.hh"

#include "PrimaryGeneratorAction.hh"
#include "Analysis.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::RunAction()
{
	fTimer = new G4Timer();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

RunAction::~RunAction()
{
	delete fTimer;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::BeginOfRunAction(const G4Run* aRun)
{
	G4cout << "RunAction::BeginOfRunAction: Run " << aRun->GetRunID() << " start!" << G4endl;
	fTimer->Start();

	G4RunManager::GetRunManager()->SetRandomNumberStore(false);
	CLHEP::HepRandom::showEngineStatus();

	// get analysis manager
	fAnalysisManager = G4AnalysisManager::Instance();
	fAnalysisManager->SetVerboseLevel(1);

	// open an output file
	fAnalysisManager->OpenFile("SAC_output_" + std::to_string(aRun->GetRunID()));

	// create histograms
	fAnalysisManager->CreateH1("hEDepGamma", "energy deposition of gammas", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepPositron", "energy deposition of positrons", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepElectron", "energy deposition of electrons", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepNeutron", "energy deposition of neutrons", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepPionPlus", "energy deposition of pion pluses", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepPionZero", "energy deposition of pion zero", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepMuonPlus", "energy deposition of muon pluses", 100, 0.0 * CLHEP::eV, 1000000.0 * CLHEP::eV);
	fAnalysisManager->CreateH1("hEDepOpticalPhotons", "energy deposition of optical photons", 100, 0.0 * CLHEP::eV, 5.0 * CLHEP::eV);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void RunAction::EndOfRunAction(const G4Run* aRun)
{
	fTimer->Stop();
	G4cout << "RunAction::EndOfRunAction: Total events = " << aRun->GetNumberOfEvent() << " | Timer: " << *fTimer << G4endl;

	// save histograms
	fAnalysisManager = G4AnalysisManager::Instance();
	fAnalysisManager->Write();
	fAnalysisManager->CloseFile();
}
