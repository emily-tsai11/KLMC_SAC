// SACDetector.cc
// --------------------------------------------------------------
// History:
//
// Created by Emanuele Leonardi (emanuele.leonardi@roma1.infn.it) 2015-12-11
// Adapted from PADME experiment by Emily Tsai (emily.tsai11@gmail.com) 2020-7-7
// --------------------------------------------------------------

#include "SACDetector.hh"

#include "G4Material.hh"
#include "G4Box.hh"
#include "G4VisAttributes.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"

#include "SACMessenger.hh"
#include "SACSD.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SACDetector::SACDetector(G4Material* material, G4LogicalVolume* motherVolume) : fMaterial(material), fMotherVolume(motherVolume)
{
	SACGeometry* Geo = SACGeometry::GetInstance();

	fCellSizeX = Geo->GetCellSizeX();
	fCellSizeY = Geo->GetCellSizeY();
	fCellSizeZ = Geo->GetCellSizeZ();

	fCellGap = Geo->GetCellGap();
	printf("Gap between SAC cells is %f\n", fCellGap);

	fNRows = Geo->GetSACNRows();
	fNCols = Geo->GetSACNCols();
	fNLayers = Geo->GetSACNLayers();
	printf("SAC dimensions are %d %d %d\n", fNRows, fNCols, fNLayers);

	fSACSizeX = Geo->GetSACSizeX();
	fSACSizeY = Geo->GetSACSizeY();
	fSACSizeZ = Geo->GetSACSizeZ();
	printf("SAC size is %f %f %f\n", fSACSizeX, fSACSizeY, fSACSizeZ);

	fSACPosX = Geo->GetSACPosX();
	fSACPosY = Geo->GetSACPosY();
	fSACPosZ = Geo->GetSACPosZ();
	printf("SAC will be placed at %f %f %f\n", fSACPosX, fSACPosY, fSACPosZ);

	fCell = new SACCell();
	fNonRefCell = new SACCell();

	fEnablePMT = Geo->GetEnablePMT();
	fEnableSiPM = Geo->GetEnableSiPM();

	if(fEnablePMT && !fEnableSiPM)
	{
		if(fEnablePMT) printf("PMTs are enabled and SiPMs are disabled\n");
		fPMT = new SACPMT();
	}
	else if(fEnableSiPM && !fEnablePMT)
	{
		if(fEnableSiPM) printf("SiPMs are enabled and PMTs are disabled\n");
		fSiPM = new SACSiPM();
	}
	else if(!fEnablePMT && !fEnableSiPM) printf("Both PMTs and SiPMS are disabled\n");
	else printf("ERROR --- Both PMTs and SiPMS are disabled\n");

	fSACMessenger = new SACMessenger();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SACDetector::~SACDetector()
{
	if(!fCell) delete fCell;
	if(!fNonRefCell) delete fNonRefCell;

	if(!fPMT) delete fPMT;
	if(!fSiPM) delete fSiPM;

	delete fSACMessenger;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SACDetector::CreateGeometry()
{
	// create main SAC box
	G4Box* fSACSolid = new G4Box(
		"SAC",
		0.5 * fSACSizeX,
		0.5 * fSACSizeY,
		0.5 * fSACSizeZ);

	fSACVolume = new G4LogicalVolume(
		fSACSolid,
		fMaterial,
		"SAC",
		0, 0, 0);
	fSACVolume->SetVisAttributes(G4VisAttributes(G4Colour::White()));

	G4RotationMatrix* fRot = new G4RotationMatrix;
	fRot->rotateY(0.1 * rad);
	fRot->rotateX(0.1 * rad);
	new G4PVPlacement(
		fRot,
		G4ThreeVector(fSACPosX, fSACPosY, fSACPosZ),
		fSACVolume,
		"SAC",
		fMotherVolume,
		false, 0, false);

	// create SAC cell
	fCell->CreateGeometry(6);

	// create SAC cell with no back coating
	fNonRefCell->CreateGeometry(5);

	// create SAC PMT or SiPM
	if(fEnablePMT && !fEnableSiPM) fPMT->CreateGeometry();
	else if(fEnableSiPM && !fEnablePMT) fSiPM->CreateGeometry();

	// position all crystals
	G4double fCrystalPosX;
	G4double fCrystalPosY;
	G4double fCrystalPosZ;
	for(G4int layer = 0; layer < fNLayers; layer++)
	{
		for(G4int row = 0; row < fNRows; row++)
		{
			for(G4int col = 0; col < fNCols; col++)
			{
				// checking if crystal exists
				if(row >= 0 && row < fNRows && col >= 0 && col < fNCols)
				{
					// get crystal position coordinates
					fCrystalPosX = (fCellSizeX + fCellGap) * (-fNCols * 0.5 + col + 0.5);
					fCrystalPosY = (fCellSizeY + fCellGap) * (-fNRows * 0.5 + row + 0.5);
					fCrystalPosZ = (1.5 - layer) * fCellSizeZ;

					G4int CellID = row * fNCols + col + layer * fNRows * fNCols;
					if(CellID % 100 == 0)
					{
						printf("Crystal position %f %f %f\n", fCrystalPosX, fCrystalPosY, fCrystalPosZ);
						printf("***** CellID %d *****\n", CellID);
					}

					// place the crystals and photomultipliers
					G4ThreeVector fCrystalPos = G4ThreeVector(fCrystalPosX, fCrystalPosY, fCrystalPosZ);
					if(layer != fNLayers - 1) // completely coated crystals
					{
						new G4PVPlacement(
							0,
							fCrystalPos,
							fCell->GetCellLogicalVolume(),
							"SACCell",
							fSACVolume,
							false, CellID, false);
					}
					else // crystals with no coating on back
					{
						new G4PVPlacement(
							0,
							fCrystalPos,
							fNonRefCell->GetNonRefCellLogicalVolume(),
							"SACNonRefCell",
							fSACVolume,
							false, CellID, false);

						if(fEnablePMT && !fEnableSiPM)
						{
							new G4PVPlacement(
								0,
								// TODO: FIGURE OUT HOW TO PLACE PMT RIGHT UP AGAINST CELL
								G4ThreeVector(fCrystalPosX, fCrystalPosY, -0.5 * fSACSizeZ),
								fPMT->GetPMTLogicalVolume(),
								"SACPMT",
								fSACVolume,
								false, 0, false);
						}
						else if(fEnableSiPM && !fEnablePMT)
						{
							new G4PVPlacement(
								0,
								// TODO: FIGURE OUT HOW TO PLACE SiPM RIGHT UP AGAINST CELL
								G4ThreeVector(fCrystalPosX, fCrystalPosY, -0.5 * fSACSizeZ),
								fSiPM->GetSiPMLogicalVolume(),
								"SACSiPM",
								fSACVolume,
								false, 0, false);
						}
					}
				}
				else
				{
					printf("SACDetector::CreateGeometry - ERROR - Requested crystal at row %d col %d\n", row, col);
				}
			}
		}
	}
}
