/****************************************************************************
** @file GeniusCarousel.h
**
** @brief
**   Genius Carousel generation.
**
**
** @ingroup GENIUS CAROUSEL
**
** @version $Rev: 61905 $
**          $URL: http://ren-svn-01/svn/products_pc/windows/genius/core/trunk/sources/lib/GeniusCarousel/GeniusCarousel.h $
**          $Date: 2011-06-09 18:52:08 +0200 (jeu., 09 juin 2011) $
**
** @author  SmarDTV Rennes - LIPPA
**
** COPYRIGHT:
**   2011 SmarDTV
**
** @history
**  - COF   - Iwedia  - v 0    - 05/2005 - Creation
**  - LIPPA - SmarDTV - v 2.00 - 05/2011 - Stream Generation NASC mode
**
******************************************************************************/

#ifndef _GENIUS_CAROUSEL_H_
#define _GENIUS_CAROUSEL_H_

class cGeniusImagesData;
class dmsDataCarousel;

class cGeniusCarousel
{
public:
   cGeniusCarousel(dmsDataCarousel* carousel);
   virtual ~cGeniusCarousel();

   bool Compile  ();
   void Generate ();
   bool Load     (wxXmlNode *node);

   dmsDataCarousel   *m_poDataCarousel;
   cGeniusImagesData *m_poImage; /* Single image*/
};



#endif /* _GENIUS_CAROUSEL_H_ */

/* GeniusCarousel.h */
