#include "BinaryPointsLoader.h"

#include <osg/Geode>
#include <osg/Point>

using namespace omega;
using namespace cyclops;

///////////////////////////////////////////////////////////////////////////////
BinaryPointsLoader::BinaryPointsLoader(): ModelLoader("points-binary")
{
}

///////////////////////////////////////////////////////////////////////////////
BinaryPointsLoader::~BinaryPointsLoader()
{
}

///////////////////////////////////////////////////////////////////////////////
bool BinaryPointsLoader::supportsExtension(const String& ext) 
{ 
	if(ext == "xyzb") return true;
	return false; 
}

///////////////////////////////////////////////////////////////////////////////
bool BinaryPointsLoader::load(ModelAsset* model)
{
    osg::ref_ptr<osg::Group> group = new osg::Group();

	bool result = loadFile(model->info->path, model->info->options, group);

    // if successful get last child and add to sceneobject
    if(result)
    {
		osg::Geode* points;
	    points = group->getChild(0)->asGeode();
	    
		model->nodes.push_back(points);

	    group->removeChild(0, 1);
    }
    return result;
}

///////////////////////////////////////////////////////////////////////////////
bool BinaryPointsLoader::loadFile(const String& filename, const String& options, osg::Group * grp)
{
	if(!grp)
	{
		return false;
	}

  	osg::Vec3Array* verticesP = new osg::Vec3Array();
  	osg::Vec4Array* verticesC = new osg::Vec4Array();

	String path;
	if(DataManager::findFile(filename, path))
	{ 
		readXYZ(path, options, verticesP, verticesC);

  		// create geometry and geodes to hold the data
  		osg::Geode* geode = new osg::Geode();
  		geode->setCullingActive(false);
  		osg::Geometry* nodeGeom = new osg::Geometry();
  		osg::StateSet *state = nodeGeom->getOrCreateStateSet();
		nodeGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0, verticesP->size()));
  		osg::VertexBufferObject* vboP = nodeGeom->getOrCreateVertexBufferObject();
  		vboP->setUsage (GL_STREAM_DRAW);

  		nodeGeom->setUseDisplayList (false);
  		nodeGeom->setUseVertexBufferObjects(true);
  		nodeGeom->setVertexArray(verticesP);
  		nodeGeom->setColorArray(verticesC);
  		nodeGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

  		geode->addDrawable(nodeGeom);
  		geode->dirtyBound();
 
		grp->addChild(geode);
		return true;
	}
	return false; 
}

///////////////////////////////////////////////////////////////////////////////
void BinaryPointsLoader::readXYZ(
	const String& filename, const String& options, osg::Vec3Array* points, osg::Vec4Array* colors)
{
	osg::Vec3f point;
	osg::Vec4f color(1.0f, 1.0f, 1.0f, 1.0f);

	// Default record size = 6 doubles (X,Y,Z,R,G,B)
	size_t recordSize = sizeof(double) * 6;

	// Check the options to see if we should read a subsection of the file
	// and / or use decimation.
	int readStart = 0;
	int readLength = 0;

	libconfig::ArgumentHelper ah;


	// create a stream to read in file
	ifstream ifs( filename.c_str() );

	string value, values;
	stringstream ss;
	stringstream ssdouble;

	while( getline( ifs, values ) )
	{
		ss << values;

		int index = 0;
		while(ss >> value)
		{
     			ssdouble << value;

     			if( index < 3 )
			{
     				ssdouble >> point[index];
			}
     			else
			{
				ssdouble >> color[index - 3];
				//color[index - 3]/=255.0;
			}

     			ssdouble.clear();
     			index++;
		}

		points->push_back(point);
		colors->push_back(color);

		ss.clear();
	}
	ifs.close();
}
