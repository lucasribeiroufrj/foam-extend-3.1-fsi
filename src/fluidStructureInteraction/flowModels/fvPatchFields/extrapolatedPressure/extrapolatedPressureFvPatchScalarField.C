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

\*---------------------------------------------------------------------------*/

#include "extrapolatedPressureFvPatchScalarField.H"
#include "addToRunTimeSelectionTable.H"
#include "fvPatchFieldMapper.H"
#include "volFields.H"
#include "uniformDimensionedFields.H"

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

namespace Foam
{

// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

extrapolatedPressureFvPatchScalarField::
extrapolatedPressureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF
)
:
    zeroGradientFvPatchScalarField(p, iF)
{}


extrapolatedPressureFvPatchScalarField::
extrapolatedPressureFvPatchScalarField
(
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const dictionary& dict
)
:
    zeroGradientFvPatchScalarField(p, iF)
{
    fvPatchField<scalar>::operator=(patchInternalField());
}


extrapolatedPressureFvPatchScalarField::
extrapolatedPressureFvPatchScalarField
(
    const extrapolatedPressureFvPatchScalarField& ptf,
    const fvPatch& p,
    const DimensionedField<scalar, volMesh>& iF,
    const fvPatchFieldMapper& mapper
)
:
    zeroGradientFvPatchScalarField(ptf, p, iF, mapper)
{}


extrapolatedPressureFvPatchScalarField::
extrapolatedPressureFvPatchScalarField
(
    const extrapolatedPressureFvPatchScalarField& ptf
)
:
    zeroGradientFvPatchScalarField(ptf)
{}


extrapolatedPressureFvPatchScalarField::
extrapolatedPressureFvPatchScalarField
(
    const extrapolatedPressureFvPatchScalarField& ptf,
    const DimensionedField<scalar, volMesh>& iF
)
:
    zeroGradientFvPatchScalarField(ptf, iF)
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

void extrapolatedPressureFvPatchScalarField::evaluate(const Pstream::commsTypes)
{
    if (!this->updated())
    {
        this->updateCoeffs();
    }

    const fvPatchField<vector>& gradP =
        patch().lookupPatchField<volVectorField, scalar>("grad(p)");

    vectorField delta = this->patch().delta();

    Field<scalar>::operator=
    (
        this->patchInternalField() + (delta & gradP.patchInternalField())
    );

    fvPatchField<scalar>::evaluate();
}


// void extrapolatedPressureFvPatchScalarField::updateCoeffs()
// {
//     if (updated())
//     {
//         return;
//     }

//     const uniformDimensionedVectorField& g =
//         db().lookupObject<uniformDimensionedVectorField>("g");

//     const fvPatchField<scalar>& rho =
//         patch().lookupPatchField<volScalarField, scalar>(rhoName_);

//     // If the variable name is "p_rgh" or "pd" assume it is p - rho*g.h
//     // and set the gradient appropriately.
//     // Otherwise assume the variable is the static pressure.
//     if
//     (
//         dimensionedInternalField().name() == "p_rgh"
//      || dimensionedInternalField().name() == "pd"
//     )
//     {
//         gradient() = -rho.snGrad()*(g.value() & patch().Cf());
//     }
//     else
//     {
//         gradient() = rho*(g.value() & patch().nf());
//     }

//     zeroGradientFvPatchScalarField::updateCoeffs();
// }


void extrapolatedPressureFvPatchScalarField::write(Ostream& os) const
{
    zeroGradientFvPatchScalarField::write(os);
    writeEntry("value", os);
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

makePatchTypeField
(
    fvPatchScalarField,
    extrapolatedPressureFvPatchScalarField
);

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
