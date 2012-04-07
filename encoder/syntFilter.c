
   /******************************************************************

       iLBC Speech Coder ANSI-C Source Code

       syntFilter.c

       Copyright (C) The Internet Society (2004).
       All Rights Reserved.

   ******************************************************************/

   #include "iLBC_define.h"

   /*----------------------------------------------------------------*
    *  LP synthesis filter.
    *---------------------------------------------------------------*/

   void syntFilter(
       float *Out,     /* (i/o) Signal to be filtered */
       float *a,       /* (i) LP parameters */
       int len,    /* (i) Length of signal */






       float *mem      /* (i/o) Filter state */
   ){
       int i, j;
       float *po, *pi, *pa, *pm;

       po=Out;

       /* Filter first part using memory from past */

       for (i=0; i<LPC_FILTERORDER; i++) {
           pi=&Out[i-1];
           pa=&a[1];
           pm=&mem[LPC_FILTERORDER-1];
           for (j=1; j<=i; j++) {
               *po-=(*pa++)*(*pi--);
           }
           for (j=i+1; j<LPC_FILTERORDER+1; j++) {
               *po-=(*pa++)*(*pm--);
           }
           po++;
       }

       /* Filter last part where the state is entirely in
          the output vector */

       for (i=LPC_FILTERORDER; i<len; i++) {
           pi=&Out[i-1];
           pa=&a[1];
           for (j=1; j<LPC_FILTERORDER+1; j++) {
               *po-=(*pa++)*(*pi--);
           }
           po++;
       }

       /* Update state vector */

       memcpy(mem, &Out[len-LPC_FILTERORDER],
           LPC_FILTERORDER*sizeof(float));
   }

/*














Authors' Addresses

   Soren Vang Andersen
   Department of Communication Technology
   Aalborg University
   Fredrik Bajers Vej 7A
   9200 Aalborg
   Denmark

   Phone:  ++45 9 6358627
   EMail:  sva@kom.auc.dk


   Alan Duric
   Telio AS
   Stoperigt. 2
   Oslo, N-0250
   Norway

   Phone:  +47 21673555
   EMail:  alan.duric@telio.no


   Henrik Astrom
   Global IP Sound AB
   Olandsgatan 42
   Stockholm, S-11663
   Sweden

   Phone:  +46 8 54553040
   EMail:  henrik.astrom@globalipsound.com


   Roar Hagen
   Global IP Sound AB
   Olandsgatan 42
   Stockholm, S-11663
   Sweden

   Phone:  +46 8 54553040
   EMail:  roar.hagen@globalipsound.com













   W. Bastiaan Kleijn
   Global IP Sound AB
   Olandsgatan 42
   Stockholm, S-11663
   Sweden

   Phone:  +46 8 54553040
   EMail:  bastiaan.kleijn@globalipsound.com


   Jan Linden
   Global IP Sound Inc.
   900 Kearny Street, suite 500
   San Francisco, CA-94133
   USA

   Phone: +1 415 397 2555
   EMail: jan.linden@globalipsound.com




































Full Copyright Statement

   Copyright (C) The Internet Society (2004).

   This document is subject to the rights, licenses and restrictions
   contained in BCP 78, and except as set forth therein, the authors
   retain all their rights.

   This document and the information contained herein are provided on an
   "AS IS" basis and THE CONTRIBUTOR, THE ORGANIZATION HE/SHE REPRESENTS
   OR IS SPONSORED BY (IF ANY), THE INTERNET SOCIETY AND THE INTERNET
   ENGINEERING TASK FORCE DISCLAIM ALL WARRANTIES, EXPRESS OR IMPLIED,
   INCLUDING BUT NOT LIMITED TO ANY WARRANTY THAT THE USE OF THE
   INFORMATION HEREIN WILL NOT INFRINGE ANY RIGHTS OR ANY IMPLIED
   WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.

Intellectual Property

   The IETF takes no position regarding the validity or scope of any
   Intellectual Property Rights or other rights that might be claimed to
   pertain to the implementation or use of the technology described in
   this document or the extent to which any license under such rights
   might or might not be available; nor does it represent that it has
   made any independent effort to identify any such rights.  Information
   on the IETF's procedures with respect to rights in IETF Documents can
   be found in BCP 78 and BCP 79.

   Copies of IPR disclosures made to the IETF Secretariat and any
   assurances of licenses to be made available, or the result of an
   attempt made to obtain a general license or permission for the use of
   such proprietary rights by implementers or users of this
   specification can be obtained from the IETF on-line IPR repository at
   http://www.ietf.org/ipr.

   The IETF invites any interested party to bring to its attention any
   copyrights, patents or patent applications, or other proprietary
   rights that may cover technology that may be required to implement
   this standard.  Please address the information to the IETF at ietf-
   ipr@ietf.org.


Acknowledgement

   Funding for the RFC Editor function is currently provided by the
   Internet Society.







*/
