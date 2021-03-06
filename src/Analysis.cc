// Analysis.cc
// --------------------------------------------------------------
// History:
//
// Created by Emily Tsai (emily.tsai11@gmail.com) 2021-1-24
// --------------------------------------------------------------

#include "Analysis.hh"
#include <string>
#include <cmath>

#include "G4Event.hh"
#include "G4PrimaryParticle.hh"

#include <TFile.h>
#include <TH1.h>
#include <TH1D.h>
#include <TH2D.h>

#include "AnalysisMessenger.hh"
#include "SACGeometry.hh"
#include "SACSD.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Analysis* Analysis::fInstance = nullptr;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Analysis* Analysis::GetInstance()
{
	if(!fInstance) fInstance = new Analysis();
	return fInstance;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Analysis::Analysis()
{
	fMessenger = new AnalysisMessenger();

	SACGeometry* Geo = SACGeometry::GetInstance();
	fSACRows = (G4double) Geo->GetSACNRows();
	fSACCols = (G4double) Geo->GetSACNCols();
	fSACLayers = (G4double) Geo->GetSACNLayers();

	// Defaults
	fTotalNEvents = -1;
	fFileName = "SACRun";

	CreateParticleList();
	CreateHistograms();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

Analysis::~Analysis()
{
	delete fMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::OpenFile()
{
	fOut = new TFile((fFileName + ".root").c_str(), "RECREATE");
	fTxt.open(fFileName + "_EnergyDeposition.txt");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::Write()
{
	for(auto& iter : fH1D) iter.second->Write();
	for(auto& iter : fH2D) iter.second->Write();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::CloseFile()
{
	fOut->Close();
	fTxt.close();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::CreateParticleList()
{
	fP.insert({"all", 0});
	fP.insert({"e+", 1});
	fP.insert({"e-", 2});
	fP.insert({"gamma", 3});
	fP.insert({"mu+", 4});
	fP.insert({"mu-", 5});
	fP.insert({"neutron", 6});
	fP.insert({"opticalphoton", 7});
	fP.insert({"other", 8});
	fP.insert({"pi+", 9});
	fP.insert({"pi-", 10});
	fP.insert({"pi0", 11});
	fP.insert({"proton", 12});
	fP.insert({"untracked", 13});
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::CreateLogBins(const int nBins, double min, double max, double* edges)
{
	double minBase = log10(min);
	double maxBase = log10(max);
	double increment = (maxBase - minBase) / nBins;

	for(int i = 0; i <= nBins; i++)
		edges[i] = pow(10.0, minBase + i * increment);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::CreateHistograms()
{
	TH1::SetDefaultSumw2(kTRUE);
	const G4int fN1DBinsX = 200;
	G4String key;

	// <Particle> energy deposition per event / incident energy
	G4double eDepBinEdges[fN1DBinsX + 1];
	CreateLogBins(fN1DBinsX, 1.0e-6, 1.0, eDepBinEdges);
	for(auto& iter : fP)
	{
		key = "h1EDep_PerEvent_" + iter.first;
		fH1D[key] = new TH1D(key.c_str(),
			("Fraction of " + iter.first + " energy deposition per event").c_str(),
			fN1DBinsX, eDepBinEdges);
	}

	// <Particle> energy deposition per SAC layer / incident energy
	for(auto& iter : fP)
	{
		key = "h1EDep_PerLayer_" + iter.first;
		fH1D[key] = new TH1D(key.c_str(),
			("Fraction of " + iter.first + " energy deposition per SAC layer").c_str(),
			(G4int) fSACLayers, 0.0, fSACLayers);
	}

	// 2D <particle> energy deposition per SAC layer (z = <#>) / incident energy
	for(G4int i = 0; i < fSACLayers; i++)
	{
		for(auto& iter : fP)
		{
			key = "h2EDepZ" + std::to_string(i) + "_PerLayer_" + iter.first;
			fH2D[key] = new TH2D(key.c_str(),
				("Fraction of " + iter.first + " energy deposition in SAC layer z = " + std::to_string(i)).c_str(),
				(G4int) fSACRows, 0.0, fSACRows,
				(G4int) fSACCols, 0.0, fSACCols);
		}
	}

	// <Particle> multiplicity per event / incident energy [1/MeV]
	for(auto& iter : fP)
	{
		key = "h1Mult_PerEvent_" + iter.first;
		fH1D[key] = new TH1D(key.c_str(),
			("Number of " + iter.first + " per event / incident energy").c_str(),
			fN1DBinsX + 20, 0.0, 110.0);
	}

	// Number of punch-through events / number of events
	key = "h1PunchThrough_PerEvent";
	fH1D[key] = new TH1D(key.c_str(),
		"Fraction of punch-through events",
		1, 0.5, 1.5);

	// Number of elastic collision events / number of events
	key = "h1Elastic_PerEvent";
	fH1D[key] = new TH1D(key.c_str(),
		"Fraction of elastic collision events",
		1, 0.5, 1.5);

	// Number of inelastic collision events / number of events
	key = "h1Inelastic_PerEvent";
	fH1D[key] = new TH1D(key.c_str(),
		"Fraction of inelastic collision events",
		1, 0.5, 1.5);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::FillHistograms(const G4Event* evt)
{
	if(fTotalNEvents == -1)
		G4cout << "[Analysis::FillHistograms] ERROR: TOTAL NUMBER OF EVENTS IS " << fTotalNEvents << "!!!" << G4endl;

	// Get incident particle name & energy
	G4PrimaryParticle* primary = evt->GetPrimaryVertex()->GetPrimary();
	// G4String primaryName = primary->GetParticleDefinition()->GetParticleName();
	G4double primaryInitE = primary->GetKineticEnergy();

	// Get hits collection
	SACHitsCollection* fSACCollection = (SACHitsCollection*) evt->GetHCofThisEvent()->GetHC(0);
	G4int nHits = fSACCollection->entries();

	// Hit attributes
	G4int cellID;
	G4int trackID;
	G4String particleName;
	// G4String volumeName;
	// G4String sdName;
	// G4String creatorProcessName;
	G4double energyDeposition;
	// G4double time;
	// G4double trackLength;
	// G4ThreeVector position;
	// G4ThreeVector localPosition;

	// Helpers to fill histograms, will need to scale # entries w/# groups
	std::map<G4int, G4bool> trackedHits;
	G4double fEDepPerEvent[] = {
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	G4double fMultPerEvent[] = {
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	G4int pIndex, x, y, z;
	G4bool isPunchThrough = false, isElastic = false, isInelastic = false;
	G4String key;

	if(nHits == 0) isPunchThrough = true;

	// Filling histograms
	for(G4int i = 0; i < nHits; i++)
	{
		// Get current hit and attributes
		SACHit* currentHit = (*fSACCollection)[i];
		cellID = currentHit->GetCellID();
		trackID = currentHit->GetTrackID();
		particleName = currentHit->GetParticleName();
		// volumeName = currentHit->GetVolumeName();
		// sdName = currentHit->GetSDName();
		// DON'T USE FOR NOW, CAUSING SEGFAULTS -- TRY AGAIN AFTER OPTICAL BOUNDARIES DEFINED
		// creatorProcessName = currentHit->GetCreatorProcessName();
		energyDeposition = currentHit->GetEnergyDeposition();
		// time = currentHit->GetTime();
		// trackLength = currentHit->GetTrackLength();
		// position = currentHit->GetPosition();
		// localPosition = currentHit->GetLocalPosition();

		try { pIndex = fP.at(particleName); }
		catch(std::out_of_range)
		{
			// G4cout << "[Analysis::FillHistograms] Particle not yet added to list --- " << particleName << G4endl;
			pIndex = fP.at("other");
			particleName = "other";
		}

		// CHANGE THIS TO BEING MOD SAC DIMENSIONS FOR BETTER PORTABILITY
		x = cellID % 10;		// x = which row (0-9)
		y = cellID / 10 % 10;	// y = which column (0-9)
		z = cellID / 100;		// z = which layer (0 = front, 1, 2, 3 = back)

		// Increment energy deposition in event
		fEDepPerEvent[fP.at("all")] += energyDeposition;
		fEDepPerEvent[pIndex] += energyDeposition;

		// Fill "once per hit" histograms
		// <Particle> energy deposition per SAC layer / incident energy
		key = "h1EDep_PerLayer_all";
		fH1D[key]->Fill(z, energyDeposition / primaryInitE / fTotalNEvents);
		key = "h1EDep_PerLayer_" + particleName;
		fH1D[key]->Fill(z, energyDeposition / primaryInitE / fTotalNEvents);

		// 2D <particle> energy deposition per SAC layer (z = <#>) / incident energy
		key = "h2EDepZ" + std::to_string(z) + "_PerLayer_all";
		fH2D[key]->Fill(x, y, energyDeposition / primaryInitE / fTotalNEvents);
		key = "h2EDepZ" + std::to_string(z) + "_PerLayer_" + particleName;
		fH2D[key]->Fill(x, y, energyDeposition / primaryInitE / fTotalNEvents);

		// Increment multiplicity in the event
		if(!trackedHits[trackID])
		{
			trackedHits[trackID] = true;
			fMultPerEvent[fP.at("all")]++;
			fMultPerEvent[pIndex]++;

			// Record if event is elastic and/or inelastic
			// Thing about photon/hadron primary particle track IDs:
			// https://www.ge.infn.it/geant4/training/ptb_2009/day4/solutions_day4_partIII.html
			if(trackID == 1) isElastic = true;
			else if(trackID > 1)
			{
				isElastic = false;
				isInelastic = true;
			}
		}
	}
	// Take out optical photon energy, since they don't obey energy conservation
	// https://geant4-forum.web.cern.ch/t/energy-deposition-is-higher-than-the-primary-source-energy/2064/3
	fEDepPerEvent[fP.at("all")] -= fEDepPerEvent[fP.at("opticalphoton")];
	// Get true untracked energy deposition
	fEDepPerEvent[fP.at("untracked")] = primaryInitE - fEDepPerEvent[fP.at("all")];

	// Fill "once per event" histograms
	for(auto& iter : fP)
	{
		// <Particle> energy deposition per event / incident energy
		key = "h1EDep_PerEvent_" + iter.first;
		fH1D[key]->Fill(fEDepPerEvent[iter.second] / primaryInitE, 1.0);
		fTxt << fEDepPerEvent[iter.second] / primaryInitE << " ";

		// <Particle> multiplicity per event / incident energy [1/MeV]
		key = "h1Mult_PerEvent_" + iter.first;
		fH1D[key]->Fill(fMultPerEvent[iter.second] / primaryInitE, 1.0);
	}
	fTxt << std::endl;

	if(isPunchThrough)
	{
		// Number of punch-through events / number of events
		key = "h1PunchThrough_PerEvent";
		fH1D[key]->Fill(1.0, 1.0);
	}
	else if(isElastic)
	{
		// Number of elastic collision events / number of events
		key = "h1Elastic_PerEvent";
		fH1D[key]->Fill(1.0, 1.0);
	}
	else if(isInelastic)
	{
		// Number of inelastic collision events / number of events
		key = "h1Inelastic_PerEvent";
		fH1D[key]->Fill(1.0, 1.0);
	}
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::PrintParticleList()
{
	G4cout << "--------------- Particle List ---------------" << G4endl;
	for(auto& iter : fP) G4cout << iter.first << " : " << iter.second << G4endl;
	G4cout << "---------------------------------------------" << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void Analysis::PrintHistograms()
{
	G4cout << "--------------- 1D Histograms ---------------" << G4endl;
	for(auto& iter : fH1D) G4cout << iter.first << G4endl;
	G4cout << "--------------- 2D Histograms ---------------" << G4endl;
	for(auto& iter : fH2D) G4cout << iter.first << G4endl;
	G4cout << "---------------------------------------------" << G4endl;
}
