/*
 *  Naf_Jeff.c
 *  cnerve
 *
 *  Created by Nikita on 2/15/10.
 *  Copyright 2010 University of Washington. All rights reserved.
 *
 */


#include <math.h>					// for misc. log/exp operations

#include <stdbool.h>				// for bool defenition
#include <stdint.h>					// for uint32_t defenition
#include <stdio.h>					// for FILE defenition

#include "../rateChannels.h"
#include "../mt19937ar.h"				// for random number generator

/****************************************************************************
					DO NOT USE THE FUNCTION BELOW.  BROKEN!
 ****************************************************************************/

/**
	@brief	Gillespie's algorithm for N_Na with 2 State Model
			closed ---> (@ \ALPHA_M rate) open
				   <--- (@ \BETA_M rate) 

			and

			inactivated ---> (@ \ALPHA_H rate) activated
						<--- (@ \BETA_H rate) 


	\param[in] N_Na 	# of channels
	\param[in] V_m		voltage current at the node where the # of open chans is being calculated (mV)
	\param[in] si		time step increment, "delt" in our jargon
	\param[in] it		iteration # (time step #)
	\param[in] node		node number for which we're performing the calculation

	\return n_Na		number of Na channels open ?




*/
#ifdef DEBUG
double Jeff_Na(const bool doInit, const uint32_t N_Na, const double V_m, const double si, const uint32_t it, const uint32_t node, const kinParam Kins, FILE *fp  __attribute__ ((unused)) ) {
#else
double Jeff_Na(const bool doInit, const uint32_t N_Na, const double V_m, const double si, const uint32_t it, const uint32_t node, const kinParam Kins) {
#endif

/*		STATIC VARIABLES	*/
	static int  jeff0[MAX_RAN_NODES], jeff1[MAX_RAN_NODES];
	static double t_occ_Na[MAX_RAN_NODES];

/* 		OTHERS 				*/

	int		state = -1;
	int		n_Na;
  double    gamma_closed,gamma_open;
  double    a_m,b_m,a_h,b_h,d_m_h = 0;
  double    lambda;
  double    u,t,t_life,tmp = 0;
  double    P[3],eta[3];

/* initialize(0) or generation(1) */
  if( unlikely( doInit )){

  /*                */
  /* initialization */
  /*                */

    /* Rate Constants for V_m=0.0 */
      a_m = rate_Na(ALPHA_M,V_m,Kins); b_m = rate_Na(BETA_M,V_m,Kins);
      a_h = rate_Na(ALPHA_H,V_m,Kins); b_h = rate_Na(BETA_H,V_m,Kins);

    /* Initialize the number of channels in each state */
      d_m_h=(a_m+b_m)*(a_m+b_m)*(a_m+b_m)*(a_h+b_h);
	  tmp = pow(a_m,3.0)*a_h; 
      jeff1[node]=(int)(N_Na*(tmp/d_m_h));
      jeff0[node]=N_Na-jeff1[node];
      

    /* gamma */
      gamma_open = b_h+3.0*b_m;
	  gamma_closed = tmp*gamma_open/(d_m_h-tmp);

    /* lambda */
      lambda = (double)jeff0[node]*gamma_closed + (double)jeff1[node]*gamma_open; 
            

    /* Lifetime */
    /* ``+si'' is important if it=[1,nd], remove it if it=[0,nd-1] */
      t_occ_Na[node]=-log(genrand_real2())/lambda;

  } else {
  /*            */
  /* generation */
  /*            */


    /* determine if t_occ is within [t,t+si) */
      t=si*(double)it;
      while ( (t<=t_occ_Na[node]) && (t_occ_Na[node]<=(t+si)) ) {

      /* rate for Na channels at V_m */
        a_m = rate_Na(ALPHA_M,V_m,Kins); b_m = rate_Na(BETA_M,V_m,Kins);
        a_h = rate_Na(ALPHA_H,V_m,Kins); b_h = rate_Na(BETA_H,V_m,Kins);

      /* gamma */
        gamma_open = b_h+3.0*b_m;
		gamma_closed = tmp*gamma_open/(d_m_h-tmp);

      /* lambda */
         lambda = (double)jeff0[node]*gamma_closed + (double)jeff1[node]*gamma_open; 
          

      /* eta */
        eta[0]= 0.0;
        eta[1]= gamma_closed * (double)jeff0[node];  
        eta[2]= gamma_open * (double)jeff1[node];
        

      /* cummulative state transition prob */
        tmp=0.0;
        for(int i=1;i<=2;i++){
          tmp+=eta[i];
          P[i]=tmp;
        }

      /* determine which one of those states */ 
        u=genrand_real2()*lambda;
        for(int i=1;i<=2;i++){
          if ((P[i-1]<=u)&&(u<P[i])) {
            state=i;
     	  }
        }

      /* update the number of channels */
        switch(state){
		case 1:  jeff0[node]--; jeff1[node]++; break; 
		case 2:  jeff1[node]--; jeff0[node]++; break;
				default:	// throw an error!
			// no need to break out here, genrand_real2()*lambda was not high enough
			// to warrant a state transition, so we just sail on
					break;
        }

      /* lifetime & update occurrence time */
        t_life=-log(genrand_real2())/lambda;
        t_occ_Na[node]+=t_life;

      }
  }
/*
      printf("   m0h0,m1h0,m2h0,m3h0 : %4d %4d %4d %4d\n",
                 m0h0[node],m1h0[node],m2h0[node],m3h0[node]);
      printf("   m0h1,m1h1,m2h1,m3h1 : %4d %4d %4d %4d\n",
                 m0h1[node],m1h1[node],m2h1[node],m3h1[node]);
*/
  n_Na=jeff1[node];

  return(n_Na);
}
	
/****************************************************************************
						DO NOT USE ABOVE FUNCTION.  BROKEN!
 ****************************************************************************/
