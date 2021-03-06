// PrimaryGeneratorActionMessenger.cc
// --------------------------------------------------------------
// History:
//
// Created by Emily Tsai (emily.tsai11@gmail.com) 2021-1-22
// --------------------------------------------------------------

#include "PrimaryGeneratorActionMessenger.hh"
#include "G4UIcmdWithAnInteger.hh"
#include "G4UIcmdWithADoubleAndUnit.hh"
#include "PrimaryGeneratorAction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorActionMessenger::PrimaryGeneratorActionMessenger(PrimaryGeneratorAction* pga) :
	G4UImessenger(), fPrimaryGeneratorAction(pga)
{
	fPrimaryGeneratorActionDir = new G4UIdirectory("/PrimaryGeneratorAction/");
	fPrimaryGeneratorActionDir->SetGuidance("UI commands to control the Primary Generator Action");

	fSetBeamTypeCmd = new G4UIcmdWithAnInteger("/PrimaryGeneratorAction/BeamType", this);
	fSetBeamTypeCmd->SetGuidance("Set beam type. Default is gammas. 1 for kaons.");
	fSetBeamTypeCmd->SetParameterName("BT", false);
	fSetBeamTypeCmd->AvailableForStates(G4State_PreInit, G4State_Idle);

	fSetBeamEnergyCmd = new G4UIcmdWithADoubleAndUnit("/PrimaryGeneratorAction/BeamEnergy", this);
	fSetBeamEnergyCmd->SetGuidance("Set beam energy. Default unit is MeV.");
	fSetBeamEnergyCmd->SetParameterName("BE", false);
	fSetBeamEnergyCmd->SetDefaultUnit("MeV");
	fSetBeamEnergyCmd->SetRange("BE > 0.0");
	fSetBeamEnergyCmd->AvailableForStates(G4State_PreInit, G4State_Idle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorActionMessenger::~PrimaryGeneratorActionMessenger()
{
	delete fSetBeamTypeCmd;
	delete fSetBeamEnergyCmd;

	delete fPrimaryGeneratorActionDir;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorActionMessenger::SetNewValue(G4UIcommand* cmd, G4String par)
{
	if(cmd == fSetBeamTypeCmd) fPrimaryGeneratorAction->SetBeamType(fSetBeamTypeCmd->GetNewIntValue(par));
	else if(cmd == fSetBeamEnergyCmd) fPrimaryGeneratorAction->SetBeamEnergy(fSetBeamEnergyCmd->GetNewDoubleValue(par));
}
