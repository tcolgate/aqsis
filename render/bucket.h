// Aqsis
// Copyright ���� 1997 - 2001, Paul C. Gregory
//
// Contact: pgregory@aqsis.com
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


/** \file
		\brief Declares the CqBucket class responsible for bookeeping the primitives and storing the results.
		\author Paul C. Gregory (pgregory@aqsis.com)
*/

//? Is imagebuffer.h included already?
#ifndef BUCKET_H_INCLUDED 
//{
#define BUCKET_H_INCLUDED 1

#include	"aqsis.h"

#include	<vector>
#include	<queue>
#include	<deque>

#include	"bitvector.h"
#include	"micropolygon.h"
#include	"renderer.h"
#include	"ri.h"
#include	"surface.h"
#include	"color.h"
#include	"vector2d.h"
#include    "imagepixel.h"

START_NAMESPACE( Aqsis )

//-----------------------------------------------------------------------
/** Class holding data about a particular bucket.
 */

class CqBucket : public IqBucket
{
public:
    CqBucket() : m_bProcessed( TqFalse )
    {}
    CqBucket( const CqBucket& From )
    {
        *this = From;
    }
    virtual ~CqBucket()
    {}

    CqBucket& operator=( const CqBucket& From )
    {
        m_ampgWaiting = From.m_ampgWaiting;
        m_agridWaiting = From.m_agridWaiting;
        m_bProcessed = From.m_bProcessed;

        return ( *this );
    }

    // Overridden from IqBucket
    virtual	TqInt	Width() const
    {
        return ( m_XSize );
    }
    virtual	TqInt	Height() const
    {
        return ( m_YSize );
    }
    virtual	TqInt	XOrigin() const
    {
        return ( m_XOrigin );
    }
    virtual	TqInt	YOrigin() const
    {
        return ( m_YOrigin );
    }
    virtual	TqInt	FilterXWidth() const
    {
        return ( m_FilterXWidth );
    }
    virtual	TqInt	FilterYWidth() const
    {
        return ( m_FilterYWidth );
    }
    virtual	TqInt	PixelXSamples() const
    {
        return ( m_PixelXSamples );
    }
    virtual	TqInt	PixelYSamples() const
    {
        return ( m_PixelYSamples );
    }

    virtual	CqColor Color( TqInt iXPos, TqInt iYPos );
    virtual	CqColor Opacity( TqInt iXPos, TqInt iYPos );
    virtual	TqFloat Coverage( TqInt iXPos, TqInt iYPos );
    virtual	TqFloat Depth( TqInt iXPos, TqInt iYPos );
    virtual	TqFloat MaxDepth( TqInt iXPos, TqInt iYPos );
    virtual	TqInt DataSize( TqInt iXPos, TqInt iYPos );
    virtual	const TqFloat* Data( TqInt iXPos, TqInt iYPos );

    static	void	InitialiseBucket( TqInt xorigin, TqInt yorigin, TqInt xsize, TqInt ysize, TqInt xfwidth, TqInt yfwidth, TqInt xsamples, TqInt ysamples, TqBool fJitter = TqTrue );
    static	void	InitialiseFilterValues();
    static	void	ImageElement( TqInt iXPos, TqInt iYPos, CqImagePixel*& pie )
    {
        iXPos -= m_XOrigin;
        iYPos -= m_YOrigin;

        // Check within renderable range
        //assert( iXPos < -m_XMax && iXPos < m_XSize + m_XMax &&
        //		iYPos < -m_YMax && iYPos < m_YSize + m_YMax );

        TqInt i = ( ( iYPos + m_YMax ) * ( m_XSize + m_FilterXWidth ) ) + ( iXPos + m_XMax );
        pie = &m_aieImage[ i ];
    }
    static	void	CombineElements();
    void	FilterBucket(TqBool empty);
    void	ExposeBucket();
    void	QuantizeBucket();
    static	void	ShutdownBucket();

    /** Add a GPRim to the stack of deferred GPrims.
	 * \param The Gprim to be added.
     */
    void	AddGPrim( const boost::shared_ptr<CqBasicSurface>& pGPrim )
	{
		m_aGPrims.push(pGPrim);
	}

    /** Add an MPG to the list of deferred MPGs.
     */
    void	AddMPG( CqMicroPolygon* pmpgNew )
    {
#ifdef _DEBUG
        std::vector<CqMicroPolygon*>::iterator end = m_ampgWaiting.end();
        for (std::vector<CqMicroPolygon*>::iterator i = m_ampgWaiting.begin(); i != end; i++)
            if ((*i) == pmpgNew)
                assert( TqFalse );
#endif
        m_ampgWaiting.push_back( pmpgNew );
    }
    /** Add a Micropoly grid to the list of deferred grids.
     */
    void	AddGrid( CqMicroPolyGridBase* pgridNew )
    {
#ifdef _DEBUG
        std::vector<CqMicroPolyGridBase*>::iterator end = m_agridWaiting.end();
        for (std::vector<CqMicroPolyGridBase*>::iterator i = m_agridWaiting.begin(); i != end; i++)
            if ((*i) == pgridNew)
                assert( TqFalse );
#endif
        m_agridWaiting.push_back( pgridNew );
    }
    /** Get a pointer to the top GPrim in the stack of deferred GPrims.
     */
    boost::shared_ptr<CqBasicSurface> pTopSurface()
    {
	if (!m_aGPrims.empty())
	{
	    return m_aGPrims.top();
	}
	else
	{
	    return boost::shared_ptr<CqBasicSurface>();
	}
    }
    /** Pop the top GPrim in the stack of deferred GPrims.
     */
    void popSurface()
    {
		m_aGPrims.pop();
    }
    /** Get a count of deferred GPrims.
     */
    TqInt cGPrims()
    {
	return ( m_aGPrims.size() );
    }
    /** Get a reference to the vetor of deferred MPGs.
     */
    std::vector<CqMicroPolygon*>& aMPGs()
    {
        return ( m_ampgWaiting );
    }
    /** Get a reference to the vetor of deferred grids.
     */
    std::vector<CqMicroPolyGridBase*>& aGrids()
    {
        return ( m_agridWaiting );
    }
    /** Get the flag that indicates if the bucket has been processed yet.
     */
    TqBool IsProcessed() const
    {
        return( m_bProcessed );
    }

    /** Mark this bucket as processed
     */
    void SetProcessed( TqBool bProc =  TqTrue)
    {
        m_bProcessed = bProc;
    }


private:
    static	TqInt	m_XSize;
    static	TqInt	m_YSize;
    static	TqInt	m_FilterXWidth;
    static	TqInt	m_FilterYWidth;
    static	TqInt	m_XMax;
    static	TqInt	m_YMax;
    static	TqInt	m_PixelXSamples;
    static	TqInt	m_PixelYSamples;
    static	TqInt	m_XOrigin;
    static	TqInt	m_YOrigin;
    static	std::vector<CqImagePixel>	m_aieImage;
    static	std::vector<std::vector<CqVector2D> >	m_aSamplePositions;///< Vector of vectors of jittered sample positions precalculated.
    static	std::vector<TqFloat>	m_aFilterValues;				///< Vector of filter weights precalculated.
    static	std::vector<TqFloat>	m_aDatas;
    static	std::vector<TqFloat>	m_aCoverages;

	// this is a compare functor for sorting surfaces in order of depth.
	struct closest_surface
	{
		bool operator()(const boost::shared_ptr<CqBasicSurface>& s1, const boost::shared_ptr<CqBasicSurface>& s2) const
		{
            if ( s1->fCachedBound() && s2->fCachedBound() )
            {
                return ( s1->GetCachedRasterBound().vecMin().z() > s2->GetCachedRasterBound().vecMin().z() );
            }

			// don't have bounds for the surface(s). I suspect we should assert here.
			return true;
		}
	};

    std::vector<CqMicroPolygon*> m_ampgWaiting;			///< Vector of vectors of waiting micropolygons in this bucket
    std::vector<CqMicroPolyGridBase*> m_agridWaiting;		///< Vector of vectors of waiting micropolygrids in this bucket

	/// A sorted list of primitives for this bucket
	std::priority_queue<boost::shared_ptr<CqBasicSurface>, std::deque<boost::shared_ptr<CqBasicSurface> >, closest_surface> m_aGPrims;
	TqBool	m_bProcessed;	///< Flag indicating if this bucket has been processed yet.
}
;

END_NAMESPACE( Aqsis )

//}  // End of #ifdef BUCKET_H_INCLUDED
#endif
