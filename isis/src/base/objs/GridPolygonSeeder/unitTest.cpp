#include <iostream>
#include <cstdlib>
#include <cmath>
#include <iomanip>

#include "geos/geom/CoordinateArraySequence.h"
#include "geos/geom/Geometry.h"
#include "geos/geom/Polygon.h"

#include "iException.h"
#include "PolygonTools.h"
#include "PolygonSeeder.h"
#include "PolygonSeederFactory.h"
#include "Pvl.h"
#include "PvlGroup.h"
#include "ProjectionFactory.h"
#include "GridPolygonSeeder.h"
#include "Preference.h"

using namespace std;
using namespace Isis;

int main() {
  Isis::Preference::Preferences(true);
  try {
    cout << "Test 1, create a seeder" << endl;

    PvlGroup alg("PolygonSeederAlgorithm");

    do {
      if(!alg.HasKeyword("Name")) {
        cout << "Test without subgrid" << endl;
        alg += PvlKeyword("Name", "Grid");
        alg += PvlKeyword("MinimumThickness", 0.3);
        alg += PvlKeyword("MinimumArea", 10);
        alg += PvlKeyword("XSpacing", 1500);
        alg += PvlKeyword("YSpacing", 1500);
      }
      else {
        cout << "Test with subgrid" << endl;
        alg += PvlKeyword("SubGrid", true);
      }

      PvlObject o("AutoSeed");
      o.AddGroup(alg);

      Pvl pvl;
      pvl.AddObject(o);
      cout << pvl << endl << endl;

      PolygonSeeder *ps = PolygonSeederFactory::Create(pvl);

      std::cout << "Test to make sure Parse did it's job" << std::endl;
      std::cout << "MinimumThickness = " << ps->MinimumThickness() << std::endl;
      std::cout << "MinimumArea = " << ps->MinimumArea() << std::endl;
      std::cout << "SubGrid = " << ((GridPolygonSeeder *)ps)->SubGrid() << std::endl;
      //    std::cout << "Spaceing = " << ((GridPolygonSeeder*)ps)->Spacing() << std::endl;


      cout << "Test 2, test a square polygon" << endl;
      try {
        // Call the seed member with a polygon
        geos::geom::CoordinateSequence *pts;
        vector<geos::geom::Geometry *> polys;

        // Create the A polygon
        pts = new geos::geom::CoordinateArraySequence();
        pts->add(geos::geom::Coordinate(0, 0));
        pts->add(geos::geom::Coordinate(0, 0.5));
        pts->add(geos::geom::Coordinate(0.5, 0.5));
        pts->add(geos::geom::Coordinate(0.5, 0));
        pts->add(geos::geom::Coordinate(0, 0));

        polys.push_back(Isis::globalFactory.createPolygon(
                          Isis::globalFactory.createLinearRing(pts), NULL));

        geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon(polys);

        cout << "Lon/Lat polygon = " << mp->toString() << endl;
        // Create the projection necessary for seeding
        PvlGroup radii = Projection::TargetRadii("MARS");
        Isis::Pvl maplab;
        maplab.AddGroup(Isis::PvlGroup("Mapping"));
        Isis::PvlGroup &mapGroup = maplab.FindGroup("Mapping");
        mapGroup += Isis::PvlKeyword("EquatorialRadius", (string)radii["EquatorialRadius"]);
        mapGroup += Isis::PvlKeyword("PolarRadius", (string)radii["PolarRadius"]);
        mapGroup += Isis::PvlKeyword("LatitudeType", "Planetocentric");
        mapGroup += Isis::PvlKeyword("LongitudeDirection", "PositiveEast");
        mapGroup += Isis::PvlKeyword("LongitudeDomain", 360);
        mapGroup += Isis::PvlKeyword("CenterLatitude", 0);
        mapGroup += Isis::PvlKeyword("CenterLongitude", 0);
        mapGroup += Isis::PvlKeyword("ProjectionName", "Sinusoidal");

        Projection *proj = Isis::ProjectionFactory::Create(maplab);

        geos::geom::MultiPolygon *xymp = PolygonTools::LatLonToXY(*mp, proj);
        vector<geos::geom::Point *> seedValues = ps->Seed(xymp);

        vector<geos::geom::Point *> points;
        for(unsigned int pt = 0; pt < seedValues.size(); pt ++) {
          if(proj->SetCoordinate(seedValues[pt]->getX(), seedValues[pt]->getY())) {
            points.push_back(Isis::globalFactory.createPoint(
                               geos::geom::Coordinate(proj->UniversalLongitude(),
                                   proj->UniversalLatitude())));
          }
          else {
            iString msg = "Unable to convert to a (lon,lat)";
            throw iException::Message(iException::Programmer, msg, _FILEINFO_);
          }
        }

        cout << setprecision(13);
        for(unsigned int i = 0; i < points.size(); i++) {
          cout << "  POINT (";
          cout << points[i]->getX() << " " << points[i]->getY() << ")" << endl;
        }
      }
      catch(iException &e) {
        e.Report();
      }

      cout << "Test 3, test for too thin" << endl;
      try {
        // Call the seed member with a polygon
        geos::geom::CoordinateSequence *pts;
        vector<geos::geom::Geometry *> polys;

        // Create the A polygon
        pts = new geos::geom::CoordinateArraySequence();
        pts->add(geos::geom::Coordinate(0, 0));
        pts->add(geos::geom::Coordinate(0, 0.5));
        pts->add(geos::geom::Coordinate(0.0125, 0.5));
        pts->add(geos::geom::Coordinate(0.0125, 0));
        pts->add(geos::geom::Coordinate(0, 0));

        polys.push_back(Isis::globalFactory.createPolygon(
                          Isis::globalFactory.createLinearRing(pts), NULL));

        geos::geom::MultiPolygon *mp = Isis::globalFactory.createMultiPolygon(polys);

        cout << "Lon/Lat polygon = " << mp->toString() << endl;

        // Create the projection necessary for seeding
        PvlGroup radii = Projection::TargetRadii("MARS");
        Isis::Pvl maplab;
        maplab.AddGroup(Isis::PvlGroup("Mapping"));
        Isis::PvlGroup &mapGroup = maplab.FindGroup("Mapping");
        mapGroup += Isis::PvlKeyword("EquatorialRadius", (string)radii["EquatorialRadius"]);
        mapGroup += Isis::PvlKeyword("PolarRadius", (string)radii["PolarRadius"]);
        mapGroup += Isis::PvlKeyword("LatitudeType", "Planetocentric");
        mapGroup += Isis::PvlKeyword("LongitudeDirection", "PositiveEast");
        mapGroup += Isis::PvlKeyword("LongitudeDomain", 360);
        mapGroup += Isis::PvlKeyword("CenterLatitude", 0);
        mapGroup += Isis::PvlKeyword("CenterLongitude", 0);
        mapGroup += Isis::PvlKeyword("ProjectionName", "Sinusoidal");
        Projection *proj = Isis::ProjectionFactory::Create(maplab);

        // NOTHING SHOULD BE PRINTED (the thickness test should not have been met)
        geos::geom::MultiPolygon *xymp = PolygonTools::LatLonToXY(*mp, proj);
        vector<geos::geom::Point *> seedValues = ps->Seed(xymp);
        for(unsigned int i = 0; i < seedValues.size(); i++) {
          cout << "Point(" << i << ") = " << seedValues[i]->toString() << endl;
        }
      }
      catch(iException &e) {
        e.Report();
      }
    }
    while(!alg.HasKeyword("SubGrid"));
  }
  catch(iException &e) {
    e.Report();
  }

  return 0;
}
