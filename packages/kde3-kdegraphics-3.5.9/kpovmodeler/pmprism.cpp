/*
**************************************************************************
                                 description
                             --------------------
    copyright            : (C) 2002 by Andreas Zehender
    email                : zehender@kde.org
**************************************************************************

**************************************************************************
*                                                                        *
*  This program is free software; you can redistribute it and/or modify  *
*  it under the terms of the GNU General Public License as published by  *
*  the Free Software Foundation; either version 2 of the License, or     *
*  (at your option) any later version.                                   *
*                                                                        *
**************************************************************************/


#include "pmprism.h"

#include "pmxmlhelper.h"
#include "pmprismedit.h"
#include "pmmemento.h"
#include "pmviewstructure.h"
#include "pm2dcontrolpoint.h"
#include "pmdistancecontrolpoint.h"
#include "pmprismmemento.h"
#include "pmsplinesegment.h"
#include "pmdefaults.h"
#include "pmenumproperty.h"
#include "pmobjectaction.h"

#include <klocale.h>

const int defaultNumberOfPoints = 6;
const PMVector defaultPoint[defaultNumberOfPoints] =
{
   PMVector(  0.5,  1.0 ),
   PMVector(  1.0,  0.0 ),
   PMVector(  0.5, -1.0 ),
   PMVector( -0.5, -1.0 ),
   PMVector( -1.0,  0.0 ),
   PMVector( -0.5,  1.0 ),
};

const bool defaultSturm = false;
const bool defaultOpen = false;
const PMPrism::SplineType defaultSplineType = PMPrism::LinearSpline;
const PMPrism::SweepType defaultSweepType = PMPrism::LinearSweep;
const double defaultHeight1 = 0.0;
const double defaultHeight2 = 1.0;

int PMPrism::s_sSteps = c_defaultPrismSSteps;
int PMPrism::s_parameterKey = 0;
PMMetaObject* PMPrism::s_pMetaObject = 0;
PMObject* createNewPrism( PMPart* part )
{
   return new PMPrism( part );
}

PMDefinePropertyClass( PMPrism, PMPrismProperty );
PMDefineEnumPropertyClass( PMPrism, PMPrism::SplineType, PMSplineTypeProperty );
PMDefineEnumPropertyClass( PMPrism, PMPrism::SweepType, PMSweepTypeProperty );

class PMPointProperty : public PMPropertyBase
{
public:
   PMPointProperty( )
         : PMPropertyBase( "splinePoints", PMVariant::Vector )
   {
      m_index[0] = 0;
      m_index[1] = 0;
   }
   virtual int dimensions( ) const { return 2; }
   virtual void setIndex( int dimension, int index )
   {
      if( dimension == 0 || dimension == 1 )
         m_index[dimension] = index;
   }
   virtual int size( PMObject* object, int dimension ) const
   {
      PMPrism* prism = ( PMPrism* ) object;
      QValueList< QValueList<PMVector> > points = prism->points( );
      if( dimension == 0 )
         return points.size( );
      else
      {
         QValueList< QValueList<PMVector> >::ConstIterator it
            = points.at( m_index[0] );
         if( it != points.end( ) )
            return ( *it ).size( );
      }
      return 0;
   }
protected:
   virtual bool setProtected( PMObject* obj, const PMVariant& var )
   {
      PMPrism* p = ( PMPrism* ) obj;
      QValueList< QValueList<PMVector> > list = p->points( );
      QValueList< QValueList<PMVector> >::Iterator sit = list.begin( );
      int i;
      PMVector v = var.vectorData( );
      v.resize( 2 );

      for( i = 0; i < m_index[0] && sit != list.end( ); ++i )
         ++sit;
      // expand the list if necessary
      for( ; i < m_index[0]; ++i )
         list.insert( sit, QValueList< PMVector >( ) );
      if( sit == list.end( ) )
         sit = list.insert( sit, QValueList< PMVector >( ) );

      QValueList<PMVector>::Iterator it = ( *sit ).begin( );

      for( i = 0; i < m_index[1] && it != ( *sit ).end( ); ++i )
         ++it;
      // expand the list if necessary
      for( ; i < m_index[1]; ++i )
         ( *sit ).insert( it, v );
      if( it == ( *sit ).end( ) )
         it = ( *sit ).insert( it, v );
      else
         *it = v;

      p->setPoints( list );
      return true;
   }
   virtual PMVariant getProtected( const PMObject* obj )
   {
      PMPrism* p = ( PMPrism* ) obj;
      QValueList< QValueList<PMVector> > list = p->points( );
      QValueList< QValueList<PMVector> >::ConstIterator sit = list.at( m_index[0] );
      if( sit == list.end( ) )
      {
         kdError( PMArea ) << "Range error in PMPrism::PointProperty::get" << endl;
         return PMVariant( );
      }

      QValueList<PMVector>::ConstIterator it = ( *sit ).at( m_index[1] );

      if( it == ( *sit ).end( ) )
      {
         kdError( PMArea ) << "Range error in PMPrism::PointProperty::get" << endl;
         return PMVariant( );
      }

      return PMVariant( *it );
   }

private:
   int m_index[2];
};

PMPrism::PMPrism( PMPart* part )
      : Base( part )
{
   int i;
   QValueList<PMVector> p;

   for( i = 0; i < defaultNumberOfPoints; ++i )
      p.append( defaultPoint[i] );
   m_points.append( p );
   m_splineType = defaultSplineType;
   m_sweepType = defaultSweepType;
   m_sturm = defaultSturm;
   m_open = defaultOpen;
   m_height1 = defaultHeight1;
   m_height2 = defaultHeight2;
}

PMPrism::PMPrism( const PMPrism& p )
      : Base( p )
{
   m_splineType = p.m_splineType;
   m_sweepType = p.m_sweepType;
   m_points = p.m_points;
   m_height1 = p.m_height1;
   m_height2 = p.m_height2;
   m_open = p.m_open;
   m_sturm = p.m_sturm;
}

PMPrism::~PMPrism( )
{
}

QString PMPrism::description( ) const
{
   return i18n( "prism" );
}

void PMPrism::serialize( QDomElement& e, QDomDocument& doc ) const
{
   QDomElement data = doc.createElement( "extra_data" );
   QDomElement p, p2;

   e.setAttribute( "spline_type", m_splineType );
   e.setAttribute( "sweep_type", m_sweepType );
   e.setAttribute( "sturm", m_sturm );
   e.setAttribute( "open", m_open );
   e.setAttribute( "height1", m_height1 );
   e.setAttribute( "height2", m_height2 );

   QValueList< QValueList<PMVector> >::ConstIterator it;
   QValueList<PMVector>::ConstIterator it2;
   for( it = m_points.begin( ); it != m_points.end( ); ++it )
   {
      p = doc.createElement( "sub_prism" );
      for( it2 = ( *it ).begin( ); it2 != ( *it ).end( ); ++it2 )
      {
         p2 = doc.createElement( "point" );
         p2.setAttribute( "vector", ( *it2 ).serializeXML( ) );
         p.appendChild( p2 );
      }
      data.appendChild( p );
   }

   e.appendChild( data );
   Base::serialize( e, doc );
}

void PMPrism::readAttributes( const PMXMLHelper& h )
{
   m_splineType = ( SplineType ) h.intAttribute( "spline_type", defaultSplineType );
   m_sweepType = ( SweepType ) h.intAttribute( "sweep_type", defaultSweepType );
   m_open = h.boolAttribute( "open", defaultOpen );
   m_sturm = h.boolAttribute( "sturm", defaultSturm );
   m_height1 = h.doubleAttribute( "height1", defaultHeight1 );
   m_height2 = h.doubleAttribute( "height2", defaultHeight2 );

   m_points.clear( );
   QValueList<PMVector> list;
   PMVector v( 2 );

   QDomElement e = h.extraData( );
   if( !e.isNull( ) )
   {
      QDomNode sp = e.firstChild( );
      while( !sp.isNull( ) )
      {
         if( sp.isElement( ) )
         {
            QDomElement spe = sp.toElement( );
            if( spe.tagName( ) == "sub_prism" )
            {
               list.clear( );
               QDomNode c = spe.firstChild( );
               while( !c.isNull( ) )
               {
                  if( c.isElement( ) )
                  {
                     QDomElement ce = c.toElement( );
                     if( ce.tagName( ) == "point" )
                     {
                        QString str = ce.attribute( "vector" );
                        if( !str.isNull( ) )
                        {
                           v.loadXML( str );
                           list.append( v );
                        }
                     }
                  }
                  c = c.nextSibling( );
               }
               m_points.append( list );
            }
         }
         sp = sp.nextSibling( );
      }
   }

   Base::readAttributes( h );
}

PMMetaObject* PMPrism::metaObject( ) const
{
   if( !s_pMetaObject )
   {
      s_pMetaObject = new PMMetaObject( "Prism", Base::metaObject( ),
                                        createNewPrism );
      s_pMetaObject->addProperty(
         new PMPrismProperty( "sturm", &PMPrism::setSturm, &PMPrism::sturm ) );
      s_pMetaObject->addProperty(
         new PMPrismProperty( "open", &PMPrism::setOpen, &PMPrism::open ) );
      s_pMetaObject->addProperty(
         new PMPrismProperty( "height1", &PMPrism::setHeight1, &PMPrism::height1 ) );
      s_pMetaObject->addProperty(
         new PMPrismProperty( "height2", &PMPrism::setHeight2, &PMPrism::height2 ) );

      PMSplineTypeProperty* p = new PMSplineTypeProperty(
         "splineType", &PMPrism::setSplineType, &PMPrism::splineType );
      p->addEnumValue( "LinearSpline", LinearSpline );
      p->addEnumValue( "QuadraticSpline", QuadraticSpline );
      p->addEnumValue( "CubicSpline", CubicSpline );
      p->addEnumValue( "BezierSpline", BezierSpline );
      s_pMetaObject->addProperty( p );

      PMSweepTypeProperty* sp = new PMSweepTypeProperty(
         "sweepType", &PMPrism::setSweepType, &PMPrism::sweepType );
      sp->addEnumValue( "LinearSweep", LinearSweep );
      sp->addEnumValue( "ConicSweep", ConicSweep );
      s_pMetaObject->addProperty( sp );

      s_pMetaObject->addProperty( new PMPointProperty( ) );
   }
   return s_pMetaObject;
}

void PMPrism::cleanUp( ) const
{
   if( s_pMetaObject )
   {
      delete s_pMetaObject;
      s_pMetaObject = 0;
   }
   Base::cleanUp( );
}

void PMPrism::setSplineType( PMPrism::SplineType t )
{
   if( m_splineType != t )
   {
      if( m_pMemento )
         m_pMemento->addData( s_pMetaObject, PMSplineTypeID, ( int ) m_splineType );
      setViewStructureChanged( );
      m_splineType = t;
   }
}

void PMPrism::setSweepType( PMPrism::SweepType t )
{
   if( m_sweepType != t )
   {
      if( m_pMemento )
         m_pMemento->addData( s_pMetaObject, PMSweepTypeID, ( int ) m_sweepType );
      setViewStructureChanged( );
      m_sweepType = t;
   }
}

void PMPrism::setSturm( bool s )
{
   if( m_sturm != s )
   {
      if( m_pMemento )
         m_pMemento->addData( s_pMetaObject, PMSturmID, m_sturm );
      m_sturm = s;
   }
}

void PMPrism::setOpen( bool o )
{
   if( m_open != o )
   {
      if( m_pMemento )
         m_pMemento->addData( s_pMetaObject, PMOpenID, m_open );
      m_open = o;
   }
}

void PMPrism::setHeight1( double h )
{
   if( m_height1 != h )
   {
      if( m_pMemento )
         m_pMemento->addData( s_pMetaObject, PMHeight1ID, m_height1 );
      m_height1 = h;
      setViewStructureChanged( );
   }
}

void PMPrism::setHeight2( double h )
{
   if( m_height2 != h )
   {
      if( m_pMemento )
         m_pMemento->addData( s_pMetaObject, PMHeight2ID, m_height2 );
      m_height2 = h;
      setViewStructureChanged( );
   }
}

void PMPrism::setPoints( const QValueList< QValueList<PMVector> >& points )
{
   if( m_points != points )
   {
      if( m_pMemento )
         ( ( PMPrismMemento* ) m_pMemento )->setPrismPoints( m_points );

      setViewStructureChanged( );
      m_points = points;
   }
}

PMDialogEditBase* PMPrism::editWidget( QWidget* parent ) const
{
   return new PMPrismEdit( parent );
}

void PMPrism::createMemento( )
{
   if( m_pMemento )
      delete m_pMemento;
   m_pMemento = new PMPrismMemento( this );
}

void PMPrism::restoreMemento( PMMemento* s )
{
   PMPrismMemento* m = ( PMPrismMemento* ) s;
   PMMementoDataIterator it( s );
   PMMementoData* data;

   for( ; it.current( ); ++it )
   {
      data = it.current( );
      if( data->objectType( ) == s_pMetaObject )
      {
         switch( data->valueID( ) )
         {
            case PMSplineTypeID:
               setSplineType( ( SplineType ) data->intData( ) );
               break;
            case PMSweepTypeID:
               setSweepType( ( SweepType ) data->intData( ) );
               break;
            case PMOpenID:
               setOpen( data->boolData( ) );
               break;
            case PMSturmID:
               setSturm( data->boolData( ) );
               break;
            case PMHeight1ID:
               setHeight1( data->doubleData( ) );
               break;
            case PMHeight2ID:
               setHeight2( data->doubleData( ) );
               break;
            default:
               kdError( PMArea ) << "Wrong ID in PMPrism::restoreMemento\n";
               break;
         }
      }
   }
   if( m->prismPointsSaved( ) )
      setPoints( m->prismPoints( ) );

   Base::restoreMemento( s );
}


void PMPrism::createViewStructure( )
{
   if( s_sSteps == 0 )
      s_sSteps = c_defaultPrismSSteps;

   int sSteps = (int)( ( (float)s_sSteps / 2 ) * ( displayDetail( ) + 1 ) );

   // calculate number of points and lines of the view structure
   QValueList< QValueList<PMVector> >::ConstIterator spit = m_points.begin( );
   int np = 0;
   for( ; spit != m_points.end( ); ++spit )
   {
      int snp = ( *spit ).count( );
      switch( m_splineType )
      {
         case LinearSpline:
            break;
         case QuadraticSpline:
            snp -= 1;
            break;
         case CubicSpline:
            snp -= 2;
            break;
         case BezierSpline:
            snp /= 3;
            break;
      }
      np += snp;
   }

   int nl = 0;
   nl = np * sSteps * 3;
   np *= sSteps * 2;

   if( m_pViewStructure )
   {
      if( m_pViewStructure->points( ).size( ) != ( unsigned ) np )
         m_pViewStructure->points( ).resize( np );
      if( m_pViewStructure->lines( ).size( ) != ( unsigned ) nl )
         m_pViewStructure->lines( ).resize( nl );
   }
   else
      m_pViewStructure = new PMViewStructure( np, nl );

   PMLineArray& lines = m_pViewStructure->lines( );
   PMPointArray& points = m_pViewStructure->points( );
   int lb = 0;
   int pb = 0;

   for( spit = m_points.begin( ); spit != m_points.end( ); ++spit )
   {
      QValueList<PMSplineSegment> segments;
      QValueList<PMVector> fullPoints = expandedPoints( *spit );

      int ns = fullPoints.count( );
      int i, j;

      switch( m_splineType )
      {
         case LinearSpline:
            ns -= 1;
            break;
         case QuadraticSpline:
            ns -= 2;
            break;
         case CubicSpline:
            ns -= 3;
            break;
         case BezierSpline:
            ns = ns / 4;
            break;
      }
      QValueList<PMVector>::Iterator it1, it2, it3, it4;

      // create the spline segments
      it1 = fullPoints.begin( );
      it2 = it1; ++it2;
      it3 = it2; ++it3;
      it4 = it3; ++it4;
      PMSplineSegment s;

      for( i = 0; i < ns; ++i )
      {
         switch( m_splineType )
         {
            case LinearSpline:
               s.calculateLinear( *it1, *it2 );
               ++it1;
               ++it2;
               break;
            case QuadraticSpline:
               s.calculateQuadratic( *it1, *it2, *it3 );
               ++it1;
               ++it2;
               ++it3;
               break;
            case CubicSpline:
               s.calculateCubic( *it1, *it2, *it3, *it4 );
               ++it1;
               ++it2;
               ++it3;
               ++it4;
               break;
            case BezierSpline:
               s.calculateBezier( *it1, *it2, *it3, *it4 );
               for( j = 0; j < 4; ++j )
               {
                  ++it1;
                  ++it2;
                  ++it3;
                  ++it4;
               }
               break;
         }
         segments.append( s );
      }

      // create the line array
      int vp = ns * sSteps;
      for( i = 0; i < vp - 1; ++i )
         lines[lb+i] = PMLine( pb + i, pb + i + 1 );
      lines[lb+vp-1] = PMLine( pb, pb + vp - 1 );
      lb += vp;
      for( i = 0; i < vp - 1; ++i )
         lines[lb+i] = PMLine( pb + vp + i, pb + vp + i + 1 );
      lines[lb+vp-1] = PMLine( pb + vp, pb + vp + vp - 1 );
      lb += vp;
      for( i = 0; i < vp; ++i )
         lines[lb+i] = PMLine( pb + i, pb + vp + i );
      lb += vp;

      // calculate the points
      PMVector point2( 2 ), point3;
      QValueList<PMSplineSegment>::Iterator sit = segments.begin( );
      int pi = 0;
      double poffset = 1.0 / sSteps;

      for( i = 0; i < ns; ++i, ++sit )
      {
         for( j = 0; j < sSteps; ++j )
         {
            point2 = ( *sit ).point( poffset * j );
            if( m_sweepType == LinearSweep )
            {
               point3[0] = point2[0];
               point3[1] = m_height1;
               point3[2] = point2[1];
               points[pb+pi] = PMPoint( point3 );
               point3[1] = m_height2;
               points[pb+pi+vp] = PMPoint( point3 );
            }
            else
            {
               point3[0] = point2[0];
               point3[1] = 1.0;
               point3[2] = point2[1];
               points[pb+pi] = PMPoint( point3 * m_height1 );
               points[pb+pi+vp] = PMPoint( point3 * m_height2 );
            }
            ++pi;
         }
      }
      pb += vp * 2;
   }
}

void PMPrism::controlPoints( PMControlPointList& list )
{
   QValueList< QValueList<PMVector> >::Iterator it1;
   QValueList<PMVector>::Iterator it2;
   int i1, i2;

   list.append( new PMDistanceControlPoint( PMVector( 0.0, 0.0, 0.0 ),
                                            PMVector( 0.0, 1.0, 0.0 ),
                                            m_height1, PMHeight1ID,
                                            i18n( "Height 1" ) ) );
   list.append( new PMDistanceControlPoint( PMVector( 0.0, 0.0, 0.0 ),
                                            PMVector( 0.0, 1.0, 0.0 ),
                                            m_height2, PMHeight2ID,
                                            i18n( "Height 2" ) ) );

   PM2DControlPoint* cp;

   for( it1 = m_points.begin( ), i1 = 0; it1 != m_points.end( ); ++it1, ++i1 )
   {
      if( m_splineType != BezierSpline )
      {
         int refb = ( *it1 ).count( ) - 1;
         if( m_splineType == CubicSpline )
            --refb;
         it2 = ( *it1 ).begin( );
         PM2DControlPoint* firstPoint = 0;
         PM2DControlPoint* secondPoint = 0;

         for( i2 = 0; it2 != ( *it1 ).end( ); ++it2, ++i2 )
         {
            cp = new PM2DControlPoint( *it2, PM2DControlPoint::PM2DXZ, i2,
                                       i18n( "Point %1.%2" ).arg( i1 + 1 ).arg( i2 + 1 ) );
            if( i2 == 0 )
               firstPoint = cp;
            else if( i2 == 1 )
               secondPoint = cp;

            cp->setThirdCoordinate( m_height2 );
            if( m_sweepType == ConicSweep )
               cp->setScale( m_height2 );
            if( ( ( m_splineType == QuadraticSpline )
                  || ( m_splineType == CubicSpline ) )
                && ( i2 == 1 ) )
               firstPoint->setBasePoint( cp );
            if( ( m_splineType == CubicSpline ) && ( i2 == ( refb + 2 ) ) )
               cp->setBasePoint( secondPoint );

            list.append( cp );

            if( ( m_splineType != BezierSpline ) && ( i2 == refb ) )
               ++i2;
         }
      }
      else
      {
         it2 = ( *it1 ).begin( );
         PM2DControlPoint* firstPoint = 0;
         PM2DControlPoint* lastPoint = 0;
         PM2DControlPoint* startPoint = 0;

         for( i2 = 0; it2 != ( *it1 ).end( ); ++it2, ++i2 )
         {
            int i2mod4 = i2 % 4;
            cp = new PM2DControlPoint( *it2, PM2DControlPoint::PM2DXZ, i2,
                                       i18n( "Point %1.%2" ).arg( i1 + 1 ).arg( i2 + 1 ) );
            if( i2mod4 == 0 )
               firstPoint = cp;
            if( i2mod4 == 2 )
               lastPoint = cp;
            if( !startPoint )
               startPoint = cp;

            cp->setThirdCoordinate( m_height2 );
            if( m_sweepType == ConicSweep )
               cp->setScale( m_height2 );
            if( i2mod4 == 1 )
               cp->setBasePoint( firstPoint );
            if( ( i2mod4 == 0 ) && lastPoint )
               lastPoint->setBasePoint( cp );

            list.append( cp );

            if( i2mod4 == 2 )
               ++i2;
         }
         if( lastPoint )
            lastPoint->setBasePoint( startPoint );
      }
   }
}

void PMPrism::controlPointsChanged( PMControlPointList& list )
{
   PMControlPointListIterator it( list );
   QValueList< QValueList<PMVector> >::Iterator spit = m_points.begin( );
   QValueList<PMVector>::Iterator pit = ( *spit ).begin( );
   PM2DControlPoint* p1;
   PMDistanceControlPoint* dcp;
   bool firstChange = true;
   bool h2changed = false;

   // IDs are ignored, quick hack, but should work
   if( it.current( )->changed( ) )
   {
      dcp = ( PMDistanceControlPoint* ) it.current( );
      setHeight1( dcp->distance( ) );
   }
   ++it;
   if( it.current( )->changed( ) )
   {
      dcp = ( PMDistanceControlPoint* ) it.current( );
      setHeight2( dcp->distance( ) );
      h2changed = true;
   }
   ++it;

   for( ; it.current( ); ++it )
   {
      p1 = ( PM2DControlPoint* ) it.current( );
      if( p1->changed( ) )
      {
         if( firstChange )
         {
            if( m_pMemento )
            {
               PMPrismMemento* m = ( PMPrismMemento* ) m_pMemento;
               if( !m->prismPointsSaved( ) )
                  m->setPrismPoints( m_points );
            }
            firstChange = false;
            setViewStructureChanged( );
         }
         ( *pit ) = p1->point( );
      }
      if( h2changed )
      {
         p1->setThirdCoordinate( m_height2 );
         if( m_sweepType == ConicSweep )
            p1->setScale( m_height2 );
      }

      ++pit;
      if( pit == ( *spit ).end( ) )
      {
         ++spit;
         pit = ( *spit ).begin( );
      }
   }
}

void PMPrism::addObjectActions( const PMControlPointList& /*cp*/,
                                QPtrList<PMObjectAction>& actions )
{
   PMObjectAction* a;

   a = new PMObjectAction( s_pMetaObject, PMSplitSegmentID,
                           i18n( "Add Point" ) );
   actions.append( a );

   a = new PMObjectAction( s_pMetaObject, PMJoinSegmentsID,
                           i18n( "Remove Point" ) );

   bool enableJoin = false;
   QValueList< QValueList<PMVector> >::ConstIterator spit = m_points.begin( );

   int minp = 4;
   switch( m_splineType )
   {
      case LinearSpline:
         minp = 4;
         break;
      case QuadraticSpline:
         minp = 5;
         break;
      case CubicSpline:
         minp = 6;
         break;
      case BezierSpline:
         minp = 6;
         break;
   }

   for( ; ( spit != m_points.end( ) ) && !enableJoin; ++spit )
      if( ( *spit ).count( ) >= ( unsigned ) minp )
         enableJoin = true;

   a->setEnabled( enableJoin );
   actions.append( a );
}

void PMPrism::objectActionCalled( const PMObjectAction* action,
                                  const PMControlPointList& cp,
                                  const QPtrList<PMVector>& cpViewPosition,
                                  const PMVector& clickPosition )
{
   if( action->objectType( ) == s_pMetaObject )
   {
      switch( action->actionID( ) )
      {
         case PMSplitSegmentID:
            splitSegment( cp, cpViewPosition, clickPosition );
            break;
         case PMJoinSegmentsID:
            joinSegments( cp, cpViewPosition, clickPosition );
            break;
         default:
            kdError( PMArea ) << "Wrong ID in PMPrism::objectActionCalled\n";
            break;
      }
   }
   else
      Base::objectActionCalled( action, cp, cpViewPosition, clickPosition );
}

void PMPrism::splitSegment( const PMControlPointList& /*cp*/,
                            const QPtrList<PMVector>& cpViewPosition,
                            const PMVector& clickPosition )
{
   // find nearest segment
   double abs = 0.0, minabs = 1e10;
   int ns = -1;
   int nsp = 0;
   int spnr = 0, pnr = 0;
   int i;
   PMVector mid( 3 ), dist( 2 );
   PMVector firstPoint( 3 );

   QPtrListIterator<PMVector> it1( cpViewPosition );
   QPtrListIterator<PMVector> it2( cpViewPosition );
   for( i = 0; i < 2; ++i ) ++it1;
   for( i = 0; i < 3; ++i ) ++it2;

   QValueList< QValueList<PMVector> >::Iterator spit = m_points.begin( );
   for( spnr = 0; spit != m_points.end( ); ++spit, ++spnr )
   {
      int nump = ( *spit ).count( );
      bool first = true;
      for( pnr = 0; pnr < nump; ++pnr )
      {
         bool skip = false;
         switch( m_splineType )
         {
            case LinearSpline:
            case BezierSpline:
               break;
            case QuadraticSpline:
               if( pnr == 0 )
                  skip = true;
               break;
            case CubicSpline:
               if( ( pnr == 0 ) || ( pnr == ( nump - 1 ) ) )
                  skip = true;
               break;
         }

         if( !skip )
         {
            if( first )
            {
               firstPoint = **it1;
               first = false;
            }

            if( ( ( m_splineType == CubicSpline ) && ( pnr == ( nump - 2 ) ) )
                || ( ( m_splineType != CubicSpline ) && ( pnr == ( nump - 1 ) ) ) )
               mid = ( **it1 + firstPoint ) / 2.0;
            else
               mid = ( **it1 + **it2 ) / 2.0;

            dist[0] = mid[0];
            dist[1] = mid[1];
            dist -= clickPosition;
            abs = dist.abs( );

            if( ( minabs > abs ) || ( ns < 0 ) )
            {
               minabs = abs;
               ns = pnr;
               nsp = spnr;
            }
         }
         ++it1;
         ++it2;
      }
   }

   // add a new segment
   QValueList< QValueList<PMVector> > newPoints = m_points;
   spit = newPoints.at( nsp );
   QValueList<PMVector> newSubPoints = *spit;

   if( m_splineType == BezierSpline )
   {
      ns /= 3;
      ns *= 3;
   }
   QValueList<PMVector>::Iterator it = newSubPoints.at( ( unsigned ) ns );
   PMVector p[4];
   QValueList<PMVector>::Iterator hit = it, eit = newSubPoints.end( );
   --eit;

   // calculate the spline segment
   PMSplineSegment segment;
   switch( m_splineType )
   {
      case LinearSpline:
         for( i = 0; i < 2; ++i )
         {
            p[i] = *hit;
            ++hit;
            if( hit == newSubPoints.end( ) )
               hit = newSubPoints.begin( );
         }
         segment.calculateLinear( p[0], p[1] );
         break;
      case QuadraticSpline:
         --hit;
         for( i = 0; i < 3; ++i )
         {
            p[i] = *hit;
            ++hit;
            if( hit == newSubPoints.end( ) )
            {
               hit = newSubPoints.begin( );
               ++hit;
            }
         }
         segment.calculateQuadratic( p[0], p[1], p[2] );
         break;
      case CubicSpline:
         --hit;
         for( i = 0; i < 4; ++i )
         {
            if( hit == eit )
            {
               hit = newSubPoints.begin( );
               ++hit;
               p[i] = *hit;
               hit = eit;
               ++i;
               if( i < 4 )
                  p[i] = *hit;
            }
            else
               p[i] = *hit;
            ++hit;
         }
         segment.calculateCubic( p[0], p[1], p[2], p[3] );
         break;
      case BezierSpline:
         for( i = 0; i < 4; ++i )
         {
            p[i] = *hit;
            ++hit;
            if( hit == newSubPoints.end( ) )
               hit = newSubPoints.begin( );
         }
         segment.calculateBezier( p[0], p[1], p[2], p[3] );
         break;
   }

   mid = segment.point( 0.5 );
   if( m_splineType != BezierSpline )
   {
      ++it;
      newSubPoints.insert( it, mid );
   }
   else
   {
      PMVector end = *it;
      ++it;
      *it = end + ( *it - end ) / 2.0;
      ++it;

      PMVector grad = segment.gradient( 0.5 ) / 4.0;

      newSubPoints.insert( it, mid - grad );
      newSubPoints.insert( it, mid );
      newSubPoints.insert( it, mid + grad );

      ++it;
      if( it == newSubPoints.end( ) )
         end = *newSubPoints.begin( );
      else
         end = *it;
      --it;
      *it = end + ( *it - end ) / 2.0;
   }
   ( *spit ) = newSubPoints;
   setPoints( newPoints );
}

void PMPrism::joinSegments( const PMControlPointList& /*cp*/,
                            const QPtrList<PMVector>& cpViewPosition,
                            const PMVector& clickPosition )
{
   // find nearest point
   double abs = 0.0, minabs = 1e10;
   int ns = -1;
   int nsp = 0;
   int spnr = 0, pnr = 0;
   int i;
   PMVector dist( 2 );

   QPtrListIterator<PMVector> it1( cpViewPosition );
   for( i = 0; i < 2; ++i ) ++it1;

   int minp = 0;
   switch( m_splineType )
   {
      case LinearSpline:
         minp = 4;
         break;
      case QuadraticSpline:
         minp = 5;
         break;
      case CubicSpline:
         minp = 6;
         break;
      case BezierSpline:
         minp = 6;
         break;
   }

   QValueList< QValueList<PMVector> >::Iterator spit = m_points.begin( );
   for( spnr = 0; spit != m_points.end( ); ++spit, ++spnr )
   {
      int nump = ( *spit ).count( );

      for( pnr = 0; pnr < nump; ++pnr )
      {
         bool skip = false;
         switch( m_splineType )
         {
            case LinearSpline:
            case BezierSpline:
               break;
            case QuadraticSpline:
               if( pnr == 0 )
                  skip = true;
               break;
            case CubicSpline:
               if( ( pnr == 0 ) || ( pnr == ( nump - 1 ) ) )
                  skip = true;
               break;
         }
         if( nump < minp )
            skip = true;

         if( !skip )
         {
            dist[0] = (**it1)[0];
            dist[1] = (**it1)[1];
            dist -= clickPosition;
            abs = dist.abs( );

            if( ( minabs > abs ) || ( ns < 0 ) )
            {
               minabs = abs;
               ns = pnr;
               nsp = spnr;
            }
         }
         ++it1;
      }
   }

   if( ns < 0 )
   {
      kdError( PMArea ) << "Not enough points in PMPrism::joinSegments\n";
      return;
   }

   // remove the segment
   QValueList< QValueList<PMVector> > newPoints = m_points;
   spit = newPoints.at( nsp );
   QValueList<PMVector> newSubPoints = *spit;
   QValueList<PMVector>::Iterator it;

   if( m_splineType != BezierSpline )
   {
      it = newSubPoints.at( ( unsigned ) ns );
      newSubPoints.remove( it );
   }
   else
   {
      int last = ( newSubPoints.count( ) - 3 ) / 3;
      ns -= 2;
      if( ns < 0 )
         ns = last;
      else
         ns /= 3;

      it = newSubPoints.at( ns * 3 + 2 );
      if( ns != last )
      {
         it = newSubPoints.remove( it );
         it = newSubPoints.remove( it );
         it = newSubPoints.remove( it );
      }
      else
      {
         newSubPoints.remove( it );
         it = newSubPoints.begin( );
         it = newSubPoints.remove( it );
         it = newSubPoints.remove( it );
         PMVector h = *it;
         it = newSubPoints.remove( it );
         newSubPoints.insert( newSubPoints.end( ), h );
      }
   }
   ( *spit ) = newSubPoints;
   setPoints( newPoints );
}

void PMPrism::setSSteps( int s )
{
   if( s >= 1 )
      s_sSteps = s;
   else
      kdDebug( PMArea ) << "PMPrism::setSSteps: S must be greater than 0\n";
   ++s_parameterKey;
}

QValueList<PMVector> PMPrism::expandedPoints( const QValueList<PMVector>& p ) const
{
   // add the missing points
   int refa = 0, refb = p.count( );
   QValueList<PMVector> result = p;

   switch( m_splineType )
   {
      case LinearSpline:
         break;
      case QuadraticSpline:
         ++refa;
         break;
      case CubicSpline:
         ++refa;
         --refb;
         break;
      case BezierSpline:
         refb = refb / 3 * 4;
         break;
   }
   QValueList<PMVector>::Iterator it1, it2, it3;
   if( m_splineType != BezierSpline )
   {
      it1 = result.at( refa );
      it2 = result.at( refb );
      result.insert( it2, *it1 );
   }
   else
   {
      int i;
      it1 = result.begin( );
      for( i = 1; it1 != result.end( ); ++it1, ++i )
      {
         if( ( i % 3 ) == 0 )
         {
            it2 = it1;
            ++it2;
            it3 = it2;
            if( it3 == result.end( ) )
               it3 = result.begin( );
            it1 = result.insert( it2, *it3 );
         }
      }
   }
   return result;
}
