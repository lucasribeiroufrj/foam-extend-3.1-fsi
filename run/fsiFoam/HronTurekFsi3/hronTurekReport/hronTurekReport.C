/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright held by original author
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Author
    Zeljko Tukovic, FSB Zagreb.  All rights reserved

\*----------------------------------------------------------------------------*/

#include "hronTurekReport.H"
#include "addToRunTimeSelectionTable.H"
#include "volFields.H"
#include "pointFields.H"
#include "boundBox.H"
#include "fluidStructureInterface.H"
#include "polyPatchID.H"

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

namespace Foam
{
    defineTypeNameAndDebug(hronTurekReport, 0);

    addToRunTimeSelectionTable
    (
        functionObject,
        hronTurekReport,
        dictionary
    );
}


// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //

bool Foam::hronTurekReport::writeData()
{    
    const fvMesh& mesh =
        time_.lookupObject<fvMesh>(regionName_);

    const fluidStructureInterface& fsi = 
        mesh.lookupObject<fluidStructureInterface>("fsiProperties");

    word cylinderPatchName("cylinder");

    polyPatchID cylinderPatch(cylinderPatchName, mesh.boundaryMesh());

    if (!cylinderPatch.active())
    {
        FatalErrorIn("hronTurekReport::writeData()")
            << "Cylinder patch name " << cylinderPatchName << " not found."
                << abort(FatalError);
    }

    label cylinderPatchIndex = cylinderPatch.index();
    
    vectorField cylinderPressureForce =
       -mesh.boundary()[cylinderPatchIndex].nf()
       *fsi.flow().patchPressureForce(cylinderPatchIndex);

    vectorField cylinderViscousForce =
        fsi.flow().patchViscousForce(cylinderPatchIndex);

    vector F = 
        gSum
        (
            cylinderPressureForce
           *mesh.magSf().boundaryField()[cylinderPatchIndex]
        )
      + gSum
        (
            cylinderViscousForce
           *mesh.magSf().boundaryField()[cylinderPatchIndex]
        );

    word platePatchName("plate");

    polyPatchID platePatch(platePatchName, mesh.boundaryMesh());

    if (!platePatch.active())
    {
        FatalErrorIn("hronTurekReport::writeData()")
            << "Plate patch name " << platePatchName << " not found."
                << abort(FatalError);
    }

    label platePatchIndex = platePatch.index();

    vectorField platePressureForce =
       -mesh.boundary()[platePatchIndex].nf()
       *fsi.flow().patchPressureForce(platePatchIndex);

    vectorField plateViscousForce =
        fsi.flow().patchViscousForce(platePatchIndex);

    F += 
        gSum
        (
            platePressureForce
           *mesh.magSf().boundaryField()[platePatchIndex]
        )
      + gSum
        (
            plateViscousForce
           *mesh.magSf().boundaryField()[platePatchIndex]
        );

    const Vector<label>& directions = mesh.geometricD();

    scalar thickness = 0.0;
    for (direction dir = 0; dir < directions.nComponents; dir++)
    {
        if (directions[dir] == -1)
        {
            thickness = mesh.bounds().span()[dir];
            break;
        }
    }

    F /= thickness + SMALL;

    // Part of the total viscous force which exists only at the moving 
    // and/or deforming interfaces

//     tensorField solidZoneSurfaceGradientOfVelocity =
//         fsi.stress().faceZoneSurfaceGradientOfVelocity
//         (
//             fsi.solidZoneIndex(),
//             fsi.solidPatchIndex()
//         );

//     vectorField solidZoneNormal = 
//         fsi.stress().faceZoneNormal
//         (
//             fsi.solidZoneIndex(),
//             fsi.solidPatchIndex()
//         );

//     vectorField solidZoneTraction = 
//         (
//            -2*tr(solidZoneSurfaceGradientOfVelocity)*solidZoneNormal
//           + (solidZoneSurfaceGradientOfVelocity&solidZoneNormal)
//         );

//     vectorField fluidZoneTraction =
//        -fsi.ggiInterpolator().slaveToMaster
//         (
//             solidZoneTraction
//         );

//     fluidZoneTraction *= 
//         fsi.flow().faceZoneMuEff
//         (
//             fsi.fluidZoneIndex(), 
//             fsi.fluidPatchIndex()
//         );

//     scalarField fluidZoneAreas
//     (
//         mag(mesh.faceAreas()),
//         mesh.faceZones()[fsi.fluidZoneIndex()]
//     );

//     vector Fm = 
//         sum
//         (
//             fluidZoneTraction
//            *fluidZoneAreas
//         );
  
    if (Pstream::master())
    {
        historyFilePtr_() 
            << mesh.time().value() << tab 
                << F.x() << tab 
                << F.y() << tab
                << fsi.outerCorr() << endl;
    }

    return true;
}

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

Foam::hronTurekReport::hronTurekReport
(
    const word& name,
    const Time& t,
    const dictionary& dict
)
:
    functionObject(name),
    name_(name),
    time_(t),
    regionName_(polyMesh::defaultRegion),
    historyFilePtr_(NULL)
{
    Info << "Creating " << this->name() << " function object." << endl;

//     if (Pstream::parRun())
//     {
//         FatalErrorIn("hronTurekReport::hronTurekReport(...)")
//             << "hronTurekReport objec function "
//                 << "is not implemented for parallel run"
//                 << abort(FatalError);
//     }

    if (dict.found("region"))
    {
        dict.lookup("region") >> regionName_;
    }

    const fvMesh& mesh =
        time_.lookupObject<fvMesh>(regionName_);

    // Create history file if not already created
    if (historyFilePtr_.empty())
    {
        // File update
        if (Pstream::master())
        {
            fileName historyDir;

            word startTimeName =
                mesh.time().timeName(mesh.time().startTime().value());

            if (Pstream::parRun())
            {
                // Put in undecomposed case (Note: gives problems for
                // distributed data running)
                historyDir = time_.path()/".."/"history"/startTimeName;
            }
            else
            {
                historyDir = time_.path()/"history"/startTimeName;
            }

//             historyDir = mesh.time().path()/"history"/startTimeName;

            // Create directory if does not exist.
            mkDir(historyDir);

            // Open new file at start up
            historyFilePtr_.reset(new OFstream(historyDir/"force.dat"));

            // Add headers to output data
            if (historyFilePtr_.valid())
            {
                historyFilePtr_()
                    << "# Time" << tab << "FD" << tab << "FL" << tab
                        << "nCorr" << endl;
            }
        }
    }
}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

bool Foam::hronTurekReport::start()
{
    return writeData();
}


bool Foam::hronTurekReport::execute()
{
    const fvMesh& mesh =
        time_.lookupObject<fvMesh>(regionName_);

    fluidStructureInterface& fsi = 
        const_cast<fluidStructureInterface&>
        (
            mesh.lookupObject<fluidStructureInterface>("fsiProperties")
        );

    if (time_.value()>2)
    {
        if (!fsi.coupled())
        {
            fsi.set("coupled", true);
        }
    }

    return writeData();
}


bool Foam::hronTurekReport::read(const dictionary& dict)
{
    if (dict.found("region"))
    {
        dict.lookup("region") >> regionName_;
    }

    return true;
}

// ************************************************************************* //
