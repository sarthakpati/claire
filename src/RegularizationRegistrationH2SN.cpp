#ifndef _REGULARIZATIONREGISTRATIONH2SN_CPP_
#define _REGULARIZATIONREGISTRATIONH2SN_CPP_

#include "RegularizationRegistrationH2SN.hpp"




namespace reg
{




/********************************************************************
 * Name: RegularizationRegistrationH2SN
 * Description: default constructor
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "RegularizationRegistrationH2SN"
RegularizationRegistrationH2SN::RegularizationRegistrationH2SN() : SuperClass()
{
}




/********************************************************************
 * Name: ~RegularizationRegistrationH2SN
 * Description: default destructor
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "~RegularizationRegistrationH2SN"
RegularizationRegistrationH2SN::~RegularizationRegistrationH2SN(void)
{
    this->ClearMemory();
}




/********************************************************************
 * Name: RegularizationRegistrationH2SN
 * Description: constructor
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "RegularizationRegistrationH2SN"
RegularizationRegistrationH2SN::RegularizationRegistrationH2SN(RegOpt* opt) : SuperClass(opt)
{

}




/********************************************************************
 * Name: EvaluateFunctional
 * Description: evaluates the functional
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "EvaluateFunctional"
PetscErrorCode RegularizationRegistrationH2SN::EvaluateFunctional(ScalarType* R, VecField* v)
{
    PetscErrorCode ierr;
    unsigned int n[3];
    IntType iosize[3];
    int isize[3],osize[3],istart[3],ostart[3];
    ScalarType *p_v1=NULL,*p_v2=NULL,*p_v3=NULL,
                *p_Lv1=NULL,*p_Lv2=NULL,*p_Lv3=NULL;
    ScalarType sqrtbeta,ipxi,scale,hd;
    double ffttimers[5]={0,0,0,0,0};

    PetscFunctionBegin;

    ierr=Assert(v != NULL,"null pointer"); CHKERRQ(ierr);

    // compute hd
    hd = this->m_Opt->GetLebesqueMeasure();

    // get regularization weight
    sqrtbeta = sqrt(this->m_Opt->GetRegularizationWeight());

    *R = 0.0;

    // if regularization weight is zero, do noting
    if (sqrtbeta != 0.0){

        ierr=this->Allocate(); CHKERRQ(ierr);

        if (this->m_WorkVecField==NULL){
            try{this->m_WorkVecField = new VecField(this->m_Opt);}
            catch (std::bad_alloc&){
                ierr=reg::ThrowError("allocation failed"); CHKERRQ(ierr);
            }
        }

        accfft_local_size_dft_r2c_t<ScalarType>(this->m_Opt->m_MiscOpt->N,
                                                isize,istart,osize,ostart,
                                                this->m_Opt->m_MiscOpt->c_comm);

        for (int i=0; i < 3; ++i){
            iosize[i] = static_cast<IntType>(osize[i]);
            n[i]      = static_cast<unsigned int>(this->m_Opt->m_MiscOpt->N[i]);
        }
        scale = this->m_Opt->ComputeFFTScale();

        ierr=VecGetArray(v->m_X1,&p_v1); CHKERRQ(ierr);
        ierr=VecGetArray(v->m_X2,&p_v2); CHKERRQ(ierr);
        ierr=VecGetArray(v->m_X3,&p_v3); CHKERRQ(ierr);

        // compute forward fft
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_v1,this->m_v1hat,ffttimers);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_v2,this->m_v2hat,ffttimers);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_v3,this->m_v3hat,ffttimers);
        this->m_Opt->IncrementCounter(FFT,3);

        ierr=VecRestoreArray(v->m_X1,&p_v1); CHKERRQ(ierr);
        ierr=VecRestoreArray(v->m_X2,&p_v2); CHKERRQ(ierr);
        ierr=VecRestoreArray(v->m_X3,&p_v3); CHKERRQ(ierr);

        sqrtbeta = sqrt(this->m_Opt->GetRegularizationWeight());

#pragma omp parallel
{
        long int w[3];
        ScalarType lapik,regop;
        IntType i;

#pragma omp for
        for (IntType i1 = 0; i1 < iosize[0]; ++i1){
            for (IntType i2 = 0; i2 < iosize[1]; i2++){
                for (IntType i3 = 0; i3 < iosize[2]; ++i3){

                    w[0] = static_cast<long int>(i1 + ostart[0]);
                    w[1] = static_cast<long int>(i2 + ostart[1]);
                    w[2] = static_cast<long int>(i3 + ostart[2]);

                    CheckWaveNumbers(w,n);

                    // compute bilaplacian operator
                    lapik = -static_cast<ScalarType>(w[0]*w[0] + w[1]*w[1] + w[2]*w[2]);

                    i=GetLinearIndex(i1,i2,i3,iosize);

                    // compute regularization operator
                    regop = scale*sqrtbeta*lapik;

                    // apply to individual components
                    this->m_Lv1hat[i][0] = regop*this->m_v1hat[i][0];
                    this->m_Lv1hat[i][1] = regop*this->m_v1hat[i][1];

                    this->m_Lv2hat[i][0] = regop*this->m_v2hat[i][0];
                    this->m_Lv2hat[i][1] = regop*this->m_v2hat[i][1];

                    this->m_Lv3hat[i][0] = regop*this->m_v3hat[i][0];
                    this->m_Lv3hat[i][1] = regop*this->m_v3hat[i][1];

                }
            }
        }

}// pragma omp parallel

        ierr=VecGetArray(this->m_WorkVecField->m_X1,&p_Lv1); CHKERRQ(ierr);
        ierr=VecGetArray(this->m_WorkVecField->m_X2,&p_Lv2); CHKERRQ(ierr);
        ierr=VecGetArray(this->m_WorkVecField->m_X3,&p_Lv3); CHKERRQ(ierr);

        // compute inverse fft
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv1hat,p_Lv1,ffttimers);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv2hat,p_Lv2,ffttimers);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv3hat,p_Lv3,ffttimers);
        this->m_Opt->IncrementCounter(FFT,3);

        // restore arrays
        ierr=VecRestoreArray(this->m_WorkVecField->m_X1,&p_Lv1); CHKERRQ(ierr);
        ierr=VecRestoreArray(this->m_WorkVecField->m_X2,&p_Lv2); CHKERRQ(ierr);
        ierr=VecRestoreArray(this->m_WorkVecField->m_X3,&p_Lv3); CHKERRQ(ierr);

        ierr=VecTDot(this->m_WorkVecField->m_X1,this->m_WorkVecField->m_X1,&ipxi); CHKERRQ(ierr); *R += ipxi;
        ierr=VecTDot(this->m_WorkVecField->m_X2,this->m_WorkVecField->m_X2,&ipxi); CHKERRQ(ierr); *R += ipxi;
        ierr=VecTDot(this->m_WorkVecField->m_X3,this->m_WorkVecField->m_X3,&ipxi); CHKERRQ(ierr); *R += ipxi;

        // multiply with regularization weight
        *R *= 0.5*hd;

        // increment fft timer
        this->m_Opt->IncreaseFFTTimers(ffttimers);

    }

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: EvaluateGradient
 * Description: evaluates first variation of regularization norm
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "EvaluateGradient"
PetscErrorCode RegularizationRegistrationH2SN::EvaluateGradient(VecField* dvR, VecField* v)
{
    PetscErrorCode ierr;
    unsigned int n[3];
    IntType iosize[3];
    int isize[3],osize[3],istart[3],ostart[3];
    ScalarType beta,hd,scale;
    double ffttimers[5]={0,0,0,0,0};
    ScalarType *p_v1=NULL,*p_v2=NULL,*p_v3=NULL,
                *p_Lv1=NULL,*p_Lv2=NULL,*p_Lv3=NULL;

    PetscFunctionBegin;

    ierr=Assert(v != NULL,"null pointer"); CHKERRQ(ierr);
    ierr=Assert(dvR != NULL,"null pointer"); CHKERRQ(ierr);

    beta = this->m_Opt->GetRegularizationWeight();

    // if regularization weight is zero, do noting
    if (beta == 0.0){
        ierr=VecSet(dvR->m_X1,0.0); CHKERRQ(ierr);
        ierr=VecSet(dvR->m_X2,0.0); CHKERRQ(ierr);
        ierr=VecSet(dvR->m_X3,0.0); CHKERRQ(ierr);
    }
    else{

        ierr=this->Allocate(); CHKERRQ(ierr);

        accfft_local_size_dft_r2c_t<ScalarType>(this->m_Opt->m_MiscOpt->N,
                                                isize,istart,osize,ostart,
                                                this->m_Opt->m_MiscOpt->c_comm);

        for (int i=0; i < 3; ++i){
            n[i]      = static_cast<unsigned int>(this->m_Opt->m_MiscOpt->N[i]);
            iosize[i] = static_cast<IntType>(osize[i]);
        }
        scale = this->m_Opt->ComputeFFTScale();

        ierr=VecGetArray(v->m_X1,&p_v1); CHKERRQ(ierr);
        ierr=VecGetArray(v->m_X2,&p_v2); CHKERRQ(ierr);
        ierr=VecGetArray(v->m_X3,&p_v3); CHKERRQ(ierr);

        // compute forward fft
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_v1,this->m_v1hat,ffttimers);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_v2,this->m_v2hat,ffttimers);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_v3,this->m_v3hat,ffttimers);
        this->m_Opt->IncrementCounter(FFT,3);

        ierr=VecRestoreArray(v->m_X1,&p_v1); CHKERRQ(ierr);
        ierr=VecRestoreArray(v->m_X2,&p_v2); CHKERRQ(ierr);
        ierr=VecRestoreArray(v->m_X3,&p_v3); CHKERRQ(ierr);

        beta = this->m_Opt->GetRegularizationWeight();

#pragma omp parallel
{
        long int w[3];
        ScalarType biharik,regop;
        IntType i;

#pragma omp for
        for (IntType i1 = 0; i1 < iosize[0]; ++i1){
            for (IntType i2 = 0; i2 < iosize[1]; i2++){
                for (IntType i3 = 0; i3 < iosize[2]; ++i3){

                    w[0] = static_cast<long int>(i1 + ostart[0]);
                    w[1] = static_cast<long int>(i2 + ostart[1]);
                    w[2] = static_cast<long int>(i3 + ostart[2]);

                    CheckWaveNumbers(w,n);

                    // compute bilaplacian operator
                    biharik = -static_cast<ScalarType>(w[0]*w[0] + w[1]*w[1] + w[2]*w[2]);
                    biharik *= biharik;

                    i=GetLinearIndex(i1,i2,i3,iosize);

                    // compute regularization operator
                    regop = scale*beta*biharik;

                    // apply to individual components
                    this->m_Lv1hat[i][0] = regop*this->m_v1hat[i][0];
                    this->m_Lv1hat[i][1] = regop*this->m_v1hat[i][1];

                    this->m_Lv2hat[i][0] = regop*this->m_v2hat[i][0];
                    this->m_Lv2hat[i][1] = regop*this->m_v2hat[i][1];

                    this->m_Lv3hat[i][0] = regop*this->m_v3hat[i][0];
                    this->m_Lv3hat[i][1] = regop*this->m_v3hat[i][1];

                }
            }
        }
}// pragma omp parallel

        ierr=VecGetArray(dvR->m_X1,&p_Lv1); CHKERRQ(ierr);
        ierr=VecGetArray(dvR->m_X2,&p_Lv2); CHKERRQ(ierr);
        ierr=VecGetArray(dvR->m_X3,&p_Lv3); CHKERRQ(ierr);

        // compute inverse fft
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv1hat,p_Lv1,ffttimers);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv2hat,p_Lv2,ffttimers);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv3hat,p_Lv3,ffttimers);
        this->m_Opt->IncrementCounter(FFT,3);

        // restore arrays
        ierr=VecRestoreArray(dvR->m_X1,&p_Lv1); CHKERRQ(ierr);
        ierr=VecRestoreArray(dvR->m_X2,&p_Lv2); CHKERRQ(ierr);
        ierr=VecRestoreArray(dvR->m_X3,&p_Lv3); CHKERRQ(ierr);

        // compute hd
        hd = this->m_Opt->GetLebesqueMeasure();

        // scale vector field
        ierr=dvR->Scale(hd); CHKERRQ(ierr);

        // increment fft timer
        this->m_Opt->IncreaseFFTTimers(ffttimers);
    }

    PetscFunctionReturn(0);
}




/********************************************************************
 * Name: HessianMatvec
 * Description: applies second variation of regularization norm to
 * a vector
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "HessianMatVec"
PetscErrorCode RegularizationRegistrationH2SN::HessianMatVec(VecField* dvvR, VecField* vtilde)
{
    PetscErrorCode ierr;
    ScalarType beta,hd;
    PetscFunctionBegin;

    ierr=Assert(vtilde != NULL,"null pointer"); CHKERRQ(ierr);
    ierr=Assert(dvvR != NULL,"null pointer"); CHKERRQ(ierr);

    beta = this->m_Opt->GetRegularizationWeight();

    // if regularization weight is zero, do noting
    if (beta == 0.0){
        ierr=VecSet(dvvR->m_X1,0.0); CHKERRQ(ierr);
        ierr=VecSet(dvvR->m_X2,0.0); CHKERRQ(ierr);
        ierr=VecSet(dvvR->m_X3,0.0); CHKERRQ(ierr);
    }
    else{ ierr=this->EvaluateGradient(dvvR,vtilde); CHKERRQ(ierr); }

    PetscFunctionReturn(0);
}





/********************************************************************
 * Name: ApplyInverseOperator
 * Description: apply the inverse of the regularization operator; we
 * can invert this operator analytically due to the spectral
 * discretization
 *******************************************************************/
#undef __FUNCT__
#define __FUNCT__ "ApplyInverseOperator"
PetscErrorCode RegularizationRegistrationH2SN::ApplyInverseOperator(VecField* Ainvx, VecField* x)
{
    PetscErrorCode ierr;
    int isize[3],osize[3],istart[3],ostart[3];
    ScalarType beta,scale;
    unsigned int n[3];
    IntType iosize[3];
    double ffttimers[5]={0,0,0,0,0};
    ScalarType *p_x1=NULL,*p_x2=NULL,*p_x3=NULL,
                *p_Lv1=NULL,*p_Lv2=NULL,*p_Lv3=NULL;

    PetscFunctionBegin;

    ierr=Assert(x != NULL,"null pointer"); CHKERRQ(ierr);
    ierr=Assert(Ainvx != NULL,"null pointer"); CHKERRQ(ierr);

    beta = this->m_Opt->GetRegularizationWeight();

    // if regularization weight is zero, do noting
    if (beta == 0.0){
        ierr=VecCopy(x->m_X1,Ainvx->m_X1); CHKERRQ(ierr);
        ierr=VecCopy(x->m_X2,Ainvx->m_X2); CHKERRQ(ierr);
        ierr=VecCopy(x->m_X3,Ainvx->m_X3); CHKERRQ(ierr);
    }
    else{

        ierr=this->Allocate(); CHKERRQ(ierr);

        accfft_local_size_dft_r2c_t<ScalarType>(this->m_Opt->m_MiscOpt->N,
                                                isize,istart,osize,ostart,
                                                this->m_Opt->m_MiscOpt->c_comm);

        for (int i=0; i < 3; ++i){
            n[i] = this->m_Opt->m_MiscOpt->N[i];
            iosize[i] = osize[i];
        }
        scale = this->m_Opt->ComputeFFTScale();

        ierr=VecGetArray(x->m_X1,&p_x1); CHKERRQ(ierr);
        ierr=VecGetArray(x->m_X2,&p_x2); CHKERRQ(ierr);
        ierr=VecGetArray(x->m_X3,&p_x3); CHKERRQ(ierr);

        // compute forward fft
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_x1,this->m_v1hat,ffttimers);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_x2,this->m_v2hat,ffttimers);
        accfft_execute_r2c_t<ScalarType,FFTScaType>(this->m_Opt->m_MiscOpt->plan,p_x3,this->m_v3hat,ffttimers);
        this->m_Opt->IncrementCounter(FFT,3);

        ierr=VecRestoreArray(x->m_X1,&p_x1); CHKERRQ(ierr);
        ierr=VecRestoreArray(x->m_X2,&p_x2); CHKERRQ(ierr);
        ierr=VecRestoreArray(x->m_X3,&p_x3); CHKERRQ(ierr);


#pragma omp parallel
{
        long int w[3];
        ScalarType biharik,regop;
        IntType i;

#pragma omp for
        for (IntType i1 = 0; i1 < iosize[0]; ++i1){
            for (IntType i2 = 0; i2 < iosize[1]; i2++){
                for (IntType i3 = 0; i3 < iosize[2]; ++i3){

                    w[0] = static_cast<long int>(i1 + ostart[0]);
                    w[1] = static_cast<long int>(i2 + ostart[1]);
                    w[2] = static_cast<long int>(i3 + ostart[2]);

                    CheckWaveNumbersInv(w,n);

                    // compute bilaplacian operator
                    biharik = -static_cast<ScalarType>(w[0]*w[0] + w[1]*w[1] + w[2]*w[2]);
                    biharik *= biharik;

                    // compute regularization operator
                    regop = (fabs(biharik) == 0) ? scale : scale/(beta*biharik);

                    i=GetLinearIndex(i1,i2,i3,iosize);

                    // apply to individual components
                    this->m_Lv1hat[i][0] = regop*this->m_v1hat[i][0];
                    this->m_Lv1hat[i][1] = regop*this->m_v1hat[i][1];

                    this->m_Lv2hat[i][0] = regop*this->m_v2hat[i][0];
                    this->m_Lv2hat[i][1] = regop*this->m_v2hat[i][1];

                    this->m_Lv3hat[i][0] = regop*this->m_v3hat[i][0];
                    this->m_Lv3hat[i][1] = regop*this->m_v3hat[i][1];

                }
            }
        }

}// pragma omp parallel

        ierr=VecGetArray(Ainvx->m_X1,&p_Lv1); CHKERRQ(ierr);
        ierr=VecGetArray(Ainvx->m_X2,&p_Lv2); CHKERRQ(ierr);
        ierr=VecGetArray(Ainvx->m_X3,&p_Lv3); CHKERRQ(ierr);

        // compute inverse fft
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv1hat,p_Lv1,ffttimers);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv2hat,p_Lv2,ffttimers);
        accfft_execute_c2r_t<FFTScaType,ScalarType>(this->m_Opt->m_MiscOpt->plan,this->m_Lv3hat,p_Lv3,ffttimers);
        this->m_Opt->IncrementCounter(FFT,3);

        // restore arrays
        ierr=VecRestoreArray(Ainvx->m_X1,&p_Lv1); CHKERRQ(ierr);
        ierr=VecRestoreArray(Ainvx->m_X2,&p_Lv2); CHKERRQ(ierr);
        ierr=VecRestoreArray(Ainvx->m_X3,&p_Lv3); CHKERRQ(ierr);

        // increment fft timer
        this->m_Opt->IncreaseFFTTimers(ffttimers);

    }

    PetscFunctionReturn(0);
}






} // end of name space

#endif //_REGULARIZATIONREGISTRATIONH2SN_CPP_