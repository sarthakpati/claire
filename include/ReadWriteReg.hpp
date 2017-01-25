/*************************************************************************
 *  Copyright (c) 2016.
 *  All rights reserved.
 *  This file is part of the XXX library.
 *
 *  XXX is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  XXX is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XXX.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef _READWRITEREG_H_
#define _READWRITEREG_H_

// library includes
#ifdef REG_HAS_NIFTI
#include "nifti1_io.h"
#endif

#if defined(PETSC_HAVE_HDF5)
#include "petscviewerhdf5.h"
#endif

#ifdef REG_HAS_PNETCDF
#include "pnetcdf.h"
#endif

#include "RegOpt.hpp"
#include "VecField.hpp"




namespace reg {




class ReadWriteReg {
 public:
    typedef ReadWriteReg Self;

    ReadWriteReg(void);
    ReadWriteReg(RegOpt*);
    ~ReadWriteReg(void);

    PetscErrorCode Read(Vec*, std::string, bool multicomponent = false);
    PetscErrorCode Read(VecField*, std::string, std::string, std::string);

    PetscErrorCode Write(Vec, std::string, bool multicomponent = false);
    PetscErrorCode WriteMC(Vec, std::string);
    PetscErrorCode Write(VecField*, std::string, std::string, std::string);

 private:
    PetscErrorCode Initialize();
    PetscErrorCode ClearMemory();

    PetscErrorCode Read(Vec*);

    PetscErrorCode Write(Vec);
    PetscErrorCode WriteMC(Vec);
    PetscErrorCode Write(VecField*);

#if defined(PETSC_HAVE_HDF5)
    PetscErrorCode ReadHDF5(Vec*);
    PetscErrorCode WriteHDF5(Vec);
#endif

#ifdef REG_HAS_PNETCDF
    PetscErrorCode ReadNC(Vec*);
    PetscErrorCode WriteNC(Vec);
#endif

    PetscErrorCode ReadBIN(Vec*);
    PetscErrorCode WriteBIN(Vec);

    PetscErrorCode ReadNetCDF(Vec);
    PetscErrorCode ReadTimeSeriesNetCDF(Vec);
    PetscErrorCode ReadBlockNetCDF(Vec, int*);

    PetscErrorCode WriteNetCDF(Vec);
    PetscErrorCode WriteTimeSeriesNetCDF(Vec);
    PetscErrorCode WriteBlockNetCDF(Vec, int*);

#ifdef REG_HAS_NIFTI
    PetscErrorCode ReadNII(Vec*);
    PetscErrorCode ReadNII(VecField*);
    PetscErrorCode ReadNII(nifti_image*);
    template <typename T> PetscErrorCode ReadNII(nifti_image*);

    PetscErrorCode WriteNII(Vec);
    PetscErrorCode WriteNII(nifti_image**);
    template <typename T> PetscErrorCode WriteNII(nifti_image**, Vec);

    PetscErrorCode GetComponentTypeNII(nifti_image*);;
    PetscErrorCode AllocateNII(nifti_image**, Vec);
#endif

    enum VoxelType{CHAR, UCHAR, SHORT, USHORT, INT, UINT, FLOAT, DOUBLE, UNDEF};
    VoxelType m_ComponentType;

    RegOpt* m_Opt;
    IntType* m_iSizeC;
    IntType* m_iStartC;
    int* m_nOffset;
    int* m_nSend;

    ScalarType* m_Data;
    IntType m_nx[3];

    std::string m_FileName;
    std::string m_FileNameX1;
    std::string m_FileNameX2;
    std::string m_FileNameX3;
};




}  // namespace reg




#endif  // _READWRITEREG_H_
